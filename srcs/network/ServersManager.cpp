/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:50 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/03 14:12:46 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServersManager.hpp"

std::vector<Server *> ServersManager::_servers;
ServersManager* ServersManager::_instance = nullptr;
Config* ServersManager::_webservConfig;

void ServersManager::signalHandler(int signal)
{
	std::cout << std::endl << "Signal " << signal << " received." << std::endl;

	// Handle cleanup tasks or other actions here
	
	// ServersManager::getInstance()->poll_fds.clear();
	delete _instance;
	// servers.clear();

	std::exit(signal); // Exit the program with the received signal as exit code
}

ServersManager::ServersManager()
{
	// Handle ctrl+c
	signal(SIGINT, ServersManager::signalHandler);

	// Add servers
	int i = 0;
	for (ServerConfig& serverConfig : _webservConfig->getServers())
	{

		DEBUG("serverConfig.port: " << serverConfig.port);
		_servers.push_back(new Server(serverConfig.ipAddress.c_str(), serverConfig.port));
		_servers[i]->setConfig(&serverConfig);
		_servers[i]->getConfig();
		// std::cout << "Server config and location: " << servers[i]->getConfig()->locations[0].path << std::endl;
		i++;
	}

	// Add all server fds to pollfd vector
	for (Server*& server : _servers)
		_fds.push_back({server->getServerSockfd(), POLLIN, 0});

	if (_servers.empty())
	{
		delete _instance;
		std::cout << "No valid servers" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::cout << "ServersManager created " << _servers.size() << " servers" << std::endl;
}

ServersManager::~ServersManager()
{
	std::cout << _servers.size() << " servers will be deleted" << std::endl;
	for (Server *server : _servers)
	{
		delete server;
	}
	delete _webservConfig;
}

void ServersManager::initConfig(char *fileNameString)
{
	_webservConfig = new Config(fileNameString);
}

ServersManager* ServersManager::getInstance()
{
	if (_webservConfig == nullptr)
		_webservConfig = new Config(DEFAULT_CONFIG); // if config is not initialized with iniConfig, DEFAULT_CONFIG will be used
	if (_instance == nullptr)
		_instance = new ServersManager();
		
	return _instance;
}

void ServersManager::run()
{
	while (true)
	{
		DEBUG("poll() waiting for an fd to be ready...");
		int ready = poll(_fds.data(), _fds.size(), -1);
		if (ready == -1)
			throw ServerException("poll() error");

		for (struct pollfd& pfd : _fds)
		{
			if (pfd.revents & POLLIN)
			{
				handleRead(pfd);
				break ;
			}
			if (pfd.revents & POLLOUT)
			{
				handleWrite(pfd.fd);
				break ;
			}
		}
	}
}

void	ServersManager::handleRead(struct pollfd& pfdReadyForRead)
{
	bool fdFound = false;

	for (Server*& server : _servers)
	{
		if (pfdReadyForRead.fd == server->getServerSockfd())
		{
			int clientSockfd = server->accepter();
			if (clientSockfd == -1)
				throw ServerException("Server failed to accept a connection");
			// Add connected client fd to pollfd vector
			_fds.push_back({clientSockfd, POLLIN, 0});
			break ;
		}
		for (t_client& client : server->getClients())
		{
			if (pfdReadyForRead.fd == client.fd)
			{
				// Request req = server->receiveRequest(pfdReadyForRead.fd);
				// if (req.getStartLine()["path"].find("cgi-bin") != std::string::npos)
				// {
				// 	Response resp;
				// 	CGIServer::handleCGI(req, *server, resp);
				// 	std::cout << resp.getBody() << std::endl;
				// }
				client.request = server->receiveRequest(pfdReadyForRead.fd);
				pfdReadyForRead.events = POLLOUT;
				fdFound = true;
				break ;
			}
		}
		if (fdFound)
			break ;
	}
}

void	ServersManager::handleWrite(int fdReadyForWrite)
{
	bool fdFound = false;

	for (Server*& server : _servers)
	{
		for (t_client& client : server->getClients())
		{
			if (fdReadyForWrite == client.fd)
			{
				server->responder(client);
				removeFromPollfd(fdReadyForWrite);
				fdFound = true;
				break ;
			}
		}
		if (fdFound)
			break ;
	}
}

void	ServersManager::removeFromPollfd(int fd)
{
	for (auto fd_it = _fds.begin(); fd_it != _fds.end(); ++fd_it)
	{
		if (fd_it->fd == fd)
		{
			_fds.erase(fd_it);
			break ;
		}
	}
}

/**
 * Previous approaches for ServersManager::run()
 */

// void ServersManager::run()
// {
	// for (Server *server : servers)
	// {
	// 	if (!server->run())
	// 		throw ServerException("Server failed to run");
	// }

	// for (Server *server : servers)
	// {config
	// 	pollfd pfd;
	// 	pfd.fd = server->getSocket();
	// 	pfd.events = POLLIN;
	// 	this->poll_fds.push_back(pfd);
	// }
	// /* Waiting for incoming connections */
	// while (true)
	// {
	// 	int ret = poll(this->poll_fds.data(), this->poll_fds.size(), -1);
	// 	if (ret < 0)
	// 	{
	// 		if (errno == EINTR || errno == EWOULDBLOCK)
	// 		{
	// 			continue;
	// 		}
	// 		throw ServerException("Poll failed");
	// 	}

	// 	for (size_t i = 0; i < poll_fds.size(); ++i)
	// 	{
	// 		if (poll_fds[i].revents & POLLIN)
	// 		{
	// 			int addrlen = sizeof(sockaddr_in);
	// 			// servers[i]->setClientSocket(accept(poll_fds[i].fd, (struct sockaddr *)&servers[i]->getSockAddress(), (socklen_t *)&addrlen));
	// 			int clientSocket = accept(poll_fds[i].fd, (struct sockaddr *)&servers[i]->getSockAddress(), (socklen_t *)&addrlen);
	// 			if (clientSocket >= 0)
	// 			{
	// 				servers[i]->setClientSocket(clientSocket);
	// 				// Set socket to non-blocking mode
	// 				int flags = fcntl(clientSocket, F_GETFL, 0);

	// 				if (flags == -1)
	// 				{
	// 					if (close(clientSocket) == -1)
	// 						throw ServerException("Failed to close socket");
	// 					throw ServerException("Failed to get socket flags");
	// 				}
	// 				if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1)
	// 				{
	// 					if (close(clientSocket) == -1)
	// 						throw ServerException("Failed to close socket");
	// 					throw ServerException("Failed to set non-blocking mode on socket");
	// 				}



	// 				// std::cout << "Locations vector size: " << servers[i]->getConfig()->locations.size() << std::endl;
	// 				// if (servers[i]->getConfig()->locations.empty())
	// 				// 	return ;
	// 				// for (Location location : servers[i]->getConfig()->locations)
	// 				// {
	// 				// 	std::cout << "Server config and location: " << location.path << std::endl;
	// 				// }

	// 				servers[i]->handleRequest3();

	// 				close(clientSocket);
	// 				std::cout << "Connection closed" << std::endl;
	// 			}
	// 		}
	// 	}
	// }
// }
