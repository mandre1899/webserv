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
		Logger::log<LogLevel::info>("Processing the configuration...");
		Config config(argv[1]);
		Logger::log<LogLevel::info>("Configuration complete; ready for start up");
		HttpServer server;

		server.createSockets();
		while (true)
			server.run();
	}
	catch (const std::exception& e)
	{
		Logger::log<LogLevel::error>(e.what());
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}