#include "ConfigParser.hpp"

/* Constructors & Destructors */
ConfigParser::ConfigParser(const char* path)
{
	std::vector<std::string> tokens = extractTokens(path);

}

ConfigParser::~ConfigParser()
{
}

/* Member Functions */
bool ConfigParser::isDelimiter(char c) const
{
	if (c == '{' || c == '}' || c == ';')
		return true;
	return false;
}

void ConfigParser::removeComment(std::string& line)
{
	size_t pos = line.find('#');
	if (pos != line.npos)
	{
		line.erase(pos);
	}
}

void ConfigParser::splitLineInTokens(std::vector<std::string>& tokens, std::string& line)
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
					tokens.push_back(tmp);
				tokens.push_back(tokenStr.substr(i, 1));
				lastSplit = i + 1;
			}
		}
		if (tokenStr == "")
			continue;
		if (lastSplit == 0)
			tokens.push_back(tokenStr);
		if (lastSplit != 0)
		{
			std::string tmp = tokenStr.substr(lastSplit);
			if (tmp != "")
				tokens.push_back(tmp);
		}
	}
}

std::vector<std::string> ConfigParser::extractTokens(const char* path)
{
	std::ifstream configFile(path);
	std::string line;
	std::vector<std::string> tokens;

	if (configFile.fail())
		throw; // todo: exception
	while (std::getline(configFile, line, '\n'))
	{
		removeComment(line);
		splitLineInTokens(tokens, line);
	}
	if (!configFile.eof())
		throw; // todo: exception
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "\"" << tokens[i] << "\", ";
	}
	return (tokens);
}

/* Basic Operators */

/* Getters & Setters */
