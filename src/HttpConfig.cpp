#include "HttpConfig.hpp"

/* Constructors & Destructors */
HttpConfig::HttpConfig(const char* path)
{
	try
	{
		ConfigLexer lexer(path);
		ConfigParser parser(lexer);
		setHttpConfDefaults();
		buildHttp(parser.getHttpCtx());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (const char* msg)
	{
		std::cout << "Caught error: " << msg << std::endl;
	}

	for (size_t i = 0; i < _serverConfs.size(); i++)
	{
		std::cout << _serverConfs[i].root << std::endl;
		std::cout << _serverConfs[i].index << std::endl;
		for (size_t j = 0; j < _serverConfs[i].location.size(); j++)
		{
			std::cout << _serverConfs[i].location["/images/"].root << std::endl;
		}
	}

}

HttpConfig::~HttpConfig()
{
}

/* Member Functions */
void HttpConfig::setHttpConfDefaults()
{
	_httpConf.root = "";
	_httpConf.index = "";
	_httpConf.autoIndex = false;
	_httpConf.clientHeaderBufferSize = 1000; // 1k
	_httpConf.clientHeaderTimeout = 30;		 // 30s
	_httpConf.clientMaxBodySize = 2000000;	 // 2M
	_httpConf.clientBodyTimeout = 30;		 // 30s
}

void HttpConfig::setServerConfDefaults(serverConf& servConf)
{
	servConf.root = _httpConf.root;
	servConf.index = _httpConf.index;
	servConf.autoIndex = _httpConf.autoIndex;
	servConf.clientHeaderBufferSize = _httpConf.clientHeaderBufferSize;
	servConf.clientHeaderTimeout = _httpConf.clientHeaderTimeout;
	servConf.clientMaxBodySize = _httpConf.clientMaxBodySize;
	servConf.clientBodyTimeout = _httpConf.clientBodyTimeout;
}

void HttpConfig::setLocationConfDefaults(locationConf& locConf, serverConf& servConf)
{
	locConf.root = servConf.root;
	locConf.index = servConf.index;
	locConf.autoIndex = servConf.autoIndex;
	locConf.clientHeaderBufferSize = servConf.clientHeaderBufferSize;
	locConf.clientHeaderTimeout = servConf.clientHeaderTimeout;
	locConf.clientMaxBodySize = servConf.clientMaxBodySize;
	locConf.clientBodyTimeout = servConf.clientBodyTimeout;
}

void HttpConfig::buildLocation(const context& ctx, serverConf& serverConf)
{
	std::string path;
	locationConf locConf;
	ConfigLexer tmp(ctx.param);
	token t;

	setLocationConfDefaults(locConf, serverConf);
	path = tmp.getNextToken().value;
	t = tmp.getNextToken();
	while (t.type != tokenType::end)
	{
		extractSharedConfig<locationConf>(locConf, ctx.conIdent, t, tmp);
		tmp.expectToken(tokenType::semi);
		t = tmp.getNextToken();
	}
	serverConf.location.insert({ path, locConf });
}

void HttpConfig::buildServer(const context& ctx, serverConf& servConf)
{
	size_t childPos = 0;

	ConfigLexer tmp(ctx.param);
	token t;
	t = tmp.getNextToken();
	while (t.type != tokenType::end)
	{
		extractSharedConfig<serverConf>(servConf, ctx.conIdent, t, tmp);
		tmp.expectToken(tokenType::semi);
		t = tmp.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		buildLocation(ctx.child[childPos], servConf);
		childPos++;
	}
}

void HttpConfig::buildHttp(const context& ctx)
{
	size_t childPos = 0;

	ConfigLexer tmp(ctx.param);
	token t;
	t = tmp.getNextToken();
	while (t.type != tokenType::end)
	{
		extractSharedConfig<httpConf>(_httpConf, ctx.conIdent, t, tmp);
		tmp.expectToken(tokenType::semi);
		t = tmp.getNextToken();
	}
	while (childPos < ctx.child.size())
	{
		serverConf& servConf = _serverConfs.emplace_back();
		setServerConfDefaults(servConf);
		buildServer(ctx.child[childPos], servConf);
		childPos++;
	}
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
HttpConfig::SyntaxError::SyntaxError(const std::string& message) : std::runtime_error(message)
{
}
