#include "CGI.hpp"
#include <cctype>

CGI::CGI(HttpParser &request, std::string cgi_exec_path, const std::string &abs_path, int epfd, int socket_fd)
	: _request(request), _exec_path(cgi_exec_path), _abs_path(abs_path), _cgi_pid(-1), _pipe_read(-1), _epfd(epfd), \
	_socket_fd(socket_fd), _body_size(0)
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

std::pair<cgi_state, int>	CGI::handle_cgi_proc()
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
					for (auto &a : _cgi_header)
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

static const std::string *find_header_case_insensitive(const std::map<std::string, std::string> &headers,
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

static std::vector<std::string> build_cgi_env(const HttpParser &request, const std::string &abs_path)
{
	std::vector<std::string> env_storage;
	const std::map<std::string, std::string> headers = request.getHeadersMap();
	env_storage.reserve(headers.size());

	for (const auto &header : headers)
	{
		std::string cgi_header_key = cgi_token(header.first);
		if (cgi_header_key == "CONTENT_LENGTH" || cgi_header_key == "CONTENT_TYPE")
			continue;
		push_env(env_storage, "HTTP_" + cgi_header_key, header.second);
	}
	const std::string *content_length = find_header_case_insensitive(headers, "content-length");
	const std::string *content_type = find_header_case_insensitive(headers, "content-type");
	std::string script_name = request.getRequestUri();
	if (script_name.empty())
		script_name = "/";

	push_env(env_storage, "REQUEST_METHOD", request.getMethodAsString());
	push_env(env_storage, "REQUEST_URI", request.getRequestUri());
	push_env(env_storage, "QUERY_STRING", request.getRequestQuery());
	push_env(env_storage, "SCRIPT_NAME", script_name);
	push_env(env_storage, "SERVER_PROTOCOL", request.getHttpVersion());
	push_env(env_storage, "GATEWAY_INTERFACE", "CGI/1.1");
	push_env(env_storage, "SCRIPT_FILENAME", abs_path);
	push_env(env_storage, "REDIRECT_STATUS", "200"); // Important for php-cgi
	if (content_length != NULL)
		push_env(env_storage, "HTTP_CONTENT_LENGTH", *content_length);
	if (content_type != NULL)
		push_env(env_storage, "HTTP_CONTENT_TYPE", *content_type);
	return env_storage;
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
		std::vector<std::string> env_storage = build_cgi_env(_request, _abs_path);
		std::vector<char*> env;
		env.reserve(env_storage.size() + 1);
		for (std::string& entry : env_storage)
			env.push_back(const_cast<char*>(entry.c_str()));
		env.push_back(NULL);
		close(p[0]);
		if (dup2(p[1], STDOUT_FILENO) == -1)
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

size_t	CGI::get_body_size(void)
{
	return _body_size;
}
