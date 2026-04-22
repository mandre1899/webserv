#include "ServiceHandler.hpp"
#include "response/ResponseStructs.hpp"

ServiceHandler::ServiceHandler(ResourceInspection data, HttpResponse& response, int epfd, int socket_fd) : _data(data), _response(response)
{
	(void)_response; // todo: Why hasn't Response been used so far?
	if (data.kind == ResourceKind::IS_FILE)
		StaticFileService(data, response);
	else if (data.kind == ResourceKind::DIRECTORY)
		StaticFileService(data, response);
	else if (data.kind == ResourceKind::CGI)
	{
		_cgiService = std::make_unique<CgiService>(_data, epfd, socket_fd);

	}
}

ServiceHandler::~ServiceHandler(void)
{
}


