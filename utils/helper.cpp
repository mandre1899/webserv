#include "helper.hpp"

//Data storage for mime types
static const std::unordered_map<std::string, std::string> mime_types =
{
	{".html", "text/html"},
	{".txt", "text/plain"},
	{".css", "text/css"},
	{".js", "application/javascript"},
	{".json", "application/json"},
	{".xml", "application/xml"},
	{".png", "image/png"},
	{".jpg", "image/jpeg"},
	{".gif", "image/gif"},
	{".svg", "image/svg+xml"},
	{".ico", "image/vnd.microsoft.icon"},
	{".woff", "font/woff"},
	{".woff2", "font/woff2"},
	{".ttf", "font/ttf"},
	{".otf", "font/otf"},
	{".mp4", "video/mp4"},
	{".mp3", "audio/mpeg"},
	{".wav", "audio/wav"},
	{".pdf", "application/pdf"},
	{".zip", "application/zip"},
	{".tar", "application/x-tar"},
	{".gz", "application/gzip"},
	{".py", "text/x-python"},
	{".php", "text/html"},
	{"notype", "application/octet-stream"}
};

std::string buildMsg(std::string msg, const filePos& pos)
{
	msg += " in " + std::string(pos.path) + ":" + std::to_string(pos.ln) + ":" + std::to_string(pos.col);
	return (msg);
}

std::string normalizePath(const std::string& path, const filePos& pos)
{
	size_t posToFind = 1;
	std::string ret = path;

	if (ret.find("../", 0) != std::string::npos)
		throw std::runtime_error(buildMsg("\"" + path + "\" is not a valid path", pos));;
	while (true)
	{
		size_t pos = ret.find("./", posToFind);
		if (pos == std::string::npos)
			break;
		posToFind = pos + 1;
		if (pos > 0 && ret[pos - 1] != '/')
			continue;
		ret.erase(pos, 1);
	}
	for (size_t i = 0; i < ret.size(); i++)
	{
		if (ret[i] == '/')
		{
			if (i >= 1 && ret[i - 1] == '/')
				ret.erase(--i, 1);
		}
	}
	return (ret);
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
		++it;
	if (!s.empty() && it == s.end())
		return true;
	return false;
}

/*
 * Mateo added
*/
//Removes leading / (not when its the first part of the path) and trailing
std::string normalize_part(const std::string& part, bool is_first)
{
	if (part.empty())
		return "";
	size_t start = 0;
	size_t end = part.size();
	if (!is_first)
	{
		while (start < end && part[start] == '/')
			++start;
	}
	while (end > start && part[end - 1] == '/')
		--end;
	return part.substr(start, end - start);
}

std::string init_root_path(void)
{
	std::filesystem::path tmpPathWebserv = std::string(g_execName);

	std::string abs = std::filesystem::absolute(tmpPathWebserv).string();

	std::size_t pos = abs.find('.');
	if (pos != std::string::npos)
		abs.erase(pos);
	abs.erase(abs.find_last_of("/"));
	return abs;
}

std::string	smart_filetype_detection(std::string& path) // Detects the content type of the file referenced by the path passed as argument
{
	size_t pos = path.find(".");
	if (pos == std::string::npos)
		return std::string();
	std::string filetype = path.substr(pos);
	auto res = mime_types.find(filetype);
	if (res == mime_types.end())
		return mime_types.at("notype");
	return res->second;
}

std::string str_tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);
	return s;
}

void trim(std::string& s)
{
	auto start = s.find_first_not_of(" \t\r\n");
	auto end = s.find_last_not_of(" \t\r\n");

	if (start == std::string::npos) {
		s.clear();
		return;
	}

	s = s.substr(start, end - start + 1);
}
