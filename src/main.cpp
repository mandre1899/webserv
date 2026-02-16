/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:59:57 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/16 17:01:30 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include "webserv.hpp"
#include <TcpSocket.hpp>

int main(void)
{
	TcpSocket s(INADDR_ANY, 8080);

	s.sBind();
	s.sListen(5);
	int clientFd = s.sAccept();
	char buffer[1024] = {0};
	s.sRecv(clientFd, buffer, sizeof(buffer));
	const char* message = "HTTP/1.1 200 OK\r\n\r\nHello!";
	s.sSend(clientFd, message, std::strlen(message), 0);
	std::cout << "Message from client: " << buffer << std::endl;
	return (0);
}