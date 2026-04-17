#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <sys/epoll.h> 
#include <variant>
#include <functional>
#include <queue>
#include <chrono>
#include "config/Config.hpp"
#include "server/Client.hpp"
#include "server/TcpSocket.hpp"
#include "utils/Logger.hpp"

#define MAX_EVENTS 5

extern volatile std::sig_atomic_t g_stopServerSignal;

struct sessionTime
{
	std::chrono::time_point<std::chrono::system_clock> timeout;
	int clientFd;
	bool operator<(const sessionTime& rhs) const;
};

class HttpServer {
private:
	int _epollFd;
	std::unordered_map<int, std::variant<TcpSocket, Client>> _map{};
	std::priority_queue<sessionTime, std::vector<sessionTime>, std::less<sessionTime>> _prioQ;

	/* Member Functions */
	void addClient(int serverFd, int clientFd, const cfg::socketCfg& socCfg);
	bool removeClient(int clientFd);
	void checkClientTimeout();

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
