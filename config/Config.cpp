#include "Config.hpp"

/* Constructors & Destructors */
Config::Config(const char* path)
{
	try
	{
		TokenStream tokens = Lexer::extractTokens(path);
		ContextNode node = ContextNode(tokens);
		setHttpConfDefaults();
		buildHttp(node.getRootNode());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (const char* msg)
	{
		std::cout << "Caught error: " << msg << std::endl;
		throw SyntaxErrorException(msg);
	}
	for (size_t i = 0; i < _wsConfs.size(); i++)
	{
		std::cout << _wsConfs[i].root << std::endl;
		std::cout << _wsConfs[i].index << std::endl;
		for (size_t j = 0; j < _wsConfs[i].location.size(); j++)
		{
			std::cout << _wsConfs[i].location["/images/"].root << std::endl;
		}
	}
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
	cfg::locationCtx locConf;
	TokenStream tmp = ctx.param;
	token t;
	ValueExtractor<cfg::locationCtx> ext;

	setLocationConfDefaults(locConf, serverCtx);
	path = tmp.getNextToken().value;
	t = tmp.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, locConf, ctxNodeIdent::location, tmp);
		tmp.expectToken(tType::semi);
		t = tmp.getNextToken();
	}
	serverCtx.location.insert({ path, locConf });
}

void Config::buildServer(const ctxNode& ctx, cfg::serverCtx& servConf)
{
	size_t childPos = 0;
	TokenStream tmp(ctx.param);
	token t;
	ValueExtractor<cfg::serverCtx> ext;

	t = tmp.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, servConf, ctxNodeIdent::server, tmp);
		tmp.expectToken(tType::semi);
		t = tmp.getNextToken();
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
	TokenStream tmp = ctx.param;
	token t;
	ValueExtractor<cfg::httpCtx> ext;

	t = tmp.getNextToken();
	while (t.type != tType::end)
	{
		ext.setValue(t.value, _httpConf, ctxNodeIdent::http, tmp);
		tmp.expectToken(tType::semi);
		t = tmp.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		cfg::serverCtx& servConf = _wsConfs.emplace_back();
		setServerConfDefaults(servConf);
		buildServer(ctx.child[childPos], servConf);
		childPos++;
	}
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
Config::SyntaxErrorException::SyntaxErrorException(const std::string& message) : std::runtime_error(message)
{
}
