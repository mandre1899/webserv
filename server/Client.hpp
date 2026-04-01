#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h> 
#include <iostream>
#include <sys/epoll.h> 
#include <unistd.h>
#include "config/configTypes.hpp"
#include "utils/Logger.hpp"

enum class Status {
	recive,
	send,
	wait,
	close
};

class Client {
private:
	int _serverFd;
	int _clientFd;
	Status _status;
	std::string _msg;
	const cfg::srvCtxCfg& _srvCfg;

	/* Member Functions */
	uint32_t recvData();
	uint32_t sendData();

public:
	/* Constructors & Destructors */
	Client() = delete;
	Client(int serverFd, int clientFd, const cfg::srvCtxCfg& srvCfg);
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
};

#endif // CLIENT_HPP