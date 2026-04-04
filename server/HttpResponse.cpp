#include "HttpResponse.hpp"

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

// Removes suffixed and prefixed slash (Helper function)
// Creates a copy of the string passed as reference.
std::string     HttpResponse::clean_string(const std::string& path)
{
  std::string tmp = path;

  if (tmp[0] == '/')
    tmp.erase(tmp.begin());
  if (tmp[tmp.length() - 1] == '/')
    tmp.erase(tmp.length() - 1);
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

bool	HttpResponse::read_file_to_body(const std::string& abs_path)
{
	std::ifstream	file;
	file.open(abs_path);
	if (!file.is_open())
		return false;
	_body_size = 0;
	std::string	buf;
	while ((std::getline(file, buf)))
	{
		buf.append("\n");
		_body.append(buf);
		_body_size += buf.size();
	}
	_headers["content-length"] = std::to_string(_body_size);//temporary will create wrapper
	file.close();
	return true;
}

static char**	parse_header_to_env(std::map<std::string, std::string> headers)
{
	static std::vector<char*> env;
	static std::vector<std::unique_ptr<char[]>> env_ptr;
	for (const auto &a : headers)
	{
		std::string concat = a.first + "=" + a.second;
    	auto ptr = std::make_unique<char[]>(concat.length() + 1);
    	std::strcpy(ptr.get(), concat.c_str());
    	env.push_back(ptr.get());
    	env_ptr.push_back(std::move(ptr));
	}
	env.push_back(nullptr);
	return &env[0];
}

bool	HttpResponse::execute_cgi_proc(const std::string& abs_path, const std::string& filetype)
{
	static const std::string	pyExec = {"/usr/bin/python3"};
	char *execPath = nullptr;
	char *scriptPath = const_cast<char*>(abs_path.c_str()); // Use const_cast to convert const char* to char*
	char **env;
	
	_status_code = 200;
	if (filetype == ".py")
		execPath = const_cast<char *>(pyExec.c_str());
	char *const argv[] = {execPath, scriptPath, NULL};
	env = parse_header_to_env(_request.getHeadersMap());
	int pid = fork();
	if (pid == 0)
	{
		dup2(_socket_fd, STDOUT_FILENO);
		execve(execPath, argv, env);
	}
	_cgi_pid = pid;
	return true;
}


//Private Member function - HELPER END

HttpResponse::HttpResponse(HttpParser &req, const cfg::srvCtxCfg& conf, int clientFd) : _request(static_cast<const HttpParser&>(req)), _config(conf), _socket_fd(clientFd)
{}

HttpResponse::HttpResponse(const HttpParser &req, const cfg::srvCtxCfg& conf, int clientFd) : _request(req), _config(conf), _socket_fd(clientFd)
{}

HttpResponse::~HttpResponse()
{}

// Con - & De- structor END
static std::pair<int, std::string>	rtn_pair(int code, std::string path)
{
	std::pair<int, std::string>	rtn;
	rtn.first = code;
	rtn.second = path;
	return rtn;
}

static bool loc_uri_comp(const std::string& loc, const std::string& uri)
{
	if (loc.empty())
		return false;

	if (uri == loc)
		return true;
	// Prefix match on path boundary:
	// "/images" matches "/images/logo.png"
	// but not "/images2/test"
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
	//Includes adding content-length
	for (const auto& a : _headers)
	{
		tmp << a.first << ": " << a.second << "\r\n";
	}
	tmp << "\r\n";
	tmp << _body;
	return tmp.str();
}

std::pair<int, std::string> HttpResponse::resolve_path(void)
{
	const std::string root_path =
		"/Volumes/MateoSSD/42Projects/network_programming/webserv_mausperg";

	const std::string uri_path = _request.getRequestUri();

	if (_config.location.empty())
		return rtn_pair(404, std::string());

	for (auto it = _config.location.begin();
		 it != _config.location.end();
		 ++it)
	{
		const std::string& location_path = it->first;
		const cfg::locationCtx& location_cfg = it->second;

		if (!loc_uri_comp(location_path, uri_path))
			continue;
		// For now using the hardcoded base path (must fix!)
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
	
	if (0 < questMark)
		return abs_path.substr(dotPos, questMark);
	return abs_path.substr(dotPos);
}

void  HttpResponse::build(void)
{
	std::pair<int, std::string> res= resolve_path();
	//Leave for edgecase checking 
	std::cout << "Absolute Path is: " << res.second << " status " << res.first << std::endl;
	_status_code = res.first;
	const std::string file_suffix = resolve_file_suffix(res.second);
	set_content_type(resolve_content_type(file_suffix));
	filetype filetype = resolve_filetype(file_suffix);
	std::cout << filetype << std::endl;
	if (filetype == STATIC_FILE)
		read_file_to_body(res.second);
	else if (filetype == DYNAMIC_FILE)
		execute_cgi_proc(res.second, file_suffix);
	
  // 1.Resolve path
  // 2.Resolve Mime type
  // 3.Resolve filetype
  //  -> if is STATIC_FILE
  //  4. read file into body string
  //  -> if is CGI
  //  4. run CGI etc.
  // 5. set client state 
}

