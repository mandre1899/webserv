#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <netinet/in.h>

typedef unsigned short http_code;

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
	http_code code; // todo: overflow protection
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
	std::optional<returnConf> returnConf;
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

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& tokens);
std::ostream& operator<<(std::ostream& os, const struct httpConf& httpConf);


#endif // WEBSERV_HPP