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
std::string     HttpResponse::clean_string(std::string& path);
{
  std::string tmp = path;

  if (tmp[0] == '/')
    tmp.erase(tmp.begin());
  if (tmp[tmp.length - 1] == '/')
    tmp.erase(tmp.length() - 1);
  return tmp;
}


//Private Member function - HELPER END

HttpResponse::HttpResponse(HttpParser &req) : _request(static_cast<const HttpParser&>(req))
{}

HttpResponse::HttpResponse(const HttpParser  &req) : _request(req)
{}

HttpResponse::~HttpResponse()
{}

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

