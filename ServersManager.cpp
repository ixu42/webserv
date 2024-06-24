#include "ServersManager.hpp"

std::vector<Server *> ServersManager::servers;
ServersManager* ServersManager::instance = nullptr;
Config* ServersManager::webservConfig;

void ServersManager::signalHandler(int signal)
{
	std::cout << std::endl << "Signal " << signal << " received." << std::endl;

	// Handle cleanup tasks or other actions here
	
	// ServersManager::getInstance()->poll_fds.clear();
	delete instance;
	// servers.clear();

	std::exit(signal); // Exit the program with the received signal as exit code
}

ServersManager::ServersManager()
{
	// Handle ctrl+c
	signal(SIGINT, ServersManager::signalHandler);

	// Add servers
	int i = 0;
	for (ServerConfig& serverConfig : webservConfig->getServers())
	{
		servers.push_back(new Server(serverConfig.ipAddress, serverConfig.port));
		servers[i]->setConfig(&serverConfig);
		servers[i]->getConfig();
		// std::cout << "Server config and location: " << servers[i]->getConfig()->locations[0].path << std::endl;
		i++;
	}

	std::cout << "ServersManager created" << std::endl;
}

ServersManager::~ServersManager()
{
	std::cout << servers.size() << " servers will be deleted" << std::endl;
	for (Server *server : servers)
	{
		delete server;
	}
	delete webservConfig;
}


void ServersManager::initConfig(char *fileNameString)
{
	webservConfig = new Config(fileNameString);
}

ServersManager* ServersManager::getInstance()
{
	if (webservConfig == nullptr)
		webservConfig = new Config(DEFAULT_CONFIG); // if config is not initialized with iniConfig, DEFAULT_CONFIG will be used
	if (instance == nullptr)
		instance = new ServersManager();
		
	return instance;
}


void ServersManager::run()
{
	// std::vector<pollfd> poll_fds;

	for (Server *server : servers)
	{
		pollfd pfd;
		pfd.fd = server->getSocket();
		pfd.events = POLLIN;
		this->poll_fds.push_back(pfd);
	}
	/* Waiting for incoming connections */
	while (true)
	{
		int ret = poll(this->poll_fds.data(), this->poll_fds.size(), -1);
		if (ret < 0)
		{
			if (errno == EINTR || errno == EWOULDBLOCK)
			{
				continue;
			}
			throw ServerException("Poll failed");
		}

		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				int addrlen = sizeof(sockaddr_in);
				// servers[i]->setClientSocket(accept(poll_fds[i].fd, (struct sockaddr *)&servers[i]->getSockAddress(), (socklen_t *)&addrlen));
				int clientSocket = accept(poll_fds[i].fd, (struct sockaddr *)&servers[i]->getSockAddress(), (socklen_t *)&addrlen);
				if (clientSocket >= 0)
				{
					servers[i]->setClientSocket(clientSocket);
					// Set socket to non-blocking mode
					int flags = fcntl(clientSocket, F_GETFL, 0);

					if (flags == -1)
					{
						if (close(clientSocket) == -1)
							throw ServerException("Failed to close socket");
						throw ServerException("Failed to get socket flags");
					}
					if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1)
					{
						if (close(clientSocket) == -1)
							throw ServerException("Failed to close socket");
						throw ServerException("Failed to set non-blocking mode on socket");
					}



					// std::cout << "Locations vector size: " << servers[i]->getConfig()->locations.size() << std::endl;
					// if (servers[i]->getConfig()->locations.empty())
					// 	return ;
					// for (Location location : servers[i]->getConfig()->locations)
					// {
					// 	std::cout << "Server config and location: " << location.path << std::endl;
					// }

					servers[i]->handleRequest3();

					close(clientSocket);
					std::cout << "Connection closed" << std::endl;
				}
			}
		}
	}

	// for (Server *server : servers)
	// {
	// 	server->shutdown();
	// }
}
