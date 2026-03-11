#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP

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
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"

class HttpConfig {
private:
	httpConf _httpConf;
	std::vector<serverConfig> _serverConfs;
	//std::map<std::string, void(*)()> _dispatch;

	/* Member Functions */
	void setHttpConfDefaults();
	bool isAutoIndex(const std::string& token) const;
	size_t siUnitToValue(const std::string& value, bool isTime) const;
	size_t extractSharedConfig(struct httpConf& httpConf, std::vector<std::string>& tokens, size_t pos);
	void extractHttpConf(std::vector<std::string>& tokens);

	//void fillDispatch();

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
	size_t getServerConfs();

	/* Member Class */
	class SyntaxError : public std::runtime_error
	{
	public:
		SyntaxError(const std::string& message);
	};
};

std::ostream& operator<<(std::ostream& os, const std::vector<serverConfig>& serverConfs);
std::ostream& operator<<(std::ostream& os, const ConfigParser& pars);

#endif // HTTPCONFIG_HPP