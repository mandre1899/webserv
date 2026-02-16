/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpSocket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:23:27 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/16 13:26:11 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <unistd.h>
#include <netinet/in.h> // Needed to use struct sockaddr_in
#include <sys/socket.h> // Needed for socket creating and binding
#include <exception>

class TcpSocket {
	private:
	bool isActive;
	int _socket;
	sockaddr_in _sockAddr;

	public:
	/* Constructors & Destructors */
		TcpSocket() = delete;
		TcpSocket(in_addr_t serverAddress, uint16_t port);
		TcpSocket(const TcpSocket& src);
		~TcpSocket();

	/* Member Functions */
		void sBind();
		void sListen(int maxConnections);
		int sAccept();
		void sRecv(int clientFd, char *buf, size_t size);
		void sSend(int clientFd, const void *buf, size_t size, int flags);
		void sClose();

	/* Basic Operators */

	/* Getters & Setters */

	/* Member Class */
	class SocketCreationError : public std::exception
	{
		public:
			virtual const char* what() const noexcept override;
	};

	class SocketIsClosedError : public std::exception
	{
		public:
			virtual const char* what() const noexcept override;
	};

};

#endif // SOCKET_HPP