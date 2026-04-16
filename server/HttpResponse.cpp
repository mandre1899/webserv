#include "HttpResponse.hpp"

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

const std::map<int, std::string> HttpResponse::_http_status_map = {
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

HttpResponse& HttpResponse::set_content_type(const std::string& type)
{
	_headers["content-type"] = type;
	return *this;
}

HttpResponse& HttpResponse::add_header(const std::string& key, const std::string& val)
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
	if (_headers.find("connection") == _headers.end())
		_headers["connection"] = "close";
	_headers["content-length"] = std::to_string(_body_size);
}

bool HttpResponse::read_file_to_body(const std::string& abs_path)
{
	// Add file size check
	struct stat file_stat;
	if (stat(abs_path.c_str(), &file_stat) != 0)
	{
		generate_error_page(404);
		return false;
	}
	std::ifstream file;
	try
	{
		file.open(abs_path, std::ios::binary);
		_body.reserve(file_stat.st_size); //reserve for better performance
		_body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		_body_size = _body.size();
		file.close();
		return true;

	}
	catch (...)
	{
		set_status_code(500);
		// Add logging
		return false;
	}
}

bool	HttpResponse::check_path_secure(const std::string& abs_path)
{
	//When config specifies a root location we can't check
	return true;
	std::filesystem::path expandedPath = abs_path;

	expandedPath = expandedPath.lexically_normal();
	if (expandedPath.string().compare(root_path.length(), _curr_loc_root.length(), _curr_loc_root) == 0 &&
		((expandedPath.string().length() - root_path.length()) == root_path.length() ||
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

bool	HttpResponse::parse_cgi_headers(void)
{
	size_t header_end = std::string::npos;
	size_t delimiter_len = 0;

	//Evaluate differnt header endings
	header_end = _body.find("\r\n\r\n");
	if (header_end != std::string::npos)
		delimiter_len = 4;
	else
	{
		header_end = _body.find("\n\n");
		if (header_end != std::string::npos)
			delimiter_len = 2;
		else
			return false;
	}
	std::cout << "Header end: " << header_end << std::endl;
	std::string header_block = _body.substr(0, header_end);
	std::string parsed_body = _body.substr(header_end + delimiter_len);
	std::istringstream header_stream(header_block);
	std::string line;

	while (std::getline(header_stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t sep = line.find(':');
		if (sep == std::string::npos)
			return false;

		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);

		_headers[key] = value;
	}

	_body = parsed_body;
	_body_size = _body.size();
	return true;
}

state HttpResponse::handle_cgi_proc()
{
	cgi_state CGI_state = _cgi->handle_cgi_proc();
	if (CGI_state == cgi_state::FINISHED_KO)
	{
		generate_error_page(500);
		set_default_headers();
		return state::response_sending;
	}
	if (CGI_state == cgi_state::FINISHED_OK)
	{
		_body = _cgi->get_body();
		_body_size = _body.size();
	 	_headers = _cgi->get_header_map();
		set_default_headers();
		return state::response_sending;
	}
	return state::response_proccessing;
}

bool HttpResponse::execute_cgi_proc(const std::string& abs_path)
{
	try
	{
		_cgi = std::make_unique<CGI>(*_request, _curr_loc.cgiPath, abs_path , _epfd, _socket_fd);
	} catch (std::exception &e)
	{
		Logger::log<LogLevel::error>("CGI object instantiation failed: ", e.what());
		return false;
	}
	if (_cgi->execute_cgi_proc() != cgi_state::FAILED) // Pass client _msg -> (send buffer)
		return true;
	return false;
}

void	HttpResponse::evaluate_vHost()
{
	const auto& headers = _request->getHeadersMap();
	if (headers.empty())
	{
		Logger::log<LogLevel::warn>("Request headers empty");
		return;
	}
	std::string hostValue = headers.find("Host")->second;
	if (hostValue.empty())
	{
		_config = _configs->vHost[0];
		return;
	}
	//Erase port specifier ":8080"
	size_t portPrefix = hostValue.find(":");
	if (portPrefix != std::string::npos)
		hostValue.erase(portPrefix);
	auto server = _configs->srvNameMap.find(hostValue);
	if (server != _configs->srvNameMap.end())
		_config = _configs->vHost[server->second];
	else
		_config = _configs->vHost[0];
}


//Private Member function - HELPER END

HttpResponse::HttpResponse(void)
{}


HttpResponse::~HttpResponse(void)
{}

// Con - & De- structor END
static struct ResolveResult	rtn_result(enum ResolveKind kind, int code, std::string value)
{
	struct ResolveResult res;

	res.code = code;
	res.kind = kind;
	res.value = value;
	return res;
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

//Removes leading / (not when its the first part of the path) and trailing
static std::string normalize_part(const std::string& part, bool is_first)
{
	if (part.empty())
		return "";
	size_t start = 0;
	size_t end = part.size();
	if (!is_first)
	{
		while (start < end && part[start] == '/')
			++start;
	}
	while (end > start && part[end - 1] == '/')
		--end;
	return part.substr(start, end - start);
}

//Concats n amounts of path args
template <typename... Args>
static std::string join_paths(const Args&... parts)
{
	std::vector<std::string> vec = { parts... };
	std::string result;
	bool first = true;
	for (const auto& part : vec)
	{
		std::string cleaned = normalize_part(part, first);
		if (cleaned.empty())
			continue;
		if (!result.empty())
			result += "/";
		result.append(cleaned);
		first = false;
	}
	return result;
}

//Returns pointer to be NULL assignable(eg. no location found)
//This function returns the most specific location requested by the client
std::optional<std::pair<std::string, cfg::wsLocationCtx>>	HttpResponse::get_most_specific_location(const std::string& uri_path)
{
	auto best_match = _config.location.end();
	size_t best_len = 0;

	for (auto it = _config.location.begin(); it != _config.location.end(); ++it)
	{
		const std::string& location_path = it->first;

		if (!loc_uri_comp(location_path, uri_path))
			continue;

		if (location_path.length() > best_len)
		{
			best_match = it;
			best_len = location_path.length();
		}
	}
	if (best_match == _config.location.end())
		return std::nullopt;
	return std::make_pair(best_match->first, best_match->second);
}

struct ResolveResult	HttpResponse::resolve_path(void)
{
	std::string uri_path = _request->getRequestUri();
	bool		is_abs_root = true; // In config its ./ or /

	//When no lacation is specified
	if (_config.location.empty())
	{
		std::string tmp;
		if (_config.root.empty())
			return rtn_result(ResolveKind::ERRORSTATUS, 404, std::string());
		if (uri_path.empty() || uri_path == "/")
		{
			if (_config.index.empty())
				return rtn_result(ResolveKind::ERRORSTATUS, 404, std::string());
			if (_config.root[0] == '.')
				tmp = join_paths(root_path, _config.root.substr(1), _config.index);
			else
				tmp = join_paths(_config.root, _config.index);
		}
		else
		{
			if (_config.root[0] == '.')
				tmp = join_paths(root_path, _config.root.substr(1), uri_path);
			else
				tmp = join_paths(_config.root, uri_path);
		}

		if (!tmp.empty() && access(tmp.c_str(), R_OK) == 0)
			return rtn_result(ResolveKind::FILEPATH, 200, tmp);
		return rtn_result(ResolveKind::FILEPATH, 404, std::string());
	}

	auto location = get_most_specific_location(uri_path);
	if (!location)
		return rtn_result(ResolveKind::ERRORSTATUS, 404, std::string());
	_curr_loc = location->second;

	//Just so relative and absolute paths can be defined in config
	if (_curr_loc.root[0] == '.')
	{
		_curr_loc_root = _curr_loc.root.substr(1);
		is_abs_root = false;
	}
	else
		_curr_loc_root = _curr_loc.root;

	//When Location defines a return
	if (_curr_loc.ret.has_value())
		return rtn_result(ResolveKind::REDIRECT, _curr_loc.ret->code, _curr_loc.ret->param);

	method_type method = _request->getMethod();
	const cfg::wsAllowMethods& allowed = _curr_loc.allowMethods;

	if ((method == method_type::GET && !allowed.isGet) ||
		(method == method_type::POST && !allowed.isPost) ||
		(method == method_type::DELETE && !allowed.isDelete))
		return rtn_result(ResolveKind::ERRORSTATUS, 405, std::string());

	if (method == method_type::POST)
		return rtn_result(ResolveKind::POST, 200, _request->getRequestUri());

	//When exact configured location is requested
	if (!_curr_loc.index.empty() && _request->getRequestUri() == location->first)
	{
		if (is_abs_root == false)
			return rtn_result(ResolveKind::FILEPATH, 200, join_paths(root_path, _curr_loc_root, _curr_loc.index));
		else
			return rtn_result(ResolveKind::FILEPATH, 200, join_paths(_curr_loc_root, _curr_loc.index));
	}
	//When exact file is requested on location
	std::string tmp;
	if (is_abs_root == false)
		tmp = join_paths(root_path, _curr_loc_root, uri_path.substr(location->first.length()));
	else
		tmp = join_paths(_curr_loc_root, uri_path.substr(location->first.length()));
	if (access(tmp.c_str(), R_OK) == 0)
		return rtn_result(ResolveKind::FILEPATH, 200, tmp);
	//Location specific error page
	std::unordered_map<http_code, std::string>::const_iterator it = _curr_loc.errorPage.find(404);
	if (it != _curr_loc.errorPage.end() && !it->second.empty())
	{
		std::string error_page_path = join_paths(root_path, _curr_loc_root, it->second);
		if (access(error_page_path.c_str(), R_OK) == 0)
			return rtn_result(ResolveKind::ERRORSTATUS, 404, error_page_path);
	}
	return rtn_result(ResolveKind::ERRORSTATUS, 404, std::string());
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

state	HttpResponse::upload_body(std::string& data)
{
	if (!_upload->uploadBody(data))
		return state::response_body;
	set_status_code(200);
	return state::response_sending;
}

void HttpResponse::generate_error_page(int status_code)
{
	std::string status_text = "Unknown Error";


	set_status_code(status_code);
	std::unordered_map<http_code, std::string>::const_iterator it_err_page = _curr_loc.errorPage.find(status_code);
	if (it_err_page != _curr_loc.errorPage.end() && !it_err_page->second.empty())
	{
		std::string error_page_path = join_paths(root_path, _curr_loc_root, it_err_page->second);
		if (access(error_page_path.c_str(), R_OK) == 0)
		{
			read_file_to_body(error_page_path);
			return;
		}
	}
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
	set_default_headers();
	Logger::log<LogLevel::info>("Server responding with error code: ", _status_code);
}

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

	if (_curr_loc.autoIndex == false)
	{
		generate_error_page(403);
		return;
	}

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
			std::string href = join_paths(base_uri, name);
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
	if (is_directory(abs_path))
		generate_dir_listing(abs_path, _request->getRequestUri());
	else
	{
		const std::string file_suffix = resolve_file_suffix(abs_path);
		set_content_type(resolve_content_type(file_suffix));

		filetype type = resolve_filetype(file_suffix);

		bool success;
		if (type == STATIC_FILE)
			success = read_file_to_body(abs_path);
		else if (type == DYNAMIC_FILE && !_curr_loc.cgiPath.empty())
			success = execute_cgi_proc(abs_path);
		else
		{
			// Momentarily dead code -> resolve_filetype always returns one of two above types
			generate_error_page(415);
			return true;
		}

		if (!success)
		{
			generate_error_page(500);
			return true;
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

state	HttpResponse::build(HttpParser* req, const cfg::srvCfg* conf, int clientFd, int epfd)
{
	_request = req;
	_configs = conf;
	_socket_fd = clientFd;
	_body_size = 0;
	_epfd = epfd;
	evaluate_vHost();
	try
	{
		struct ResolveResult res = resolve_path();

		set_status_code(res.code); //Safety
		if (res.kind == ResolveKind::REDIRECT)
		{
			_headers["location"] = res.value;
			return state::response_sending;
		}
		if (res.kind == ResolveKind::ERRORSTATUS)
		{
			//If location specifies custom error page its handled within the function
			generate_error_page(res.code);
			return state::response_sending;
		}
		if (res.kind == ResolveKind::POST)
		{
			try
			{

				_upload = std::make_unique<HttpUpload>(_request->getHeadersMap(), _curr_loc.uploadStore);
				return (state::response_body); //
			}
			catch (const std::exception& e)
			{
				// todo: Logger 
				return (state::error);
			}
		}

		const std::string& abs_path = res.value;

		if (!check_path_secure(abs_path))
		{
			generate_error_page(403);
			return state::response_sending;
		}

		if (access(abs_path.c_str(), R_OK) != 0)
		{
			//Not sure if errno is allowed to use by subject
			generate_error_page((errno == EACCES) ? 403 : 404);
			return state::response_sending;
		}

		if (method_type::DELETE == _request->getMethod())
		{
			if (!delete_request(abs_path))
			{
				generate_error_page(403);
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
		generate_error_page(500);
	}
	return state::response_sending;
}

const std::string	HttpResponse::get_response_str() const
{
	std::ostringstream	tmp;

	tmp << "HTTP/1.1 " << _status_code << " " << _http_status_map.find(_status_code)->second << "\r\n";
	for (const auto& a : _headers)
		tmp << a.first << ": " << a.second << "\r\n";
	tmp << "\r\n";
	if (!_body.empty())
		tmp << _body;
	return tmp.str();
}
