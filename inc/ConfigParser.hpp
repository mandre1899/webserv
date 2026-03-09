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

struct socketConf
{
	in_addr_t address;
	uint16_t port;
};

struct returnConf
{
	http_code code;
	std::string param;
};

struct allowMethodsConf
{
	bool isGet;
	bool isPost;
	bool isDelete;
};

struct errorPageConf
{
	http_code code;
	std::string path;
};

// corresponds to the http{} scope in the conf file
struct httpConf
{
	std::string root;
	std::string index;
	bool autoIndex;
	std::optional<errorPageConf>  errorPage;
	size_t clientHeaderBufferSize; // todo: overflow protection
	size_t clientHeaderTimeout;	// todo: oveerflow protection
	size_t clientMaxBodySize; // todo: overflow protection
	size_t clientBodyTimeout;	// todo: oveerflow protection
};

struct sharedConfig : public httpConf
{
	std::optional<returnConf> webReturn;
};

// corresponds to the location{} scope in the conf file
struct locationConf : public sharedConfig
{
	std::string uploadStore;
	std::string fastcgiPath;
	std::string fastcgiIndex;
	allowMethodsConf allowMethods;
};

// corresponds to the server{} scope in the conf file
struct serverConfig : public sharedConfig
{
	socketConf listen;
	std::string serverName;
	std::map<std::string, locationConf> location;
};

class ConfigParser {
private:
	httpConf httpConf;
	std::vector<serverConfig> serverConfs;
	/* Member Functions */
	bool isDelimiter(char c) const;
	void removeComment(std::string& line);
	void splitLineInTokens(std::vector<std::string>& tokens, std::string& line);
	std::vector<std::string> extractTokens(const char* path);


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

};

#endif // CONFIGPARSER_HPP