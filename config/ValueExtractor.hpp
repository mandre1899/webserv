#ifndef VALUEEXTRACTOR_HPP
#define VALUEEXTRACTOR_HPP

#include <arpa/inet.h>
#include <string>
#include "configTypes.hpp"
#include "TokenStream.hpp"
#include "ContextNode.hpp"


#define KILO 1000
#define MEGA 1000000
#define GIGA 1000000000

// http:     root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, | client_header_timeout, client_header_buffer_size  |
// server:   root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, | client_header_timeout, client_header_buffer_size, | return, | listen, server_name
// location: root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, |                                                   | return, | allow_methods, upload_store

template<typename T>
class ValueExtractor {
private:
	std::unordered_map<std::string, std::function<void(T&, TokenStream&)>> _dispatch;

	unsigned long _stoul(const std::string& str)
	{
		if (str.size() < 1)
			throw std::runtime_error(" This is not a valid positive number, or the number is too large.");
		size_t pos = 0;
		unsigned long nbr;
		try
		{
			nbr = std::stoul(str, &pos);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(" This is not a valid positive number, or the number is too large.");
		}
		if (pos != str.size() || str[0] == '-')
			throw std::runtime_error(" This is not a valid positive number, or the number is too large.");
		return (nbr);
	}

	bool _isAutoIndex(const token& token)
	{
		if (token.value == "on")
			return true;
		else if (token.value == "off")
			return false;
		else
			throw std::runtime_error(buildMsg("Not a valid AutoIndex", token.pos));
	}

