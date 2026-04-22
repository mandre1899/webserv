#include "ServiceHandler.hpp"

ServiceHandler::ServiceHandler(ResourceInspection data, HttpResponse& response, int epfd, int socket_fd, Client *currClient)
	: _data(data), _response(response), _epfd(epfd), _socket_fd(socket_fd), _currClient(currClient)
{}

ServiceHandler::~ServiceHandler(void)
{
}

state	ServiceHandler::select_service()
{
	if (_data.kind == ResourceKind::IS_FILE)
	{
		StaticFileService(_data, _response);
		return state::response_sending;
	}
	else if (_data.kind == ResourceKind::DIRECTORY)
	{
		StaticFileService(_data, _response);
		return state::response_sending;
	}
	else if (_data.kind == ResourceKind::CGI)
	{
		_service_state = ServiceState::PROCESSING_CGI;
		_cgiService = std::make_unique<CgiService>(_data, _epfd, _socket_fd);
		_cgiService->execute_cgi_proc(_currClient);
		return state::response_proccessing;
	}
	else if (_data.kind == ResourceKind::UPLOAD_DIR)
	{
		_service_state = ServiceState::PROCESSING_POST;
		_uploadService = std::make_unique<HttpUpload>();
		return state::response_proccessing;
		//TODO
	}
	return state::error;
}

state	ServiceHandler::run(FdContext *ctx, std::string msg)
{
	if (_service_state == ServiceState::PROCESSING_CGI)
	{
		if (ctx->kind == FdKind::CGI_STDOUT_READ)
		{
			auto res = _cgiService->handle_cgi_read();
			if (res.first == cgi_state::FINISHED_OK)
			{
				_response.set_body(_cgiService->get_body());
				_cgiService->parse_cgi_headers();
				header_map headers = _cgiService->get_header_map();
				_response.set_header(headers.begin(), headers.end());
				return state::response_sending;
			}
			else if (res.first == cgi_state::FINISHED_KO)
			{
				Logger::log<LogLevel::error>("CGI execution failed with status: ", res.second);
				return state::error;
			}
			else if (res.first == cgi_state::RUNNING)
			{
				return state::response_proccessing;
			}
		}
		else if (ctx->kind == FdKind::CGI_STDIN_WRITE)
		{
			auto res = _cgiService->handle_cgi_write(msg);
			if (res.first == cgi_state::FINISHED_KO)
			{
				Logger::log<LogLevel::error>("Failed to write CGI body");
				return state::error;
			}
			return state::response_proccessing;
		}
	}
	return state::error;
}

