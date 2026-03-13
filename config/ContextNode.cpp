#include "ContextNode.hpp"

/* Constructors & Destructors */
ContextNode::ContextNode(TokenStream& tokens)
{
	try
	{
		createCtx(tokens);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (const char* msg)
	{
		std::cout << "Caught error: " << msg << std::endl;
	}
	Logger::log<LogLevel::trace>("ContextNode::ContextNode\n", _rootNode);
}

ContextNode::ContextNode(const ContextNode& src)
{
	_rootNode.conIdent = src._rootNode.conIdent;
	_rootNode.param = src._rootNode.param;
	_rootNode.child = src._rootNode.child;
}

ContextNode::ContextNode(ContextNode&& src)
{
	_rootNode.conIdent = std::move(src._rootNode.conIdent);
	_rootNode.param = std::move(src._rootNode.param);
	_rootNode.child = std::move(src._rootNode.child);
}

ContextNode::~ContextNode()
{
}

/* Member Functions */
void ContextNode::fillCtx(TokenStream& tokens, ctxNode& ctx)
{
	token token = tokens.getNextToken();

	while (token.type != tType::end)
	{
		if (token.type == tType::value)
		{
			if (token.value == "server")
			{
				ctxNode& next = ctx.child.emplace_back();
				next.conIdent = ctxNodeIdent::server;
				tokens.expectToken(tType::lBrace);
				fillCtx(tokens, next);
			}
			else if (token.value == "location")
			{
				ctxNode& next = ctx.child.emplace_back();
				next.conIdent = ctxNodeIdent::location;
				next.param.push_back(tokens.getNextToken());
				tokens.expectToken(tType::lBrace);
				fillCtx(tokens, next);
			}
			else
			{
				ctx.param.push_back(token);
			}
		}
		if (token.type == tType::semi)
		{
			ctx.param.push_back(token);
		}
		if (token.type == tType::rBrace)
		{
			return;
		}
		token = tokens.getNextToken();
	}
	throw "unexpectet end of file expectet }"; // todo
}

void ContextNode::createCtx(TokenStream& tokens)
{
	token token;
	token = tokens.getNextToken();

	if (token.value != "http")
	{
		throw "expectet http"; // todo
	}
	_rootNode.conIdent = ctxNodeIdent::http;
	tokens.expectToken(tType::lBrace);
	fillCtx(tokens, _rootNode);
}

/* Basic Operators */
ContextNode& ContextNode::operator=(const ContextNode& src)
{
	if (this != &src)
	{
		_rootNode.conIdent = src._rootNode.conIdent;
		_rootNode.param = src._rootNode.param;
		_rootNode.child = src._rootNode.child;
	}
	return *this;
}

ContextNode& ContextNode::operator=(ContextNode&& src)
{
	if (this != &src)
	{
		_rootNode.conIdent = std::move(src._rootNode.conIdent);
		_rootNode.param = std::move(src._rootNode.param);
		_rootNode.child = std::move(src._rootNode.child);
	}
	return *this;
}

/* Getters & Setters */

std::ostream& operator<<(std::ostream& os, const ctxNodeIdent& ident)
{
	if (ident == ctxNodeIdent::http)
		os << "http";
	else if (ident == ctxNodeIdent::server)
		os << "server";
	else if (ident == ctxNodeIdent::location)
		os << "location";
	return (os);
}

std::ostream& operator<<(std::ostream& os, const ctxNode& ctx)
{
	os << "---http ctxNode---\n";
	os << ctx.conIdent << ":" << std::endl;
	os << "    ";
	for (size_t i = 0; i < ctx.param.getRawTokens().size(); i++)
	{
		os << "\"" << ctx.param.getRawTokens()[i].value << "\", ";
	}
	os << std::endl;
	for (size_t j = 0; j < ctx.child.size(); j++)
	{
		ctxNode child = ctx.child[j];
		os << "    " << child.conIdent << ":" << std::endl;
		os << "        ";
		for (size_t b = 0; b < child.param.getRawTokens().size(); b++)
		{
			os << "\"" << child.param.getRawTokens()[b].value << "\", ";
		}
		os << std::endl;

		for (size_t c = 0; c < child.child.size(); c++)
		{
			ctxNode subChild = child.child[c];
			os << "        " << subChild.conIdent << ":" << std::endl;
			os << "            ";
			for (size_t d = 0; d < subChild.param.getRawTokens().size(); d++)
			{
				os << "\"" << subChild.param.getRawTokens()[d].value << "\", ";
			}
			os << std::endl;
		}
	}
	return os;
}
