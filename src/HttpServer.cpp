/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:43:34 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/23 11:51:01 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"

/* Constructors & Destructors */
HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
}

/* Member Functions */
void HttpServer::addClient(int serverFd, int clientFd)
{
	struct pollfd pollFd;

	_clientMap.insert({ clientFd, Client(serverFd, clientFd) });
	pollFd.fd = clientFd;
	pollFd.events = POLLIN;
	_pfd.push_back(pollFd);
}

void HttpServer::removeClient(int clientFd)
{
	_clientMap.erase(clientFd);
	for (size_t i = 0; i < _pfd.size(); i++)
	{
		if (_pfd[i].fd == clientFd)
		{
			_pfd.erase(_pfd.begin() + i);
		}
	}
}

void HttpServer::createSockets()
{
	size_t socketCount = 2;
	int socketPort[2] = { 8085, 8081 };

	for (size_t i = 0; i < socketCount; i++)
	{
		struct pollfd pollFd;

		_socket.push_back(TcpSocket(INADDR_ANY, socketPort[i]));
		pollFd.fd = _socket[i].getSocketFd();
		pollFd.events = POLLIN;
		_pfd.push_back(pollFd);
	}
}

void HttpServer::run()
{
	int ret = poll(_pfd.data(), _pfd.size(), -1);
	(void)ret; // TODO: optimise speed
	for (size_t i = 0; i < _pfd.size(); i++)
	{
		if (_pfd[i].revents & POLLIN && i < _socket.size() && _pfd[i].fd == _socket[i].getSocketFd()) // if (serverSocket)...
		{
			addClient(_pfd[i].fd, _socket[i].acceptClient());
		}
		else // if (clientSocket)...
		{
			Client& c = (_clientMap.find(_pfd[i].fd))->second;
			_pfd[i].events = c.handleConnection(_pfd[i].revents);
			if (_pfd[i].events & POLLHUP)
				removeClient(_pfd[i].fd);
		}
	}
}

/* Basic Operators */

/* Getters & Setters */
