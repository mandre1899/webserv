#ifndef CONFIGLEXER_HPP
#define CONFIGLEXER_HPP

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include "TokenStream.hpp"
#include "utils/Logger.hpp"

class ConfigLexer {
private:
	/* Member Functions */
	static bool isDelimiter(char c);
	static void removeComment(std::string& line);
	static void splitLineInTokens(TokenStream& ts, filePos& pos, std::string& line);

public:
	/* Constructors & Destructors */
	ConfigLexer() = delete;
	ConfigLexer(const ConfigLexer& src) = delete;
	ConfigLexer(ConfigLexer&& src) = delete;
	~ConfigLexer() = delete;

	/* Member Functions */
	static TokenStream extractTokens(const char* path);

	/* Basic Operators */
	ConfigLexer& operator=(const ConfigLexer& src) = delete;
	ConfigLexer& operator=(ConfigLexer&& src) = delete;

	/* Getters & Setters */
};

#endif // CONFIGLEXER_HPP