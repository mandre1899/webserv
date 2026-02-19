/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:43:34 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/19 16:43:38 by mausperg         ###   ########.fr       */
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
void HttpServer::createSockets()
{
	size_t socketCout = 2;
	int socketPort[2] = { 8080, 8081 };

	for (size_t i = 0; i < socketCout; i++)
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
	(void)ret;
	for (size_t i = 0; i < _pfd.size(); i++)
	{
		if (_pfd[i].revents & POLLIN)
		{
			if (_pfd[i].fd == _socket[i].getSocketFd())
			{
				Client c(_pfd[i].fd, _socket[i].acceptClient());
				_clientMap.insert({ c.getFd(), c }); // implment getFD()

			}
			else
			{
				_clientMap[_pfd[i].fd].read();
			}
		}
	}
}

/* Basic Operators */

/* Getters & Setters */
