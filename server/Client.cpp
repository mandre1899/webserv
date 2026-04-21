#include "Client.hpp"
#include <cerrno>

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd, int epollFd, const cfg::srvCfg& srvCfg) : _serverFd(serverFd), _clientFd(clientFd), _epollFd(epollFd), _sendedBytes(0), _srvCfg(srvCfg)
{
	_status = Status::close;
	_msg = "";
	_parsingState = state::parsing_request_line;
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(_srvCfg.vHost[0].clientHeaderTimeout);
}

Client::Client(Client&& src) noexcept : _srvCfg(src._srvCfg)
{
	_serverFd = src._serverFd;
	_clientFd = src._clientFd;
	_epollFd = src._epollFd;
	_status = src._status;
	_msg = src._msg;
	_parsingState = src._parsingState;
	_parsedHttp = std::move(src._parsedHttp);
	_sendedBytes = src._sendedBytes;
	_timeout = src._timeout;
	src._serverFd = -1;
	src._clientFd = -1;
	src._epollFd = -1;
	src._msg = "";
	src._status = Status::close;
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

	ret = recv(_clientFd, buffer, sizeof(buffer), 0);
	if (ret < 1)
		throw std::runtime_error("Client connection interrupted or client cosed the connection");
	_msg.append(buffer, ret);
}

/* Member Functions */
uint32_t Client::recvData()
{
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(_srvCfg.vHost[0].clientHeaderTimeout);
	while (_parsingState == state::parsing_request_line || _parsingState == state::parsing_headers)
	{
		auto pos = _msg.find("\r\n");
		_headerBytes = pos + 2;
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
			throw std::runtime_error("clientHeaderBufferSize limit reached");
	}
	if (_parsingState == state::parsing_done)
	{
		Logger::log<LogLevel::trace>("Client::recvData() ", _parsedHttp);
		return (sendData());
	}
	return (EPOLLIN);
}

uint32_t Client::sendData()
{
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(CGI_TIMEOUT_TIME);
	std::make_unique<RequestHandler>(_parsedHttp, _srvCfg); // Throws internally with catching

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

uint32_t Client::handleConnection(uint32_t event)
{
	try
	{
		if (event & EPOLLIN)
		{
			// I need to check if a pipe is processing
			if (_parsingState == state::response_proccessing)
				return (sendData());
			else if (_parsingState == state::response_body)
			{
				recvMsg();
				return (sendData());
			}
			else
			{
				recvMsg();
				return (recvData());
			}
		}
		if (event & EPOLLOUT)
			return (sendData());
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
	catch (const std::exception& e)
	{
		return (EPOLLHUP);
	}
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}


std::chrono::time_point<std::chrono::system_clock> Client::getTimeout() const
{
	return (_timeout);
}
