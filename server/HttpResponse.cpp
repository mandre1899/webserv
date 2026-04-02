#include "HttpResponse.hpp"

HttpResponse::HttpResponse(HttpParser &req) : _request(static_cast<const HttpParser&>(req))
{}

HttpResponse::HttpResponse(const HttpParser  &req) : _request(req)
{}

HttpResponse::~HttpResponse()
{}



