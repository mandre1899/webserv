#include "Client.hpp"

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd, int epollFd, const cfg::srvCfg& srvCfg) : _serverFd(serverFd), _clientFd(clientFd), _epollFd(epollFd), _sendedBytes(0), _srvCfg(srvCfg)
{
	_msg = "";
	_parsingState = state::parsing_request_line;
	_version = std::chrono::system_clock::now();
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(_srvCfg.vHost[0].clientHeaderTimeout);
}

Client::Client(Client&& src) noexcept : _srvCfg(src._srvCfg)
{
	_serverFd = src._serverFd;
	_clientFd = src._clientFd;
	_epollFd = src._epollFd;
	_msg = src._msg;
	_parsingState = src._parsingState;
	_parsedHttp = std::move(src._parsedHttp);
	_sendedBytes = src._sendedBytes;
	_version = src._version;
	_timeout = src._timeout;
	src._serverFd = -1;
	src._clientFd = -1;
	src._epollFd = -1;
	src._msg = "";
	src._parsingState = state::error;
	src._sendedBytes = 0;
}

Client::~Client()
{
	if (_clientFd != -1)
		close(_clientFd);
}

void Client::recvMsg()
{
	char buffer[1024] = {};
	int ret;

	ret = recv(_clientFd, buffer, sizeof(buffer) , 0);
	if (ret < 1)
		throw std::runtime_error("Client connection interrupted or client cosed the connection");
	_msg.append(buffer, ret);
}

/* Member Functions */
uint32_t Client::recvData(FdContext* fdCon)
{
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(_srvCfg.vHost[0].clientHeaderTimeout);
	while (_parsingState == state::parsing_request_line || _parsingState == state::parsing_headers)
	{
		auto pos = _msg.find("\r\n");
		if (pos == std::string::npos)
			break;
		std::string line = _msg.substr(0, pos);
		if (_parsingState == state::parsing_request_line)
		{
			_parsedHttp.parseRequestLine(line);
			_parsingState = state::parsing_headers;
		}
		else if (_parsingState == state::parsing_headers)
		{
			_headerBytes = pos + 2;
			if (line.empty())
			{
				_parsingState = state::parsing_done;
				_msg.erase(0, pos + 2);
				break;
			}
			_parsedHttp.parseHeaders(line);
		}
		_msg.erase(0, pos + 2);
		if (_headerBytes > _srvCfg.vHost[0].clientHeaderBufferSize && _srvCfg.vHost[0].clientHeaderBufferSize != 0)
			throw HttpParser::HeadersTooLarge();
	}
	if (_parsingState == state::parsing_done)
	{
		_parsedHttp.checkHeader();
		Logger::log<LogLevel::trace>("Client::recvData() ", _parsedHttp);
		return (sendData(fdCon));
	}
	return (EPOLLIN);
}

uint32_t Client::sendData(FdContext* fdCon)
{
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(CGI_TIMEOUT_TIME);
	if (fdCon->kind == FdKind::CLIENT_SOCKET)
	{
		if (_parsingState == state::response_proccessing)
			return (EPOLLOUT);
	}
	if (_parsingState == state::parsing_done)
	{
		try {
			_request_handler = std::make_unique<RequestHandler>(_parsedHttp, _response, _srvCfg);
			ResourceInspection data = _request_handler->get_result();
			if (data.has_redirect)
			{
				int redir_code = data.redirect_code;
				if ((redir_code % 300) < 10)
				{
					_response.set_status(redir_code);
					_response.set_header("Location", data.redirect_target);
				}
				else
					_response.set_default_reponse(redir_code, *_request_handler->get_location());
				_response.set_default_headers();
				_parsingState = state::response_sending;
			}
			else if (!data.exists)
			{
				_response.set_default_reponse(404, *_request_handler->get_location());
				_parsingState = state::response_sending;
			}
			else
			{
				_service_handler = std::make_unique<ServiceHandler>(data, _response, _epollFd, _clientFd, this, _srvCfg.vHost[0].clientMaxBodySize);
				_parsingState = _service_handler->select_service();
				return (EPOLLOUT);
			}
		}
		catch (const RequestHandler::MethodNotAllowed &e) {
			Logger::log<LogLevel::warn>("Method not allowed: ", e.what());
			_parsingState = state::response_sending;
		}
		catch (const RequestHandler::PathResolutionFailed &e) {
			Logger::log<LogLevel::error>("Path resolution failed: ", e.what());
			_response.set_default_reponse(400);
			_parsingState = state::response_sending;
		}
		catch (const RequestHandler::BadRequest &e) {
			Logger::log<LogLevel::warn>("Bad request: ", e.what());
			_response.set_default_reponse(400);
			_parsingState = state::response_sending;
		}
		catch (const std::exception &e) {
			Logger::log<LogLevel::error>("Request handler exception: ", e.what());
			_response.set_default_reponse(500);
			_parsingState = state::response_sending;
		}
		catch (...) {
			Logger::log<LogLevel::error>("Unknown exception in request handler");
			_response.set_default_reponse(500);
			_parsingState = state::response_sending;
		}
	}
	else if (_parsingState == state::response_proccessing ||
			_parsingState == state::response_body)
	{
		_parsingState = _service_handler->run(fdCon, _msg);
		if (_parsingState == state::response_body)
			return (EPOLLIN);
		return (EPOLLOUT);
	}
	std::string msg = _response.get_response_str();
	_parsingState = state::response_sending;
	ssize_t ret = send(_clientFd, msg.c_str() + _sendedBytes, msg.size() - _sendedBytes, 0);
	if (ret > 0)
	{
		_sendedBytes += ret;
		if (_sendedBytes >= msg.size())
			return (EPOLLHUP);
		return (EPOLLOUT);
	}
	return (EPOLLHUP);
}

uint32_t Client::handleConnection(uint32_t event, FdContext* fdCon)
{
	try
	{
		if (event & EPOLLIN)
		{ // I need to check if a pipe is processing
			if (_parsingState == state::response_proccessing)
				return (sendData(fdCon));
			else if (_parsingState == state::response_body)
			{
				recvMsg();
				return (sendData(fdCon));
			}
			else
			{
				recvMsg();
				return (recvData(fdCon));
			}
		}
		if (event & EPOLLOUT)
			return (sendData(fdCon));
		if (event & EPOLLHUP)
			return (EPOLLHUP);
		// if (event & EPOLLPRI)
		// 	throw "Unexpectet call"; // todo add all epoll events
		// if (event & EPOLLRDHUP)
		// 	throw "Unexpectet call";
		// if (event & EPOLLERR)
		// 	throw "Unexpectet call";
		return (EPOLLHUP);
	}
	catch (const HttpParser::RequestUriTooLarge& e)
	{
		// TODO send respone with code 414
		_response.set_default_reponse(414);
		_parsingState = state::response_sending;
		return (EPOLLOUT);
	}
	catch (const HttpParser::HeadersTooLarge& e)
	{
		//TODO send respone with 400
		_response.set_default_reponse(400);
		_parsingState = state::response_sending;
		return (EPOLLOUT);
	}
	catch (const HttpParser::HttpVersionConflict& e)
	{
		//TODO send bad request with 400
		_response.set_default_reponse(400);
		_parsingState = state::response_sending;
		return (EPOLLOUT);
	}
	catch (const std::exception& e)
	{
		_response.set_default_reponse(500);
		_parsingState = state::response_sending;
		return (EPOLLOUT);
	}
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}

std::chrono::system_clock::time_point Client::getVersion() const
{
	return (_version);
}

std::chrono::system_clock::time_point Client::getTimeout() const
{
	return (_timeout);
}
