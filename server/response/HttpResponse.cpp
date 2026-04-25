#include "HttpResponse.hpp"
#include <unordered_map>

const status_map	HttpResponse::_status_code_map =
{
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},

	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},

	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},

	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{415, "Unsupported Media Type"},
	{418, "I'm a teapot"},
	{429, "Too Many Requests"},

	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"}
};

std::ostream& operator<<(std::ostream& os, HttpResponse& obj)
{
	os << "Status code: " << obj.get_status() << '\n';

	os << "Headers:\n";
	for (const auto& header : obj.get_headers())
	{
		os << "  " << header.first
			<< ": " << header.second << '\n';
	}

	os << "\nBody:\n";
	os << obj.get_body();

	return os;
}

HttpResponse::HttpResponse(void) : _status_code(-1)
{
}

HttpResponse::~HttpResponse(void)
{
}

//Buissness logic extern
void	HttpResponse::set_default_headers(void)
{
	if (_header.find("server") == _header.end())
		_header["server"] = "Webserv/1.0";
	if (_header.find("date") == _header.end()) {
		time_t now = time(0);
		struct tm tm_struct;
		gmtime_r(&now, &tm_struct);
		char buf[100];
		strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_struct);
		_header["date"] = std::string(buf);
	}
	// Add connection header based on HTTP version
	if (_header.find("connection") == _header.end())
		_header["connection"] = "close";
	_header["content-length"] = std::to_string(_body.size());
}

void	HttpResponse::set_default_reponse(int status_code)
{
	std::string status_text = "Unknown Error";

	_status_code = status_code;
	status_map::const_iterator it = _status_code_map.find(status_code);
	if (it != _status_code_map.end())
		status_text = it->second;
	else
		status_text = "Error not found in the map";
	std::ostringstream body;

	body << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "  <meta charset=\"UTF-8\">\n"
		<< "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		<< "  <title>" << _status_code << " " << status_text << "</title>\n"
		<< "  <style>\n"
		<< "    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
		<< "    h1 { font-size: 50px; }\n"
		<< "    p { font-size: 20px; color: #555; }\n"
		<< "  </style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "  <h1>" << _status_code << "</h1>\n"
		<< "  <p>" << status_text << "</p>\n"
		<< "  <hr>\n"
		<< "  <p>Websmurf</p>\n"
		<< "</body>\n"
		<< "</html>\n";

	_body = body.str();
	set_default_headers();
	Logger::log<LogLevel::info>("Server responding with error code: ", status_code);

}

void	HttpResponse::set_default_reponse(int status_code, const cfg::wsLocationCtx* location, const cfg::vHostCfg *server, const std::string &base_path)
{
	if (location)
	{
		std::unordered_map<http_code, std::string>::const_iterator it_err_page = location->errorPage.find(status_code);
		if (it_err_page != location->errorPage.end() && !it_err_page->second.empty())
		{
			std::string error_page_path = join_paths(base_path, it_err_page->second);
			if (access(error_page_path.c_str(), R_OK) == 0)
			{
				set_status(status_code);
				read_file_to_body(error_page_path);
				set_default_headers();
				return;
			}
		}
	}
	if (server)
	{
		std::unordered_map<http_code, std::string>::const_iterator it_err_page = server->errorPage.find(status_code);
		if (it_err_page != server->errorPage.end() && !it_err_page->second.empty())
		{
			std::string error_page_path = join_paths(base_path, it_err_page->second);
			if (access(error_page_path.c_str(), R_OK) == 0)
			{
				set_status(status_code);
				read_file_to_body(error_page_path);
				set_default_headers();
				return;
			}
		}
	}
	set_default_reponse(status_code);
}

const std::string	HttpResponse::get_response_str(void) const
{
	std::ostringstream	tmp;
	
	std::string status_text = "Unknown Error";

	status_map::const_iterator it = _status_code_map.find(_status_code);
	if (it != _status_code_map.end())
		status_text = it->second;
	else
		status_text = "Error not found in the map";
	tmp << "HTTP/1.1 " << _status_code << " " << status_text << "\r\n";
	for (const auto& a : _header)
		tmp << a.first << ": " << a.second << "\r\n";
	tmp << "\r\n";
	if (!_body.empty())
		tmp << _body;
	return tmp.str();

}

bool	HttpResponse::read_file_to_body(std::string& abs_path)
{
	struct stat file_stat;
	if (stat(abs_path.c_str(), &file_stat) != 0)
	{
		set_default_reponse(404);
		return false;
	}
	std::ifstream file;
	try
	{
		file.open(abs_path, std::ios::binary);
		_body.reserve(file_stat.st_size); //reserve for better performance
		_body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		return true;

	}
	catch (...)
	{
		set_default_reponse(500);
		Logger::log<LogLevel::error>("Failed to read rile to body: ", abs_path);
		return false;
	}
}

//Setters
void	HttpResponse::set_status(int status_code)
{
	_status_code = status_code;
}

void HttpResponse::set_header(header_map::iterator begin, header_map::iterator end) // Replaces the values if matching keys with the new ones passed
{
	for (; begin != end; ++begin)
		_header[begin->first] = begin->second;
}

void HttpResponse::set_header(header_map::const_iterator begin, header_map::const_iterator end) // Replaces the values if matching keys with the new ones passed
{
	for (; begin != end; ++begin)
		_header[begin->first] = begin->second;
}

void HttpResponse::set_header(std::string key, std::string value)
{
	_header[key] = value;
}

void	HttpResponse::set_body(std::string body)
{
	_body = body;
}

void	HttpResponse::set_content_type(std::string type)
{
	header_map tmp = { {"Content-Type", type} };
	set_header(tmp.begin(), tmp.end());
}

//Getter
int	HttpResponse::get_status(void) const
{
	return (_status_code);
}

header_map	HttpResponse::get_headers(void) const
{
	return _header;
}

std::string	HttpResponse::get_body(void) const
{
	return _body;
}