	size_t _siUnitToValue(const token& token, bool isTime)
	{
		const std::string& str = token.value;

		if (str.size() < 2)
			throw std::runtime_error(buildMsg("This is not a valid unit", token.pos));
		char unit = str[str.size() - 1];
		size_t value;
		try
		{
			value = _stoul(str.substr(0, str.size() - 1));
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(buildMsg(e.what(), token.pos));
		}
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
			throw std::runtime_error(buildMsg("This is not a valid unit", token.pos));
			break;
		}
		return value;
	}

	void _root(T& confStruct, TokenStream& ts)
	{
		confStruct.root = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _index(T& confStruct, TokenStream& ts)
	{
		confStruct.index = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _autoIndex(T& confStruct, TokenStream& ts)
	{
		confStruct.autoIndex = _isAutoIndex(ts.getNextToken());
		ts.expectToken(tType::semi);
	}

	void _errorPage(T& confStruct, TokenStream& ts)
	{
		unsigned long nbr;
		try
		{
			nbr = _stoul(ts.getNextValue());
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(buildMsg(e.what(), ts.getPositon()));
		}
		if (nbr > 999)
			throw std::range_error(buildMsg("The http code is out of range", ts.getPositon()));
		confStruct.errorPage.emplace();
		confStruct.errorPage->code = static_cast<short>(nbr);
		confStruct.errorPage->path = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _clientHeaderBufferSize(T& confStruct, TokenStream& ts)
	{
		confStruct.clientHeaderBufferSize = _siUnitToValue(ts.getNextToken(), false);
		ts.expectToken(tType::semi);
	}

	void _clientHeaderTimeout(T& confStruct, TokenStream& ts)
	{
		confStruct.clientHeaderTimeout = _siUnitToValue(ts.getNextToken(), true);
		ts.expectToken(tType::semi);
	}

	void _clientMaxBodySize(T& confStruct, TokenStream& ts)
	{
		confStruct.clientMaxBodySize = _siUnitToValue(ts.getNextToken(), false);
		ts.expectToken(tType::semi);
	}

	void _clientBodyTimeout(T& confStruct, TokenStream& ts)
	{
		confStruct.clientBodyTimeout = _siUnitToValue(ts.getNextToken(), true);
		ts.expectToken(tType::semi);
	}

	void _cgiPath(T& confStruct, TokenStream& ts)
	{
		confStruct.cgiPath = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _cgiIndex(T& confStruct, TokenStream& ts)
	{
		confStruct.cgiIndex = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _listen(T& confStruct, TokenStream& ts)
	{
		std::string tValue = ts.getNextValue();
		std::string ipStr = "0.0.0.0";
		std::string portStr = tValue;
		size_t pos = tValue.find(":");

		if (pos != std::string::npos)
		{
			ipStr = tValue.substr(0, pos);
			portStr = tValue.substr(pos + 1);
		}
		unsigned long nbr;
		try
		{
			nbr = _stoul(portStr);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(buildMsg(e.what(), ts.getPositon()));
		}

		if (nbr > UINT16_MAX)
			throw std::range_error(buildMsg("This is not a valid port number.", ts.getPositon()));
		confStruct.listen.port = static_cast<uint16_t>(nbr);

		struct in_addr addr;
		int result = inet_pton(AF_INET, ipStr.c_str(), &addr);
		if (result == 0)
			throw std::runtime_error(buildMsg("This is not a valid IP address", ts.getPositon()));
		if (result == -1)
			throw std::runtime_error(buildMsg("A system error occurred while transforming the IP string into an in_addr", ts.getPositon()));
		confStruct.listen.address = addr.s_addr;
		ts.expectToken(tType::semi);
	}

	void _serverName(T& confStruct, TokenStream& ts)
	{
		confStruct.serverName = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _allowMethods(T& confStruct, TokenStream& ts)
	{
		token token = ts.getNextToken();
		confStruct.allowMethods.isGet = false;
		confStruct.allowMethods.isPost = false;
		confStruct.allowMethods.isDelete = false;
		while (token.type != tType::semi)
		{
			if (token.value == "GET")
				confStruct.allowMethods.isGet = true;
			else if (token.value == "POST")
				confStruct.allowMethods.isPost = true;
			else if (token.value == "DELETE")
				confStruct.allowMethods.isDelete = true;
			else
				throw UnexpectedIdentifierException(ts.getPositon());
			token = ts.getNextToken();
		}
	}

	void _retConf(T& confStruct, TokenStream& ts)
	{
		unsigned long nbr;
		try
		{
			nbr = _stoul(ts.getNextValue());
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(buildMsg(e.what(), ts.getPositon()));
		}
		if (nbr > 999)
			throw std::range_error(buildMsg("This is not a valid http code", ts.getPositon()));
		confStruct.retConf.emplace();
		confStruct.retConf->code = static_cast<http_code>(nbr);
		token t = ts.getNextToken();
		if (t.type == tType::semi)
			return;
		confStruct.retConf->param = t.value;
		ts.expectToken(tType::semi);
	}

	void _uploadStore(T& confStruct, TokenStream& ts)
	{
		confStruct.uploadStore = ts.getNextValue();
		ts.expectToken(tType::semi);
	}

	void _duplicate(T& confStruct, TokenStream& ts)
	{
		(void)confStruct;
		throw std::runtime_error(buildMsg("Duplicate directive found", ts.getPositon()));
	}


public:
	/* Constructors & Destructors */
	ValueExtractor()
	{
		_dispatch["root"] = [this](T& confStruct, TokenStream& ts) {
			_root(confStruct, ts);
			};
		_dispatch["index"] = [this](T& confStruct, TokenStream& ts) {
			_index(confStruct, ts);
			};
		_dispatch["autoindex"] = [this](T& confStruct, TokenStream& ts) {
			_autoIndex(confStruct, ts);
			};
		_dispatch["error_page"] = [this](T& confStruct, TokenStream& ts) {
			_errorPage(confStruct, ts);
			};
		if constexpr (!std::is_same_v<cfg::locationCtx, T>)
		{
			_dispatch["client_header_buffer_size"] = [this](T& confStruct, TokenStream& ts) {
				_clientHeaderBufferSize(confStruct, ts);
				};
			_dispatch["client_header_timeout"] = [this](T& confStruct, TokenStream& ts) {
				_clientHeaderTimeout(confStruct, ts);
				};
		}
		_dispatch["client_max_body_size"] = [this](T& confStruct, TokenStream& ts) {
			_clientMaxBodySize(confStruct, ts);
			};
		_dispatch["client_body_timeout"] = [this](T& confStruct, TokenStream& ts) {
			_clientBodyTimeout(confStruct, ts);
			};
		_dispatch["cgi_path"] = [this](T& confStruct, TokenStream& ts) {
			_cgiPath(confStruct, ts);
			};
		_dispatch["cgi_index"] = [this](T& confStruct, TokenStream& ts) {
			_cgiIndex(confStruct, ts);
			};
		if constexpr (std::is_same_v<cfg::srvCtxCfg, T> || std::is_same_v<cfg::locationCtx, T>)
		{
			_dispatch["return"] = [this](T& confStruct, TokenStream& ts) {
				_retConf(confStruct, ts);
				};
		}
		if constexpr (std::is_same_v<cfg::srvCtxCfg, T>)
		{
			_dispatch["listen"] = [this](T& confStruct, TokenStream& ts) {
				_listen(confStruct, ts);
				};
			_dispatch["server_name"] = [this](T& confStruct, TokenStream& ts) {
				_serverName(confStruct, ts);
				};
		}
		if constexpr (std::is_same_v<cfg::locationCtx, T>)
		{
			_dispatch["allow_methods"] = [this](T& confStruct, TokenStream& ts) {
				_allowMethods(confStruct, ts);
				};
			_dispatch["upload_store"] = [this](T& confStruct, TokenStream& ts) {
				_uploadStore(confStruct, ts);
				};
		}
	}

	ValueExtractor(const ValueExtractor& src) = delete;
	ValueExtractor(ValueExtractor&& src) = delete;
	~ValueExtractor() = default;

	/* Member Functions */
	void setValue(const std::string& id, T& confStruct, TokenStream& ts)
	{
		auto it = _dispatch.find(id);
		if (it == _dispatch.end())
			throw UnexpectedIdentifierException(ts.getPositon());
		it->second(confStruct, ts);
		it->second = [this](T& confStruct, TokenStream& ts) {
			_duplicate(confStruct, ts);
			};
	}

	/* Basic Operators */
	ValueExtractor& operator=(const ValueExtractor& src) = delete;
	ValueExtractor& operator=(ValueExtractor&& src) = delete;

	/* Getters & Setters */

	/* Member Class */

	class UnexpectedIdentifierException : public std::runtime_error
	{
	public:
		UnexpectedIdentifierException(const filePos& pos)
			: std::runtime_error(buildMsg("Unexpected identifier", pos).c_str())
		{
		}
	};
};

#endif // VALUEEXTRACTOR_HPP