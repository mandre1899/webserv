#ifndef STATE_MACHINES_H
# define STATE_MACHINES_H

enum class Status {
	recive,
	send,
	wait,
	close
};

enum class state {
	parsing_request_line = 0,
	parsing_headers,
	parsing_body,
	parsing_done,
	response_proccessing,
	response_sending,
	error
};

#endif // !STATE_MACHINES_H
