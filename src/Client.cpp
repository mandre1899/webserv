#include "Client.hpp"

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd) : _serverFd(serverFd), _clientFd(clientFd)
{
	(void)_serverFd;
	(void)_clientFd;
	_status = RECIVE;
	_msg = "";
	_parsingState = parsing_state::request_line;
}

Client::Client(Client&& src) noexcept
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
	src._status = CLOSE;
	src._parsingState = parsing_state::error;

}

Client::~Client()
{
	if (_clientFd != -1)
		close(_clientFd);
}

/* Member Functions */
short Client::recvData()
{
	char buffer[20] = {};
	int ret;

	ret = recv(_clientFd, buffer, sizeof(buffer) - 1, 0);
	if (ret == -1)
		return (POLLHUP); // evt. throw exception
	std::cout << "recevied" << buffer << std::endl;
	_msg.append(buffer, ret);
	while (_parsingState == parsing_state::request_line || _parsingState == parsing_state::headers)
	{
		auto pos = _msg.find("\r\n");
		if (pos == std::string::npos)
			break;
		std::string line = _msg.substr(0, pos);
		if (_parsingState == parsing_state::request_line)
		{
			_parsedHttp.parseRequestLine(line);
			_parsingState = parsing_state::headers;
		}
		else if (_parsingState == parsing_state::headers)
		{
			if (line.empty())
			{
				Logger::log<LogLevel::TRACE>(this->_parsedHttp);
				_msg.erase(0, pos + 2);
				_parsingState = parsing_state::body;
				std::cout << "reached body parsing" << std::endl;
				break;
			}
			_parsedHttp.parseHeaders(line);
		}
		_msg.erase(0, pos + 2);
	}
	if (_parsedHttp.getMethod() == method_type::DELETE || _parsedHttp.getMethod() == method_type::GET)
		_parsingState = parsing_state::done;
	else
	{
		_parsedHttp.parseBody(_msg);
	}
	if (_parsingState == parsing_state::done)
	{
		Logger::log<LogLevel::TRACE>("Client::recvData() - received client msg: \n---\n", _msg, "---");
		//sendData();
		return (POLLOUT);
	}
	if (ret == 0)
		return (POLLHUP);
	return (POLLIN);
}

short Client::sendData()
{
	const char* message = "HTTP/1.1 200 OK\r\nContent-Length: 42\r\n\r\n<html><h1>Hello from WebServ!</h1></html>";
	int ret = send(_clientFd, message, 81, 0);
	Logger::log<LogLevel::TRACE>("Client::sendData() - send return value ist: ", ret);
	return (POLLHUP);
}

short Client::handleConnection(short revents)
{
	if (revents & POLLIN)
		return (recvData());
	if (revents & POLLPRI)
		throw "Unexpectet call";
	if (revents & POLLOUT)
		return (sendData());
	if (revents & POLLRDHUP)
		throw "Unexpectet call";
	if (revents & POLLERR)
		throw "Unexpectet call";
	if (revents & POLLHUP)
		return (POLLHUP);
	if (revents & POLLNVAL)
		throw "Unexpectet call";
	return (POLLHUP);
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}
