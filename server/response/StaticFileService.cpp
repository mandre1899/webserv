#include "StaticFileService.hpp"

StaticFileService::StaticFileService(ResourceInspection data, HttpResponse &response)
	: _data(data), _response(response)
{	
	if (!data.delete_file)
	{
		if (data.kind == ResourceKind::IS_FILE)
		{
			_response.set_content_type(smart_filetype_detection(data.abs_path));
			response.set_status(200);
			response.read_file_to_body(data.abs_path);
		}
		else if (data.kind == ResourceKind::DIRECTORY)
				generate_dir_listing();
	}
	else
		delete_ressource();
}

StaticFileService::~StaticFileService(void)
{}

void	StaticFileService::delete_ressource()
{
	_response.set_default_headers();

	if (_data.kind == ResourceKind::DIRECTORY)
	{
		if (rmdir(_data.abs_path.c_str()) == 0)
			_response.set_default_reponse(204, _data.currLoc, _data.currServer, _data.base_path);
		else
			_response.set_default_reponse(409, _data.currLoc, _data.currServer, _data.base_path);
	}
	else if (_data.kind == ResourceKind::IS_FILE)
	{
		if (unlink(_data.abs_path.c_str()) == 0)
			_response.set_default_reponse(204, _data.currLoc, _data.currServer, _data.base_path);
		else
			_response.set_default_reponse(500, _data.currLoc, _data.currServer, _data.base_path);
	}
}

//Buisness logic intern
void	StaticFileService::generate_dir_listing()
{
	std::ostringstream ss;

	ss << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "  <meta charset=\"UTF-8\">\n"
		<< "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		<< "  <title>" << _data.request_uri << " - Dir listing</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "  <ul>\n";

	DIR* dir = opendir(_data.abs_path.c_str());
	if (dir != NULL)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			//Skips listing of current dir -> .
			if (name == ".")
				continue;
			std::string full_fs_path = _data.abs_path;
			if (!full_fs_path.empty() && full_fs_path[full_fs_path.size() - 1] != '/')
				full_fs_path += '/';
			full_fs_path += name;
			struct stat st;
			bool is_dir = (stat(full_fs_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
			std::string href = join_paths(_data.request_uri, name);
			if (is_dir && href[href.size() - 1] != '/')
				href += '/';
			ss << "    <li><a href=\"" + href + "\">"
				<< name;
			if (is_dir)
				ss << "/";
			ss << "</a></li>\n";
		}
		closedir(dir);
	}
	else
		ss << "    <li>Could not open directory</li>\n";

	ss << "  </ul>\n"
		<< "</body>\n"
		<< "</html>\n";
	_response.set_content_type("text/html");
	_response.set_default_headers();
	_response.set_status(200);
	_response.set_body(ss.str());
}

