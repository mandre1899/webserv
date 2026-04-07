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
#include <set>
#include <bits/stdc++.h>
#include "utils/Logger.hpp"
#include "config/Lexer.hpp"
#include "config/ContextNode.hpp"
#include "config/configTypes.hpp"
#include "config/ValueExtractor.hpp"

class Config
{
private:
	cfg::httpCtx _httpConf;
	std::vector<cfg::vHost> _hostConfs;
	std::unordered_map<std::string, cfg::srvCfg> _srvConfs; // key: ip address + port | value: cfg::srvCfg
	std::unordered_map<uint16_t, cfg::socketCfg> _socketCfg; // key: port | value: cfg::socketCfg


	/* Member Functions */
	void setHttpConfDefaults();
	void setServerConfDefaults(cfg::vHost& servConf);
	void setLocationConfDefaults(cfg::locationCtx& locConf, cfg::vHost& servConf);
	void buildLocation(const ctxNode& ctx, cfg::vHost& serverCtx);
	void buildServer(const ctxNode& ctx, cfg::vHost& serverCtx);
	void buildHttp(const ctxNode& ctx);
	void buildServerConf(const char* path);

public:
	/* Constructors & Destructors */
	Config() = delete;
	Config(const char* path);
	Config(const Config& src) = delete;
	Config(Config&& src) = delete;
	~Config();

	/* Member Functions */
	const cfg::vHost& vHostAt(size_t idx) const;
	size_t vHostSize() const;
	const cfg::srvCfg& srvAt(size_t idx) const;
	size_t srvSize() const;
	const cfg::socketCfg& socAt(size_t idx) const;
	size_t socSize() const;

	/* Basic Operators */
	Config& operator=(const Config& src) = delete;
	Config& operator=(Config&& src) = delete;

	/* Getters & Setters */

	/* Member Class */
	class SyntaxErrorException : public std::runtime_error
	{
	public:
		SyntaxErrorException(const std::string& message);
	};
};

std::ostream& operator<<(std::ostream& os, const Config& src);

#endif // CONFIG_HPP