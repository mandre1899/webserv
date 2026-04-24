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
	for (const auto& [key, value] : _map)
	{
		if (value->kind == FdKind::SERVER_SOCKET)
			delete std::get<TcpSocket*>(value->socket);
		else
			delete std::get<Client*>(value->socket);
		delete value;
	}
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
	FdContext* clientContext = new FdContext;
	Client* c = new Client(serverFd, clientFd, _epollFd, srvCfg);
	clientContext->socket = c;
	clientContext->fd = clientFd;
	clientContext->kind = FdKind::CLIENT_SOCKET;
	_map.insert({ clientFd, clientContext });
	event.events = EPOLLIN;
	event.data.ptr = clientContext;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event))
		throw std::runtime_error("Failed to add file descriptor to epoll");
	_prioQ.push({ c->getTimeout(), clientFd, c->getVersion() });
	Logger::log<LogLevel::debug>("HttpServer::addClient() - Client added with FD ", clientFd);
}

void HttpServer::removeClient(FdContext* clientContext)
{
	struct epoll_event event;

	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientContext->fd, &event))
		throw std::runtime_error("Failed to delete file descriptor to epoll");
	_map.erase(clientContext->fd);
	Logger::log<LogLevel::debug>("HttpServer::removeClient - Removed client with FD ", clientContext->fd);
	delete std::get<Client*>(clientContext->socket);
	delete clientContext;
}

void HttpServer::createSockets(Config& conf)
{
	for (size_t i = 0; i < conf.socSize(); i++)
	{
		const struct cfg::socketCfg& srv = conf.socAt(i);
		struct epoll_event event;

		FdContext* clientContext = new FdContext;
		clientContext->socket = new TcpSocket(srv.port, srv);
		clientContext->fd = std::get<TcpSocket*>(clientContext->socket)->getSocketFd();
		clientContext->kind = FdKind::SERVER_SOCKET;
		_map.insert({ clientContext->fd, clientContext });
		event.events = EPOLLIN;
		event.data.ptr = clientContext;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientContext->fd, &event))
			throw std::runtime_error("Failed to add file descriptor to epoll");
		Logger::log<LogLevel::trace>("HttpServer::createSockets() - Socket created with FD ", clientContext->fd);
	}
}

void HttpServer::checkClientTimeout()
{
	auto now = std::chrono::system_clock::now();
	while (!_prioQ.empty())
	{
		sessionTime top = _prioQ.top();
		if (now < top.timeout)
			break;
		_prioQ.pop();
		int cFd = top.clientFd;
		auto it = _map.find(cFd);
		if (it == _map.end())
			continue;
		FdContext* fdCon = it->second;
		Client* c = std::get<Client*>(fdCon->socket);
		if (c->getVersion() != top.version)
			continue;
		if (now > c->getTimeout())
			removeClient(fdCon);
		else
			_prioQ.push({ c->getTimeout(), c->getFd(), top.version });
		now = std::chrono::system_clock::now();
	}
}

void HttpServer::run()
{
	struct epoll_event events[MAX_EVENTS];
	int eventCount = epoll_wait(_epollFd, events, MAX_EVENTS, 1000);
	if (eventCount == -1)
	{
		if (g_stopServerSignal)
			return;
		throw std::runtime_error("System error occurred in the epoll_wait loop");
	}
	for (int i = 0; i < eventCount; i++)
	{
		FdContext* fdCon = static_cast<FdContext*>(events[i].data.ptr);
		FdKind type = fdCon->kind;
		if (fdCon->kind == FdKind::SERVER_SOCKET) // if (serverSocket)...
		{
			TcpSocket* socket = std::get<TcpSocket*>(fdCon->socket);
			addClient(fdCon->fd, socket->acceptClient(), socket->getConfig());
		}
		else // if (clientSocket)...
		{
			Client* c = std::get<Client*>(fdCon->socket);
			events[i].events = c->handleConnection(events[i].events, fdCon);
			if (type == FdKind::CGI_STDOUT_READ || type == FdKind::CGI_STDIN_WRITE ||
				type == FdKind::CGI_STDERR_READ)
				continue;
			if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fdCon->fd, &events[i]))
				throw std::runtime_error("Failed to modify file descriptor in epoll");
			if (events[i].events & EPOLLHUP)
				removeClient(fdCon);
		}
	}
	checkClientTimeout();
}

/* Basic Operators */

/* Getters & Setters */


bool sessionTime::operator<(const sessionTime& rhs) const
{
	return (rhs.timeout < timeout);
}
