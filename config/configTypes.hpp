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
	struct returnCfg
	{
		http_code code;
		std::string param;
	};

	struct allowMethodsCfg
	{
		bool isGet;
		bool isPost;
		bool isDelete;
	};

	struct errorPageCfg
	{
		http_code code;
		std::string path;
	};

	// corresponds to the http{} scope in the conf file
	struct httpCtx
	{
		std::string root;
		std::string index;
		bool autoIndex;
		std::optional<errorPageCfg>  errorPage;
		size_t clientHeaderBufferSize;
		size_t clientHeaderTimeout;
		size_t clientMaxBodySize;
		size_t clientBodyTimeout;
		std::string cgiPath;
		std::string cgiIndex;
	};

	struct sharedCfg : public httpCtx
	{
		std::optional<returnCfg> retConf;
	};

	// corresponds to the location{} scope in the conf file
	struct locationCtx : public sharedCfg
	{
		std::string uploadStore;
		allowMethodsCfg allowMethods;
	};

	// corresponds to the server{} scope in the conf file
	struct vHostCfg : public sharedCfg
	{
		in_addr_t address;
		uint16_t port = 80;
		std::vector<std::string> serverName;
		std::unordered_map<std::string, locationCtx> location;
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
		std::unordered_map<std::string, srvCfg&> socketMap; // key: ip address, value: srvCfg
	};
}

#endif // CONFIGTYPES_HPP