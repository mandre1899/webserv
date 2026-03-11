#include "HttpConfig.hpp"

/* Constructors & Destructors */
HttpConfig::HttpConfig(const char* path)
{
	ConfigLexer lexer(path);
	ConfigParser parser(lexer);

}

HttpConfig::~HttpConfig()
{
}

/* Member Functions */

void HttpConfig::setHttpConfDefaults()
{
	_httpConf.root = "";
	_httpConf.index = "";
	_httpConf.autoIndex = false;
	_httpConf.clientHeaderBufferSize = 1000; // 1k
	_httpConf.clientHeaderTimeout = 30; // 30s
	_httpConf.clientMaxBodySize = 2000000; // 2M
	_httpConf.clientBodyTimeout = 30; //30s
}

bool HttpConfig::isAutoIndex(const std::string& token) const
{
	if (token == "on")
		return true;
	else if (token == "off")
		return false;
	else
		throw SyntaxError("not valid AutoIndex"); //todo
}

size_t HttpConfig::siUnitToValue(const std::string& str, bool isTime) const
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

size_t HttpConfig::extractSharedConfig(struct httpConf& httpConf, std::vector<std::string>& tokens, size_t pos)
{
	if (tokens[pos] == "root")
	{
		httpConf.root = tokens.at(++pos);
	}
	else if (tokens[pos] == "index")
	{
		httpConf.index = tokens.at(++pos);
	}
	else if (tokens[pos] == "autoindex")
	{
		httpConf.autoIndex = isAutoIndex(tokens.at(++pos));
	}
	else if (tokens[pos] == "error_page")
	{
		int code = std::stoi(tokens.at(++pos));
		if (code > __SHRT_MAX__ || code < 0)
			throw std::range_error("http code out of range");
		httpConf.errorPage->code = static_cast<short>(code);
		httpConf.errorPage->path = tokens.at(++pos);
	}
	else if (tokens[pos] == "client_header_buffer_size")
	{
		httpConf.clientHeaderBufferSize = siUnitToValue(tokens.at(++pos), false);
	}
	else if (tokens[pos] == "client_header_timeout")
	{
		httpConf.clientHeaderTimeout = siUnitToValue(tokens.at(++pos), true);
	}
	else if (tokens[pos] == "client_max_body_size")
	{
		httpConf.clientMaxBodySize = siUnitToValue(tokens.at(++pos), false);
	}
	else if (tokens[pos] == "client_body_timeout")
	{
		httpConf.clientBodyTimeout = siUnitToValue(tokens.at(++pos), true);
	}
	else
	{
		throw SyntaxError("error!"); // todo
	}
	if (++pos; tokens[pos] != ";")
		throw SyntaxError("no ;"); // todo
	return pos;
}

void HttpConfig::extractHttpConf(std::vector<std::string>& tokens)
{
	// tokens.size() != 3 || todo: wird das benoetig?
	if (tokens.at(0) != "http" || tokens.at(1) != "{")
		throw SyntaxError("expectet http {"); // ToDo
	for (size_t i = 2; i < (tokens.size() - 1); i++)
	{
		if (tokens[i] == "server")
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
		}
		else
		{
			i = extractSharedConfig(_httpConf, tokens, i);
		}
	}
	if (tokens.at(tokens.size() - 1) != "}")
		throw SyntaxError("error!"); // todo
	Logger::log<LogLevel::trace>("ConfigParser::extractHttpConf()\n", _httpConf);
}

// void ConfigParser::indexHandler()
// {

// }

// void ConfigParser::fillDispatch()
// {
// 	_dispatch.insert("index", );
// }

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
HttpConfig::SyntaxError::SyntaxError(const std::string& message) : std::runtime_error(message)
{
}
