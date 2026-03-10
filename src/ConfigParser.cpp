#include "ConfigParser.hpp"

/* Constructors & Destructors */
ConfigParser::ConfigParser(const char* path)
{
	std::vector<std::string> tokens = extractTokens(path);
	try
	{
		extractHttpConf(tokens);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	Logger::log<LogLevel::trace>("ConfigParser::ConfigParser()\n", _httpConf);
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
	Logger::log<LogLevel::trace>("ConfigParser::extractTokens()\n", tokens);
	return (tokens);
}

bool ConfigParser::isAutoIndex(const std::string& token) const
{
	if (token == "on")
		return true;
	else if (token == "off")
		return false;
	else
		throw SyntaxError("not valid AutoIndex"); //todo
}

size_t ConfigParser::siUnitToValue(const std::string& str, bool isTime) const
{
	if (str.size() < 2)
		throw SyntaxError("Unit is to small"); // todo
	char unit = str[str.size() - 1];
	size_t value = std::stoi(str.substr(0, str.size() - 1));
	if (isTime && unit == 's')
		return value;
	switch (unit)
	{
		case 'k':
			value *= KILO;
			break;
		case 'M':
			value *= MEGA;
			break;
		case 'G':
			value *= GIGA;
			break;
		default:
			throw SyntaxError("not a valid unit"); // todo
			break;
	}
	return value;
}

void ConfigParser::extractHttpConf(std::vector<std::string>& tokens)
{
	// tokens.size() != 3 || 
	if (tokens.at(0) != "http" || tokens.at(1) != "{")
		throw SyntaxError("expectet http {"); // ToDo
	for (size_t i = 2; i < (tokens.size() - 1); i++)
	{
		if (tokens[i] == "root")
		{
			_httpConf.root = tokens.at(++i);
		}
		else if (tokens[i] == "index")
		{
			_httpConf.index = tokens.at(++i);
		}
		else if (tokens[i] == "autoindex")
		{
			_httpConf.autoIndex = isAutoIndex(tokens[++i]);
		}
		else if (tokens[i] == "error_page")
		{
			int code = std::stoi(tokens.at(++i));
			if (code > __SHRT_MAX__ || code < 0)
				throw std::range_error("http code out of range");
			_httpConf.errorPage->code = static_cast<short>(code);
			_httpConf.errorPage->path = tokens.at(++i);
		}
		else if (tokens[i] == "client_header_buffer_size")
		{
			_httpConf.clientHeaderBufferSize = siUnitToValue(tokens.at(++i), false);
		}
		else if (tokens[i] == "client_header_timeout")
		{
			_httpConf.clientHeaderTimeout = siUnitToValue(tokens.at(++i), true);
		}
		else if (tokens[i] == "client_max_body_size")
		{
			_httpConf.clientMaxBodySize = siUnitToValue(tokens.at(++i), false);
		}
		else if (tokens[i] == "client_body_timeout")
		{
			_httpConf.clientBodyTimeout = siUnitToValue(tokens.at(++i), true);
		}
		else if (tokens[i] == "server")
		{
			size_t qoutState = 1;
			if (tokens[++i] != "{")
				throw SyntaxError("expectet {"); //todo
			while (qoutState != 0)
			{
				if (tokens[i] == "{")
					qoutState++;
				if (tokens[i] == "}")
					qoutState--;
				i++;
			}
			continue;
		}
		else
		{
			throw SyntaxError("error!"); // todo
		}
		if (++i; tokens[i] != ";")
			throw SyntaxError("no ;"); // todo

	}
	if (tokens.at(tokens.size() - 1) != "}")
		throw SyntaxError("error!"); // todo
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
ConfigParser::SyntaxError::SyntaxError(const std::string& message) : std::runtime_error(message)
{
}
