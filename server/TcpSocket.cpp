#include "TcpSocket.hpp"

/* Constructors & Destructors */
TcpSocket::TcpSocket(uint16_t port, const cfg::socketCfg& config) : _config(config)
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == -1)
		throw SocketError();
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = INADDR_ANY;
	_sockAddr.sin_port = htons(port);
	int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // | SO_KEEPALIVE // protecten?
	if (bind(_socket, reinterpret_cast<sockaddr*>(&_sockAddr), sizeof(_sockAddr)) == -1)
		throw SocketError();
	if (listen(_socket, _maxRequests) == -1)
		throw SocketError();
}

TcpSocket::TcpSocket(TcpSocket&& src) noexcept : _config(src._config)
{
	_socket = src._socket;
	_sockAddr = src._sockAddr;
	src._socket = -1;
}

TcpSocket::~TcpSocket()
{
	if (_socket != -1)
		close(_socket);
}

/* Member Functions */
int TcpSocket::acceptClient()
{
	int fd = accept(_socket, nullptr, nullptr);
	if (fd == -1)
		throw AcceptFailed();
	// int flags = fcntl(fd, F_GETFL, 0);
	// if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	// {
	// 	close(fd);
	// 	throw AcceptFailed();
	// }
	return (fd);
}

/* Basic Operators */

/* Getters & Setters */
int TcpSocket::getSocketFd() const
{
	return (_socket);
}

sockaddr_in TcpSocket::getSocketAddr() const
{
	return (_sockAddr);
}

const cfg::socketCfg& TcpSocket::getConfig() const
{
	return (_config);
}

/* Member Class */
const char* TcpSocket::SocketError::what() const noexcept
{
	return (strerror(errno));
}

const char* TcpSocket::AcceptFailed::what() const noexcept
{
	return ("An error occurred while accepting the client.");
}
