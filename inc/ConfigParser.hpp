#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <netinet/in.h>
#include <map>
#include <vector>
#include "webserv.hpp"

struct httpConf
{
	int clientBodyBufferSize; // todo: overflow protection
	int clientMaxBodySize;	// todo: oveerflow protection
	bool autoIndex;
	std::string index;
	std::string errorPage;
};

struct webSocket
{
	in_addr_t address;
	uint16_t port;
};

struct webRetrun
{
	http_code code;
	std::string param;
};

struct webLimitExcept
{
	bool isGet;
	bool isPost;
	bool isDelete;
};

struct webFastcgiParam
{
	std::string option;
	std::string param;
};

struct locationConf
{
	bool autoIndex;
	std::string index;
	webLimitExcept limitExcept;
	webRetrun webReturn;
	int clientBodyBufferSize; // todo: overflow protection
	int clientMaxBodySize;	// todo: oveerflow protection
	std::string uploadStore;
	std::string errorPage;
	bool includeFastcgiParams;
	std::string fastcgiPass;
	webFastcgiParam fastcgiParam;
	std::string fastcgiIndex;
};

struct serverConfig {
	webSocket listen;
	std::string serverName;
	bool autoIndex;
	std::string root;
	std::string index;
	int clientBodyBufferSize; // todo: overflow protection
	int clientMaxBodySize;	// todo: oveerflow protection
	webRetrun webReturn;
	std::string uploadStore;
	std::string errorPage;
	std::map<std::string, locationConf> location;
	httpConf& httpConf;
};

class ConfigParser {
private:
	httpConf httpConf;
	std::vector<serverConfig> serverConfs;
	/* Member Functions */

public:
	/* Constructors & Destructors */
	ConfigParser() = delete;
	ConfigParser(const char& path);
	ConfigParser(const ConfigParser& src) = delete;
	ConfigParser(ConfigParser&& src) = delete;
	~ConfigParser();

	/* Member Functions */

	/* Basic Operators */
	ConfigParser& operator=(const ConfigParser& src) = delete;
	ConfigParser& operator=(ConfigParser&& src) = delete;

	/* Getters & Setters */
	size_t getServerVector();

};

#endif // CONFIGPARSER_HPP