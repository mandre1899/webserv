#ifndef HELPER_HPP
#define HELPER_HPP

#include <stddef.h>
#include <string>
#include <vector>
#include <stdexcept>

struct filePos
{
	const char* path; // file path
	size_t ln; // line
	size_t col; // column
};

std::string buildMsg(std::string msg, const filePos& pos);
std::string normalizePath(const std::string& path, const filePos& pos);
void is_number(const std::string& s);

//Mateo added
template <typename... Args>
std::string join_paths(const Args&... parts);

#endif // HELPER_HPP


