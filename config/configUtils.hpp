#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include <stddef.h>
#include <string>
#include <stdexcept>

struct filePos
{
	const char* path; // file path
	size_t ln; // line
	size_t col; // column
};

std::string buildMsg(std::string msg, const filePos& pos);
std::string normalizePath(const std::string& path, const filePos& pos);

#endif // CONFIGUTILS_HPP


