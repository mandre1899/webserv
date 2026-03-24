#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include "TokenStream.hpp"
#include "utils/Logger.hpp"

class Lexer {
private:
	/* Member Functions */
	static bool isDelimiter(char c);
	static void removeComment(std::string& line);
	static void splitLineInTokens(TokenStream& ts, filePos& pos, std::string& line);

public:
	/* Constructors & Destructors */
	Lexer() = delete;
	Lexer(const Lexer& src) = delete;
	Lexer(Lexer&& src) = delete;
	~Lexer() = delete;

	/* Member Functions */
	static TokenStream extractTokens(const char* path);

	/* Basic Operators */
	Lexer& operator=(const Lexer& src) = delete;
	Lexer& operator=(Lexer&& src) = delete;

	/* Getters & Setters */

	/* Member Class */
	class FileReadException : public std::runtime_error
	{
	public:
		FileReadException();
	};

};

#endif // LEXER_HPP