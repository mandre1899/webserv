#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <sys/poll.h>
#include "server/Client.hpp"
#include "server/TcpSocket.hpp"
#include "utils/Logger.hpp"

class HttpServer {
private:
	std::vector<TcpSocket> _serverSockets{};
	std::unordered_map<int, Client> _clientMap{};
	std::vector<struct pollfd> _pfd{};

	/* Member Functions */
	void addClient(int serverFd, int clientFd);
	void removeClient(int clientFd);

public:
	/* Constructors & Destructors */
	HttpServer();
	HttpServer(const HttpServer& src) = delete;
	HttpServer(HttpServer&& src) noexcept = delete;
	~HttpServer();

	/* Member Functions */
	void parseConfig(std::string path);
	void createSockets();
	void run();

	/* Basic Operators */
	HttpServer& operator=(const HttpServer& src) = delete;
	HttpServer& operator=(HttpServer&& src) noexcept = delete;

	/* Getters & Setters */

};

#endif // HTTPSERVER_HPP