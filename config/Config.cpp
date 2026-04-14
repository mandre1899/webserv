#include "Config.hpp"

/* Constructors & Destructors */
Config::Config(const char* path)
{
	TokenStream tokens = Lexer::extractTokens(path);
	ContextNode node = ContextNode(tokens);
	setHttpConfDefaults();
	buildHttp(node.getRootNode());
	buildSrvCfg(path);
	buildSocketCfg();
	Logger::log<LogLevel::debug>("Config::Config()\n", *this);
}

Config::~Config()
{
}

/* Member Functions */
void Config::setHttpConfDefaults()
{
	_httpConf.root = "";
	_httpConf.index = "";
	_httpConf.autoIndex = false;
	_httpConf.clientHeaderBufferSize = 1000; // 1k
	_httpConf.clientHeaderTimeout = 30;		 // 30s
	_httpConf.clientMaxBodySize = 2000000;	 // 2M
	_httpConf.clientBodyTimeout = 30;		 // 30s
}

void Config::setServerConfDefaults(cfg::vHostCfg& servConf)
{
	servConf.root = _httpConf.root;
	servConf.index = _httpConf.index;
	servConf.autoIndex = _httpConf.autoIndex;
	servConf.clientHeaderBufferSize = _httpConf.clientHeaderBufferSize;
	servConf.clientHeaderTimeout = _httpConf.clientHeaderTimeout;
	servConf.clientMaxBodySize = _httpConf.clientMaxBodySize;
	servConf.clientBodyTimeout = _httpConf.clientBodyTimeout;
}

void Config::setLocationConfDefaults(cfg::wsLocationCtx& locConf, cfg::vHostCfg& servConf)
{
	locConf.root = servConf.root;
	locConf.index = servConf.index;
	locConf.autoIndex = servConf.autoIndex;
	locConf.clientHeaderBufferSize = servConf.clientHeaderBufferSize;
	locConf.clientHeaderTimeout = servConf.clientHeaderTimeout;
	locConf.clientMaxBodySize = servConf.clientMaxBodySize;
	locConf.clientBodyTimeout = servConf.clientBodyTimeout;
}

void Config::buildLocation(const ctxNode& ctx, cfg::vHostCfg& serverCtx)
{
	std::string path;
	filePos locationPos;
	cfg::wsLocationCtx locConf;
	TokenStream ts = ctx.param;
	token t;
	ValueExtractor<cfg::wsLocationCtx> ext;

	setLocationConfDefaults(locConf, serverCtx);
	path = ts.getNextToken().value;
	path = normalizePath(path, ts.getPositon());
	locationPos = ts.getPositon();
	t = ts.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, locConf, ts);
		t = ts.getNextToken();
	}
	auto it = serverCtx.location.insert({ path, locConf });
	if (!it.second)
		throw std::runtime_error(buildMsg("Duplicate location \"" + path + "\"", locationPos));;
}

void Config::build_vHost(const ctxNode& ctx, cfg::vHostCfg& servConf)
{
	size_t childPos = 0;
	TokenStream ts(ctx.param);
	token t;
	ValueExtractor<cfg::vHostCfg> ext;

	t = ts.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, servConf, ts);
		t = ts.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		buildLocation(ctx.child[childPos], servConf);
		childPos++;
	}
}

void Config::buildHttp(const ctxNode& ctx)
{
	size_t childPos = 0;
	TokenStream ts = ctx.param;
	token t;
	ValueExtractor<cfg::wsHttpCtx> ext;

	t = ts.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, _httpConf, ts);
		t = ts.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		cfg::vHostCfg& servConf = _hostConfs.emplace_back();
		setServerConfDefaults(servConf);
		build_vHost(ctx.child[childPos], servConf);
		childPos++;
	}
}

void Config::buildSrvCfg(const char* path)
{
	for (size_t i = 0; i < this->vHostSize(); i++)
	{
		cfg::vHostCfg host = this->vHostAt(i);
		std::string id = std::to_string(host.address) + std::to_string(host.port);
		auto it = _srvConfs.insert({ id, cfg::srvCfg() });
		cfg::srvCfg& server = it.first->second;
		if (!it.second && host.serverName.empty())
		{
			struct in_addr addr;
			addr.s_addr = host.address;
			Logger::log<LogLevel::warn>("Conflicting server name \"" + std::string(inet_ntoa(addr)) + ":" + std::to_string(host.port) + "\" " + std::string(path) + " is ignored");
			continue;
		}
		size_t pos = server.vHost.size();
		server.vHost.push_back(host);
		server.address = host.address;
		server.port = host.port;
		for (size_t j = 0; j < host.serverName.size(); j++)
		{
			bool isDubplicate = !server.srvNameMap.insert({ host.serverName[j], pos }).second;
			if (isDubplicate)
				throw std::runtime_error("Conflicting server name \"" + host.serverName[j] + "\" " + std::string(path));
		}
	}
}

