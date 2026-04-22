#ifndef SERVERTYPES_HPP
#define SERVERTYPES_HPP

#include <variant>

#define CGI_TIMEOUT_TIME 30

#ifndef STATEMACHINES_CLASSDEFINE
#define STATEMACHINES_CLASSDEFINE

class Client;
class TcpSocket;

#endif



enum class state {
	parsing_request_line = 0,
	parsing_headers,
	parsing_done,
	response_proccessing,
	response_body,
	response_sending,
	error
};

enum class FdKind {
	SERVER_SOCKET,
	CLIENT_SOCKET,
	CGI_STDIN_WRITE,
	CGI_STDOUT_READ,
	CGI_STDERR_READ
};

struct FdContext {
	int fd;
	FdKind kind;
	std::variant<Client*, TcpSocket*> socket;
};

#endif // SERVERTYPES_HPP
