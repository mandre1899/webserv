#include "CgiService.hpp"
#include "server/Client.hpp"
#include <fcntl.h>

//CGI ENV Helpers
static std::string to_lower(std::string value)
{
	for (char &c : value)
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return value;
}

static std::string cgi_token(const std::string &token)
{
	std::string out;
	out.reserve(token.size());
	for (char c : token)
	{
		const unsigned char xc = static_cast<unsigned char>(c);
		if (std::isalnum(xc))
			out.push_back(static_cast<char>(std::toupper(xc)));
		else
			out.push_back('_');
	}
	return out;
}

static const std::string *find_header_case_insensitive(header_map &headers,
	const std::string &key_lower)
{
	for (const auto &header : headers)
	{
		if (to_lower(header.first) == key_lower)
			return &header.second;
	}
	return NULL;
}

static void push_env(std::vector<std::string> &env_storage, const std::string &key, const std::string &value)
{
	env_storage.push_back(key + "=" + value);
}

static std::vector<std::string> build_cgi_env(ResourceInspection &data)
{
	std::vector<std::string> env_storage;
	env_storage.reserve(data.request_headers.size());

	for (const auto &header : data.request_headers)
	{
		std::string cgi_header_key = cgi_token(header.first);
		if (cgi_header_key == "CONTENT_LENGTH" || cgi_header_key == "CONTENT_TYPE")
			continue;
		push_env(env_storage, "HTTP_" + cgi_header_key, header.second);
	}
	const std::string *content_length = find_header_case_insensitive(data.request_headers, "content-length");
	const std::string *content_type = find_header_case_insensitive(data.request_headers, "content-type");

	push_env(env_storage, "REQUEST_METHOD", data.request_method);
	push_env(env_storage, "REQUEST_URI", data.request_uri);
	push_env(env_storage, "QUERY_STRING", data.request_query); 
	//push_env(env_storage, "SCRIPT_NAME", script_name);
	//push_env(env_storage, "SERVER_PROTOCOL", data.request_version);.
	push_env(env_storage, "SCRIPT_FILENAME", data.abs_path);
	push_env(env_storage, "REDIRECT_STATUS", "200"); // Important for php-cgi
	if (content_length != NULL)
		push_env(env_storage, "HTTP_CONTENT_LENGTH", *content_length);
	if (content_type != NULL)
		push_env(env_storage, "HTTP_CONTENT_TYPE", *content_type);
	return env_storage;
}

CgiService::CgiService(ResourceInspection &data, int epfd, int socket_fd)
	: _data(data), _cgi_pid(-1), _pipe_read(-1), _pipe_write(-1), _epfd(epfd), \
	_socket_fd(socket_fd)
{
	std::vector<std::string> env_storage = build_cgi_env(data);
	_cgi_envs.reserve(env_storage.size() + 1);
	for (std::string& entry : env_storage)
		_cgi_envs.push_back(const_cast<char*>(entry.c_str()));
	_cgi_envs.push_back(NULL);
}

CgiService::~CgiService()
{
	if (_pipe_read != -1)
		close(_pipe_read);
	if (_cgi_pid != -1 )
		kill(_cgi_pid, SIGKILL);
	if (_pipe_write != -1)
		close(_pipe_write);

	for (auto &a : _ctxTrack)
		delete a;
}

bool	CgiService::parse_cgi_headers(void)
{
	size_t header_end = std::string::npos;
	size_t delimiter_len = 0;

	//Evaluate differnt header endings
	header_end = _body.find("\r\n\r\n");
	if (header_end != std::string::npos)
		delimiter_len = 4;
	else
	{
		header_end = _body.find("\n\n");
		if (header_end != std::string::npos)
			delimiter_len = 2;
		else
			return false;
	}
	std::string header_block = _body.substr(0, header_end);
	std::string parsed_body = _body.substr(header_end + delimiter_len);
	std::istringstream header_stream(header_block);
	std::string line;

	while (std::getline(header_stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t sep = line.find(':');
		if (sep == std::string::npos)
			return false;

		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);

		_cgi_headers[key] = value;
	}

	_body = parsed_body;
	return true;
}

