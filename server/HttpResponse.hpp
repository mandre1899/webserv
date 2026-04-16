#ifndef HTTP_RESPONSE_H
# define HTTP_RESPONSE_H

#include <unistd.h>
#include <sstream>
#include "./HttpParser.hpp"
#include "config/configTypes.hpp"
#include <sys/wait.h>
#include <cstdlib>
#include <signal.h>
#include <string>
#include "config/Config.hpp"
#include "sys/stat.h"
#include <dirent.h>
#include "StateMachines.hpp"
#include "HttpUpload.hpp"
#include <unistd.h>
#include "CGI.hpp"
#include <filesystem>
#include <memory>

extern const char* g_execName;

enum filetype {
	STATIC_FILE = 0,
	DYNAMIC_FILE,
	UNKNOWN
};

enum class ResolveKind
{
	FILEPATH = 0,
	POST,
	REDIRECT,
	ERRORSTATUS
};

struct ResolveResult
{
	int code;
	ResolveKind kind;
	std::string value;
};

class HttpResponse
{
private:
	int									_status_code;
	std::string							_content_type;
	std::unordered_map<std::string, std::string>  _headers;
	std::string							_body;
	size_t								_body_size;
	bool								_is_built;
	HttpParser* 						_request;
	const cfg::srvCfg* _configs;
	cfg::vHostCfg						_config; //Evalutated vHost
	cfg::wsLocationCtx					_curr_loc;
	std::string							_curr_loc_root;

	const static std::map<std::string, std::string>		_mime_types;
	static const std::map<int, std::string>				_http_status_map;
	const static std::map<int, std::string>         	_status_messages;

	//POST specific
	std::unique_ptr<HttpUpload>			_upload;
	//CGI specific
	std::unique_ptr<CGI>				_cgi;
	int									_socket_fd;
	int									_epfd;
	// Helper functions
	std::string													abstract_filename(const std::string& path);
	std::string													get_status_message(int code) const;
	std::string													get_file_extension(const std::string& path) const;
	bool														read_file_to_body(const std::string& abs_path); //reads the file to the _body member string
	bool														execute_cgi_proc(const std::string& abs_path);
	HttpResponse&												set_content_type(const std::string& type);
	HttpResponse&												add_header(const std::string& key, const std::string& val);
	void														evaluate_vHost(void);
	std::optional<std::pair<std::string, cfg::wsLocationCtx>>	get_most_specific_location(const std::string& uri_path);
	// Path resolution
	struct ResolveResult				resolve_path(void);
	std::string							resolve_content_type(const std::string& abs_path);
	bool								check_path_secure(const std::string& abs_path);
	filetype	 						resolve_filetype(const std::string& file_suffix);

	//Directroy listing
	bool						is_directory(const std::string& path);
	void						generate_dir_listing(const std::string& dir_path, const std::string& uri_path);

	// Response building
	void					set_default_headers(void);
	void					handle_static_file(const std::string& file_path);
	void					handle_cgi_request(const std::string& script_path);
	void					handle_error(int error_code);
	std::string				read_file(const std::string& file_path);
	void					generate_error_page(int status_code);

	//Http methods
	bool			delete_request(const std::string& abs_path);
	bool			get_request(const std::string& abs_path);

	int						get_status_code(void) const;
	std::string				get_body(void) const;
	bool					is_built(void) const;
	bool 					is_path_safe(const std::string& abs_path, const std::string& base_path);
public:
	HttpResponse(void);
	HttpResponse(const HttpResponse& obj) = delete;
	HttpResponse& operator=(const HttpResponse& obj) = delete;
	~HttpResponse(void);

	state					build(HttpParser* req, const cfg::srvCfg* conf, int clientFd, int epfd);
	const std::string		get_response_str(void) const; //Returns the response as string
	state					handle_cgi_proc(void);
	state					upload_body(std::string& data);
	bool					parse_cgi_headers(void);// CGI Helper

	// Setters
	void					set_status_code(int code);
	void					set_header(const std::string& key, const std::string& value);
	void					set_body(const std::string& body);

	// Getters
};

#endif // !HTTP_RESPONSE_H
