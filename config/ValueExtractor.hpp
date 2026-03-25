#ifndef VALUEEXTRACTOR_HPP
#define VALUEEXTRACTOR_HPP

#include <arpa/inet.h>
#include "configTypes.hpp"
#include "TokenStream.hpp"
#include "ContextNode.hpp"


#define KILO 1000
#define MEGA 1000000
#define GIGA 100000000

template<typename T>
class ValueExtractor {
private:
	std::unordered_map<std::string, std::function<void(T&, TokenStream&)>> _dispatch; //todo

	unsigned long _stoul(const std::string& str)
	{
		if (str.size() < 1)
			throw "not a valid number"; // todo
		size_t pos = 0;
		unsigned long nbr = std::stoul(str, &pos);
		if (pos != str.size() || str[0] == '-')
			throw "not a valid number"; // todo
		return (nbr);
	}

	bool _isAutoIndex(const std::string& token)
	{
		if (token == "on")
			return true;
		else if (token == "off")
			return false;
		else
			throw "not valid AutoIndex"; // todo
	}

	size_t _siUnitToValue(const std::string& str, bool isTime)
	{
		if (str.size() < 2)
			throw "Unit is to small"; // todo
		char unit = str[str.size() - 1];
		size_t value = _stoul(str.substr(0, str.size() - 1));
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

	void _root(T& confStruct, TokenStream& tokens)
	{
		confStruct.root = tokens.getNextToken().value;
		tokens.expectToken(tType::semi);
	}

	void _index(T& confStruct, TokenStream& tokens)
	{
		confStruct.index = tokens.getNextToken().value;
		tokens.expectToken(tType::semi);
	}

	void _autoIndex(T& confStruct, TokenStream& tokens)
	{
		confStruct.autoIndex = _isAutoIndex(tokens.getNextToken().value);
		tokens.expectToken(tType::semi);
	}

	void _errorPage(T& confStruct, TokenStream& tokens)
	{
		int code = std::stoi(tokens.getNextToken().value);
		if (code > __SHRT_MAX__ || code < 0)
			throw std::range_error("http code out of range"); // todo
		confStruct.errorPage->code = static_cast<short>(code);
		confStruct.errorPage->path = tokens.getNextToken().value;
		tokens.expectToken(tType::semi);
	}

	void _clientHeaderBufferSize(T& confStruct, TokenStream& tokens)
	{
		confStruct.clientHeaderBufferSize = _siUnitToValue(tokens.getNextToken().value, false);
		tokens.expectToken(tType::semi);
	}

	void _clientHeaderTimeout(T& confStruct, TokenStream& tokens)
	{
		confStruct.clientHeaderTimeout = _siUnitToValue(tokens.getNextToken().value, true);
		tokens.expectToken(tType::semi);
	}

	void _clientMaxBodySize(T& confStruct, TokenStream& tokens)
	{
		confStruct.clientMaxBodySize = _siUnitToValue(tokens.getNextToken().value, false);
		tokens.expectToken(tType::semi);
	}

	void _clientBodyTimeout(T& confStruct, TokenStream& tokens)
	{
		confStruct.clientBodyTimeout = _siUnitToValue(tokens.getNextToken().value, true);
		tokens.expectToken(tType::semi);
	}

	void _listen(T& confStruct, TokenStream& tokens)
	{
		std::string tValue = tokens.getNextValue();
		std::string ipStr = "0.0.0.0";
		std::string portStr = tValue;
		size_t pos = tValue.find(":");

		if (pos != std::string::npos)
		{
			ipStr = tValue.substr(0, pos);
			portStr = tValue.substr(pos + 1);
		}
		unsigned long nbr = _stoul(portStr);
		if (nbr > UINT16_MAX)
			throw "port number is to big/not a valid port number";
		confStruct.listen.port = static_cast<uint16_t>(nbr);

		struct in_addr addr;
		int result = inet_pton(AF_INET, ipStr.c_str(), &addr);
		if (result == 0)
			throw "not a valid ip address";
		if (result == -1)
			throw "system error accur, check errono";
		confStruct.listen.address = addr.s_addr;
		tokens.expectToken(tType::semi);
	}

	void _serverName(T& confStruct, TokenStream& tokens)
	{
		confStruct.serverName = tokens.getNextToken().value; // todo: servername validation
		tokens.expectToken(tType::semi);
	}

	void _allowMethods(T& confStruct, TokenStream& tokens)
	{
		token token;
		while (token.type != tType::semi)
		{
			token = tokens.getNextToken();
			if (token.value == "GET")
				confStruct.allowMethods.isGet = true;
			else if (token.value == "POST")
				confStruct.allowMethods.isPost = true;
			else if (token.value == "DELETE")
				confStruct.allowMethods.isDelete = true;
			else
				throw UnexpectedIdentifierException();
		}
	}


public:
	/* Constructors & Destructors */
	ValueExtractor()
	{
		_dispatch["root"] = [this](T& confStruct, TokenStream& tokens) {
			_root(confStruct, tokens);
			};
		_dispatch["index"] = [this](T& confStruct, TokenStream& tokens) {
			_index(confStruct, tokens);
			};
		_dispatch["autoindex"] = [this](T& confStruct, TokenStream& tokens) {
			_autoIndex(confStruct, tokens);
			};
		_dispatch["error_page"] = [this](T& confStruct, TokenStream& tokens) {
			_errorPage(confStruct, tokens);
			};
		_dispatch["client_header_buffer_size"] = [this](T& confStruct, TokenStream& tokens) {
			_clientHeaderBufferSize(confStruct, tokens);
			};
		_dispatch["client_header_timeout"] = [this](T& confStruct, TokenStream& tokens) {
			_clientHeaderTimeout(confStruct, tokens);
			};
		_dispatch["client_max_body_size"] = [this](T& confStruct, TokenStream& tokens) {
			_clientMaxBodySize(confStruct, tokens);
			};
		_dispatch["client_body_timeout"] = [this](T& confStruct, TokenStream& tokens) {
			_clientBodyTimeout(confStruct, tokens);
			};
		if constexpr (std::is_same_v<cfg::serverCtx, T>)
		{
			_dispatch["listen"] = [this](T& confStruct, TokenStream& tokens) {
				_listen(confStruct, tokens);
				};
			_dispatch["server_name"] = [this](T& confStruct, TokenStream& tokens) {
				_serverName(confStruct, tokens);
				};
		}
		if constexpr (std::is_same_v<cfg::locationCtx, T>)
		{
			_dispatch["allow_methods"] = [this](T& confStruct, TokenStream& tokens) {
				_allowMethods(confStruct, tokens);
				};
		}
	}

	ValueExtractor(const ValueExtractor& src) = delete;
	ValueExtractor(ValueExtractor&& src) = delete;
	~ValueExtractor() = default;

	/* Member Functions */
	void setValue(const std::string& id, T& confStruct, TokenStream& tokens)
	{
		auto it = _dispatch.find(id);
		if (it == _dispatch.end())
			throw UnexpectedIdentifierException();
		_dispatch[id](confStruct, tokens);
	}

	/* Basic Operators */
	ValueExtractor& operator=(const ValueExtractor& src) = delete;
	ValueExtractor& operator=(ValueExtractor&& src) = delete;

	/* Getters & Setters */

	/* Member Class */
	class UnexpectedIdentifierException : public std::exception
	{
	public:
		virtual const char* what() const noexcept override
		{
			return ("Unexpected identifier.");
		}
	};

};

#endif // VALUEEXTRACTOR_HPP