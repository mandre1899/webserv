#include "configUtils.hpp"

std::string buildMsg(std::string msg, const filePos& pos)
{
	msg += " in " + std::string(pos.path) + ":" + std::to_string(pos.ln) + ":" + std::to_string(pos.col);
	return (msg);
}