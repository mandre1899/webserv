/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpSocket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:23:30 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/16 13:23:53 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <TcpSocket.hpp>

/* Constructors & Destructors */
TcpSocket::TcpSocket(in_addr_t serverAddress, uint16_t port)
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == -1)
		throw SocketError();
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = serverAddress;
	_sockAddr.sin_port = htons(port);
	int opt = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 	// | SO_KEEPALIVE // protecten?
	if (bind(_socket, reinterpret_cast<sockaddr*>(&_sockAddr), sizeof(_sockAddr)) == -1)
		throw SocketError();
	if (listen(_socket, _maxRequests) == -1)
		throw SocketError();
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
		throw AcceptFaild();
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(fd);
		throw AcceptFaild();
	}
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

/* Member Class */
const char* TcpSocket::SocketError::what() const noexcept
{
	return (strerror(errno));
}

const char* TcpSocket::AcceptFaild::what() const noexcept
{
	return ("An error occurred while accepting the client.");
}
