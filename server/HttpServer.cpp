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
void HttpServer::addClient(int serverFd, int clientFd, const cfg::socketCfg& socCfg)
{
	struct epoll_event event;
	struct sockaddr_in local_addr;
	socklen_t len = sizeof(local_addr);

	getsockname(clientFd, (struct sockaddr*)&local_addr, &len);
	std::string id = std::to_string(local_addr.sin_addr.s_addr) + std::to_string(socCfg.port);
	auto it = socCfg.socketMap.find(id);
	if (it == socCfg.socketMap.end())
	{
		std::string id = std::to_string(INADDR_ANY) + std::to_string(socCfg.port);
		it = socCfg.socketMap.find(id);
		if (it == socCfg.socketMap.end())
		{
			close(clientFd);
			return;
		}
	}
	const cfg::srvCfg& srvCfg = it->second;
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	_map.insert({ clientFd, Client(serverFd, clientFd, _epollFd, srvCfg) });
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
	for (size_t i = 0; i < conf.socSize(); i++)
	{
		const struct cfg::socketCfg& srv = conf.socAt(i);
		struct epoll_event event;

		TcpSocket tmp = TcpSocket(srv.port, srv);
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
