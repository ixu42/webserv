/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/06/24 23:37:32 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "debug.hpp"
#include <cstring> // memset()
#include <arpa/inet.h> // htons()
#include <signal.h> // signal()
#include <poll.h> // poll()

volatile bool Server::_running = true;

Server::Server() : _serverSocket(Socket()), _port(8080), _backlog(10)
{
	DEBUG("Server constructor called");

	std::memset((char *)&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	if (!_serverSocket.create() ||
		!_serverSocket.bindAddress(_address) ||
		!_serverSocket.listenForConnections(_backlog))
	{
		std::cerr << "Failed to launch server\n";
		exit(EXIT_FAILURE);
	}

	// add _serverSocket fd to _fds vector for polling
	_fds.push_back({_serverSocket.getSockfd(), POLLIN, 0});
}

Server::~Server()
{
	DEBUG("Server destructor called");

	for (int client_socket : _clientSockfds)
		close(client_socket);
}

void	Server::signalHandler(int signum)
{
	std::cout << "\nSignal (" << signum << ") received. "
				<< "Shutting down server...\n";
	_running = false;
}

bool	Server::run()
{
	DEBUG("Server::run() called");

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	while (_running)
	{
		// poll for events
		DEBUG("poll() waiting for an fd to be ready");
		int ret = poll(_fds.data(), _fds.size(), -1);
		if (ret == -1)
		{
			if (errno == EINTR)
				return false; // poll() interrupted by signal
			std::cerr << "poll() failed\n";
			return false;
		}

		// handle events
		for (size_t i = 0; i < _fds.size(); ++i)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _serverSocket.getSockfd())
					accepter(); // accept new connection
				else
					handler(_fds[i].fd); // handle read operation
			}
			if (_fds[i].revents & POLLOUT)	
				responder(_fds[i].fd); // handle write operation
		}
	}
	return true;
}

bool	Server::accepter()
{
	DEBUG("Server::accepter() called");

	struct sockaddr_in clientAddr;
	int clientSockfd = _serverSocket.acceptConnection(clientAddr);
	if (clientSockfd == -1)
		return false;
	std::cout << "\n=== CONNECTION ESTABLISHED WITH CLIENT (SOCKET FD: "
				<< clientSockfd << ") ===\n";

	_clientSockfds.push_back(clientSockfd);
	_fds.push_back({clientSockfd, POLLIN, 0});
	return true;
}

void	Server::handler(int clientSockfd)
{
	DEBUG("Server::handler() called");

	char buffer[1024];
	int bytesRead = read(clientSockfd, buffer, sizeof(buffer));
	if (bytesRead == -1)
		std::cerr << "read() error\n";
	else if (bytesRead == 0)
	{
		// connection closed by client
		close(clientSockfd);
		removeClientSocket(clientSockfd);
		std::cout << "\n=== CLIENT DISCONNECTED (SOCKET FD: " 
					<< clientSockfd << ")===\n\n";
	}
	else
	{
		// handle received data (e.g. process HTTP request)
		std::cout << "\n--- Received data from client ---\n\n"
					<< std::string(buffer, bytesRead);

		// change poll event to POLLOUT to write the response later
		for (auto &fd : _fds)
		{
			if (fd.fd == clientSockfd)
			{
				fd.events = POLLOUT;
				break;
			}
		}
	}
}

void	Server::responder(int clientSockfd)
{
	DEBUG("Server::responder() called");

	const std::string& response = getResponse();
	write(clientSockfd, response.c_str(), response.length());

	// after writing, close the connection
	close(clientSockfd);
	removeClientSocket(clientSockfd);
	std::cout << "\n=== RESPONSE SENT AND CONNECTION CLOSED (SOCKET FD: "
				<< clientSockfd << ") ===\n";
}

void	Server::removeClientSocket(int clientSockfd)
{
	// remove from _clientSockfd vector
	for (auto it = _clientSockfds.begin(); it != _clientSockfds.end(); ++it)
	{
		if (*it == clientSockfd)
		{
			_clientSockfds.erase(it);
			break ;
		}
	}

	// remove from _fds vector
	for (auto fd_it = _fds.begin(); fd_it != _fds.end(); ++fd_it)
	{
		if (fd_it->fd == clientSockfd)
		{
			_fds.erase(fd_it);
			break ;
		}
	}
}

const std::string	Server::getResponse()
{
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: 19\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += "Hello from server!";
	return response;
}
