/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:59 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/20 16:49:09 by mausperg         ###   ########.fr       */
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
short Client::handleConnection(short revents)
{
	char buffer[20] = {};
	int ret;

	if (revents & POLLIN)
	{
		ret = recv(_clientFd, buffer, sizeof(buffer) - 1, 0);
		std::cout << "ret from recv: " << ret << std::endl;
		buffer[ret] = '\0';
		_msg.append(buffer);
		if (_msg.find_last_of("\r\n\r\n") != std::string::npos)
		{
			std::cout << "Message from client: " << _msg << std::endl;
			return (POLLOUT);
		}
		if (ret == 0)
			return (POLLHUP);
		return (POLLIN);
	}
	else if (revents & POLLPRI)
	{
		throw "Unexpectet call";
	}
	else if (revents & POLLOUT)
	{
		const char* message = "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nHello!";
		send(_clientFd, message, 46, 0);
		//throw "Unexpectet call";
	}
	else if (revents & POLLRDHUP)
	{
		throw "Unexpectet call";
	}
	else if (revents & POLLERR)
	{
		throw "Unexpectet call";
	}
	else if (revents & POLLHUP)
	{
		return (POLLHUP);
	}
	else if (revents & POLLNVAL)
	{
		throw "Unexpectet call";
	}
	return (POLLHUP);
}

/* Basic Operators */

/* Getters & Setters */
int Client::getFd() const
{
	return (_clientFd);
}
