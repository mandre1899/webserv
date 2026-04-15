/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:48:32 by tschulle          #+#    #+#             */
/*   Updated: 2026/04/14 15:36:19 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

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

static std::pair<std::string, std::string> request_uri_normalization(const std::string& request_uri)
{
	std::string	uri = request_uri;
	std::string	query_string;

	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos)
	{
		query_string = uri.substr(query_pos + 1);
		uri = uri.substr(0, query_pos);
	}

	std::string normalized_uri;
	normalized_uri.reserve(uri.size());
	bool previous_is_slash = false;
	for (char c : uri)
	{
		if (c == '/')
		{
			if (!previous_is_slash)
				normalized_uri.push_back(c);
			previous_is_slash = true;
		}
		else
		{
			normalized_uri.push_back(c);
			previous_is_slash = false;
		}
	}
	if (normalized_uri.empty())
		normalized_uri = "/";
	return std::make_pair(normalized_uri, query_string);
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
	auto uri_and_query_str = request_uri_normalization(temp);
	_request_URI = uri_and_query_str.first;
	_query_str = uri_and_query_str.second;
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
	auto end = s.find_last_not_of(" \t\r\n"); // todo: add more header validtion

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

void is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
		++it;
	if (!s.empty() && it == s.end())
		return;
	throw HttpParser::HttpParsingError();
}

std::string extractBoundary(std::string& value)
{
	if (value.substr(0, 30).compare("multipart/form-data; boundary=") != 0)
		throw HttpParser::HttpParsingError();
	std::string boundary = value.substr(30);
	return ("--" + boundary);
}

void HttpParser::parseBodyFromTmp(std::string path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		throw HttpParser::HttpParsingError();
	auto boundary_header = _headers_map.find("Content-Type");
	if (boundary_header == _headers_map.end())
		throw HttpParser::HttpParsingError();
	_boundary = extractBoundary(boundary_header->second);
	//std::cout << "BOUNDARY CHECK :" << _boundary << std::endl;
	std::string temp;
	getline(in, temp);
	//std::cout << temp << "\n" << std::endl;
	if (!temp.empty() && temp.back() == '\r')
		temp.pop_back();
	if (temp != "--" + _boundary)
		throw HttpParser::HttpParsingError();
	getline(in, temp);
	//std::cout << temp << "\n" << std::endl;
	std::string filename = temp.substr(temp.find("filename=") + 10);
	if (filename.empty())
		throw HttpParser::HttpParsingError();
	filename.erase(filename.find('"'));
	//std::cout << filename << "\n" << std::endl;


}

// void validateBodyRequest(std::string& line)
// {

// }

void  HttpParser::processBody(std::string& line)
{
	static std::string buffer = "";
	static int bodyState = 0;
	static std::string filename;

	buffer.append(line, 0, line.size());
	if (bodyState == 0) // read body boundery
	{
		size_t pos = buffer.find(_boundary + "\r\n");

		if (pos == 0)
		{
			if (_boundary.size() + 2 == buffer.size())
				buffer.erase();
			else
				buffer = buffer.substr(_boundary.size() + 2);
			bodyState = 1;
		}
		if (buffer.size() > _boundary.size())
			throw HttpParsingError();
	}
	if (bodyState == 1) // read headder
	{
		while (true)
		{
			size_t posEnd = buffer.find("\r\n\r\n");
			if (posEnd == std::string::npos)
				return;
			size_t subLen = buffer.find_first_of("\r\n");
			if (subLen == std::string::npos)
				throw HttpParsingError();
			std::string sub = buffer.substr(0, subLen);
			size_t pos = sub.find(':');
			if (pos == std::string::npos)
				throw HttpParsingError();
			std::string key = sub.substr(0, pos);
			if (key.empty())
				throw HttpParsingError();
			std::string value = sub.substr(pos + 1);
			if (key == "Content-Disposition")
			{
				size_t fileNamePos = value.find("filename="); // todo: more validation
				if (fileNamePos == std::string::npos)
					throw HttpParsingError();
				fileNamePos += 9;
				if (value[fileNamePos] != '\"')
					throw HttpParsingError();
				fileNamePos++;
				for (size_t i = fileNamePos; i < value.size(); i++)
				{
					if (value[i] == ';' && fileNamePos != i && value[i - 1] == '\"')
					{
						filename = value.substr(fileNamePos, (i - 1 - fileNamePos));
						break;
					}
					if (i == value.size() - 1 && fileNamePos != i && value[i] == '\"')
					{
						filename = value.substr(fileNamePos, (i - fileNamePos));
						break;
					}
				}
			}
			buffer = buffer.substr(subLen + 2);
			if (posEnd == subLen)
			{
				buffer = buffer.substr(2);
				bodyState = 2;
				std::cout << "buff end: \"" << buffer << "\" " << pos << std::endl;
				break;
			}
		}
	}
	if (bodyState == 2) // read body data
	{
		if (filename.empty())
			throw HttpParsingError();
		std::cout << filename << " <- filname" << std::endl;
		std::ofstream outfile(filename, std::ios_base::app | std::ios_base::out); // maby it is better to leave the file open, and add the fd to the destructor
		if (outfile.fail())
			throw HttpParsingError(); //todo: maby send other exception
		outfile << buffer;
		buffer.erase();
		outfile.close();
	}
}

/*
// curl -X POST http://localhost:8085/upload \
//   -F "file1=@test.txt" \
//   -F "file2=@b.png"s
*/

bool HttpParser::parseBody(std::string& line)
{
	static unsigned long bytes_read = 0;
	static std::string buffer;
	static size_t contentLength = 0;

	if (bytes_read == 0)
	{
		auto search = _headers_map.find("Content-Length");
		if ((search == _headers_map.end())) // todo:  Ab HTTP/1.1 -> _headers_map.find("Transfer-Encoding") != _headers_map.end()
			throw HttpParsingError();
		is_number(search->second);

		contentLength = std::stoul(search->second);
		auto boundary_header = _headers_map.find("Content-Type");
		if (boundary_header == _headers_map.end())
			throw HttpParser::HttpParsingError();
		_boundary = extractBoundary(boundary_header->second);
	}
	bytes_read += line.size();
	if (bytes_read > contentLength)
		throw HttpParser::HttpParsingError();
	//std::cout << line << std::endl;
	processBody(line);
	if (bytes_read == contentLength)
		return true; // need to reset bytes_read? can same client get here a second time?
	return false;
}


// curl -X POST http://localhost:8085/upload 
//   -F "myfile=@./test.txt"

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

std::string HttpParser::getRequestQuery() const
{
	return (_query_str);
}

std::string HttpParser::getHttpVersion() const
{
	return (_http_version);
}

std::map<std::string, std::string>	HttpParser::getHeadersMap() const
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
