#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <unistd.h>
#include <netinet/in.h> // Needed to use struct sockaddr_in
#include <sys/socket.h> // Needed for socket creating and binding
#include <fcntl.h>
#include <exception>
#include <string.h>
#include <errno.h>
#include "utils/Logger.hpp"
#include "config/Config.hpp"

class TcpSocket {
private:
	int _socket;
	sockaddr_in _sockAddr{};
	static const u_int16_t _maxRequests = 10;
	const cfg::srvCtxCfg& _srvCfg;

public:
	/* Constructors & Destructors */
	TcpSocket() = delete;
	TcpSocket(in_addr_t serverAddress, uint16_t port, const cfg::srvCtxCfg& srv);
	TcpSocket(const TcpSocket& src) = delete;
	TcpSocket(TcpSocket&& src) noexcept;
	~TcpSocket();

	/* Member Functions */
	int acceptClient();

	/* Basic Operators */
	TcpSocket& operator=(const TcpSocket& src) = delete;
	TcpSocket& operator=(TcpSocket&& src) noexcept = delete;

	/* Getters & Setters */
	int getSocketFd() const;
	sockaddr_in getSocketAddr() const;
	const cfg::srvCtxCfg& getConfig() const;

	/* Member Class */
	class SocketError : public std::exception
	{
	public:
		virtual const char* what() const noexcept override;
	};

	class AcceptFailed : public std::exception
	{
	public:
		virtual const char* what() const noexcept override;
	};
};

#endif // SOCKET_HPP