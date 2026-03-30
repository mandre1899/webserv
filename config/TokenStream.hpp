#ifndef TOKENSTREAM_HPP
#define TOKENSTREAM_HPP

#include <string>
#include <vector>
#include <iostream>
#include "configUtils.hpp"

enum class tType
{
	value,
	lBrace,
	rBrace,
	semi,
	undefined,
	end
};

struct token
{
	tType type;
	std::string value;
	filePos pos;
};

class TokenStream {

private:
	std::vector<token> _tokens;
	size_t _tokenPos = {};
	token	tEnd = {};

	/* Member Functions */
	tType getType(const std::string& strToken);

public:
	/* Constructors & Destructors */
	TokenStream();
	TokenStream(const TokenStream& src);
	TokenStream(TokenStream&& src);
	~TokenStream();

	/* Member Functions */
	void push_back(const std::string& strToken, filePos pos);
	void push_back(const token& token);
	const token& getNextToken();
	const std::string& getNextValue();
	void expectToken(tType type);
	void expectToken(const std::string value);
	void reset();

	/* Basic Operators */
	TokenStream& operator=(const TokenStream& src);
	TokenStream& operator=(TokenStream&& src);

	/* Getters & Setters */
	const filePos& getPositon() const; //todo
	const std::vector<token>& getRawTokens() const;

	/* Member Class */
	class UnexpectedTokenException : public std::runtime_error
	{
	public:
		UnexpectedTokenException(const filePos& pos);

	};
	class TokenStreamEndException : public std::runtime_error
	{
	public:
		TokenStreamEndException(const filePos& pos);

	};
};

std::ostream& operator<<(std::ostream& os, const TokenStream& ts);

#endif // TOKENSTREAM_HPP