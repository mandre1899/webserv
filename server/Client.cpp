#include "Client.hpp"

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg) : _serverFd(serverFd), _clientFd(clientFd), _srvCfg(srvCfg)
{
	_status = Status::close;
	_msg = "";
}

Client::Client(Client&& src) noexcept : _srvCfg(src._srvCfg)
{
	_serverFd = src._serverFd;
	_clientFd = src._clientFd;
	_status = src._status;
	_msg = src._msg;
	src._serverFd = -1;
	src._clientFd = -1;
	src._msg = "";
	src._status = Status::close;
}

Client::~Client()
{
	close(_clientFd);
}

/* Member Functions */
uint32_t Client::recvData()
{
	char buffer[20] = {};
	int ret;

	ret = recv(_clientFd, buffer, sizeof(buffer) - 1, 0);
	if (ret == -1)
		return (EPOLLHUP); // evt. throw exception
	_msg.append(buffer, ret);
	std::string end = _msg.substr((_msg.length() - 4));
	if (end == "\r\n\r\n")
	{
		Logger::log<LogLevel::trace>("Client::recvData() - received client msg: \n---\n", _msg, "---");
		sendData();
	}
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
