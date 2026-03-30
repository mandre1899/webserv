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
	size_t colPos = 0;

	while (ss >> tokenStr)
	{
		size_t lastSplit = 0;
		for (size_t i = 0; i < tokenStr.size(); i++)
		{
			if (isDelimiter(tokenStr[i]))
			{
				std::string tmp = tokenStr.substr(lastSplit, (i - lastSplit));
				if (tmp != "")
				{
					pos.col = colPos + lastSplit;
					std::cout << "1" << tmp << pos.col << std::endl;
					ts.push_back(tmp, pos);
				}
				pos.col = colPos + i;
				std::cout << "2" << tokenStr.substr(i, 1) << pos.col << std::endl;
				ts.push_back(tokenStr.substr(i, 1), pos);
				lastSplit = i + 1;
			}
		}
		if (tokenStr == "")
			continue;
		if (lastSplit == 0)
		{
			pos.col = colPos;
			std::cout << "3" << tokenStr << pos.col << std::endl;
			ts.push_back(tokenStr, pos);
		}
		if (lastSplit != 0)
		{
			std::string tmp = tokenStr.substr(lastSplit);
			if (tmp != "")
			{
				pos.col = colPos + lastSplit;
				std::cout << "4" << tmp << pos.col << std::endl;
				ts.push_back(tmp, pos);
			}
		}
		std::streamoff ssPos = ss.tellg();
		if (ssPos != -1)
			colPos = ssPos + 1;
	}
}

TokenStream Lexer::extractTokens(const char* path)
{
	TokenStream ts;
	filePos pos = {};
	std::string line;
	std::ifstream configFile(path);

	pos.path = path;
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
	if (pos.ln == 0)
		throw EmptyFileException();
	Logger::log<LogLevel::trace>("Lexer::extractTokens()\n", ts);
	return (ts);
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
Lexer::FileReadException::FileReadException()
	: std::runtime_error("The file is unreadable. Please check the file permissions and confirm that the file exists.")
{
}

Lexer::EmptyFileException::EmptyFileException()
	: std::runtime_error("The configuration file is empty.")
{
}