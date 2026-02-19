/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mausperg <mausperg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 13:44:57 by mausperg          #+#    #+#             */
/*   Updated: 2026/02/19 16:37:25 by mausperg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h> 
#include <iostream>

enum e_status {
	RECIVE,
	SEND,
	WAIT,
	CLOSE
};

class Client {
private:
	int _serverFd;
	int _clientFd;
	e_status _status;
	std::string _msg;

public:
	/* Constructors & Destructors */
	Client() = delete;
	Client(int serverFd, int clientFd);
	Client(const Client& src) = delete;
	Client(Client&& src) = delete;
	~Client();

	/* Member Functions */
	void read();

	/* Basic Operators */
	Client& operator=(const Client& rhs) = delete;

	/* Getters & Setters */

};

#endif // CLIENT_HPP