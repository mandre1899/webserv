#include "ConfigLexer.hpp"

/* Constructors & Destructors */
ConfigLexer::ConfigLexer(const char* path)
{
	std::ifstream configFile(path);
	if (configFile.fail())
		throw; // todo: exception
	extractTokens(configFile);
	configFile.close();
}

ConfigLexer::~ConfigLexer()
{
}

/* Member Functions */

bool ConfigLexer::isDelimiter(char c) const
{
	if (c == '{' || c == '}' || c == ';')
		return true;
	return false;
}

void ConfigLexer::removeComment(std::string& line)
{
	size_t pos = line.find('#');
	if (pos != line.npos)
	{
		line.erase(pos);
	}
}

void ConfigLexer::splitLineInTokens(std::vector<std::string>& tokens, std::string& line)
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

void ConfigLexer::extractTokens(std::ifstream& configFile)
{
	std::string line;

	while (std::getline(configFile, line, '\n'))
	{
		removeComment(line);
		splitLineInTokens(_tokens, line);
	}
	if (!configFile.eof())
		throw; // todo: exception
	Logger::log<LogLevel::trace>("ConfigLexer::extractTokens()\n", _tokens);
}

token ConfigLexer::getNextToken()
{
	token token;

	if (_tokenCount >= _tokens.size())
	{
		token.type = tokenType::end;
	}
	else if (_tokens[_tokenCount] == "{")
	{
		token.type = tokenType::lBrace;
	}
	else if (_tokens[_tokenCount] == "}")
	{
		token.type = tokenType::rBrace;
	}
	else if (_tokens[_tokenCount] == ";")
	{
		token.type = tokenType::semi;
	}
	else
	{
		token.type = tokenType::value;
		token.value = _tokens[_tokenCount];
	}
	_tokenCount++;
	return(token);
}

/* Basic Operators */

/* Getters & Setters */
// filePos ConfigLexer::getPos() const
// {
// 	// todo

// }