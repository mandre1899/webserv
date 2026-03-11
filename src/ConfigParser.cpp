#include "ConfigParser.hpp"

/* Constructors & Destructors */
ConfigParser::ConfigParser(ConfigLexer &lexer)
{
	try
	{
		createCtx(lexer);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (const char *msg)
	{
		std::cout << "Caught error: " << msg << std::endl;
	}
	Logger::log<LogLevel::trace>("ConfigParser::ConfigParser()\n", _httpCtx);
}

ConfigParser::~ConfigParser()
{
}

/* Member Functions */
void ConfigParser::expectToken(ConfigLexer &lexer, tokenType type)
{
	token token = lexer.getNextToken();
	if (token.type != type)
	{
		throw "unexpectet token!";
	}
}

void ConfigParser::fillCtx(ConfigLexer &lexer, context &ctx)
{
	token token = lexer.getNextToken();

	while (token.type != tokenType::end)
	{
		if (token.type == tokenType::value)
		{
			if (token.value == "server")
			{
				context &next = ctx.child.emplace_back();
				next.conIdent = contextIdent::server;
				expectToken(lexer, tokenType::lBrace);
				fillCtx(lexer, next);
			}
			else if (token.value == "location")
			{
				context &next = ctx.child.emplace_back();
				next.conIdent = contextIdent::location;
				next.param.push_back(lexer.getNextToken());
				expectToken(lexer, tokenType::lBrace);
				fillCtx(lexer, next);
			}
			else
			{
				ctx.param.push_back(token);
			}
		}
		if (token.type == tokenType::rBrace)
		{
			return;
		}
		token = lexer.getNextToken();
	}
	throw "unexpectet end of file expectet }"; // todo
}

void ConfigParser::createCtx(ConfigLexer &lexer)
{
	token token;
	token = lexer.getNextToken();

	if (token.value != "http")
	{
		throw "expectet http"; // todo
	}
	_httpCtx.conIdent = contextIdent::http;
	expectToken(lexer, tokenType::lBrace);
	fillCtx(lexer, _httpCtx);
}

/* Basic Operators */

/* Getters & Setters */
const context &ConfigParser::getHttpCtx() const
{
	return (_httpCtx);
}

/* Member Class */

std::ostream &operator<<(std::ostream &os, const contextIdent &ident)
{
	if (ident == contextIdent::http)
		os << "http";
	else if (ident == contextIdent::server)
		os << "server";
	else if (ident == contextIdent::location)
		os << "location";
	return (os);
}

std::ostream &operator<<(std::ostream &os, const context &ctx)
{
	os << "---http context---\n";
	os << ctx.conIdent << ":" << std::endl;
	os << "    ";
	for (size_t i = 0; i < ctx.param.size(); i++)
	{
		os << "\"" << ctx.param[i].value << "\", ";
	}
	os << std::endl;
	for (size_t j = 0; j < ctx.child.size(); j++)
	{
		context child = ctx.child[j];
		os << "    " << child.conIdent << ":" << std::endl;
		os << "        ";
		for (size_t b = 0; b < child.param.size(); b++)
		{
			os << "\"" << child.param[b].value << "\", ";
		}
		os << std::endl;

		for (size_t c = 0; c < child.child.size(); c++)
		{
			context subChild = child.child[c];
			os << "        " << subChild.conIdent << ":" << std::endl;
			os << "            ";
			for (size_t d = 0; d < subChild.param.size(); d++)
			{
				os << "\"" << subChild.param[d].value << "\", ";
			}
			os << std::endl;
		}
	}
	return os;
}