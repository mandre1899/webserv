#ifndef HTTP_RESPONSE_H
# define HTTP_RESPONSE_H

#include "./HttpParser.hpp"
#include "../config/Config.hpp"

enum filetype {
  STATIC_FILE,
  DYNAMIC_FILE,
  UNKNOWN // Currently not used maybe used for more robustness
};

class HttpResponse
{
j int                                 _status_code;
  std::string                         _content_type;
  std::map<std::string, std::string>  _headers;
  std::string                         _body;
  //Request Params END
  const HttpParser&   _request;
  const Config&       _config;

  const static std::map<std::string, std::string>  _mime_types; // Static MimeTypes map

  //Private Member function - HELPER
  std::string     clean_string(std::string& path); // Removes suffixed and prefixed slash (Helper function)

  //Private Member function - MAIN
  std::pair<int, std::string> resolve_path(const std::string& uri);
  std::string                 resolve_content_type(const std::string& abs_path);
  filetype                    resolve_filetype(std::string& abs_path); // STATIC, DYNAMIC, UNKNOWN (Improve ??)
  
  public:
    HttpResponse(void) = delete;
    HttpResponse(const HttpParser &req);
    HttpResponse(HttpParser &req);
    ~HttpResponse(void);

  //
  void  build(void); // Builds response

};

#endif // !HTTP_RESPONSE_H
