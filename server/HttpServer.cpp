#include "HttpServer.hpp"

/* Constructors & Destructors */
HttpServer::HttpServer()
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw std::runtime_error("Failed to create epoll file descriptor");
}

HttpServer::~HttpServer()
{
	close(_epollFd);
}

/* Member Functions */
void HttpServer::addClient(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg)
{
	struct epoll_event event;

	_clientMap.insert({ clientFd, Client(serverFd, clientFd, srvCfg) });
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event))
		throw std::runtime_error("Failed to add file descriptor to epoll");
	Logger::log<LogLevel::debug>("HttpServer::addClient() - Client added with FD ", clientFd);
}

void HttpServer::removeClient(int clientFd)
{
	struct epoll_event event;

	_clientMap.erase(clientFd);
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, &event))
		throw std::runtime_error("Failed to delete file descriptor to epoll");
	Logger::log<LogLevel::debug>("HttpServer::removeClient - Removed client with FD ", clientFd);
}

void HttpServer::createSockets(Config& conf)
{
	for (size_t i = 0; i < conf.size(); i++)
	{
		const struct cfg::srvCtxCfg& srv = conf.at(i);
		struct epoll_event event;

		_serverSockets.push_back(TcpSocket(srv.listen.address, srv.listen.port, srv));
		event.events = EPOLLIN;
		event.data.fd = _serverSockets[i].getSocketFd();
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSockets[i].getSocketFd(), &event))
			throw std::runtime_error("Failed to add file descriptor to epoll");
		Logger::log<LogLevel::trace>("HttpServer::createSockets() - Socket created with FD ", _serverSockets[i].getSocketFd());
	}
}

void HttpServer::run()
{
	struct epoll_event events[MAX_EVENTS];
	int eventCount = epoll_wait(_epollFd, events, MAX_EVENTS, 30000);
	if (eventCount == -1)
		throw "System error occurred in the epoll_wait loop";
	for (int i = 0; i < eventCount; i++)
	{
		if (_serverSockets.find(events[i].data.fd) != _serverSockets.end()) // if (serverSocket)...
		{
			TcpSocket& socket = _serverSockets[events[i].data.u64];
			// if (events->events & EPOLLIN)
			// {
			addClient(events[i].data.fd, socket.acceptClient(), socket.getConfig());
			//}
		}
		else // if (clientSocket)...
		{
			Client& c = (_clientMap.find(events[i].data.fd))->second;
			events[i].events = c.handleConnection(events[i].events);
			// if (_pfd[i].events & POLLHUP)
			// 	removeClient(_pfd[i].fd);
		}
	}
}

/* Basic Operators */

/* Getters & Setters */
