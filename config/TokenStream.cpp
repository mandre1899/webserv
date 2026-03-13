#include "config/TokenStream.hpp"

/* Constructors & Destructors */
TokenStream::TokenStream()
{
	tEnd.type = tType::end;
	tEnd.value = {};
	tEnd.pos = {};
}

TokenStream::TokenStream(const TokenStream& src) : TokenStream()
{
	_tokens = src._tokens;
	_tokenPos = src._tokenPos;
}

TokenStream::TokenStream(TokenStream&& src) : TokenStream()
{
	_tokens = std::move(src._tokens);
	_tokenPos = src._tokenPos;
	src._tokenPos = 0;
}

TokenStream::~TokenStream()
{
}

/* Member Functions */
tType TokenStream::getType(const std::string& strToken)
{
	if (strToken == "{")
	{
		return (tType::lBrace);
	}
	else if (strToken == "}")
	{
		return (tType::rBrace);
	}
	else if (strToken == ";")
	{
		return (tType::semi);
	}
	else
	{
		return (tType::value);
	}
}

void TokenStream::push_back(const std::string& strToken, filePos pos)
{
	_tokens.push_back({ getType(strToken), strToken, pos });
}

void TokenStream::push_back(const token& token)
{
	_tokens.push_back(token);
}

const token& TokenStream::getNextToken()
{
	const token& tmp = _tokens[_tokenPos];
	if (_tokenPos < (_tokens.size() - 1))
		_tokenPos++;
	else
		return(tEnd);
	return(tmp);
}

const std::string& TokenStream::getNextValue()
{
	const token& tmp = _tokens[_tokenPos];
	if (_tokenPos < (_tokens.size() - 1))
		_tokenPos++;
	else
		throw "Can not retrun Value, TokenStream end"; // todo
	return(tmp.value);
}

void TokenStream::expectToken(tType type)
{
	if (this->getNextToken().type != type)
	{
		throw "unexpectet token!";
	}
}

void TokenStream::reset()
{
	_tokenPos = 0;
}

/* Basic Operators */
TokenStream& TokenStream::operator=(const TokenStream& src)
{
	if (this != &src)
	{
		_tokens = _tokens;
		_tokenPos = src._tokenPos;
	}
	return *this;
}

TokenStream& TokenStream::operator=(TokenStream&& src)
{
	if (this != &src)
	{
		_tokens = std::move(src._tokens);
		_tokenPos = src._tokenPos;
		src._tokenPos = 0;
	}
	return *this;
}

/* Getters & Setters */
const filePos& TokenStream::getPositon() const
{
	return(_tokens[_tokenPos].pos);
}

const std::vector<token>& TokenStream::getRawTokens() const
{
	return (_tokens);
}

std::ostream& operator<<(std::ostream& os, const TokenStream& ts)
{
	const std::vector<token>& tokens = ts.getRawTokens();
	for (size_t i = 0; i < (tokens.size() - 1); i++)
	{
		os << "\"" << tokens[i].value << "\", ";
	}
	os << "\"" << tokens[tokens.size() - 1].value << "\"" << std::endl;
	return (os);
}