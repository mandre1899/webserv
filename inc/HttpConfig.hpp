#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP

#include <string>
#include <netinet/in.h>
#include <unordered_map>
#include <vector>
#include "webserv.hpp"
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include "Logger.hpp"
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"
#include "extrectValue.hpp"

class HttpConfig
{
private:
	httpConf _httpConf;
	std::vector<serverConf> _serverConfs;
	std::unordered_map<std::string, void (HttpConfig::*)(contextIdent)> _dispatch; //todo

	/* Member Functions */
	void setHttpConfDefaults();
	void setServerConfDefaults(serverConf& servConf);
	void setLocationConfDefaults(locationConf& locConf, serverConf& servConf);
	void buildLocation(const context& ctx, serverConf& serverConf);
	void buildServer(const context& ctx, serverConf& serverConf);
	void buildHttp(const context& ctx);

public:
	/* Constructors & Destructors */
	HttpConfig() = delete;
	HttpConfig(const char* path);
	HttpConfig(const HttpConfig& src) = delete;
	HttpConfig(HttpConfig&& src) = delete;
	~HttpConfig();

	/* Member Functions */

	/* Basic Operators */
	HttpConfig& operator=(const HttpConfig& src) = delete;
	HttpConfig& operator=(HttpConfig&& src) = delete;

	/* Getters & Setters */
	size_t getServerConfs(); //todo

	/* Member Class */
	class SyntaxError : public std::runtime_error
	{
	public:
		SyntaxError(const std::string& message);
	};
};

#endif // HTTPCONFIG_HPP