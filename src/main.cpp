/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:59:57 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/19 15:16:16 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <TcpSocket.hpp>
#include <HttpServer.hpp>
#include <cstring>
#include <iostream>

int	main(void)
{
	HttpServer server;

	server.createSockets();
	while (true)
		server.run();
	return (0);
}