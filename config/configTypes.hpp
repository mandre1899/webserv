#ifndef CONFIGTYPES_HPP
#define CONFIGTYPES_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>
#include <netinet/in.h>

typedef unsigned short http_code;

namespace cfg
{
	struct wsReturn
	{
		http_code code;
		std::string param;
	};

	struct wsAllowMethods
	{
		bool isGet;
		bool isPost;
		bool isDelete;
	};

	struct wsErrorPage
	{
		http_code code;
		std::string path;
	};

	// corresponds to the http{} scope in the conf file
	struct wsHttpCtx
	{
		std::string root;
		std::string index;
		bool autoIndex;
		std::optional<wsErrorPage>  errorPage;
		size_t clientHeaderBufferSize;
		size_t clientHeaderTimeout;
		size_t clientMaxBodySize;
		size_t clientBodyTimeout;
		std::string cgiPath;
		std::string cgiIndex;
	};

	struct wsShared : public wsHttpCtx
	{
		std::optional<wsReturn> ret;
	};

	// corresponds to the location{} scope in the conf file
	struct wsLocationCtx : public wsShared
	{
		std::string uploadStore;
		wsAllowMethods allowMethods;
	};

	// corresponds to the server{} scope in the conf file
	struct vHostCfg : public wsShared
	{
		in_addr_t address;
		uint16_t port = 80;
		std::vector<std::string> serverName;
		std::unordered_map<std::string, wsLocationCtx> location;
	};

	struct srvCfg
	{
		in_addr_t address;
		uint16_t port;
		std::unordered_map<std::string, int> srvNameMap; // key: serverName, value: vHost index
		std::vector<vHostCfg> vHost;
	};

	struct socketCfg
	{
		uint16_t port;
		std::unordered_map<std::string, const srvCfg&> socketMap; // key: ip address + port, value: srvCfg
	};
}

#endif // CONFIGTYPES_HPP