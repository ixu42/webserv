#include "ServerManager.hpp"

std::vector<Server *> ServerManager::servers;

void ServerManager::signalHandler(int signal)
{
	std::cout << "Signal " << signal << " received." << std::endl;

	// int opt = 1;
	// setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Handle cleanup tasks or other actions here
	// For example, gracefully shut down the server
	for (Server *server : servers)
	{
		server->shutdown();
	}
	exit(signal); // Exit the program with the received signal as exit code
}

ServerManager::ServerManager()
{
	signal(SIGINT, ServerManager::signalHandler);
	servers.push_back(new Server("127.0.0.1", 8003));
	servers.push_back(new Server("127.0.0.1", 8004));
}

ServerManager::~ServerManager()
{
	for (Server *server : servers)
	{
		delete server;
	}
}

void ServerManager::run()
{
	std::vector<pollfd> poll_fds;

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
