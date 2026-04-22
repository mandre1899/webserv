#ifndef CGI_SERVICE_H
# define CGI_SERVICE_H

#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <cctype>

#include "server/response/ResponseStructs.hpp"
#include "typedefs.hpp"
#include "utils/Logger.hpp"
#include "server/serverTypes.hpp"

enum cgi_state {
	FAILED,
	RUNNING,
	FINISHED_OK,
	FINISHED_KO
};

class CgiService
{
	std::string					_bin_exec_path;
	const std::string& _script_path;
	int							_cgi_pid;
	int							_pipe_read;
	int							_pipe_write;
	int							_epfd;
	int							_socket_fd;
	std::string					_body; //Temporary CGI body
	std::vector<char*>			_cgi_envs; // They get created at initialisation
	header_map					_cgi_headers;


public:
	CgiService(ResourceInspection& data, int epfd, int socket_fd);
	~CgiService(void);
	CgiService(CgiService& obj) = delete;
	CgiService	operator=(CgiService& obj) = delete;
	bool						parse_cgi_headers(void);
	cgi_state   				execute_cgi_proc();
	std::pair<cgi_state, int>	handle_cgi_proc();

	//GETTERS
	std::string										get_body(void);
	std::unordered_map<std::string, std::string>	get_header_map(void);
};

#endif // !CGI_H
