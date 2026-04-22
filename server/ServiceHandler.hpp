#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H

#include "response/HttpResponse.hpp"
#include "response/ResponseStructs.hpp"
#include "response/StaticFileService.hpp"
#include "response/HttpUploadService.hpp"
#include "response/CgiService.hpp"
#include "server/serverTypes.hpp"

#include <memory>

class Client;

enum ServiceState
{
	PROCESSING_STATICFILE,
	PROCESSING_CGI,
	PROCESSING_POST
};

class ServiceHandler
{
	ResourceInspection				_data; //The logical resolve from request
	HttpResponse					&_response; // A reference for the response
	int								_epfd;
	int								_socket_fd;
	Client							*_currClient;
	ServiceState					_service_state;
	std::unique_ptr<CgiService>		_cgiService;
	std::unique_ptr<HttpUpload>		_uploadService;
	public:
		ServiceHandler(void) = delete;
		ServiceHandler(ResourceInspection data, HttpResponse &response, int epfd, int socket_fd, Client *currClient);
		ServiceHandler	operator=(const ServiceHandler &obj) = delete;
		~ServiceHandler(void);

		state	select_service(void);
		state	run(FdContext *ctx, std::string msg);
};

#endif // !SERVICE_HANDLER_H