void Config::buildSocketCfg()
{
	for (size_t i = 0; i < this->srvSize(); i++)
	{
		const cfg::srvCfg& srv = this->srvAt(i);
		auto it = _socketCfg.insert({ srv.port, cfg::socketCfg() });
		cfg::socketCfg& socket = it.first->second;
		socket.port = srv.port;
		std::string id = std::to_string(srv.address) + std::to_string(srv.port);
		socket.socketMap.insert({ id, srv });

	}
}

const cfg::vHostCfg& Config::vHostAt(size_t idx) const
{
	return (_hostConfs.at(idx));
}

size_t Config::vHostSize() const
{
	return (_hostConfs.size());
}

const cfg::srvCfg& Config::srvAt(size_t idx) const
{
	auto it = _srvConfs.begin();
	it = std::next(it, idx);
	return (it->second);
}

size_t Config::srvSize() const
{
	return (_srvConfs.size());
}

const cfg::socketCfg& Config::socAt(size_t idx) const
{
	auto it = _socketCfg.begin();
	it = std::next(it, idx);
	return (it->second);
}

size_t Config::socSize() const
{
	return (_socketCfg.size());
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
Config::SyntaxErrorException::SyntaxErrorException(const std::string& message) : std::runtime_error(message)
{
}

std::ostream& operator<<(std::ostream& os, const Config& src)
{
	for (size_t i = 0; i < src.vHostSize(); i++)
	{
		os << "-------------------server config-------------------\n";
		cfg::vHostCfg ctx = src.vHostAt(i);
		os << "listen: \n";
		struct in_addr addr;
		addr.s_addr = ctx.address;
		os << "   address: " << inet_ntoa(addr) << std::endl;
		os << "   port: " << ctx.port << std::endl;
		if (!ctx.serverName.empty())
		{
			os << "serverName:";
			for (size_t j = 0; j < ctx.serverName.size(); j++)
			{
				os << " " << ctx.serverName[j];
			}
			os << std::endl;
		}
		if (!ctx.root.empty())
			os << "root: " << ctx.root << std::endl;
		if (!ctx.index.empty())
			os << "index: " << ctx.index << std::endl;
		os << "autoIndex: " << std::boolalpha << ctx.autoIndex << std::endl;
		if (!ctx.errorPage.empty())
		{
			os << "errorPage: \n";
			for (const auto& entry : ctx.errorPage)
			{
				os << "   code: " << entry.first << std::endl;
				os << "   path: " << entry.second << std::endl;
			}
		}
		os << "clientHeaderBufferSize: " << ctx.clientHeaderBufferSize << std::endl;
		os << "clientHeaderTimeout: " << ctx.clientHeaderTimeout << std::endl;
		os << "clientMaxBodySize: " << ctx.clientMaxBodySize << std::endl;
		os << "clientBodyTimeout: " << ctx.clientBodyTimeout << std::endl;
		if (!ctx.cgiPath.empty())
			os << "cgiPath: " << ctx.cgiPath << std::endl;
		if (!ctx.cgiIndex.empty())
			os << "cgiIndex: " << ctx.cgiIndex << std::endl;
		if (ctx.ret.has_value())
		{
			os << "return: \n";
			os << "   code: " << ctx.ret->code << std::endl;
			if (!ctx.ret->param.empty())
				os << "         param: " << ctx.ret->param << std::endl;
		}
		if (!ctx.location.empty())
		{
			os << "location: ";
			for (std::unordered_map<std::string, cfg::wsLocationCtx>::const_iterator it = ctx.location.begin(); it != ctx.location.end(); ++it)
			{
				os << "\n   path: " << it->first << std::endl;
				const cfg::wsLocationCtx& loc = it->second;

				if (!loc.root.empty())
					os << "      root: " << loc.root << std::endl;
				if (!loc.index.empty())
					os << "      index: " << loc.index << std::endl;
				os << "      autoIndex: " << loc.autoIndex << std::endl;
				if (!loc.errorPage.empty())
				{
					os << "      errorPage: \n";
					for (const auto& entry : loc.errorPage)
					{
						os << "   code: " << entry.first << std::endl;
						os << "   path: " << entry.second << std::endl;
					}
				}
				if (loc.ret.has_value())
				{
					os << "      return: \n";
					os << "         code: " << loc.ret->code << std::endl;
					if (!loc.ret->param.empty())
						os << "         param: " << loc.ret->param << std::endl;
				}
				if (!loc.cgiPath.empty())
					os << "      cgiPath: " << loc.cgiPath << std::endl;
				if (!loc.cgiIndex.empty())
					os << "      cgiIndex: " << loc.cgiIndex << std::endl;
				if (!loc.uploadStore.empty())
					os << "      uploadStore: " << loc.uploadStore << std::endl;
				os << "      allowMethods: \n";
				os << "         GET: " << loc.allowMethods.isGet;
				os << " | POST: " << loc.allowMethods.isPost;
				os << " | DELETE: " << loc.allowMethods.isDelete;
			}
		}
	}
	return(os);
}
