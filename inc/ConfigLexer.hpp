#ifndef CONFIGLEXER_HPP
#define CONFIGLEXER_HPP

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include "Logger.hpp"

enum class tokenType
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
	tokenType type;
	std::string value;
};

struct filePos
{
	size_t ln; // line
	size_t col; // column
};

class ConfigLexer {
private:
	std::vector<token> _tokens;
	size_t _tokenCount = {};
	filePos _pos = {};

	/* Member Functions */
	bool isDelimiter(char c) const;
	void removeComment(std::string& line);
	void splitLineInTokens(std::vector<std::string>& tokens, std::string& line);
	std::vector<std::string> extractTokens(std::ifstream& configFile);
	void setTokenTyp(std::vector<std::string>& tokens);

public:
	/* Constructors & Destructors */
	ConfigLexer() = delete;
	ConfigLexer(const char* path);
	ConfigLexer(const std::vector<token>& tokens);
	ConfigLexer(const ConfigLexer& src) = delete;
	ConfigLexer(ConfigLexer&& src) = delete;
	~ConfigLexer();

	/* Member Functions */
	const token& getNextToken();
	void expectToken(tokenType type);

	/* Basic Operators */
	ConfigLexer& operator=(const ConfigLexer& src) = delete;
	ConfigLexer& operator=(ConfigLexer&& src) = delete;

	/* Getters & Setters */
	filePos getPos() const; //todo
};

#endif // CONFIGLEXER_HPP