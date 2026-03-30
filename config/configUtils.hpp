#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include <stddef.h>
#include <string>

struct filePos
{
	const char* path; // file path
	size_t ln; // line
	size_t col; // column
};

std::string buildMsg(std::string msg, const filePos& pos);

#endif // CONFIGUTILS_HPP


