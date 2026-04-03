#ifndef HTTP_RESPONSE_H
# define HTTP_RESPONSE_H

#include <unistd.h>
#include <sstream>
#include "./HttpParser.hpp"
#include "config/configTypes.hpp"
#include "config/Config.hpp"

enum filetype {
	STATIC_FILE,
	DYNAMIC_FILE,
	UNKNOWN
};

class HttpResponse
{
	private:
		int                                 _status_code;
		std::string                         _http_version;
		std::string                         _content_type;
		std::map<std::string, std::string>  _headers;
		std::string                         _body;
		bool                                _is_built;
		const HttpParser&                   _request;
		const cfg::srvCtxCfg&				_config;

		const static std::map<std::string, std::string>  _mime_types;
		const static std::map<int, std::string>          _status_messages;

		// Helper functions
		std::string     clean_string(const std::string& path);
		std::string     get_status_message(int code) const;
		std::string     get_file_extension(const std::string& path) const;

		// Path resolution
		std::pair<int, std::string> resolve_path(void);
		std::string                 resolve_content_type(const std::string& abs_path);
		filetype                    resolve_filetype(const std::string& abs_path);

		// Response building
		void            set_default_headers(void);
		void            handle_static_file(const std::string& file_path);
		void            handle_cgi_request(const std::string& script_path);
		void            handle_error(int error_code);
		std::string     read_file(const std::string& file_path);

		int         get_status_code(void) const;
		std::string get_body(void) const;
		bool        is_built(void) const;

	public:
		HttpResponse(void) = delete;
		HttpResponse(const HttpParser &req, const cfg::srvCtxCfg& conf);
		HttpResponse(HttpParser &req, const cfg::srvCtxCfg& conf);
		~HttpResponse(void);

		void        build(void);
		std::string serialize(void) const;

		// Setters
		void        set_status_code(int code);
		void        set_header(const std::string& key, const std::string& value);
		void        set_body(const std::string& body);

		// Getters
};

#endif // !HTTP_RESPONSE_H
