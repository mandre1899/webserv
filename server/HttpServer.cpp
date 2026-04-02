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

	event.events = EPOLLIN;
	event.data.fd = clientFd;
	_map.insert({ clientFd, Client(serverFd, clientFd, srvCfg) });
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event))
		throw std::runtime_error("Failed to add file descriptor to epoll");
	Logger::log<LogLevel::debug>("HttpServer::addClient() - Client added with FD ", clientFd);
}

void HttpServer::removeClient(int clientFd)
{
	struct epoll_event event;

	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, &event))
		throw std::runtime_error("Failed to delete file descriptor to epoll");
	_map.erase(clientFd);
	Logger::log<LogLevel::debug>("HttpServer::removeClient - Removed client with FD ", clientFd);
}

void HttpServer::createSockets(Config& conf)
{
	for (size_t i = 0; i < conf.size(); i++)
	{
		const struct cfg::srvCtxCfg& srv = conf.at(i);
		struct epoll_event event;

		TcpSocket tmp = TcpSocket(srv.listen.address, srv.listen.port, srv);
		event.events = EPOLLIN;
		event.data.fd = tmp.getSocketFd();
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, tmp.getSocketFd(), &event))
			throw std::runtime_error("Failed to add file descriptor to epoll");
		Logger::log<LogLevel::trace>("HttpServer::createSockets() - Socket created with FD ", tmp.getSocketFd());
		_map.insert({ tmp.getSocketFd(), std::move(tmp) });
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
		auto it = _map.find(events[i].data.fd);
		if (std::holds_alternative<TcpSocket>(it->second)) // if (serverSocket)...
		{
			TcpSocket& socket = std::get<TcpSocket>(it->second);
			addClient(events[i].data.fd, socket.acceptClient(), socket.getConfig());
		}
		else // if (clientSocket)...
		{
			Client& c = std::get<Client>(it->second);
			events[i].events = c.handleConnection(events[i].events);
			if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]))
				throw std::runtime_error("Failed to modifie file descriptor in epoll");
			if (events[i].events & EPOLLHUP)
				removeClient(events[i].data.fd);
		}
	}
}

/* Basic Operators */

/* Getters & Setters */
