/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:59 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/20 11:16:13 by mausperg         ###   ########.fr       */
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

Client::Client(Client&& src)
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
	char buffer[20] = { 0 };
	int ret;

	if (revents & POLLIN)
	{
		ret = recv(_clientFd, buffer, sizeof(buffer), 0);
		std::cout << "ret from recv: " << ret << std::endl;
		std::cout << "Message from client: " << buffer << std::endl;
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
		throw "Unexpectet call";
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
