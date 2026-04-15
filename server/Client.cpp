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

/* Member Functions */
uint32_t Client::recvData()
{
	char buffer[120] = {};
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
				if (_parsedHttp.getMethod() == method_type::POST)
					_parsingState = state::parsing_body;
				else
					_parsingState = state::parsing_done;
				break;
			}
			_parsedHttp.parseHeaders(line);
		}
		_msg.erase(0, pos + 2);
	}
	if (_parsingState == state::parsing_body)
	{
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
	_timeout = std::chrono::system_clock::now() + std::chrono::seconds(CGI_TIMEOUT_TIME);
	if (_parsingState == state::parsing_done)
		_parsingState = _response.build(&_parsedHttp, &_srvCfg, _clientFd, _epollFd);
	else if (_parsingState == state::response_proccessing)
		_parsingState = _response.handle_cgi_proc();
	if (_parsingState == state::response_proccessing)
		return (EPOLLIN);
	if (_parsingState != state::response_sending)
		return (EPOLLHUP);
	std::string msg = _response.get_response_str();
	while (static_cast<size_t>(_sendedBytes) < msg.size())
	{
		ssize_t ret = send(_clientFd, msg.c_str() + _sendedBytes, msg.size() - _sendedBytes, MSG_DONTWAIT);
		if (ret > 0)
		{
			_sendedBytes += ret;
			continue ;
		}
		if (ret == 0)
			return (EPOLLHUP);
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return (EPOLLOUT);
		if (msg.size() == static_cast<size_t>(_sendedBytes))
			return (EPOLLHUP);
		return (EPOLLOUT);
	}
	return (EPOLLHUP);
}

uint32_t Client::handleConnection(uint32_t event)
{
	if (event & EPOLLIN)
	{
		// I need to check if a pipe is processing
		if (_parsingState != state::response_proccessing)
			return (recvData());
		else
			return (sendData());
	}
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


std::chrono::time_point<std::chrono::system_clock> Client::getTimeout() const
{
	return (_timeout);
}
