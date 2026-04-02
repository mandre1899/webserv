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
	  {"notype", "application/octet-stream"}
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

size_t	HttpResponse::get_vserver_idx(void) const
{
	std::string_view host = _request.getHeadersMap().find("Host")->second;
	for (size_t i = 0; i < _config.size(); i++)
	{
		if (host == _config.at(i).serverName)
			return i;
	}
	return 0;
}


//Private Member function - HELPER END

HttpResponse::HttpResponse(HttpParser &req, const Config& conf) : _request(static_cast<const HttpParser&>(req)), _config(conf)
{}

HttpResponse::HttpResponse(const HttpParser &req, const Config& conf) : _request(req), _config(conf)
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

static bool matches_location(const std::string& path, const std::string& loc)
{
    if (path == loc)
		return true;
    if (path.size() <= loc.size())
		return false;

    return path.compare(0, loc.size(), loc) == 0 &&
       path[loc.size()] == '/';
}

std::pair<int, std::string>	HttpResponse::resolve_path(void)
{
	cfg::srvCtxCfg server = _config.at(get_vserver_idx());
	cfg::locationCtx location;
	if (server.location.empty())
		return  rtn_pair(404, std::string());
	for (const auto& loc : server.location)
	{
		//path == location || path.starts_with(location + "/")
		if (matches_location(loc.first, _request.getRequestUri()))
			cfg::locationCtx location = loc.second;
	}
	return rtn_pair(404, server.errorPage->path);
}

filetype  HttpResponse::resolve_filetype(std::string& abs_path)
{
  if (abs_path.find(".php") || abs_path.find(".py"))
    return DYNAMIC_FILE;
  return STATIC_FILE;
}

void  HttpResponse::build(void)
{
  // 1.Resolve path
  // 2.Resolve Mime type
  // 3.Resolve filetype
  //  -> if is STATIC_FILE
  //  4. read file into body string
  //  -> if is CGI
  //  4. run CGI etc.
  // 5. set client state 
}

