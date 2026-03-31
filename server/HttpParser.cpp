/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tschulle <tschulle@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:48:32 by tschulle          #+#    #+#             */
/*   Updated: 2026/03/31 14:16:36 by tschulle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include <fstream>

/* Constructors & Destructors */
HttpParser::HttpParser()
	: _method(method_type::undefined)
{
}

HttpParser::HttpParser(HttpParser&& src)
	: _method(src._method), _request_URI(std::move(src._request_URI)), \
	_http_version(std::move(src._http_version)), _headers_map(std::move(src._headers_map))
{
	// TODO body information
}

HttpParser::~HttpParser()
{
}

/* Member Functions */
void HttpParser::parseRequestLine(std::string& line)
{
	std::stringstream ss(line);
	std::string temp;
	char del = ' ';
	getline(ss, temp, del);
	if (temp == "GET")
		_method = method_type::GET;
	else if (temp == "POST")
		_method = method_type::POST;
	else if (temp == "DELETE")
		_method = method_type::DELETE;
	else
		throw HttpParsingError();

	getline(ss, temp, del);
	_request_URI = temp;
	if (temp[0] != '/')
		throw HttpParsingError();

	getline(ss, temp, del);
	if (!(temp == "HTTP/1.1" || temp == "HTTP/1.0"))
		throw HttpParsingError();
	_http_version = temp;
	if (!ss.eof()) //extra space seems okay tho ... so trim space at end
	{
		throw HttpParsingError();
	}
}

static void trim(std::string& s)
{
	auto start = s.find_first_not_of(" \t\r\n");
	auto end = s.find_last_not_of(" \t\r\n");

	if (start == std::string::npos) {
		s.clear();
		return;
	}

	s = s.substr(start, end - start + 1);
}


void HttpParser::parseHeaders(std::string& line)
{
	auto pos = line.find(':');
	if (pos == std::string::npos)
		throw HttpParsingError();
	std::string key = line.substr(0, pos);
	if (key.empty())
		throw HttpParsingError();
	if (key.find_first_of(" \t\r\n") != std::string::npos)
		throw HttpParsingError();
	//TODO maybe do more validation on header names
	std::string value = line.substr(pos + 1);
	trim(value);
	_headers_map.insert(std::pair<std::string, std::string>{key, value});
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
		++it;
	return !s.empty() && it == s.end();
}


void HttpParser::parseBody(std::string& line)
{
	static int bytes_read = 0;
	if (!((_headers_map.find("Content-Length") != _headers_map.end()) != (_headers_map.find("Transfer-Encoding") != _headers_map.end())))
		throw HttpParsingError();

	std::ofstream	ofs("test");
	if (!ofs)
	{
		std::cerr << "failed to establish outstream" << std::endl;
	}
	if (auto search = _headers_map.find("Content-Length"); is_number(search->second))
	{
		while (search->second > bytes_read)
		{
			ofs << line;
			bytes_read += sizeof(line);
		}
	}
	ofs.close();
}

const char* HttpParser::HttpParsingError::what() const noexcept
{
	return ("Http parsing error\n");
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
		// TODO body information
	}
	return *this;
}

HttpParser& HttpParser::operator=(HttpParser&& src) noexcept
{
	if (this != &src)
	{
		_method = src._method;
		_request_URI = std::move(src._request_URI);
		_http_version = std::move(src._http_version);
		_headers_map = std::move(src._headers_map);
	}
	return *this;
}

/* Getters & Setters */

method_type HttpParser::getMethod() const
{
	return (_method);
}

std::string HttpParser::getMethodAsString() const
{
	switch (_method)
	{
		case method_type::GET: return ("GET");
		case method_type::DELETE: return ("DELETE");
		case method_type::POST: return ("POST");
		case method_type::undefined: return ("undefined");
		default: return ("undefined");
	}
}

std::string HttpParser::getRequestUri() const
{
	return (_request_URI);
}

std::string HttpParser::getHttpVersion() const
{
	return (_http_version);
}
auto HttpParser::getHeadersMap() const
{
	return (_headers_map);
}


std::ostream& operator<<(std::ostream& o, const HttpParser& out)
{
	std::cout << "\nMethod type is: " << out.getMethodAsString()
		<< "\nrequest URI is: " << out.getRequestUri()
		<< "\nhttp version is: " << out.getHttpVersion();
	for (const auto& [key, value] : out.getHeadersMap())
	{
		std::cout << "\n[" << key << "]" << value;
	}
	std::cout << std::endl;
	return (o);
}
