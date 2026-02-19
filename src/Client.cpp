/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:59 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/19 16:37:03 by mausperg         ###   ########.fr       */
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

Client::~Client()
{
}

/* Member Functions */
void Client::read()
{
	char buffer[20] = { 0 };

	int ret = recv(_clientFd, buffer, sizeof(buffer), 0);
	// if (ret == 0)
	// {
	// 	//close(pfds[i].fd);
	// 	// pfds[i].fd = 0;
	// 	// pfds[i].events = 0;
	// 	// pfds[i].revents = 0;
	// 	// fdCount--;
	// 	continue;
	// }
	std::cout << "ret from recv: " << ret << std::endl;
	std::cout << "Message from client: " << buffer << std::endl;

}

/* Basic Operators */

/* Getters & Setters */
