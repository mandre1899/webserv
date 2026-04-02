#ifndef HTTP_RESPONSE_H
# define HTTP_RESPONSE_H

#include "./HttpParser.hpp"

class HttpResponse
{
  const HttpParser& _request;

  
  
  public:
    HttpResponse(void) = delete;
    HttpResponse(const HttpParser &req);
    HttpResponse(HttpParser &req);
    ~HttpResponse(void);

  //
  void  build(void); // Builds response

};

#endif // !HTTP_RESPONSE_H
