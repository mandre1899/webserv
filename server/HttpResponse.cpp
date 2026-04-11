#include "HttpResponse.hpp"

#include <filesystem>
// Temporary

static std::string init_root_path()
{
	std::filesystem::path tmpPathWebserv = std::string(g_execName);

	std::string abs = std::filesystem::absolute(tmpPathWebserv).string();

	std::size_t pos = abs.find('.');
	if (pos != std::string::npos)
		abs.erase(pos);
	abs.erase(abs.find_last_of("/"));
	
	return abs;
}

//Absolute Path to the binary
std::string root_path = init_root_path();

const std::map<std::string, std::string>  HttpResponse::_mime_types = 
{
	{".html", "text/html"},
	{".css", "text/css"},
	{".js", "application/javascript"},
	{".json", "application/json"},
	{".xml", "application/xml"},
	{".png", "image/png"},
	{".jpg", "image/jpeg"},
	{".gif", "image/gif"},
	{".svg", "image/svg+xml"},
	{".ico", "image/vnd.microsoft.icon"},
	{".woff", "font/woff"},
	{".woff2", "font/woff2"},
	{".ttf", "font/ttf"},
	{".otf", "font/otf"},
	{".mp4", "video/mp4"},
	{".mp3", "audio/mpeg"},
	{".wav", "audio/wav"},
	{".pdf", "application/pdf"},
	{".zip", "application/zip"},
	{".tar", "application/x-tar"},
	{".gz", "application/gzip"},
	{".py", "text/x-python"},
	{".php", "text/html"},
	{"notype", "application/octet-stream"}
};

const std::map<int, std::string> HttpResponse::_http_status_map= {
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},

	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},

	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},

	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{415, "Unsupported Media Type"},
	{418, "I'm a teapot"},
	{429, "Too Many Requests"},

	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"}
};
// Static data storages END

std::string     HttpResponse::abstract_filename(const std::string& path)
{
	std::string tmp = path.substr(path.find_last_of("/") + 1);
	return tmp;
}

HttpResponse	&HttpResponse::set_content_type(const std::string& type)
{
	_headers["content-type"] = type;
	return *this;
}

HttpResponse&	HttpResponse::add_header(const std::string& key, const std::string& val)
{
	_headers[key] = val;
	return *this;
}

void	HttpResponse::set_default_headers(void)
{
    if (_headers.find("server") == _headers.end())
        _headers["server"] = "Webserv/1.0";
    if (_headers.find("date") == _headers.end()) {
        time_t now = time(0);
        struct tm tm_struct;
        gmtime_r(&now, &tm_struct);
        char buf[100];
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_struct);
        _headers["date"] = std::string(buf);
    }
    
    // Add connection header based on HTTP version
    if (_headers.find("connection") == _headers.end()) {
        _headers["connection"] = "close";
    }
	_headers["content-length"] = std::to_string(_body_size);
}

bool HttpResponse::read_file_to_body(const std::string& abs_path)
{
	// Add file size check
	struct stat file_stat;
	if (stat(abs_path.c_str(), &file_stat) != 0) {
		set_status_code(404);
		return false;
	}
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		file.open(abs_path, std::ios::binary);
		_body.reserve(file_stat.st_size); //reserve for better performance
		_body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		_body_size = _body.size();
		file.close();
		return true;

	} catch (const std::ifstream::failure& e) {
		set_status_code(500);
		// Add logging
		return false;
	}
}

bool	HttpResponse::check_path_secure(const std::string& abs_path)
{
	std::filesystem::path expandedPath = abs_path;
	expandedPath = expandedPath.lexically_normal();
	if (expandedPath.string().compare(0, root_path.length(), root_path) == 0 &&
	    (expandedPath.string().length() == root_path.length() ||
     	expandedPath.string()[root_path.length()] == '/'))
		return true; // Path is safe
	return false;
}

void HttpResponse::set_status_code(int code) //default wrapper
{
    if (_http_status_map.find(code) == _http_status_map.end()) {
        _status_code = 500;
        return;
    }
    _status_code = code;
}

