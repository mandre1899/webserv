#ifndef CGI_H
# define CGI_H

#include "../utils/Logger.hpp"
#include"StateMachines.hpp"
#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sys/epoll.h>
#include "HttpParser.hpp"


class CGI
{
	HttpParser	&_request;
	std::string	_abs_path;
	int			_cgi_pid;
	int			_pipe_read;
	int			_epfd;
	int			_body_size;
	std::string	_body;


	public:
		CGI(HttpParser &request, std::string abs_path, int epfd);
		~CGI(void);
		CGI(CGI &obj) = delete;
		CGI	operator=(CGI &obj) = delete;
		bool		parse_cgi_headers(void);
		bool		execute_cgi_proc(const std::string& abs_path);
		state		handle_cgi_proc();
};

#endif // !CGI_H
