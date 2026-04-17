#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <sys/socket.h> 
#include <iostream>
#include <sys/epoll.h> 
#include <unistd.h>
#include <chrono>
#include "config/configTypes.hpp"
#include "HttpResponse.hpp"
#include "utils/Logger.hpp"
#include "HttpParser.hpp"
#include "StateMachines.hpp"

class Client {
private:
	int _serverFd;
	int _clientFd;
	int _epollFd;
	std::chrono::time_point<std::chrono::system_clock> _timeout;
	Status _status;
	std::string _msg;
	HttpParser _parsedHttp;
	size_t	_headerBytes = {};
	size_t	_sendedBytes = {};
	state _parsingState;
	const cfg::srvCfg& _srvCfg;
	HttpResponse	_response;

	/* Member Functions */
	void recvMsg();
	uint32_t recvData();
	uint32_t sendData();

public:
	/* Constructors & Destructors */
	Client() = delete;
	Client(int serverFd, int clientFd, int epollFd, const cfg::srvCfg& srvCfg);
	Client(const Client& src) = delete;
	Client(Client&& src) noexcept;
	~Client();

	/* Member Functions */
	uint32_t handleConnection(uint32_t event);

	/* Basic Operators */
	Client& operator=(const Client& src) = delete;
	Client& operator=(Client&& src) = delete;

	/* Getters & Setters */
	int getFd() const;
	std::chrono::time_point<std::chrono::system_clock> getTimeout() const;
};

#endif // CLIENT_HPP
