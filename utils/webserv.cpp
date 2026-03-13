#include "webserv.hpp"

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& tokens)
{
	for (size_t i = 0; i < (tokens.size() - 1); i++)
	{
		os << "\"" << tokens[i] << "\", ";
	}
	os << "\"" << tokens[tokens.size() - 1] << "\"" << std::endl;
	return (os);
}

std::ostream& operator<<(std::ostream& os, const struct cfg::httpCtx& httpCtx)
{
	os << "---httpCtx---\n";
	os << "root: " << httpCtx.root << std::endl;
	os << "index: " << httpCtx.index << std::endl;
	os << "autoindex: " << std::boolalpha << httpCtx.autoIndex << std::endl;
	if (!httpCtx.errorPage.has_value())
		os << "error_page { code: " << httpCtx.errorPage->code << " path: " << httpCtx.errorPage->path << " }" << std::endl;
	else
		os << "error_page: {}" << std::endl;
	os << "client_header_buffer_size: " << httpCtx.clientHeaderBufferSize << std::endl;
	os << "client_header_timeout: " << httpCtx.clientHeaderTimeout << std::endl;
	os << "client_max_body_size: " << httpCtx.clientMaxBodySize << std::endl;
	os << "client_body_timeout: " << httpCtx.clientBodyTimeout << std::endl;
	return (os);
}

std::ostream& operator<<(std::ostream& os, const std::vector<cfg::serverCtx>& serverConfs)
{
	os << "---serverConfs---\n";

	for (size_t i = 0; i < serverConfs.size(); i++)
	{
		os << static_cast<cfg::httpCtx>(serverConfs[i]);
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
	return os;
}

