#include "RequestHandler.hpp"

static bool	is_cgi_script(const std::string& path)
{
	if (path.find(".php") != std::string::npos)
		return true;
	else if (path.find(".py") != std::string::npos)
		return true;
	return false;
}

static std::string getCgiExecutor(const cfg::vHostCfg* serverConfig, const cfg::wsLocationCtx* locationConfig)
{
	if (locationConfig != nullptr && !locationConfig->cgiPath.empty())
		return locationConfig->cgiPath;
	if (serverConfig != nullptr && !serverConfig->cgiPath.empty())
		return serverConfig->cgiPath;
	return "";
}

static std::string getUploadStore(const cfg::wsLocationCtx* locationConfig)
{
	if (locationConfig != nullptr && !locationConfig->uploadStore.empty())
		return locationConfig->uploadStore;
	return "";
}

RequestHandler::RequestHandler(const HttpParser& request, HttpResponse& response, const cfg::srvCfg& config)
	: _request(request), _response(response), _config(config), _serverConfig(nullptr), _locationConfig(nullptr)
{
	resolveVHost();
	resolveLocation();
	methodLocationValidation();
	std::string abs_path = pathResolution();
	filesystemInspection(abs_path);
}

RequestHandler::~RequestHandler(void)
{
}

ResourceInspection& RequestHandler::get_result()
{
	return _inspection_result;
}

/*
 * Buissness logic start
*/
void	RequestHandler::resolveVHost(void)
{
	const auto& headers = _request.getHeadersMap();
	if (headers.empty() || _config.vHost.empty())
	{
		Logger::log<LogLevel::warn>("Request headers or virtual hosts empty");
		throw RequestHandler::BadRequest();
	}
	header_map::const_iterator matchPair;
	if ((matchPair = headers.find("host")) == headers.end())
	{
		_serverConfig = &_config.vHost[0];
		return;
	}
	std::string hostValue = matchPair->second;
	if (hostValue.empty())
	{
		_serverConfig = &_config.vHost[0];
		return;
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
			uri.compare(0, loc.size(), loc) == 0)
		return true;
	return false;
}

void	RequestHandler::resolveLocation(void)
{
	if (_serverConfig == nullptr)
		throw RequestHandler::BadRequest();
	_base_path = _serverConfig->root;
	if(_base_path.empty())
		_base_path = init_root_path();
	else
	{
		if (_base_path[0] == '.')
			_base_path = join_paths(init_root_path(), _base_path.substr(1));
		else
			_base_path = _serverConfig->root;
	}
	if (_serverConfig->location.empty())
		return ;
	auto best_match = _serverConfig->location.end();
	size_t best_len = 0;

	std::string request_uri = _request.getRequestUri();
	for (auto it = _serverConfig->location.begin(); it != _serverConfig->location.end(); ++it)
	{
		const std::string& location_path = it->first;


		if (!loc_uri_comp(location_path, request_uri))
			continue;

		if (location_path.length() > best_len)
		{
			best_match = it;
			best_len = location_path.length();
		}
	}
	if (best_match != _serverConfig->location.end())
	{
		_location_uri = best_match->first;
		_locationConfig = &best_match->second;
		_inspection_result.currLoc = _locationConfig;
	}
}

void	RequestHandler::methodLocationValidation(void)
{
	if (_locationConfig == nullptr)
		return;
	auto requestMethods = _request.getMethod();
	if ((requestMethods == method_type::GET && !_locationConfig->allowMethods.isGet) ||
		(requestMethods == method_type::POST && !_locationConfig->allowMethods.isPost) ||
		(requestMethods == method_type::DELETE && !_locationConfig->allowMethods.isDelete))
	{
		_response.set_default_reponse(405, _locationConfig, _serverConfig);
		throw RequestHandler::MethodNotAllowed();
	}
}

std::string RequestHandler::pathResolution(void)
{
    if (_serverConfig == nullptr)
        throw RequestHandler::BadRequest();

    if (_locationConfig != nullptr)
    {
        std::string loc_root = _locationConfig->root;
		if (loc_root.empty())
			_base_path = init_root_path();
		else
		{
			if (loc_root[0] == '.')
				_base_path = join_paths(init_root_path(), loc_root.substr(1));
			else
				_base_path = loc_root;
		}
	
        if (!_locationConfig->index.empty()
            && _request.getRequestUri() == _location_uri)
            return join_paths(_base_path, _locationConfig->index);
        std::string rest = _request.getRequestUri().substr(_location_uri.length());

        return join_paths(_base_path, rest);
    }
    if (_request.getRequestUri() == "/" && !_serverConfig->index.empty())
        return join_paths(_base_path, _serverConfig->index);
    return join_paths(_base_path, _request.getRequestUri());
}

void	RequestHandler::populateRequestMetadata(const std::string &abs_path)
{
	_inspection_result.abs_path = abs_path;
	_inspection_result.base_path = _base_path;
	_inspection_result.request_uri = _request.getRequestUri();
	_inspection_result.request_query = _request.getRequestQuery();
	_inspection_result.request_headers = _request.getHeadersMap();
	_inspection_result.request_method = _request.getMethodAsString();

	const auto& headers = _request.getHeadersMap();
	_inspection_result.body_expected = (headers.find("Content-Length") != headers.end() ||
		headers.find("content-length") != headers.end());

	if (_serverConfig)
		_inspection_result.currServer = _serverConfig;
}

