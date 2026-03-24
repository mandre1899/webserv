#include "Lexer.hpp"

/* Constructors & Destructors */

/* Member Functions */
bool Lexer::isDelimiter(char c)
{
	if (c == '{' || c == '}' || c == ';')
		return true;
	return false;
}

void Lexer::removeComment(std::string& line)
{
	size_t commentPos = line.find('#');
	if (commentPos != line.npos)
	{
		line.erase(commentPos);
	}
}

void Lexer::splitLineInTokens(TokenStream& ts, filePos& pos, std::string& line) // todo:: count pos.col
{
	std::stringstream ss(line);
	std::string tokenStr;

	while (ss >> tokenStr)
	{
		size_t lastSplit = 0;
		for (size_t i = 0; i < tokenStr.size(); i++)
		{
			if (isDelimiter(tokenStr[i]))
			{
				std::string tmp = tokenStr.substr(lastSplit, (i - lastSplit));
				if (tmp != "")
					ts.push_back(tmp, pos);
				ts.push_back(tokenStr.substr(i, 1), pos);
				lastSplit = i + 1;
			}
		}
		if (tokenStr == "")
			continue;
		if (lastSplit == 0)
			ts.push_back(tokenStr, pos);
		if (lastSplit != 0)
		{
			std::string tmp = tokenStr.substr(lastSplit);
			if (tmp != "")
				ts.push_back(tmp, pos);
		}
	}
}

TokenStream Lexer::extractTokens(const char* path)
{
	TokenStream ts;
	filePos pos = {};
	std::string line;
	std::ifstream configFile(path);

	if (configFile.fail())
		throw FileReadException();
	while (std::getline(configFile, line, '\n'))
	{
		pos.ln++;
		removeComment(line);
		splitLineInTokens(ts, pos, line);
	}
	if (!configFile.eof())
		throw FileReadException();
	configFile.close();
	Logger::log<LogLevel::trace>("Lexer::extractTokens()\n", ts);
	return (ts);
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
Lexer::FileReadException::FileReadException()
	: std::runtime_error("File Read Exception")
{
}