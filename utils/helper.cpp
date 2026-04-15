#include "helper.hpp"

std::string buildMsg(std::string msg, const filePos& pos)
{
	msg += " in " + std::string(pos.path) + ":" + std::to_string(pos.ln) + ":" + std::to_string(pos.col);
	return (msg);
}

std::string normalizePath(const std::string& path, const filePos& pos)
{
	std::string ret = path;
	if (ret.back() == '/' && ret.size() > 1)
	{
		ret = ret.substr(0, ret.size() - 1);
		if (ret.back() == '/')
			throw std::runtime_error(buildMsg("\"" + path + "\" is not a valid path", pos));;
	}
	return (ret);
}

void is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
		++it;
	if (!s.empty() && it == s.end())
		return;
	throw std::runtime_error("is_number: invalid number found");
}