std::pair<cgi_state, int>	CgiService::handle_cgi_read()
{
	char buf[4096];
	while (true)
	{
		ssize_t read_bytes = read(_pipe_read, buf, sizeof(buf));
		if (read_bytes > 0)
		{
			Logger::log<LogLevel::info>("Read ", read_bytes, " bytes from CGI pipe");
			_body.append(buf, static_cast<std::size_t>(read_bytes));
			continue;
		}
		if (read_bytes == 0)
		{
			epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_read, NULL);
			close(_pipe_read);
			_pipe_read = -1;
			int status = 0;
			pid_t ret = waitpid(_cgi_pid, &status, WNOHANG);
			if (ret == -1)
			{
				Logger::log<LogLevel::error>("waitpid failed for CGI child");
				_cgi_pid = -1;
				return std::make_pair(cgi_state::FINISHED_KO, 500);
			}
			if (ret > 0)
			{
				_cgi_pid = -1;
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				{
					if (!parse_cgi_headers()) // Parses the header from cgi to HTTP headers
					{
						Logger::log<LogLevel::error>("CGI script didn't produced HTTP header");
						return std::make_pair(cgi_state::FINISHED_KO, 500);
					}
					std::cout << "CGI HEADERS" << std::endl;
					for (auto &a : _cgi_headers)
						std::cout << a.first + "=" + a.second << std::endl;
					return std::make_pair(cgi_state::FINISHED_OK, 200);
				}
				Logger::log<LogLevel::error>("CGI exited with status: ", WEXITSTATUS(status));
				return std::make_pair(cgi_state::FINISHED_KO, 500);
			}
			return std::make_pair(cgi_state::FINISHED_OK, 200);
		}
		Logger::log<LogLevel::error>("Read failed status: ", errno);
		epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_read, NULL);
		close(_pipe_read);
		_pipe_read = -1;
		_cgi_pid = -1;
		return std::make_pair(cgi_state::FINISHED_KO, 500);
	}
}

std::pair<cgi_state, int>	CgiService::handle_cgi_write(const std::string& body_data)
{
	ssize_t written_bytes = write(_pipe_write, body_data.c_str(), body_data.size());
	if (written_bytes < 0)
	{
		Logger::log<LogLevel::error>("Write to CGI pipe failed: ", errno);
		epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_write, NULL);
		close(_pipe_write);
		_pipe_write = -1;
		return std::make_pair(cgi_state::FINISHED_KO, 500);
	}

	if (written_bytes == static_cast<ssize_t>(body_data.size()))
	{
		Logger::log<LogLevel::info>("Wrote ", written_bytes, " bytes to CGI pipe");
		epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_write, NULL);
		close(_pipe_write);
		_pipe_write = -1;
		return std::make_pair(cgi_state::RUNNING, 200);
	}

	Logger::log<LogLevel::warn>("Partial write to CGI: ", written_bytes, " out of ", body_data.size());
	return std::make_pair(cgi_state::RUNNING, 200);
}

cgi_state	CgiService::execute_cgi_proc(Client *currClient)
{
	const std::string execLocPath = _data.cgi_path;
	char* execPath = NULL;
	char* scriptPath = const_cast<char*>(_data.abs_path.c_str());
	int out[2] = {-1, -1};
	int in[2] = {-1, -1};
	bool	expectedBody = _data.body_expected;

	//set_status_code(200);
	execPath = const_cast<char*>(execLocPath.c_str());
	if (pipe(out) == -1)
	{
		Logger::log<LogLevel::error>("Pipe creation failed");
		return cgi_state::FAILED;
	}
	if (expectedBody)
	{
		if (pipe(in) == -1)
		{
			close(out[0]);
			close(out[1]);
			Logger::log<LogLevel::error>("Pipe creation failed");
			return cgi_state::FAILED;
		}
	}
	int pid = fork();
	if (pid < 0)
	{
		Logger::log<LogLevel::error>("fork() failed");
		if (in[0] != -1)
			close(in[0]);
		if (in[1] != -1)
			close(in[1]);
		close(out[0]);
		close(out[1]);
		return cgi_state::FAILED;
	}
	if (pid == 0)
	{
		char* const argv[] = { execPath, scriptPath, NULL };
		if (expectedBody)
		{
			close(in[1]);
			if (dup2(in[0], STDIN_FILENO) == -1)
				_exit(1);
			close(in[0]);
		}
		close(out[0]);
		if (dup2(out[1], STDOUT_FILENO) == -1)
			_exit(1);
		if (dup2(out[1], STDERR_FILENO) == -1)
			_exit(1);
		close(out[1]);
		execve(execPath, argv, _cgi_envs.data());
		Logger::log<LogLevel::error>("Execve failed");
		_exit(1);
	}
	close(out[1]);
	set_epoll_event(EPOLLIN | EPOLLRDHUP, out[0], FdKind::CGI_STDOUT_READ, currClient);
	if (expectedBody)
	{
		_pipe_write = in[1];
		set_epoll_event(EPOLLOUT | EPOLLRDHUP, in[1], FdKind::CGI_STDIN_WRITE, currClient);
	}
	else
	{
		if (in[0] != -1)
			close(in[0]);
	}
	_cgi_pid = pid;
	_pipe_read = out[0];
	return cgi_state::RUNNING;
}

void		CgiService::set_epoll_event(uint32_t events, int fd, FdKind kind, Client *currClient)
{
	epoll_event	ev;
	FdContext	*tmp = new FdContext;
	
	_ctxTrack.push_back(tmp);
	tmp->fd = fd;
	tmp->kind = kind;
	tmp->socket = currClient;
	ev.events = events;
	ev.data.fd = fd;
	ev.data.ptr = (void *)tmp;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd , &ev) == -1)
	{
		Logger::log<LogLevel::error>("Epoll assignment of pipe read failed");
		close(fd);
	}
}

std::string	CgiService::get_body(void)
{
	return _body;	
}

header_map	CgiService::get_header_map(void)
{
	return _cgi_headers;
}

