/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:48:39 by tschulle          #+#    #+#             */
/*   Updated: 2026/04/02 15:53:38 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include "utils/Logger.hpp"

enum class method_type
{
	GET = 0,
	POST,
	DELETE, //lower case delete is keyword
	undefined
};

class HttpParser {
private:
	method_type _method;
	std::string _request_URI;
	std::string _http_version;
	std::map<std::string, std::string> _headers_map;
	std::string _boundary;

public:
	/* Constructors & Destructors */
	HttpParser();
	HttpParser(const HttpParser& src) = delete;
	HttpParser(HttpParser&& src);
	~HttpParser();

	/* Member Functions */
	void parseRequestLine(std::string& line);
	void parseHeaders(std::string& line);
	bool parseBody(std::string& data);
	void parseBodyFromTmp(std::string path);
	class HttpParsingError : public std::exception {
	public:
		const char* what() const noexcept override;
	};

	/* Basic Operators */
	HttpParser& operator=(const HttpParser& src);
	HttpParser& operator=(HttpParser&& src) noexcept;

	/* Getters & Setters */
	method_type getMethod() const;
	std::string getMethodAsString() const;
	std::string getRequestUri() const;
	std::string getHttpVersion() const;
	std::map<std::string, std::string> getHeadersMap() const;
};

std::ostream& operator<<(std::ostream& o, const HttpParser& out);

#endif // HTTPPARSER_HPP
