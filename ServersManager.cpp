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
	for (auto& server : _servers)
	{
		pollfd serverFd = {server->getServerSockfd(), POLLIN, 0};
		_fds.push_back(serverFd);
	}

	std::cout << "ServersManager created" << std::endl;
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
		int ready = poll(_fds.data(), _fds.size(), -1);
		if (ready == -1)
			throw ServerException("poll() error");

		for (auto& pfd : _fds)
		{
/* 			if ((pfd.revents & POLLERR) || (pfd.revents & POLLHUP))
			{
				close(pfd.fd);
				removeFromPollfd(pfd.fd);
				for (auto& server : _servers)
				{
					server->removeFromClientSockfds(pfd.fd);
				}
			} */


			// if (pfd.revents & POLLIN)
			// {
			// 	handleRead(pfd.fd);
			// 	pfd.revents &= ~POLLIN;
			// 	if (pfd.revents & POLLOUT)
			// 		handleWrite(pfd.fd);
			// }
			if (pfd.revents & POLLIN) {
				try 
				{
					Request req = handleRead(pfd.fd);
					// Clear POLLIN flag
					pfd.revents &= ~POLLIN;
					if (req._request.size() != 0)
					{
						handleWrite(pfd.fd);
						pfd.revents &= ~POLLOUT; }
					else
					{
						close(pfd.fd);
						removeFromPollfd(pfd.fd);
						for (auto& server : _servers)
						{
							server->removeFromClientSockfds(pfd.fd);
						}
					}
				}
				catch (ServerException& e)
				{

					std::cerr << "Error: " << e.what() << std::endl;
				}
            }
			// else if (pfd.revents & POLLOUT)
			// {
            //     // Clear POLLOUT flag
            //     handleWrite(pfd.fd);
            //     pfd.revents &= ~POLLOUT;
            // }
		}
	}
}

Request	ServersManager::handleRead(int fdReadyForRead)
{
	if (fdReadyForRead < 0)
		throw ServerException("Invalid fd");
	bool fdFound = false;

	DEBUG(fdReadyForRead << " handleRead() called");
	for (auto& server : _servers)
	{
		if (fdReadyForRead == server->getServerSockfd())
		{
			int clientSockfd = server->accepter();
			if (clientSockfd == -1)
				throw ServerException("Server failed to accept a connection");
			// Add connected client fd to pollfd vector
			_fds.push_back({clientSockfd, POLLIN | POLLOUT | POLLERR | POLLHUP, 0});
			// server->receiveRequest(fdReadyForRead);
			// fdReadyForRead = clientSockfd;
			break ;
		}
		// else
		// {
			for (auto& clientSockfd : server->getClientSockfds())
			{
				DEBUG("Finding clientSockfd in clientSockfds: " << clientSockfd << " == " << fdReadyForRead);
				if (fdReadyForRead == clientSockfd)
				{
					return server->receiveRequest(fdReadyForRead);
					// if (req._request.size() == 0)
					// 	removeFromPollfd(clientSockfd);
					fdFound = true;
					DEBUG("Found fd");
					break ;
				}
			}
			if (fdFound)
			{
				DEBUG("found fd - break");
				break ;
			}
		// }
	}
	throw ServerException("no valid request");
}

void	ServersManager::handleWrite(int fdReadyForWrite)
{
	DEBUG(fdReadyForWrite << " handleWrite() called");
	bool fdFound = false;

	for (auto& server : _servers)
	{
		for (auto& clientSockfd : server->getClientSockfds())
		{
			if (fdReadyForWrite == clientSockfd)
			{
				server->responder(fdReadyForWrite);
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
