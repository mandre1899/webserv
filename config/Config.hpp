#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <netinet/in.h>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include "utils/Logger.hpp"
#include "config/ConfigLexer.hpp"
#include "config/ConfigParser.hpp"
#include "config/extrectValue.hpp"
#include "config/configTypes.hpp"
#include "utils/webserv.hpp"

class Config
{
private:
	cfg::httpCtx _httpConf;
	std::vector<cfg::serverCtx> _wsConfs;
	std::unordered_map<std::string, void (Config::*)(contextIdent)> _dispatch; //todo

	/* Member Functions */
	void setHttpConfDefaults();
	void setServerConfDefaults(cfg::serverCtx& servConf);
	void setLocationConfDefaults(cfg::locationCtx& locConf, cfg::serverCtx& servConf);
	void buildLocation(const context& ctx, cfg::serverCtx& serverCtx);
	void buildServer(const context& ctx, cfg::serverCtx& serverCtx);
	void buildHttp(const context& ctx);

public:
	/* Constructors & Destructors */
	Config() = delete;
	Config(const char* path);
	Config(const Config& src) = delete;
	Config(Config&& src) = delete;
	~Config();

	/* Member Functions */

	/* Basic Operators */
	Config& operator=(const Config& src) = delete;
	Config& operator=(Config&& src) = delete;

	/* Getters & Setters */
	size_t getServerConfs(); //todo

	/* Member Class */
	class SyntaxError : public std::runtime_error
	{
	public:
		SyntaxError(const std::string& message);
	};
};

#endif // CONFIG_HPP