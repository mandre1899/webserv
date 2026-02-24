#include <webserv.hpp>
#include <TcpSocket.hpp>
#include <HttpServer.hpp>
#include <cstring>
#include <iostream>
#include <Logger.hpp>

int	main(void)
{
	HttpServer server;

	server.createSockets();
	while (true)
		server.run();
	return (0);
}