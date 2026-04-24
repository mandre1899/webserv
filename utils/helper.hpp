#ifndef HELPER_HPP
#define HELPER_HPP

#include <stddef.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

extern const char* g_execName;

struct filePos
{
	const char* path; // file path
	size_t ln; // line
	size_t col; // column
};

std::string buildMsg(std::string msg, const filePos& pos);
std::string normalizePath(const std::string& path, const filePos& pos);
bool is_number(const std::string& s);

#include <iostream>
//Mateo added
//Concats n amounts of path args
std::string normalize_part(const std::string& part, bool is_first);
template <typename... Args>
std::string join_paths(const Args&... parts)
{
	std::vector<std::string> vec = { parts... };
	std::string result;
	bool first = true;
	for (const auto& part : vec)
	{
		std::string cleaned = normalize_part(part, first);
		if (cleaned.empty())
			continue;
		if (!result.empty() && result != "/")
			result += "/";
		result.append(cleaned);
		first = false;
	}
	return result;
}
std::string init_root_path(void);
std::string	smart_filetype_detection(std::string& path);
std::string str_tolower(std::string s);
void trim(std::string& s);

#endif // HELPER_HPP