static std::vector<char*> parse_header_to_env(std::map<std::string, std::string> headers)
{
	std::vector<char*> env;
	std::vector<std::unique_ptr<char[]>> env_ptr;
	for (const auto &a : headers)
	{
		std::string concat = a.first + "=" + a.second;
		auto ptr = std::make_unique<char[]>(concat.length() + 1);
		std::strcpy(ptr.get(), concat.c_str());
		env.push_back(ptr.get());
		env_ptr.push_back(std::move(ptr));
	}
	env.push_back(nullptr);
	return env;
}

state HttpResponse::handle_cgi_proc()
{
	char buf[4096];
	while (true)
	{
		ssize_t read_bytes = read(_pipe_read, buf, sizeof(buf));
		if (read_bytes > 0)
		{
			Logger::log<LogLevel::info>("Read ", read_bytes, " bytes from CGI pipe");
			_body.append(buf, static_cast<std::size_t>(read_bytes));
			_body_size += static_cast<std::size_t>(read_bytes);
			continue;
		}
		if (read_bytes == 0)
		{
			epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_read, NULL);
			close(_pipe_read);
			_pipe_read = -1;
			int status = 0;
			pid_t ret = waitpid(_cgi_pid, &status, WNOHANG);
			if (ret == -1)
			{
				Logger::log<LogLevel::error>("waitpid failed for CGI child");
				_cgi_pid = -1;
				set_status_code(500);
				generate_error_page();
				set_default_headers();
				return state::response_sending;
			}
			if (ret > 0)
			{
				_cgi_pid = -1;

				if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				{
					set_default_headers();
					return state::response_sending;
				}
				Logger::log<LogLevel::error>("CGI exited with status: ", WEXITSTATUS(status));
				set_status_code(500);
				generate_error_page();
				set_default_headers();
				return state::response_sending;
			}
			set_default_headers();
			return state::response_sending;
		}
		Logger::log<LogLevel::error>("Read failed status: ", errno);
		epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_read, NULL);
		close(_pipe_read);
		_pipe_read = -1;
		_cgi_pid = -1;
		set_status_code(500);
		generate_error_page();
		set_default_headers();
		return state::response_sending;
	}
}

bool HttpResponse::execute_cgi_proc(const std::string& abs_path, const std::string& filetype)
{ static const std::string pyExec = "/usr/bin/python3";
	static const std::string phpExec = "/usr/bin/php";
	char* execPath = NULL;
	char* scriptPath = const_cast<char*>(abs_path.c_str());
	
	int p[2];

	set_status_code(200);
	if (filetype == ".py")
		execPath = const_cast<char*>(pyExec.c_str());
	if (filetype == ".php")
		execPath = const_cast<char*>(phpExec.c_str());
	if (pipe(p) == -1)
	{
		Logger::log<LogLevel::error>("Pipe creation failed");
		close(p[0]);
		close(p[1]);
		return false;
	}
	int pid = fork();
	if (pid < 0)
	{
		Logger::log<LogLevel::error>("fork() failed");
		close(p[0]);
		close(p[1]);
		return false;
	}
	if (pid == 0)
	{
		char *const argv[] = {execPath, scriptPath, NULL};
		std::vector<char*> tmp = parse_header_to_env(_request->getHeadersMap());
		char** env = tmp.data();
		close(p[0]);
		if (dup2(p[1], STDOUT_FILENO) == -1)
			_exit(1);
		if (dup2(p[1], STDERR_FILENO) == -1)
			_exit(1);
		close(p[1]);
		execve(execPath, argv, env);
		Logger::log<LogLevel::error>("Execve failed");
		_exit(1);
	}
	close(p[1]);
	epoll_event	ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = _socket_fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, p[0], &ev) == -1)
	{
		Logger::log<LogLevel::error>("Epoll assignment of pipe read failed");
		close(p[0]);
		return false;
	}
	_cgi_pid = pid;
	_pipe_read = p[0];
	return true;
}

