#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <unordered_map>
#include <ostream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include "config/configTypes.hpp"
#include "utils/helper.hpp"
#include "utils/Logger.hpp"

typedef std::unordered_map<std::string, std::string>	header_map;
typedef std::unordered_map<int, std::string>			status_map;

class HttpResponse
{
	int							_status_code;
	header_map					_header;
	std::string					_body;
	static const status_map		_status_code_map;

	//Buissness logic intern
	void	set_default_headers(void);
	bool	read_file_to_body(std::string &abs_path);
	public:
		HttpResponse(void);
		~HttpResponse(void);
		HttpResponse	operator=(const HttpResponse &obj) = delete;
		HttpResponse(HttpResponse &obj) = delete;

		//Buissness logic extern
		void	set_default_reponse(int status_code); // Set default error page
		void	set_default_reponse(int status_code, cfg::wsLocationCtx &location); // With check for default error page overwrite
		//Setters
		void	set_status(int status_code);
		void	set_header(header_map::iterator begin, header_map::iterator end);	// Insert headers at the end
		void	set_header(header_map::const_iterator begin, header_map::const_iterator end);	// replaces matching keys 
		void	set_body(std::string &body);
		//Getter
		int					get_status(void) const;
		header_map			get_headers(void) const;
		std::string			get_body(void) const;
		const std::string	get_response_str(void) const;
};

std::ostream&	operator<<(std::ostream& os, HttpResponse& obj);

#endif // !define HTTP_RESPONSE_H
