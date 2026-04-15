#ifndef STATE_MACHINES_H
# define STATE_MACHINES_H

#define CGI_TIMEOUT_TIME 30

enum class Status {
	recive,
	send,
	wait,
	close
};

enum class state {
	parsing_request_line = 0,
	parsing_headers,
	parsing_done,
	response_proccessing,
	response_body,
	response_sending,
	error
};

#endif // !STATE_MACHINES_H