void	HttpResponse::evaluate_vHost()
{
	const auto& headers = _request->getHeadersMap();
	if (headers.empty())
	{
		Logger::log<LogLevel::warn>("Request headers empty");
		return ;
	}
	std::string hostValue = headers.find("Host")->second;
	//Erase port specifier ":8080"
	size_t portPrefix = hostValue.find(":");
	if (portPrefix != std::string::npos)
		hostValue.erase(portPrefix);
	auto server = _configs->srvNameMap.find(hostValue);
	if (server != _configs->srvNameMap.end())
		_config = _configs->vHost[server->second];
	else
		_config = _configs->vHost[0];
	Logger::log<LogLevel::info>("Client connected to virtual server: ", _config.serverName[0]);
}


//Private Member function - HELPER END

HttpResponse::HttpResponse(void) : _cgi_pid(-1), _pipe_read(-1)
{}


HttpResponse::~HttpResponse(void)
{
	//RAII
	if (_cgi_pid != -1)
		kill(_cgi_pid, SIGKILL);
	if (_pipe_read != -1)
		close (_pipe_read);
}

// Con - & De- structor END
static std::pair<int, std::string>	rtn_pair(int code, std::string path)
{
	std::pair<int, std::string>	rtn;
	rtn.first = code;
	rtn.second = path;
	return rtn;
}

//Improved is also now matching /index.html on / location -> specifix ressource requesting
static bool loc_uri_comp(const std::string& loc, const std::string& uri)
{
	if (loc.empty())
		return false;
	if (loc == "/")
		return !uri.empty() && uri[0] == '/';
	if (uri == loc)
		return true;
	if (uri.size() > loc.size() &&
		uri.compare(0, loc.size(), loc) == 0 &&
		uri[loc.size()] == '/')
		return true;
	return false;
}

static std::string join_path(const std::string& left, const std::string& right) //Wrapper safer path concat
{
	if (left.empty())
		return right;
	if (right.empty())
		return left;

	if (left[left.size() - 1] == '/' && right[0] == '/')
		return left + right.substr(1);
	if (left[left.size() - 1] != '/' && right[0] != '/')
		return left + "/" + right;
	return left + right;
}

const std::string	HttpResponse::get_response_str() const
{
	std::ostringstream	tmp;

	tmp << "HTTP/1.1 " << _status_code << " " << _http_status_map.find(_status_code)->second << "\r\n";
	for (const auto& a : _headers)
		tmp << a.first << ": " << a.second << "\r\n";
	tmp << "\r\n";
	tmp << _body;
	return tmp.str();
}

std::pair<int, std::string> HttpResponse::resolve_path(void)
{
	const std::string uri_path = _request->getRequestUri();

	if (_config.location.empty())
		return rtn_pair(404, std::string());
	
	for (auto it = _config.location.begin();
			it != _config.location.end(); ++it)
	{
		const std::string& location_path = it->first;
		const cfg::wsLocationCtx& location_cfg = it->second;

		if (!loc_uri_comp(location_path, uri_path))
			continue;

		if (location_cfg.ret.has_value())
			return rtn_pair(301, location_cfg.ret->param);

		// Check method for location
		method_type method = _request->getMethod();
		const cfg::wsAllowMethods& allowed = location_cfg.allowMethods;

		if ((method == method_type::GET && !allowed.isGet) ||
				(method == method_type::POST && !allowed.isPost) ||
				(method == method_type::DELETE && !allowed.isDelete))
			return rtn_pair(405, std::string());

		// For now using the hardcoded base path (must fix!)
		if (!location_cfg.index.empty() && _request->getRequestUri() == location_path)
			return rtn_pair(200, join_path(root_path + "/" + location_cfg.root, location_cfg.index));
		std::string tmp = join_path(root_path + "/" + location_cfg.root, uri_path);
		if (access(tmp.c_str(), R_OK) == 0)
			return rtn_pair(200, tmp);
		if (!location_cfg.index.empty())
			return rtn_pair(200, join_path(root_path + "/" + location_cfg.root, location_cfg.index));
		return rtn_pair(404, std::string());
	}
	if (_config.errorPage)
		return rtn_pair(404, _config.errorPage->path);
	return rtn_pair(404, std::string());
}

