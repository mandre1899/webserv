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
#include <unordered_map>
#include <vector>

enum cgi_state {
	FAILED,
	RUNNING,
	FINISHED_OK,
	FINISHED_KO
};

class CGI
{
	HttpParser										&_request;
	std::string										_exec_path;
	const std::string&								_abs_path;
	int												_cgi_pid;
	int												_pipe_read;
	int												_epfd;
	int												_socket_fd;
	int												_body_size;
	std::string										_body;
	std::unordered_map<std::string, std::string>	_cgi_header;


	public:
		CGI(HttpParser &request, std::string cgi_exec_path, const std::string& abs_path, int epfd, int socket_fd);
		~CGI(void);
		CGI(CGI &obj) = delete;
		CGI	operator=(CGI &obj) = delete;
		bool			parse_cgi_headers(void);
		cgi_state   	execute_cgi_proc();
		cgi_state		handle_cgi_proc();

		//GETTERS
		std::string										get_body(void);
		std::unordered_map<std::string, std::string>	get_header_map(void);
};

#endif // !CGI_H
