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



//Private Member function - HELPER END

HttpResponse::HttpResponse(HttpParser &req, const cfg::srvCtxCfg& conf) : _request(static_cast<const HttpParser&>(req)), _config(conf)
{}

HttpResponse::HttpResponse(const HttpParser &req, const cfg::srvCtxCfg& conf) : _request(req), _config(conf)
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

static bool	loc_uri_comp(const std::string& loc, std::string& uri)
{
	if (loc.rfind(uri, 0) != std::string::npos)
		return true;
	return false;
}

std::pair<int, std::string>	HttpResponse::resolve_path(void)
{
	std::string root_path = "/Volumes/MateoSSD/42Projects/network_programming/webserv_mausperg";
	//std::string uri_path = clean_string(_request.getRequestUri());
	std::string uri_path = _request.getRequestUri();
	if (_config.location.empty())
		return  rtn_pair(404, std::string());
	for (const auto& loc : _config.location)
	{
		if (loc_uri_comp(loc.first, uri_path))
		{
			std::cout << "Location root: " << loc.second.root << std::endl;
			return rtn_pair(200, root_path + "/" + loc.second.index);
			// URI has valid location block match ablolute path
		}
	}
	return rtn_pair(404, _config.errorPage->path);
}

filetype  HttpResponse::resolve_filetype(const std::string& abs_path)
{
  if (abs_path.find(".php") || abs_path.find(".py"))
    return DYNAMIC_FILE;
  return STATIC_FILE;
}

void  HttpResponse::build(void)
{
	std::pair<int, std::string> res= resolve_path();
	std::cout << "Absolute Path is: " << res.second << std::endl;
  // 1.Resolve path
  // 2.Resolve Mime type
  // 3.Resolve filetype
  //  -> if is STATIC_FILE
  //  4. read file into body string
  //  -> if is CGI
  //  4. run CGI etc.
  // 5. set client state 
}

