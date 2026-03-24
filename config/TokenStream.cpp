#include "config/TokenStream.hpp"

/* Constructors & Destructors */
TokenStream::TokenStream()
{
	tEnd.type = tType::end;
	tEnd.value = {};
	tEnd.pos.col = 0;
	tEnd.pos.ln = 0;
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
	if (_tokenPos >= _tokens.size())
	{
		tEnd.pos.ln = _tokens[_tokenPos - 1].pos.ln;
		return tEnd;
	}
	const token& tmp = _tokens[_tokenPos];
	_tokenPos++;
	return(tmp);
}

const std::string& TokenStream::getNextValue()
{
	const token& tmp = _tokens[_tokenPos];
	if (_tokenPos < (_tokens.size() - 1))
		_tokenPos++;
	else
		throw TokenStreamEndException();
	return(tmp.value);
}

void TokenStream::expectToken(tType type)
{
	if (this->getNextToken().type != type)
	{
		throw UnexpectedTokenException();
	}
}

void TokenStream::expectToken(const std::string value)
{
	if (this->getNextToken().value != value)
	{
		throw UnexpectedTokenException();
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
		_tokens = src._tokens;
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
	if (_tokenPos >= _tokens.size())
	{
		return (_tokens[_tokenPos - 1].pos);
	}
	return(_tokens[_tokenPos].pos);
}

const std::vector<token>& TokenStream::getRawTokens() const
{
	return (_tokens);
}

/* Member Class */
const char* TokenStream::UnexpectedTokenException::what() const noexcept
{
	return ("Unexpected token.");
}

const char* TokenStream::TokenStreamEndException::what() const noexcept
{
	return ("The token stream is at the end and cannot return a value.");
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