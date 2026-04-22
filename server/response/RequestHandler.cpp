#include "RequestHandler.hpp"

RequestHandler::RequestHandler(const HttpParser &request, const cfg::srvCfg &config)
	: _request(request), _config(config), _serverConfig(nullptr), _locationConfig(nullptr)
{
	resolveVHost();
	resolveLocation();
	methodLocationValidation();
	std::string abs_path = pathResolution();
	filesystemInspection(abs_path);
}

RequestHandler::~RequestHandler(void)
{}

ResourceInspection &RequestHandler::get_result()
{
	return _inspection_result;
}

/*
 * Buissness logic start
*/
void	RequestHandler::resolveVHost(void)
{
	const auto& headers = _request.getHeadersMap();
	if (headers.empty())
	{
		Logger::log<LogLevel::warn>("Request headers empty");
		throw std::logic_error("RequestHandler: request headers empty");
	}
	header_map::const_iterator matchPair;
	if ((matchPair = headers.find("Host")) == headers.end())
	{
		_serverConfig = &_config.vHost[0];
		return ;
	}
	std::string hostValue = matchPair->second;
	if (hostValue.empty())
	{
		_serverConfig = &_config.vHost[0];
		return ;
	}
	//Erase port specifier ":8080" (if present)
	size_t portPrefix = hostValue.find(":");
	if (portPrefix != std::string::npos)
		hostValue.erase(portPrefix);
	auto server = _config.srvNameMap.find(hostValue);
	if (server != _config.srvNameMap.end())
		_serverConfig = &_config.vHost[server->second];
	else
		_serverConfig = &_config.vHost[0];
}

//Improved is also now matching /index.html on / location -> specifix ressource requesting
static bool loc_uri_comp(const std::string& loc, const std::string& uri)
{
	if (loc.empty())
		return false;
	if (loc == "/")
		return !uri.empty() && uri[0] == '/';
	if (uri == loc)
		return true;
	if (uri.size() > loc.size() &&
		uri.compare(0, loc.size(), loc) == 0 &&
		uri[loc.size()] == '/')
		return true;
	return false;
}

void	RequestHandler::resolveLocation(void)
{
	auto best_match = _serverConfig->location.end();
	size_t best_len = 0;

	for (auto it = _serverConfig->location.begin(); it != _serverConfig->location.end(); ++it)
	{
		const std::string& location_path = it->first;

		if (!loc_uri_comp(location_path, _request.getRequestUri()))
			continue;

		if (location_path.length() > best_len)
		{
			best_match = it;
			best_len = location_path.length();
		}
	}
	if (best_match == _serverConfig->location.end())
		_locationConfig = nullptr;
	_location_uri = best_match->first;
	_locationConfig = &best_match->second;
}

void	RequestHandler::methodLocationValidation(void)
{
	auto requestMethods = _request.getMethod();
	if ((requestMethods == method_type::GET && !_locationConfig->allowMethods.isGet) ||
		(requestMethods == method_type::POST && !_locationConfig->allowMethods.isPost) ||
		(requestMethods == method_type::DELETE && !_locationConfig->allowMethods.isDelete))
		throw RequestHandler::MethodNotAllowed();
}

std::string	RequestHandler::pathResolution(void)
{
	if (_serverConfig->location.empty())
	{
		if (!_serverConfig->index.empty() &&
				!_serverConfig->index.empty())
		{
			if (_serverConfig->root[0] == '.')
				return join_paths(init_root_path(), _serverConfig->root.substr(1), _serverConfig->index);
			else
				return join_paths(_serverConfig->root, _serverConfig->index);
		}
		else
		{
		}
	}
	bool	is_abs = true; // ./ is a relative file and / is a absolute
	std::string loc_root = _locationConfig->root;
	if (_locationConfig->root[0] == '.')
	{
		is_abs = false;
		loc_root = loc_root.substr(1); // ./html -> /html for path concat
	}
	//Exact location was requested
	if (!_locationConfig->index.empty() && _request.getRequestUri() == _location_uri)
	{
		if (is_abs == false)
			return join_paths(init_root_path(), loc_root, _locationConfig->index);
		else
			return join_paths(loc_root, _locationConfig->index);
	}
	//When exact file is requested on location
	std::string tmp;
	std::string uri_path = _request.getRequestUri();
	if (is_abs == false)
		return join_paths(init_root_path(), loc_root, uri_path.substr(_location_uri.length()));
	return join_paths(init_root_path(), uri_path.substr(_location_uri.length()));
}

void	RequestHandler::filesystemInspection(std::string &abs_path)
{
	struct stat file_stat;
	
	_inspection_result.abs_path = abs_path;
	_inspection_result.request_uri = _request.getRequestUri();
	_inspection_result.request_query = _request.getRequestQuery();
	_inspection_result.request_headers = _request.getHeadersMap();
	_inspection_result.request_method = _request.getMethodAsString();
	
	// Check if path exists
	if (stat(abs_path.c_str(), &file_stat) != 0)
	{
		_inspection_result.status = NOT_FOUND;
		_inspection_result.exists = false;
		_inspection_result.is_file = false;
		_inspection_result.is_directory = false;
		_inspection_result.readable = false;
		_inspection_result.writable = false;
		_inspection_result.executable = false;
		_inspection_result.kind = NONE;
		return;
	}
	
	_inspection_result.exists = true;
	
	// Check permissions
	_inspection_result.readable = (access(abs_path.c_str(), R_OK) == 0);
	_inspection_result.writable = (access(abs_path.c_str(), W_OK) == 0);
	_inspection_result.executable = (access(abs_path.c_str(), X_OK) == 0);
	
	// Determine if it's a directory or file
	if (S_ISDIR(file_stat.st_mode))
	{
		_inspection_result.is_directory = true;
		_inspection_result.is_file = false;
		
		// Check if this is an upload directory
		if (!_locationConfig->uploadStore.empty())
		{
			_inspection_result.kind = UPLOAD_DIR;
			if (_inspection_result.writable)
			{
				_inspection_result.status = OK;
			}
			else
			{
				_inspection_result.status = FORBIDDEN;
			}
		}
		else
		{
			_inspection_result.kind = DIRECTORY;
			if (_locationConfig->autoIndex)
			{
				_inspection_result.status = OK;
			}
			else
			{
				_inspection_result.status = FORBIDDEN;
			}
		}
	}
	else if (S_ISREG(file_stat.st_mode))
	{
		_inspection_result.is_file = true;
		_inspection_result.is_directory = false;
		
		// Check if this is a CGI script
		if (!_locationConfig->cgiPath.empty())
		{
			_inspection_result.kind = CGI;
			_inspection_result.cgi_path = _locationConfig->cgiPath;
			if (_inspection_result.readable && _inspection_result.executable)
			{
				_inspection_result.status = OK;
			}
			else
			{
				_inspection_result.status = FORBIDDEN;
			}
		}
		else
		{
			_inspection_result.kind = IS_FILE;
			if (_inspection_result.readable)
			{
				_inspection_result.status = OK;
			}
			else
			{
				_inspection_result.status = FORBIDDEN;
			}
		}
	}
	else
	{
		_inspection_result.is_file = false;
		_inspection_result.is_directory = false;
		_inspection_result.kind = NONE;
		_inspection_result.status = FORBIDDEN;
	}
}


/*
 * Exceptions
*/

const char	*RequestHandler::MethodNotAllowed::what() const noexcept
{
	return "method not allowed";
}

