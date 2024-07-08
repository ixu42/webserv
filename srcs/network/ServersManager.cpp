/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:50 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/08 13:33:22 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServersManager.hpp"

std::vector<Server*> ServersManager::_servers;
ServersManager* ServersManager::_instance = nullptr;
Config* ServersManager::_webservConfig;

void ServersManager::signalHandler(int signal)
{
	std::cout << TEXT_MAGENTA << "\n[INFO] Shutting down the server(s)..." << RESET << std::endl;
	LOG_DEBUG("Signal ", signal, " received.");

	// Handle cleanup tasks or other actions here
	
	// ServersManager::getInstance()->poll_fds.clear();
	delete _instance;
	std::exit(signal); // Exit the program with the received signal as exit code
}

ServersManager::ServersManager()
{
	// Handle ctrl+c
	signal(SIGINT, ServersManager::signalHandler); /* ctrl + c */
	signal(SIGTSTP, ServersManager::signalHandler); /* ctrl + z */
	signal(SIGQUIT, ServersManager::signalHandler); /* ctrl + \ */

	// Add servers
	LOG_DEBUG("ServersManager creating servers... Servers in config: ", _webservConfig->getServersConfigsMap().size());

	for (auto& [ipPortKey, serverConfigs] : _webservConfig->getServersConfigsMap())
	{
		Server* foundServer = nullptr;
		for (auto& server : _servers)
		{
			if (server->getIpAddress() == serverConfigs[0].ipAddress && server->getPort() == serverConfigs[0].port)
			{
				foundServer = server;
				break ;
			}
		}
		if (!foundServer)
		{
			try
			{
				_servers.push_back(new Server(serverConfigs[0].ipAddress.c_str(), serverConfigs[0].port));
				foundServer = _servers.back();
			}
			catch (const std::exception& e)
			{
				LOG_DEBUG("Failed to launch server: ", e.what());
			}
		}
		if (foundServer)
			foundServer->setConfig(serverConfigs);
	}

	// Add all server fds to pollfd vector
	for (Server*& server : _servers)
		_fds.push_back({server->getServerSockfd(), POLLIN, 0});

	if (_servers.empty())
	{
		delete _instance;
		LOG_ERROR("No valid servers");
		std::exit(EXIT_FAILURE);
	}
	LOG_INFO("ServersManager created ", _servers.size(), " servers");
}

ServersManager::~ServersManager()
{
	LOG_DEBUG(_servers.size(), " server(s) will be deleted");
	for (Server *server : _servers)
	{
		delete server;
	}
	delete _webservConfig;
}

void ServersManager::initConfig(const char *fileNameString)
{
	_webservConfig = new Config(fileNameString);
}

ServersManager* ServersManager::getInstance()
{
	if (_webservConfig == nullptr)
		_webservConfig = new Config(DEFAULT_CONFIG); // if config is not initialized with initConfig, DEFAULT_CONFIG will be used
	if (_instance == nullptr)
		_instance = new ServersManager();
		
	return _instance;
}

void ServersManager::run()
{
	while (true)
	{
		LOG_DEBUG("poll() waiting for an fd to be ready...");
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
				// client.request = server->receiveRequest(pfdReadyForRead.fd);
				server->handler(server, client);
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
				server->responder(client, *server);
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
