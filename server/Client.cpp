#include "Client.hpp"

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg) : _serverFd(serverFd), _clientFd(clientFd), _srvCfg(srvCfg)
{
	_status = Status::close;
	_msg = "";
	_parsingState = state::parsing_request_line;
}

Client::Client(Client&& src) noexcept : _srvCfg(src._srvCfg)
{
	_serverFd = src._serverFd;
	_clientFd = src._clientFd;
	_status = src._status;
	_msg = src._msg;
	_parsingState = src._parsingState;
	_parsedHttp = std::move(src._parsedHttp);
	src._serverFd = -1;
	src._clientFd = -1;
	src._msg = "";
	src._status = Status::close;
	src._parsingState = state::error;

}

Client::~Client()
{
	if (_clientFd != -1)
		close(_clientFd);
}

/* Member Functions */
uint32_t Client::recvData()
{
	char buffer[20] = {};
	int ret;

	ret = recv(_clientFd, buffer, sizeof(buffer) - 1, 0);
	if (ret < 1)
		return (EPOLLHUP); // evt. throw exception
	_msg.append(buffer, ret);
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
			if (line.empty())
			{
				Logger::log<LogLevel::trace>(this->_parsedHttp);
				_msg.erase(0, pos + 2);
				_parsingState = state::parsing_body;
				std::cout << "reached body parsing" << std::endl;
				break;
			}
			_parsedHttp.parseHeaders(line);
		}
		_msg.erase(0, pos + 2);
	}
	if ((_parsedHttp.getMethod() == method_type::DELETE || _parsedHttp.getMethod() == method_type::GET) && _parsingState == state::parsing_body)
		_parsingState = state::parsing_done;
	else if (_parsingState == state::parsing_body)
	{
		std::cout << this->_parsedHttp << std::endl;
		if (_parsedHttp.parseBody(_msg))
			_parsingState = state::parsing_done;
		_msg.erase();
	}
	if (_parsingState == state::parsing_done)
		return (EPOLLOUT);
	if (ret == 0)
		return (EPOLLHUP);
	return (EPOLLIN);
}

uint32_t Client::sendData()
{
	const char* message = "HTTP/1.1 200 OK\r\nContent-Length: 42\r\n\r\n<html><h1>Hello from WebServ!</h1></html>";
	int ret = send(_clientFd, message, 81, 0);
	Logger::log<LogLevel::trace>("Client::sendData() - send return value ist: ", ret);
	return (EPOLLHUP);
}

uint32_t Client::handleConnection(uint32_t event)
{
	if (event & EPOLLIN)
		return (recvData());
	if (event & EPOLLPRI)
		throw "Unexpectet call"; //todo
	if (event & EPOLLOUT)
		return (sendData());
	if (event & EPOLLRDHUP)
		throw "Unexpectet call";
	if (event & EPOLLERR)
		throw "Unexpectet call";
	if (event & EPOLLHUP)
		return (EPOLLHUP);
	// todo add all epoll events
	return (EPOLLHUP);
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}
