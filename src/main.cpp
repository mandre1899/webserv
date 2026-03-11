#include "webserv.hpp"
#include "TcpSocket.hpp"
#include "HttpServer.hpp"
#include <cstring>
#include <iostream>
#include "Logger.hpp"
#include "HttpConfig.hpp"

int	main(int argc, char** argv)
{
	if (argc == 1)
	{
		Logger::log<LogLevel::error>("No configuration file provided, not enough arguments.");
		return (EXIT_FAILURE);
	}
	if (argc > 2)
	{
		Logger::log<LogLevel::error>("There are too many arguments.");
		return (EXIT_FAILURE);
	}
	HttpConfig config(argv[1]);

	// HttpServer server;

	// server.createSockets();
	// while (true)
	// 	server.run();
	// return (EXIT_SUCCESS);
}