/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:43:18 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/20 10:04:26 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <Client.hpp>
#include <TcpSocket.hpp>
#include <sys/poll.h>

class HttpServer {
private:
	std::vector<TcpSocket> _socket;
	std::map<int, Client> _clientMap;
	std::vector<struct pollfd> _pfd;

	/* Member Functions */
	void addClient(int serverFd, int clientFd);
	void removeClient(int clientFd);

public:
	/* Constructors & Destructors */
	HttpServer();
	HttpServer(const HttpServer& src) = delete;
	HttpServer(HttpServer&& src) = delete;
	~HttpServer();

	/* Member Functions */
	void parseConfig(std::string path);
	void createSockets();
	void run();

	/* Basic Operators */
	HttpServer& operator=(const HttpServer& rhs) = delete;

	/* Getters & Setters */

};

#endif // HTTPSERVER_HPP