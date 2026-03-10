#include "webserv.hpp"

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& tokens)
{
	for (size_t i = 0; i < (tokens.size() - 1); i++)
	{
		os << "\"" << tokens[i] << "\", ";
	}
	os << tokens[tokens.size() - 1] << std::endl;
	return (os);
}

std::ostream& operator<<(std::ostream& os, const struct httpConf& httpConf)
{
	os << "---httpConf---\n";
	os << "root: " << httpConf.root << std::endl;
	os << "index: " << httpConf.index << std::endl;
	os << "autoindex: " << std::boolalpha << httpConf.autoIndex << std::endl;
	if (!httpConf.errorPage.has_value())
		os << "error_page { code: " << httpConf.errorPage->code << " path: " << httpConf.errorPage->path << " }" << std::endl;
	else
		os << "error_page: {}" << std::endl;
	os << "client_header_buffer_size: " << httpConf.clientHeaderBufferSize << std::endl;
	os << "client_header_timeout: " << httpConf.clientHeaderTimeout << std::endl;
	os << "client_max_body_size: " << httpConf.clientMaxBodySize << std::endl;
	os << "client_body_timeout: " << httpConf.clientBodyTimeout << std::endl;
	return (os);
}

std::ostream& operator<<(std::ostream& os, const std::vector<serverConfig>& serverConfs)
{
	os << "---serverConfs---\n";

	for (size_t i = 0; i < serverConfs.size(); i++)
	{
		os << static_cast<httpConf>(serverConfs[i]);
		// os << "root: " << serverConfs[i].root << std::endl;
		// os << "index: " << serverConfs[i].index << std::endl;
		// os << "autoindex: " << std::boolalpha << serverConfs[i].autoIndex << std::endl;
		// if (!serverConfs[i].errorPage.has_value())
		// 	os << "error_page { code: " << serverConfs[i].errorPage->code << " path: " << serverConfs[i].errorPage->path << " }" << std::endl;
		// else
		// 	os << "error_page: {}" << std::endl;
		// os << "client_header_buffer_size: " << serverConfs[i].clientHeaderBufferSize << std::endl;
		// os << "client_header_timeout: " << serverConfs[i].clientHeaderTimeout << std::endl;
		// os << "client_max_body_size: " << serverConfs[i].clientMaxBodySize << std::endl;
		// os << "client_body_timeout: " << serverConfs[i].clientBodyTimeout << std::endl;
	}
}