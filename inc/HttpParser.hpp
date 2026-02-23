/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:48:39 by tschulle          #+#    #+#             */
/*   Updated: 2026/02/23 17:37:25 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>
#include <sstream>
#include <map>
#include <iostream>

enum METHOD_TYPE
{
	GET = 0,
	POST,
	DELETE,
};

class HttpParser {
private:
	METHOD_TYPE _method;
	std::string _request_URI;
	std::string _http_version;
	std::map<std::string, std::string> _headers_map;
	//body information

public:
	/* Constructors & Destructors */
	HttpParser();
	HttpParser(const HttpParser& src);
	HttpParser(HttpParser&& src);
	HttpParser(std::string plaintext);
	~HttpParser();

	/* Member Functions */
	void parse_request_mothod(std::string& msg) const;

	/* Basic Operators */
	HttpParser& operator=(const HttpParser& src);
	HttpParser& operator=(HttpParser&& src);

	/* Getters & Setters */
	METHOD_TYPE get_method() const;
	std::string get_request_uri() const;
	std::string get_http_version() const;
	auto get_headers_map() const;
};

std::ostream& operator<<(std::ostream& o, HttpParser& out);

#endif // HTTPPARSER_HPP