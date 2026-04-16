#include "CGI.hpp"

CGI::CGI(HttpParser &request, std::string cgi_exec_path, const std::string &abs_path, int epfd, int socket_fd) : _request(request), _exec_path(cgi_exec_path), _abs_path(abs_path), _epfd(epfd), _socket_fd(socket_fd)
{}

CGI::~CGI()
{
	if (_pipe_read != -1)
		close(_pipe_read);
	if (_cgi_pid != -1 )
		kill(_cgi_pid, SIGKILL);
}

bool	CGI::parse_cgi_headers(void)
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

		_cgi_header[key] = value;
	}

	_body = parsed_body;
	_body_size = _body.size();
	return true;
}

cgi_state	CGI::handle_cgi_proc()
{
	char buf[4096];
	while (true)
	{
		ssize_t read_bytes = read(_pipe_read, buf, sizeof(buf));
		if (read_bytes > 0)
		{
			Logger::log<LogLevel::info>("Read ", read_bytes, " bytes from CGI pipe");
			_body.append(buf, static_cast<std::size_t>(read_bytes));
			_body_size += static_cast<std::size_t>(read_bytes);
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
				return cgi_state::FINISHED_KO;
			}
			if (ret > 0)
			{
				_cgi_pid = -1;
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				{
					if (!parse_cgi_headers()) // Parses the header from cgi to HTTP headers
						return cgi_state::FINISHED_OK;
					return cgi_state::FINISHED_OK;
				}
				Logger::log<LogLevel::error>("CGI exited with status: ", WEXITSTATUS(status));
				return cgi_state::FINISHED_KO;
			}
			return cgi_state::FINISHED_KO;
		}
		Logger::log<LogLevel::error>("Read failed status: ", errno);
		epoll_ctl(_epfd, EPOLL_CTL_DEL, _pipe_read, NULL);
		close(_pipe_read);
		_pipe_read = -1;
		_cgi_pid = -1;
		return cgi_state::FINISHED_KO;
	}
}

static std::vector<std::string> parse_header_to_env(const std::map<std::string, std::string>& headers)
{
	std::vector<std::string> env;
	env.reserve(headers.size());
	for (const auto& a : headers)
		env.push_back(a.first + "=" + a.second);
	return env;
}

cgi_state	CGI::execute_cgi_proc()
{
	const std::string execLocPath = _exec_path;
	char* execPath = NULL;
	char* scriptPath = const_cast<char*>(_abs_path.c_str());
	int p[2];

	//set_status_code(200);
	execPath = const_cast<char*>(execLocPath.c_str());
	if (pipe(p) == -1)
	{
		Logger::log<LogLevel::error>("Pipe creation failed");
		return cgi_state::FAILED;
	}
	int pid = fork();
	if (pid < 0)
	{
		Logger::log<LogLevel::error>("fork() failed");
		close(p[0]);
		close(p[1]);
		return cgi_state::FAILED;
	}
	if (pid == 0)
	{
		char* const argv[] = { execPath, scriptPath, NULL };
		std::vector<std::string> env_storage = parse_header_to_env(_request.getHeadersMap());
		std::vector<char*> env;
		env_storage.push_back("REQUEST_METHOD=" + _request.getMethodAsString());
		env_storage.push_back("REQUEST_URI=" + _request.getRequestUri());
		env_storage.push_back("REDIRECT_STATUS=200");
		env_storage.push_back("SCRIPT_FILENAME=" + _abs_path); // Important for php-cgi!!!
		env.reserve(env_storage.size() + 1);
		for (std::string& entry : env_storage)
			env.push_back(const_cast<char*>(entry.c_str()));
		env.push_back(NULL);
		close(p[0]);
		if (dup2(p[1], STDOUT_FILENO) == -1)
			_exit(1);
		if (dup2(p[1], STDERR_FILENO) == -1)
			_exit(1);
		close(p[1]);
		execve(execPath, argv, env.data());
		Logger::log<LogLevel::error>("Execve failed");
		_exit(1);
	}
	close(p[1]);
	epoll_event	ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.fd = _socket_fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, p[0], &ev) == -1)
	{
		Logger::log<LogLevel::error>("Epoll assignment of pipe read failed");
		close(p[0]);
		return cgi_state::FAILED;
	}
	_cgi_pid = pid;
	_pipe_read = p[0];
	return cgi_state::RUNNING;
}

std::string	CGI::get_body(void)
{
	return _body;	
}

std::unordered_map<std::string, std::string>	CGI::get_header_map(void)
{
	return _cgi_header;
}

