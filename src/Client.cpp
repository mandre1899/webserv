/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:59 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/23 11:58:29 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/* Constructors & Destructors */
Client::Client(int serverFd, int clientFd) : _serverFd(serverFd), _clientFd(clientFd)
{
	(void)_serverFd;
	(void)_clientFd;
	_status = RECIVE;
	_msg = "";
}

Client::Client(Client&& src) noexcept
{
	_serverFd = src._serverFd;
	_clientFd = src._clientFd;
	_status = src._status;
	_msg = src._msg;
	src._serverFd = -1;
	src._clientFd = -1;
	src._msg = "";
	src._status = CLOSE;
}

Client::~Client()
{
}

/* Member Functions */
short Client::recvData()
{
	char buffer[20] = {};
	int ret;

	ret = recv(_clientFd, buffer, sizeof(buffer) - 1, 0);
	if (ret == -1)
		return (POLLHUP); // evt. throw exception
	_msg.append(buffer, ret);
	std::string end = _msg.substr((_msg.length() - 4));
	if (end == "\r\n\r\n")
	{
		std::cout << "Message from client: " << _msg << std::endl;
		sendData();
	}
	if (ret == 0)
		return (POLLHUP);
	return (POLLIN);
}

short Client::sendData()
{
	const char* message = "HTTP/1.1 200 OK\r\nContent-Length: 42\r\n\r\n<html><h1>Hello from WebServ!</h1></html>";
	int ret = send(_clientFd, message, 81, 0);
	std::cout << "Send ret = " << ret << std::endl;
	return (POLLHUP);
}

short Client::handleConnection(short revents)
{
	if (revents & POLLIN)
		return (recvData());
	if (revents & POLLPRI)
		throw "Unexpectet call";
	if (revents & POLLOUT)
		return (sendData());
	if (revents & POLLRDHUP)
		throw "Unexpectet call";
	if (revents & POLLERR)
		throw "Unexpectet call";
	if (revents & POLLHUP)
		return (POLLHUP);
	if (revents & POLLNVAL)
		throw "Unexpectet call";
	return (POLLHUP);
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}
