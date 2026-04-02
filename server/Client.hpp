#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h> 
#include <iostream>
#include <sys/poll.h>
#include <unistd.h>
#include "utils/Logger.hpp"
#include "HttpParser.hpp"

enum class Status {
	recive,
	send,
	wait,
	close
};

enum class state {
	parsing_request_line = 0,
	parsing_headers,
	parsing_body,
	parsing_done,
	error
};

class Client {
private:
	int _serverFd;
	int _clientFd;
	Status _status;
	std::string _msg;
	HttpParser _parsedHttp;
	state _parsingState;

	/* Member Functions */
	short recvData();
	short sendData();

public:
	/* Constructors & Destructors */
	Client() = delete;
	Client(int serverFd, int clientFd);
	Client(const Client& src) = delete;
	Client(Client&& src) noexcept;
	~Client();

	/* Member Functions */
	short handleConnection(short revents);

	/* Basic Operators */
	Client& operator=(const Client& src) = delete;
	Client& operator=(Client&& src) = delete;

	/* Getters & Setters */
	int getFd() const;
};

#endif // CLIENT_HPP