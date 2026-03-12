#include "ConfigLexer.hpp"

/* Constructors & Destructors */
ConfigLexer::ConfigLexer(const char* path)
{
	std::ifstream configFile(path);
	if (configFile.fail())
		throw; // todo: exception
	std::vector<std::string> strTokens = extractTokens(configFile);
	configFile.close();
	setTokenTyp(strTokens);
}

ConfigLexer::ConfigLexer(const std::vector<token>& tokens)
{
	_tokens = tokens;
	if (tokens[tokens.size() - 1].type != tokenType::end)
		_tokens.push_back({ tokenType::end, "" });
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

std::vector<std::string> ConfigLexer::extractTokens(std::ifstream& configFile)
{
	std::string line;
	std::vector<std::string> tokens;

	while (std::getline(configFile, line, '\n'))
	{
		removeComment(line);
		splitLineInTokens(tokens, line);
	}
	if (!configFile.eof())
		throw; // todo: exception
	Logger::log<LogLevel::trace>("ConfigLexer::extractTokens()\n", tokens);
	return (tokens);
}

void ConfigLexer::setTokenTyp(std::vector<std::string>& tokens)
{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		token token;
		if (tokens[i] == "{")
		{
			token.type = tokenType::lBrace;
			token.value = "{";
		}
		else if (tokens[i] == "}")
		{
			token.type = tokenType::rBrace;
			token.value = "}";
		}
		else if (tokens[i] == ";")
		{
			token.type = tokenType::semi;
			token.value = ";";
		}
		else
		{
			token.type = tokenType::value;
			token.value = tokens[i];
		}
		_tokens.push_back(token);
	}
}

const token& ConfigLexer::getNextToken()
{
	const token& tmp = _tokens[_tokenCount];
	if (_tokenCount < (_tokens.size() - 1))
		_tokenCount++;
	return(tmp);
}

void ConfigLexer::expectToken(tokenType type)
{
	token token = this->getNextToken();
	if (token.type != type)
	{
		throw "unexpectet token!";
	}
}

/* Basic Operators */

/* Getters & Setters */
// filePos ConfigLexer::getPos() const
// {
// 	// todo

// }