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

/*
 * Mateo added
*/
//Removes leading / (not when its the first part of the path) and trailing
static std::string normalize_part(const std::string& part, bool is_first)
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

//Concats n amounts of path args
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
		if (!result.empty())
			result += "/";
		result.append(cleaned);
		first = false;
	}
	return result;
}

