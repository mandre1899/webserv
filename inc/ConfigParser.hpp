#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <netinet/in.h>
#include <map>
#include <vector>
#include "webserv.hpp"
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include "Logger.hpp"

#define KILO 1000
#define MEGA 1000000
#define GIGA 100000000


class ConfigParser {
private:
	httpConf _httpConf;
	std::vector<serverConfig> _serverConfs;
	/* Member Functions */

	bool isDelimiter(char c) const;
	void removeComment(std::string& line);
	void splitLineInTokens(std::vector<std::string>& tokens, std::string& line);
	std::vector<std::string> extractTokens(const char* path);

	bool isAutoIndex(const std::string& token) const;
	size_t siUnitToValue(const std::string& value, bool isTime) const;
	void extractHttpConf(std::vector<std::string>& tokens);


public:
	/* Constructors & Destructors */
	ConfigParser() = delete;
	ConfigParser(const char* path);
	ConfigParser(const ConfigParser& src) = delete;
	ConfigParser(ConfigParser&& src) = delete;
	~ConfigParser();

	/* Member Functions */

	/* Basic Operators */
	ConfigParser& operator=(const ConfigParser& src) = delete;
	ConfigParser& operator=(ConfigParser&& src) = delete;

	/* Getters & Setters */
	size_t getServerConfs();

	/* Member Class */
	class SyntaxError : public std::runtime_error
	{
	public:
		SyntaxError(const std::string& message);
	};

};

#endif // CONFIGPARSER_HPP