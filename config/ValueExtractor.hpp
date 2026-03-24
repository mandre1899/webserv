#ifndef VALUEEXTRACTOR_HPP
#define VALUEEXTRACTOR_HPP

#include "configTypes.hpp"
#include "TokenStream.hpp"
#include "ContextNode.hpp"

#define KILO 1000
#define MEGA 1000000
#define GIGA 100000000

template<typename T>
class ValueExtractor {
private:
	std::unordered_map<std::string, std::function<void(T&, ctxNodeIdent, TokenStream&)>> _dispatch; //todo

	bool _isAutoIndex(const std::string& token)  // todo move to a .cpp file
	{
		if (token == "on")
			return true;
		else if (token == "off")
			return false;
		else
			throw "not valid AutoIndex"; // todo
	}

	size_t _siUnitToValue(const std::string& str, bool isTime) // todo move to a .cpp file
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

	void _root(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.root = tokens.getNextToken().value;
	}

	void _index(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.index = tokens.getNextToken().value;
	}

	void _autoIndex(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.autoIndex = _isAutoIndex(tokens.getNextToken().value);
	}

	void _errorPage(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
		{
			int code = std::stoi(tokens.getNextToken().value);
			if (code > __SHRT_MAX__ || code < 0)
				throw std::range_error("http code out of range"); // todo
			confStruct.errorPage->code = static_cast<short>(code);
			confStruct.errorPage->path = tokens.getNextToken().value;
		}
	}

	void _clientHeaderBufferSize(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.clientHeaderBufferSize = siUnitToValue(tokens.getNextToken().value, false);
	}

	void _clientHeaderTimeout(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.clientHeaderTimeout = siUnitToValue(tokens.getNextToken().value, true);
	}

	void _clientMaxBodySize(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.clientMaxBodySize = siUnitToValue(tokens.getNextToken().value, false);
	}

	void _clientBodyTimeout(T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		if (ctx == ctxNodeIdent::http || ctx == ctxNodeIdent::server || ctx == ctxNodeIdent::location)
			confStruct.clientBodyTimeout = siUnitToValue(tokens.getNextToken().value, true);
	}

public:
	/* Constructors & Destructors */
	ValueExtractor()
	{
		_dispatch["root"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_root(confStruct, ctx, tokens);
			};
		_dispatch["index"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_index(confStruct, ctx, tokens);
			};
		_dispatch["autoindex"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_autoIndex(confStruct, ctx, tokens);
			};
		_dispatch["error_page"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_errorPage(confStruct, ctx, tokens);
			};
		_dispatch["client_header_buffer_size"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_clientHeaderBufferSize(confStruct, ctx, tokens);
			};
		_dispatch["client_header_timeout"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_clientHeaderTimeout(confStruct, ctx, tokens);
			};
		_dispatch["client_max_body_size"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_clientMaxBodySize(confStruct, ctx, tokens);
			};
		_dispatch["client_body_timeout"] = [this](T& confStruct, ctxNodeIdent ctx, TokenStream& tokens) {
			_clientBodyTimeout(confStruct, ctx, tokens);
			};
	}

	ValueExtractor(const ValueExtractor& src) = delete;
	ValueExtractor(ValueExtractor&& src) = delete;
	~ValueExtractor() = default;

	/* Member Functions */
	void setValue(const std::string& id, T& confStruct, ctxNodeIdent ctx, TokenStream& tokens)
	{
		_dispatch[id](confStruct, ctx, tokens);
	}

	/* Basic Operators */
	ValueExtractor& operator=(const ValueExtractor& src) = delete;
	ValueExtractor& operator=(ValueExtractor&& src) = delete;

	/* Getters & Setters */

};

#endif // VALUEEXTRACTOR_HPP