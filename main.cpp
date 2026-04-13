#include "server/TcpSocket.hpp"
#include "server/HttpServer.hpp"
#include <cstring>
#include <iostream>
#include <csignal>
#include "utils/Logger.hpp"
#include "config/Config.hpp"

const char* g_execName = "./webserv";

volatile std::sig_atomic_t g_stopServerSignal = 0;

void sigactionHandler(int signalId)
{
	(void)signalId;
	g_stopServerSignal = 1;
}

void setupSignalHandler()
{
	std::signal(SIGINT, sigactionHandler);
	std::signal(SIGQUIT, sigactionHandler);
	std::signal(SIGTERM, sigactionHandler);
}

int	main(int argc, char** argv)
{
	setupSignalHandler();
	g_execName = argv[0];
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

		server.createSockets(config);
		while (!g_stopServerSignal)
			server.run();
	}
	catch (const std::exception& e)
	{
		Logger::log<LogLevel::error>(e.what());
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
