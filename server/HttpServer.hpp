#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <sys/epoll.h> 
#include "config/Config.hpp"
#include "server/Client.hpp"
#include "server/TcpSocket.hpp"
#include "utils/Logger.hpp"

#define MAX_EVENTS 5

class HttpServer {
private:
	int _epollFd;
	std::vector<TcpSocket> _serverSockets{};
	std::unordered_map<int, Client> _clientMap{};

	/* Member Functions */
	void addClient(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg);
	void removeClient(int clientFd);

public:
	/* Constructors & Destructors */
	HttpServer();
	HttpServer(const HttpServer& src) = delete;
	HttpServer(HttpServer&& src) noexcept = delete;
	~HttpServer();

	/* Member Functions */
	void createSockets(Config& conf);
	void run();

	/* Basic Operators */
	HttpServer& operator=(const HttpServer& src) = delete;
	HttpServer& operator=(HttpServer&& src) noexcept = delete;

	/* Getters & Setters */

};

#endif // HTTPSERVER_HPP