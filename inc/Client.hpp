/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:57 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/23 17:24:39 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h> 
#include <iostream>
#include <sys/poll.h>
#include "HttpParser.hpp"

enum e_status {
	RECIVE,
	SEND,
	WAIT,
	CLOSE
};

enum parsing_state {
	REQUEST_LINE = 0,
	HEADERS,
	BODY,
	DONE,
	ERROR
};

class Client {
private:
	int _serverFd;
	int _clientFd;
	e_status _status;
	std::string _msg;
	HttpParser _parsedHttp;
	parsing_state _parsingState;

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