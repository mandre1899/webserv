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
		throw SocketCreationError();
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = serverAddress;
	_sockAddr.sin_port = htons(port);
	isActive = true;
}

TcpSocket::TcpSocket(const TcpSocket& src) : TcpSocket(src._sockAddr.sin_addr.s_addr, ntohs(src._sockAddr.sin_port))
{
}

TcpSocket::~TcpSocket()
{
	if (isActive)
		close(_socket);
}

/* Member Functions */
void TcpSocket::sBind()
{
	if (!isActive)
		throw SocketIsClosedError();
	bind(_socket, reinterpret_cast<sockaddr *>(&_sockAddr), sizeof(_sockAddr));
}

void TcpSocket::sListen(int maxConnections)
{
	if (!isActive)
		throw SocketIsClosedError();
	listen(_socket, maxConnections);
}

int TcpSocket::sAccept()
{
	if (!isActive)
		throw SocketIsClosedError();
	return(accept(_socket, nullptr, nullptr));
}

void TcpSocket::sRecv(int clientFd, char *buf, size_t size)
{
	recv(clientFd, buf, size, 0);	
}

void TcpSocket::sSend(int clientFd, const void *buf, size_t size, int flags)
{
	send(clientFd, buf, size, flags);
}

void TcpSocket::sClose()
{
	if (!isActive)
		throw SocketIsClosedError();
	close(_socket);
	isActive = false;
}

/* Basic Operators */

/* Getters & Setters */

/* Member Class */
const char *TcpSocket::SocketCreationError::what() const noexcept
{
	return ("Socket creation failed.");
}

const char *TcpSocket::SocketIsClosedError::what() const noexcept
{
	return ("Socket is closed, the action cannot be performed.");
}
