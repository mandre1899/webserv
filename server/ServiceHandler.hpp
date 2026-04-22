#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H

#include "response/HttpResponse.hpp"
#include "response/ResponseStructs.hpp"
#include "response/StaticFileService.hpp"
#include "response/HttpUploadService.hpp"
#include "response/CgiService.hpp"

#include <memory>

enum ServiceState
{
	PROCESSING_CGI,
	PROCESSING_POST
};

class ServiceHandler
{
	ResourceInspection				_data; //The logical resolve from request
	HttpResponse					&_response; // A reference for the response
	std::unique_ptr<CgiService>		_cgiService;
	public:
		ServiceHandler(void) = delete;
		ServiceHandler(ResourceInspection data, HttpResponse &response, int epfd, int socket_fd);
		ServiceHandler	operator=(const ServiceHandler &obj) = delete;
		~ServiceHandler(void);
};

#endif // !SERVIC
