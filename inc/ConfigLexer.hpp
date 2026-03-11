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
	std::vector<std::string> _tokens;
	size_t _tokenCount = {};
	filePos _pos = {};

	/* Member Functions */
	bool isDelimiter(char c) const;
	void removeComment(std::string& line);
	void splitLineInTokens(std::vector<std::string>& tokens, std::string& line);
	void extractTokens(std::ifstream& configFile);

public:
	/* Constructors & Destructors */
	ConfigLexer() = delete;
	ConfigLexer(const char* path);
	ConfigLexer(const ConfigLexer& src) = delete;
	ConfigLexer(ConfigLexer&& src) = delete;
	~ConfigLexer();

	/* Member Functions */
	token getNextToken();

	/* Basic Operators */
	ConfigLexer& operator=(const ConfigLexer& src) = delete;
	ConfigLexer& operator=(ConfigLexer&& src) = delete;

	/* Getters & Setters */
	filePos getPos() const;

};

#endif // CONFIGLEXER_HPP