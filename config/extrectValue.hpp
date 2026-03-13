#ifndef EXTRECTVALUE_HPP
#define EXTRECTVALUE_HPP

#include <vector>
#include "config/ConfigParser.hpp"

#define KILO 1000
#define MEGA 1000000
#define GIGA 100000000

inline bool isAutoIndex(const std::string& token)  // todo move to a .cpp file
{
	if (token == "on")
		return true;
	else if (token == "off")
		return false;
	else
		throw "not valid AutoIndex"; // todo
}

inline size_t siUnitToValue(const std::string& str, bool isTime) // todo move to a .cpp file
{
	if (str.size() < 2)
		throw "Unit is to small"; // todo
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
			throw "not a valid unit"; // todo
			break;
	}
	return value;
}

template <typename T>
void extractSharedConfig(T& confStruct, ctxNodeIdent ctx, const token& t, ConfigLexer& tokens)
{
	(void)ctx;
	if (t.value == "root")
	{
		confStruct.root = tokens.getNextToken().value;
	}
	else if (t.value == "index")
	{
		confStruct.index = tokens.getNextToken().value;
	}
	else if (t.value == "autoindex")
	{
		confStruct.autoIndex = isAutoIndex(tokens.getNextToken().value);
	}
	else if (t.value == "error_page")
	{
		int code = std::stoi(tokens.getNextToken().value);
		if (code > __SHRT_MAX__ || code < 0)
			throw std::range_error("http code out of range");
		confStruct.errorPage->code = static_cast<short>(code);
		confStruct.errorPage->path = tokens.getNextToken().value;
	}
	else if (t.value == "client_header_buffer_size")
	{
		confStruct.clientHeaderBufferSize = siUnitToValue(tokens.getNextToken().value, false);
	}
	else if (t.value == "client_header_timeout")
	{
		confStruct.clientHeaderTimeout = siUnitToValue(tokens.getNextToken().value, true);
	}
	else if (t.value == "client_max_body_size")
	{
		confStruct.clientMaxBodySize = siUnitToValue(tokens.getNextToken().value, false);
	}
	else if (t.value == "client_body_timeout")
	{
		confStruct.clientBodyTimeout = siUnitToValue(tokens.getNextToken().value, true);
	}
	else
	{
		throw "error!"; // todo
	}
}

#endif // EXTRECTVALUE_HPP