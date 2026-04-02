#include "Config.hpp"

/* Constructors & Destructors */
Config::Config(const char* path)
{
	TokenStream tokens = Lexer::extractTokens(path);
	ContextNode node = ContextNode(tokens);
	setHttpConfDefaults();
	buildHttp(node.getRootNode());
	checkConfig(path);
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

void Config::setServerConfDefaults(cfg::serverCtx& servConf)
{
	servConf.root = _httpConf.root;
	servConf.index = _httpConf.index;
	servConf.autoIndex = _httpConf.autoIndex;
	servConf.clientHeaderBufferSize = _httpConf.clientHeaderBufferSize;
	servConf.clientHeaderTimeout = _httpConf.clientHeaderTimeout;
	servConf.clientMaxBodySize = _httpConf.clientMaxBodySize;
	servConf.clientBodyTimeout = _httpConf.clientBodyTimeout;
}

void Config::setLocationConfDefaults(cfg::locationCtx& locConf, cfg::serverCtx& servConf)
{
	locConf.root = servConf.root;
	locConf.index = servConf.index;
	locConf.autoIndex = servConf.autoIndex;
	locConf.clientHeaderBufferSize = servConf.clientHeaderBufferSize;
	locConf.clientHeaderTimeout = servConf.clientHeaderTimeout;
	locConf.clientMaxBodySize = servConf.clientMaxBodySize;
	locConf.clientBodyTimeout = servConf.clientBodyTimeout;
}

void Config::buildLocation(const ctxNode& ctx, cfg::serverCtx& serverCtx)
{
	std::string path;
	filePos locationPos;
	cfg::locationCtx locConf;
	TokenStream ts = ctx.param;
	token t;
	ValueExtractor<cfg::locationCtx> ext;

	setLocationConfDefaults(locConf, serverCtx);
	path = ts.getNextToken().value;
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

void Config::buildServer(const ctxNode& ctx, cfg::serverCtx& servConf)
{
	size_t childPos = 0;
	TokenStream ts(ctx.param);
	token t;
	ValueExtractor<cfg::serverCtx> ext;

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
	ValueExtractor<cfg::httpCtx> ext;

	t = ts.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, _httpConf, ts);
		t = ts.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		cfg::serverCtx& servConf = _wsConfs.emplace_back();
		setServerConfDefaults(servConf);
		buildServer(ctx.child[childPos], servConf);
		childPos++;
	}
}

void Config::checkConfig(const char* path)
{
	std::set<std::string> srvIdent;

	for (size_t i = 0; i < this->size(); i++)
	{
		cfg::serverCtx ctx = this->at(i);
		std::string id = std::to_string(ctx.listen.address) + std::to_string(ctx.listen.port) + ctx.serverName;
		auto it = srvIdent.insert(id);
		if (!it.second)
			throw std::runtime_error("Conflicting server name/address " + std::string(path));
	}
}

const cfg::serverCtx& Config::at(size_t idx) const
{
	return (_wsConfs.at(idx));
}

size_t Config::size() const
{
	return (_wsConfs.size());
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
Config::SyntaxErrorException::SyntaxErrorException(const std::string& message) : std::runtime_error(message)
{
}

std::ostream& operator<<(std::ostream& os, const Config& src)
{
	for (size_t i = 0; i < src.size(); i++)
	{
		os << "-------------------server config-------------------\n";
		cfg::serverCtx ctx = src.at(i);
		os << "listen: \n";
		struct in_addr addr;
		addr.s_addr = ctx.listen.address;
		os << "   address: " << inet_ntoa(addr) << std::endl;
		os << "   port: " << ctx.listen.port << std::endl;
		if (!ctx.serverName.empty())
			os << "serverName: " << ctx.serverName << std::endl;
		if (!ctx.root.empty())
			os << "root: " << ctx.root << std::endl;
		if (!ctx.index.empty())
			os << "index: " << ctx.index << std::endl;
		os << "autoIndex: " << std::boolalpha << ctx.autoIndex << std::endl;
		if (ctx.errorPage.has_value())
		{
			os << "errorPage: \n";
			os << "   code: " << ctx.errorPage->code << std::endl;
			os << "   path: " << ctx.errorPage->path << std::endl;
		}
		os << "clientHeaderBufferSize: " << ctx.clientHeaderBufferSize << std::endl;
		os << "clientHeaderTimeout: " << ctx.clientHeaderTimeout << std::endl;
		os << "clientMaxBodySize: " << ctx.clientMaxBodySize << std::endl;
		os << "clientBodyTimeout: " << ctx.clientBodyTimeout << std::endl;
		if (!ctx.cgiPath.empty())
			os << "cgiPath: " << ctx.cgiPath << std::endl;
		if (!ctx.cgiIndex.empty())
			os << "cgiIndex: " << ctx.cgiIndex << std::endl;
		if (ctx.retConf.has_value())
		{
			os << "return: \n";
			os << "   code: " << ctx.retConf->code << std::endl;
			if (!ctx.retConf->param.empty())
				os << "         param: " << ctx.retConf->param << std::endl;
		}
		if (!ctx.location.empty())
		{
			os << "location: ";
			for (std::unordered_map<std::string, cfg::locationCtx>::const_iterator it = ctx.location.begin(); it != ctx.location.end(); ++it)
			{
				os << "\n   path: " << it->first << std::endl;
				const cfg::locationCtx& loc = it->second;

				if (!loc.root.empty())
					os << "      root: " << loc.root << std::endl;
				if (!loc.index.empty())
					os << "      index: " << loc.index << std::endl;
				os << "      autoIndex: " << loc.autoIndex << std::endl;
				if (loc.errorPage.has_value())
				{
					os << "      errorPage: \n";
					os << "         code: " << loc.errorPage->code << std::endl;
					os << "         path: " << loc.errorPage->path << std::endl;
				}
				if (loc.retConf.has_value())
				{
					os << "      return: \n";
					os << "         code: " << loc.retConf->code << std::endl;
					if (!loc.retConf->param.empty())
						os << "         param: " << loc.retConf->param << std::endl;
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
