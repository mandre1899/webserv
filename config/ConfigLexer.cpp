#include "ConfigLexer.hpp"

/* Constructors & Destructors */

/* Member Functions */
bool ConfigLexer::isDelimiter(char c)
{
	if (c == '{' || c == '}' || c == ';')
		return true;
	return false;
}

void ConfigLexer::removeComment(std::string& line)
{
	size_t commentPos = line.find('#');
	if (commentPos != line.npos)
	{
		line.erase(commentPos);
	}
}

void ConfigLexer::splitLineInTokens(TokenStream& ts, filePos& pos, std::string& line) // todo:: count pos.col
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

TokenStream ConfigLexer::extractTokens(const char* path)
{
	TokenStream ts;
	filePos pos = {};
	std::string line;
	std::ifstream configFile(path);

	if (configFile.fail())
		throw "cout not open file"; // todo: exception
	while (std::getline(configFile, line, '\n'))
	{
		pos.ln++;
		removeComment(line);
		splitLineInTokens(ts, pos, line);
	}
	if (!configFile.eof())
		throw; // todo: exception
	configFile.close();
	Logger::log<LogLevel::trace>("ConfigLexer::extractTokens()\n", ts);
}

/* Basic Operators */

/* Getters & Setters */