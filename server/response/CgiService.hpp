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
#include <fcntl.h>
#include <vector>
#include <cctype>

#include "server/response/ResponseStructs.hpp"
#include "typedefs.hpp"
#include "utils/Logger.hpp"
#include "server/serverTypes.hpp"

class Client;

enum cgi_state {
	FAILED,
	RUNNING,
	FINISHED_OK,
	FINISHED_KO
};

class CgiService
{
	ResourceInspection& _data;
	int							_cgi_pid;
	int							_pipe_read;
	int							_pipe_write;
	int							_epfd;
	int							_socket_fd;
	std::string					_body; //Temporary CGI body
	std::vector<char*>			_cgi_envs; // They get created at initialisation
	header_map					_cgi_headers;
	std::vector<FdContext*>		_ctxTrack;
	size_t						_postWriteTrack; //tracks the bytest already written to the pipe stdin
	size_t						_postContentLength;

	//Test
	bool		_writeIsDel;
	bool		_readIsDel;

	void		set_epoll_event(uint32_t events, int fd, FdKind kind, Client* currClient);
	void		reactivate_client(struct FdContext *clientCtx);
public:
	CgiService(ResourceInspection& data, int epfd, int socket_fd, size_t client_max_body);
	~CgiService(void);
	CgiService(CgiService& obj) = delete;
	CgiService	operator=(CgiService& obj) = delete;
	bool						parse_cgi_headers(void);
	cgi_state   				execute_cgi_proc(Client* currClient);
	std::pair<cgi_state, int>	handle_cgi_read();
	std::pair<cgi_state, int>	handle_cgi_write(std::string& body_data, struct FdContext *clientCtx);

	//GETTERS
	std::string										get_body(void);
	std::unordered_map<std::string, std::string>	get_header_map(void);

	//Exceptions
	class ExceedesClientMaxBody : public std::exception {
	public:
		const char* what() const noexcept override;
	};
};

#endif // !CGI_H