std::string	HttpResponse::resolve_content_type(const std::string& filetype)
{
	for (auto a : _mime_types)
	{
		if (a.first == filetype)
			return a.second;
	}
	return _mime_types.find("notype")->second;
}

filetype  HttpResponse::resolve_filetype(const std::string& file_suffix)
{
	if (file_suffix == ".php" || file_suffix == ".py")
		return DYNAMIC_FILE;
	return STATIC_FILE;
}

static const std::string resolve_file_suffix(const std::string& abs_path)
{
	std::string res;
	size_t	dotPos = abs_path.find(".");
	size_t	questMark = abs_path.find("?");

	if (questMark != std::string::npos)
		return abs_path.substr(dotPos, questMark);
	return abs_path.substr(dotPos);
}

void HttpResponse::generate_error_page(void)
{
	std::string status_text = "Unknown Error";

	std::map<int, std::string>::const_iterator it = _http_status_map.find(_status_code);
	if (it != _http_status_map.end())
		status_text = it->second;
	else
		status_text = "Error not found in the map";
	std::ostringstream body;

	set_content_type("text/html");
	body << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "  <meta charset=\"UTF-8\">\n"
		<< "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		<< "  <title>" << _status_code << " " << status_text << "</title>\n"
		<< "  <style>\n"
		<< "    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
		<< "    h1 { font-size: 50px; }\n"
		<< "    p { font-size: 20px; color: #555; }\n"
		<< "  </style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "  <h1>" << _status_code << "</h1>\n"
		<< "  <p>" << status_text << "</p>\n"
		<< "  <hr>\n"
		<< "  <p>Websmurf</p>\n"
		<< "</body>\n"
		<< "</html>\n";

	_body = body.str();
	_body_size = _body.size();
	Logger::log<LogLevel::info>("Server responding with error code: ", _status_code);
}

//directory listing

bool	HttpResponse::is_directory(const std::string& path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != 0)
        return false;
    return S_ISDIR(file_stat.st_mode);
}

void HttpResponse::generate_dir_listing(const std::string& dir_path, const std::string& uri_path)
{
	std::ostringstream ss;

	std::string base_uri = uri_path;

	ss << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "  <meta charset=\"UTF-8\">\n"
		<< "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		<< "  <title>" << base_uri << " - Dir listing</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "  <ul>\n";

	DIR* dir = opendir(dir_path.c_str());
	if (dir != NULL)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			//Skips listing of current dir -> .
			if (name == ".")
				continue;
			std::string full_fs_path = dir_path;
			if (!full_fs_path.empty() && full_fs_path[full_fs_path.size() - 1] != '/')
				full_fs_path += '/';
			full_fs_path += name;
			struct stat st;
			bool is_dir = (stat(full_fs_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
			std::string href = base_uri + name;
			if (is_dir && href[href.size() - 1] != '/')
				href += '/';
			ss << "    <li><a href=\"" + href + "\">"
				<< name;
			if (is_dir)
				ss << "/";
			ss << "</a></li>\n";
		}
		closedir(dir);
	}
	else
		ss << "    <li>Could not open directory</li>\n";

	ss << "  </ul>\n"
		<< "</body>\n"
		<< "</html>\n";

	_content_type = "text/html";
	_body = ss.str();
	_body_size = _body.size();
}

//directory listing end

