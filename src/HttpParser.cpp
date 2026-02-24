/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:48:32 by tschulle          #+#    #+#             */
/*   Updated: 2026/02/24 13:37:01 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

/* Constructors & Destructors */
HttpParser::HttpParser()
{
}

HttpParser::HttpParser(const HttpParser& src)
{
	(void)src;
}

HttpParser::HttpParser(HttpParser&& src)
{
	(void)src;
}

// HttpParser::HttpParser(std::string plaintext)
// {
// 	(void)plaintext;
// }

HttpParser::~HttpParser()
{
}

/* Member Functions */
void HttpParser::parse_request_mothod(std::string& msg) const
{
	// if ()
	(void)msg;
}


/* Basic Operators */
HttpParser& HttpParser::operator=(const HttpParser& src)
{
	if (this != &src)
	{
		this->_method = src._method;
		this->_request_URI = src._request_URI;
		this->_http_version = src._http_version;
		this->_headers_map = src._headers_map;
		//body information
	}
	return *this;
}

HttpParser& HttpParser::operator=(HttpParser&& src)
{
	if (this != &src)
	{

	}
	return *this;
}

/* Getters & Setters */

METHOD_TYPE HttpParser::get_method() const
{
	return (_method);
}
std::string HttpParser::get_request_uri() const
{
	return (_request_URI);
}

std::string HttpParser::get_http_version() const
{
	return (_http_version);
}
auto HttpParser::get_headers_map() const
{
	return (_headers_map);
}


std::ostream& operator<<(std::ostream& o, HttpParser& out)
{
	std::cout << "Method type is: " << out.get_method()
		<< "request URI is: " << out.get_request_uri()
		<< "http version is: " << out.get_http_version();
	for (const auto& [key, value] : out.get_headers_map())
	{
		std::cout << "[" << key << "]" << value << std::endl;
	}
	return (o);
}