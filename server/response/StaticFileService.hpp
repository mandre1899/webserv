#ifndef STATIC_FILE_SERVICE_H
#define STATIC_FILE_SERVICE_H

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>

#include "HttpResponse.hpp"
#include "ResponseStructs.hpp"
#include "ResponseStructs.hpp"

class StaticFileService
{
	ResourceInspection	_data;
	HttpResponse		&_response;

	// Buissness logic intern
	void	generate_dir_listing();
	void	delete_ressource();
	public:
		StaticFileService(ResourceInspection data, HttpResponse &response);
		~StaticFileService(void);
};

#endif // !STATIC_FILE_SERVICE_H
