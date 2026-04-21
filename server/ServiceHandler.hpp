#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H

#include "response/HttpResponse.hpp"
#include "response/ResponseStructs.hpp"
#include "server/response/StaticFileService.hpp"
#include <memory>

enum ServiceState
{
	PROCESSING_CGI,
	PROCESSING_POST
};

class ServiceHandler
{
	ResourceInspection	_data; //The logical resolve from request
	HttpResponse		&_response; // A reference for the response
	std::unique_ptr<StaticFileService>	_staticFileService;
	public:
		ServiceHandler(ResourceInspection data, HttpResponse &response);
		~ServiceHandler(void);
};

#endif // !SERVIC
