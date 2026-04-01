#include "HttpServer.hpp"

/* Constructors & Destructors */
HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
}

/* Member Functions */
void HttpServer::addClient(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg)
{
	struct pollfd pollFd;

	_clientMap.insert({ clientFd, Client(serverFd, clientFd, srvCfg) });
	pollFd.fd = clientFd;
	pollFd.events = POLLIN;
	_pfd.push_back(pollFd);
	Logger::log<LogLevel::debug>("HttpServer::addClient() - Client added with FD ", clientFd);
}

void HttpServer::removeClient(int clientFd)
{
	_clientMap.erase(clientFd);
	for (size_t i = 0; i < _pfd.size(); i++)
	{
		if (_pfd[i].fd == clientFd)
		{
			_pfd.erase(_pfd.begin() + i);
		}
	}
	Logger::log<LogLevel::debug>("HttpServer::removeClient - Removed client with FD ", clientFd);
}

void HttpServer::createSockets(Config& conf)
{
	for (size_t i = 0; i < conf.size(); i++)
	{
		const struct cfg::srvCtxCfg& srv = conf.at(i);
		struct pollfd pollFd;

		_serverSockets.push_back(TcpSocket(srv.listen.address, srv.listen.port, srv));
		pollFd.fd = _serverSockets[i].getSocketFd();
		pollFd.events = POLLIN;
		_pfd.push_back(pollFd);
		Logger::log<LogLevel::trace>("HttpServer::createSockets() - Socket created with FD ", pollFd.fd);
	}
}

void HttpServer::run()
{
	size_t pfdSize = _pfd.size();
	int ret = poll(_pfd.data(), pfdSize, -1);
	(void)ret; // TODO: optimise speed
	for (size_t i = 0; i < pfdSize; i++)
	{
		if (i < _serverSockets.size()) // if (serverSocket)...
		{
			if (_pfd[i].revents & POLLIN)
			{
				addClient(_pfd[i].fd, _serverSockets[i].acceptClient(), _serverSockets[i].getConfig());
			}
		}
		else // if (clientSocket)...
		{
			Client& c = (_clientMap.find(_pfd[i].fd))->second;
			_pfd[i].events = c.handleConnection(_pfd[i].revents);
			if (_pfd[i].events & POLLHUP)
				removeClient(_pfd[i].fd);
		}
	}
}

/* Basic Operators */

/* Getters & Setters */
