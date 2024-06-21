#include "ServersManager.hpp"

std::vector<Server *> ServersManager::servers;
ServersManager* ServersManager::instance = nullptr;

void ServersManager::signalHandler(int signal)
{
	std::cout << "Signal " << signal << " received." << std::endl;

	// int opt = 1;
	// setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Handle cleanup tasks or other actions here
	
	// ServersManager::getInstance()->poll_fds.clear();
	delete instance;
	// servers.clear();
	std::exit(signal); // Exit the program with the received signal as exit code
}

ServersManager::ServersManager()
{
	signal(SIGINT, ServersManager::signalHandler);
	servers.push_back(new Server("127.0.0.1", 8001));
	servers.push_back(new Server("127.0.0.1", 8002));
	std::cout << "ServersManager created" << std::endl;
}

ServersManager::~ServersManager()
{
	for (Server *server : servers)
	{
		server->~Server();
		delete server;
	}
}

ServersManager* ServersManager::getInstance()
{
	if (instance == nullptr)
	{
		instance = new ServersManager();
	}
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
		poll_fds.push_back(pfd);
	}

	while (true)
	{
		int ret = poll(poll_fds.data(), poll_fds.size(), -1);
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
					servers[i]->handleRequest();
				}
			}
		}
	}

	for (Server *server : servers)
	{
		server->shutdown();
	}
}
