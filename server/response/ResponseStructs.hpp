#ifndef RESPONSE_STRUCTS_H
#define RESPONSE_STRUCTS_H

#include "typedefs.hpp"
#include "../../config/configTypes.hpp"
#include "server/HttpParser.hpp"

#include <string>

enum ResourceKind
{
	NONE,
	IS_FILE,
	DIRECTORY,
	CGI,
	UPLOAD_DIR
};

enum InspectStatus
{
	OK,
	NOT_FOUND,
	FORBIDDEN,
	METHOD_NOT_ALLOWED,
	CGI_NOT_CONFIGURED,
	CGI_INVALID,
	REDIRECT,
	ERROR
};

struct ResourceInspection
{
	InspectStatus    status;
	ResourceKind     kind;

	std::string		request_uri;
	std::string		request_query;
	header_map		request_headers;
	std::string		request_method;

	std::string      abs_path;
	std::string      cgi_path;
	std::string      index_path;

	bool             exists = false;
	bool             is_file;

	bool             is_directory;
	bool             readable; // Maybe this is useful somehow
	bool             writable; // Maybe this is useful somehow
	bool             executable;

	bool             body_required;     // e.g. POST to CGI/upload
	bool             body_expected;     // request says body may come

	// Redirect
	http_code        redirect_code;     // NEW: 301, 302, 307, etc.
	std::string      redirect_target;
};

#endif // !RESPONSE_STRUCTS_H
