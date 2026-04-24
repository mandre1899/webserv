#include "ServiceHandler.hpp"

ServiceHandler::ServiceHandler(ResourceInspection data, HttpResponse& response, int epfd, int socket_fd, Client* currClient, size_t clientMaxBodySize)
	: _data(data), _response(response), _epfd(epfd), _socket_fd(socket_fd), _currClient(currClient), _clientMaxBodySize(clientMaxBodySize)
{
}

ServiceHandler::~ServiceHandler(void)
{
}

state	ServiceHandler::select_service()
{
	if (_data.kind == ResourceKind::IS_FILE)
	{
		StaticFileService(_data, _response);
		_response.set_default_headers();
		return state::response_sending;
	}
	else if (_data.kind == ResourceKind::DIRECTORY)
	{
		if (_data.status == InspectStatus::OK)
		{
			StaticFileService(_data, _response);
			_response.set_default_headers();
			return state::response_sending;
		}
		else
			_response.set_default_reponse(404, _data.currLoc, _data.currServer);
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
		try
		{
			_service_state = ServiceState::PROCESSING_POST;
			_uploadService = std::make_unique<HttpUploadService>(_data.request_headers, _data.upload_path, _clientMaxBodySize);
			return state::response_body;
		}
		catch (const HttpUploadService::MissingLengthException& e)
		{
			Logger::log<LogLevel::warn>("Upload missing Content-Length: ", e.what());
			_response.set_default_reponse(411);
			return state::response_sending;
		}
		catch (const HttpUploadService::BadRequestException& e)
		{
			Logger::log<LogLevel::warn>("Upload bad request: ", e.what());
			_response.set_default_reponse(400);
			return state::response_sending;
		}
		catch (const HttpUploadService::BodyTooLargeException& e)
		{
			Logger::log<LogLevel::warn>("Upload body too large: ", e.what());
			_response.set_default_reponse(413);
			return state::response_sending;
		}
		catch (const std::exception& e)
		{
			Logger::log<LogLevel::error>("Upload initialization failed: ", e.what());
			_response.set_default_reponse(500);
			return state::response_sending;
		}
	}
	return state::error;
}

state	ServiceHandler::run(FdContext* ctx, std::string msg)
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
				int status_code = 200;
				if (headers.find("Status") != headers.end()) {
				    try {
				        status_code = std::stoi(headers.at("Status"));
				    } catch (const std::exception& e) {
				        Logger::log<LogLevel::warn>("Invalid CGI Status header, defaulting to 200: ", e.what());
				        status_code = 200;
				    }
				}
				_response.set_status(status_code);
				_response.set_header(headers.begin(), headers.end());
				_response.set_default_headers();
				return state::response_sending;
			}
			else if (res.first == cgi_state::FINISHED_KO)
			{
				_response.set_default_reponse(500);
				Logger::log<LogLevel::error>("CGI execution failed with status: ", res.second);
				return state::error;
			}
			else if (res.first == cgi_state::RUNNING)
				return state::response_proccessing;
		}
		else if (ctx->kind == FdKind::CGI_STDIN_WRITE)
		{
			auto res = _cgiService->handle_cgi_write(msg);
			if (res.first == cgi_state::FINISHED_KO)
			{
				_response.set_default_reponse(500);
				Logger::log<LogLevel::error>("Failed to write CGI body");
				return state::error;
			}
			else if (res.first == cgi_state::FINISHED_OK)
				return state::response_proccessing;
			return state::response_body;
		}
	}
	else if (_service_state == ServiceState::PROCESSING_POST)
	{
		try
		{
			if (_uploadService->uploadBody(msg))
			{
				Logger::log<LogLevel::info>("Upload completed successfully");
				_response.set_status(201);
				_response.set_body("File uploaded successfully\r\n");
				_response.set_content_type("text/plain");
				_response.set_default_headers();
				return state::response_sending;
			}
			else
			{
				return state::response_body;
			}
		}
		catch (const HttpUploadService::BadRequestException& e)
		{
			Logger::log<LogLevel::warn>("Upload parsing error: ", e.what());
			_response.set_default_reponse(400);
			return state::response_sending;
		}
		catch (const HttpUploadService::BodyTooLargeException& e)
		{
			Logger::log<LogLevel::warn>("Upload exceeded max size: ", e.what());
			_response.set_default_reponse(413);
			return state::response_sending;
		}
		catch (const HttpUploadService::InternelErrorException& e)
		{
			Logger::log<LogLevel::error>("Upload internal error: ", e.what());
			_response.set_default_reponse(500);
			return state::response_sending;
		}
		catch (const std::exception& e)
		{
			Logger::log<LogLevel::error>("Upload exception: ", e.what());
			_response.set_default_reponse(500);
			return state::response_sending;
		}
	}
	return state::error;
}

