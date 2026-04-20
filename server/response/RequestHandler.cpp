#include "RequestHandler.hpp"

RequestHandler::RequestHandler(const HttpParser &request, cfg::srvCfg &config)
	: _request(request), _config(config), _serverConfig(nullptr), _locationConfig(nullptr)
{
	try {
		resolveVHost();
		resolveLocation();
		methodLocationValidation();
		pathResolution();
		filesystemInspection();
	}
	catch (RequestHandler::MethodNotAllowed &e)
	{
		Logger::log<LogLevel::error>("RequestHandler: ", e.what());
		//Generate matching error 
	}
	catch (std::exception &e)
	{
		Logger::log<LogLevel::error>("RequestHandler: ", e.what());
	}
	catch (...)
	{
		Logger::log<LogLevel::error>("RequestHandler: catched undefined error!");
	}
}

RequestHandler::~RequestHandler(void)
{}

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
	std::map<std::string, std::string>::const_iterator matchPair;
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
		loc_root.substr(1); // ./html -> /html for path concat
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


/*
 * Exceptions
*/

const char	*RequestHandler::MethodNotAllowed::what() const noexcept
{
	return "method not allowed";
}




