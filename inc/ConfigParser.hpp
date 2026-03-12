#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <netinet/in.h>
#include <map>
#include <vector>
#include "webserv.hpp"
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include "Logger.hpp"
#include "ConfigLexer.hpp"

#define KILO 1000
#define MEGA 1000000
#define GIGA 100000000

// http:     root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, | client_header_timeout, client_header_buffer_size  |
// server:   root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, | client_header_timeout, client_header_buffer_size, | return, | listen, server_name
// location: root, index, autoindex, error_page, client_body_timeout, client_max_body_size, fastcgi_path, fastcgi_index, |                                                   | return, | allow_methods, upload_store

enum class contextIdent
{
	http,
	server,
	location
};

struct context
{
	contextIdent conIdent;
	std::vector<token> param;
	std::vector<context> child;
};

class ConfigParser
{
private:
	context _httpCtx;

	/* Member Functions */
	void fillCtx(ConfigLexer& lexer, context& con);
	void createCtx(ConfigLexer& lexer);

public:
	/* Constructors & Destructors */
	ConfigParser() = delete;
	ConfigParser(ConfigLexer& lexer);
	ConfigParser(const ConfigParser& src) = delete;
	ConfigParser(ConfigParser&& src) = delete;
	~ConfigParser();

	/* Member Functions */

	/* Basic Operators */
	ConfigParser& operator=(const ConfigParser& src) = delete;
	ConfigParser& operator=(ConfigParser&& src) = delete;

	/* Getters & Setters */
	const context& getHttpCtx() const;

	/* Member Class */
};

std::ostream& operator<<(std::ostream& os, const context& ctx);

#endif // CONFIGPARSER_HPP