bool	HttpResponse::delete_request(const std::string& abs_path)
{
	struct stat path_stat;
	if (stat(abs_path.c_str(), &path_stat) != 0)
		return false;
	if (S_ISDIR(path_stat.st_mode))
	{
		//No recursive deletion
		if (rmdir(abs_path.c_str()) != 0)
		{
			if (errno == 39)
				Logger::log<LogLevel::error>("Dir removal failed with code: ", errno, " (no recursive folder deletion allowed!)");
			else
				Logger::log<LogLevel::error>("Dir removal failed with code: ", errno, " (check for permissions of program executing user!)");
			return false;
		}
		Logger::log<LogLevel::info>("Deleted dir: ", abs_path);
		_body = "<html lang=\"en-US\">\n<body>\n<h1>Directory \"" + abstract_filename(abs_path) + "/" + "\" deleted.</h1>\n</body>\n</html>";
		return true;
	}
	else if (unlink(abs_path.c_str()) == 0)
	{
		Logger::log<LogLevel::info>("Deleted file: ", abs_path);
		_body = "<html lang=\"en-US\">\n<body>\n<h1>File \"" + abstract_filename(abs_path) + "\" deleted.</h1>\n</body>\n</html>";
		return true;
	}
	else
		Logger::log<LogLevel::error>("File removal failed with code: ", errno, " (check for permissions of program executing user!)");
	return false;
}

bool	HttpResponse::get_request(const std::string& abs_path)
{
	if(is_directory(abs_path))
		generate_dir_listing(abs_path, _request->getRequestUri());
	else
	{
		const std::string file_suffix = resolve_file_suffix(abs_path);
		set_content_type(resolve_content_type(file_suffix));

		filetype type = resolve_filetype(file_suffix);

		bool success;
		if (type == STATIC_FILE)
			success = read_file_to_body(abs_path);
		else if (type == DYNAMIC_FILE)
			success = execute_cgi_proc(abs_path, file_suffix);
		else
		{
			// Momentarily dead code -> resolve_filetype always returns one of two above types
			set_status_code(415); // Unsupported media typpe
			success = false;
		}

		if (!success)
		{
			generate_error_page();
			return false;
		}
		set_default_headers();
		_is_built = true;
		if (type == DYNAMIC_FILE)
			return false; //To set state machine flag to proccessing
	}
	return true;
}

// Question: do I handle the file storing or is this part of the parser and I just return matching response
//bool	HttpResponse::post_request(const std::string& abs_path)
//{
//	
//}

state	HttpResponse::build(HttpParser *req, const cfg::srvCfg* conf, int clientFd, int epfd)
{
	_request = req;
	_configs = conf;
	_socket_fd = clientFd;
	_body_size = 0;
	_epfd = epfd;
	evaluate_vHost();
	try
	{
		std::pair<int, std::string> res = resolve_path();
		
		Logger::log<LogLevel::info>("Absolute path of requested file: ", res.second);

		set_status_code(res.first);
		//Simple redirect
		if (_status_code == 301 || _status_code == 302 ||
				_status_code == 303)
		{
			_headers["location"]= res.second;
			return state::response_sending;
		}
		if (_status_code != 200)
		{
			generate_error_page();
			return state::response_sending;
		}

		const std::string& abs_path = res.second;
		
		if (!check_path_secure(abs_path))
		{
			set_status_code(403);
			generate_error_page();
			return state::response_sending;
		}

		if (access(abs_path.c_str(), R_OK) != 0)
		{
			//Not sure if errno is allowed to use by subject
			set_status_code((errno == EACCES) ? 403 : 404);
			generate_error_page();
			return state::response_sending;
		}

		if (method_type::DELETE == _request->getMethod())
		{
			if (!delete_request(abs_path))
			{
				set_status_code(403);
				generate_error_page();
				return state::response_sending;
			}

		}
		//else if (method_type::POST == _request.getMethod()) // Return a success message(depending on state)
		//	post_request(abs_path);
		else if (method_type::GET == _request->getMethod())
		{
			if (!get_request(abs_path))
				return state::response_proccessing; //CGI is running
		}
	}
	catch (const std::exception& e)
	{
		set_status_code(500);
		generate_error_page();
	}
	return state::response_sending;
}
