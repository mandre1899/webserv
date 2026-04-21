#include "ServiceHandler.hpp"
#include "response/ResponseStructs.hpp"

ServiceHandler::ServiceHandler(ResourceInspection data, HttpResponse &response) : _data(data), _response(response)
{
	if (data.kind == ResourceKind::IS_FILE)
		StaticFileService(data, response);
	else if (data.kind == ResourceKind::DIRECTORY)
		StaticFileService(data, response);
}

ServiceHandler::~ServiceHandler(void)
{}


