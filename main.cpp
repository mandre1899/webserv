#include "utils/webserv.hpp"
#include "server/TcpSocket.hpp"
#include "server/HttpServer.hpp"
#include <cstring>
#include <iostream>
#include "utils/Logger.hpp"
#include "config/Config.hpp"

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
	try
	{
		Config config(argv[1]);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}


	// HttpServer server;

	// server.createSockets();
	// while (true)
	// 	server.run();
	// return (EXIT_SUCCESS);
}