void	RequestHandler::handleNoLocationConfig(const std::string &abs_path)
{
	struct stat file_stat;

	if (!inspectFilesystemStats(abs_path, file_stat))
		return;

	inspectResourcePermissions(abs_path);
	classifyAndConfigureResource(abs_path, file_stat);
}

bool	RequestHandler::inspectFilesystemStats(const std::string &abs_path, struct stat &file_stat)
{
	if (stat(abs_path.c_str(), &file_stat) != 0)
	{
		Logger::log<LogLevel::trace>("Resource not found: ", abs_path);
		_inspection_result.status = NOT_FOUND;
		_inspection_result.exists = false;
		_inspection_result.is_file = false;
		_inspection_result.is_directory = false;
		_inspection_result.readable = false;
		_inspection_result.writable = false;
		_inspection_result.executable = false;
		_inspection_result.kind = NONE;
		return false;
	}
	
	_inspection_result.exists = true;
	return true;
}

void	RequestHandler::inspectResourcePermissions(const std::string &abs_path)
{
	_inspection_result.readable = (access(abs_path.c_str(), R_OK) == 0);
	_inspection_result.writable = (access(abs_path.c_str(), W_OK) == 0);
	_inspection_result.executable = (access(abs_path.c_str(), X_OK) == 0);
}

void	RequestHandler::classifyAndConfigureResource(const std::string &abs_path, const struct stat &file_stat)
{
	if (S_ISDIR(file_stat.st_mode))
	{
		_inspection_result.is_directory = true;
		_inspection_result.is_file = false;

		std::string uploadStore = getUploadStore(_locationConfig);
		if (!uploadStore.empty())
		{
			_inspection_result.kind = UPLOAD_DIR;
			_inspection_result.body_required = true;
			_inspection_result.upload_path = join_paths(_base_path, uploadStore);
			_inspection_result.status = (_inspection_result.writable) ? OK : FORBIDDEN;
		}
		else
		{
			_inspection_result.kind = DIRECTORY;
			_inspection_result.body_required = false;
			if (_locationConfig != nullptr && _locationConfig->autoIndex)
				_inspection_result.status = OK;
			else
				_inspection_result.status = FORBIDDEN;
		}
	}
	else if (S_ISREG(file_stat.st_mode))
	{
		_inspection_result.is_file = true;
		_inspection_result.is_directory = false;
		
		if (is_cgi_script(abs_path))
		{
			std::string cgiExec = getCgiExecutor(_serverConfig, _locationConfig);
			if (!cgiExec.empty())
			{
				_inspection_result.kind = CGI;
				_inspection_result.cgi_path = join_paths(_base_path, cgiExec);
				_inspection_result.body_required = (_request.getMethod() == method_type::POST);
				_inspection_result.status = (_inspection_result.readable && _inspection_result.executable) ? OK : FORBIDDEN;
			}
			else
			{
				_inspection_result.kind = IS_FILE;
				_inspection_result.body_required = false;
				_inspection_result.status = (_inspection_result.readable) ? OK : FORBIDDEN;
			}
		}
		else
		{
			_inspection_result.kind = IS_FILE;
			_inspection_result.body_required = false;
			_inspection_result.status = (_inspection_result.readable) ? OK : FORBIDDEN;
		}
	}
	else
	{
		Logger::log<LogLevel::warn>("Ressource not found: ", abs_path);
		_inspection_result.is_file = false;
		_inspection_result.is_directory = false;
		_inspection_result.kind = NONE;
		_inspection_result.status = FORBIDDEN;
		_inspection_result.body_required = false;
	}
}

void	RequestHandler::filesystemInspection(std::string& abs_path)
{
	struct stat file_stat;

	populateRequestMetadata(abs_path);

	if (_locationConfig == nullptr)
	{
		handleNoLocationConfig(abs_path);
		return;
	}

	if (_locationConfig->ret.has_value())
	{
		_inspection_result.has_redirect = true;
		_inspection_result.redirect_code = _locationConfig->ret->code;
		_inspection_result.redirect_target = _locationConfig->ret->param;
		return ;
	}

	// Inspect filesystem stats
	if (!inspectFilesystemStats(abs_path, file_stat))
		return;

	// Inspect permissions and classify resource
	inspectResourcePermissions(abs_path);
	classifyAndConfigureResource(abs_path, file_stat);
}

const cfg::wsLocationCtx* RequestHandler::get_location()
{
	if (_locationConfig != nullptr)
		return _locationConfig;
	return (nullptr);
}

const cfg::vHostCfg*	RequestHandler::get_vHost(void)
{
	if (_serverConfig != nullptr)
		return _serverConfig;
	return nullptr;
}

/*
 * Exceptions
*/

const char* RequestHandler::MethodNotAllowed::what() const noexcept
{
	return "method not allowed";
}

const char* RequestHandler::PathResolutionFailed::what() const noexcept
{
	return "path resolution failed";
}

const char* RequestHandler::BadRequest::what() const noexcept
{
	return "bad request";
}

const char* RequestHandler::NotFoundExcept404::what() const noexcept
{
	return "No index or root configured as well as no location";
}

