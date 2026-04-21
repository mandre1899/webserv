#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <memory>
#include <exception>

#include "ResponseStructs.hpp"
#include "utils/Logger.hpp"
#include "server/HttpParser.hpp"
#include "StaticFileService.hpp"
#include "utils/helper.hpp"
#include <sys/stat.h>

class RequestHandler
{
	const HttpParser			&_request;
	const cfg::srvCfg			&_config;			// global config
	const cfg::vHostCfg			*_serverConfig;		// Have to be pointers to instantiate
	const cfg::wsLocationCtx	*_locationConfig;	// them with NULL
	std::string					_location_uri;		// Uri of the location

	// Result of this class
	ResourceInspection			_inspection_result;

	// Buissness logic member functions
	void		resolveVHost(void); // Resolves vHost assigns the _serverConfig
	void		resolveLocation(void); // Resolves the requested location assigns the _locationConfig
	void		methodLocationValidation(void); // Checks the requested method 
												// with the allowed methods of the location
	std::string	pathResolution(void); // Logically resolves the path without checking for 
									  // ressource availablility
	void		filesystemInspection(std::string &abs_path); // Checks given path for ressource availablility
	public:
		RequestHandler(const HttpParser &request, const cfg::srvCfg &config);
		~RequestHandler(void);
		ResourceInspection get_result();

	// Exceptions
	class MethodNotAllowed : public std::exception {
		public:
			const char* what() const noexcept override;
	};
};

#endif // ! REQUEST_HANDLER_H

