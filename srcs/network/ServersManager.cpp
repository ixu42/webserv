/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:50 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/14 23:48:29 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServersManager.hpp"

std::vector<Server*> ServersManager::_servers;
ServersManager* ServersManager::_instance = nullptr;
Config* ServersManager::_webservConfig = nullptr;

ServersManager::ServersManager()
{
	// Add servers
	LOG_DEBUG("ServersManager creating servers... Servers in config: ", _webservConfig->getServersConfigsMap().size());

	// iterate according to keys because map is ordered and we can not use unordered map as the order is not guaranteed
	for (auto& key : _webservConfig->getServersConfigsMapKeys())
	{
		std::vector<ServerConfig> serverConfigs = _webservConfig->getServersConfigsMap()[key];

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
		throw ServerException("No valid servers");
	}
	LOG_INFO("ServersManager created ", _servers.size(), " servers");
	for (Server*& server : _servers)
	{
		std::string ipAddr = server->getIpAddress();
		if (ipAddr.empty())
			ipAddr = "0.0.0.0";
		LOG_INFO("Server ipAddr: ", ipAddr, ", port: ", server->getPort());		
	}
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
	while (!g_signalReceived.load())
	{
		// LOG_DEBUG("poll() waiting for an fd to be ready...");
		int ready = poll(_fds.data(), _fds.size(), -1);
		// LOG_DEBUG("poll() returned: ", ready);
		if (ready == -1)
		{
			if (errno == EINTR)
				continue ;
			else
				throw ServerException("poll() error");
		}
		for (struct pollfd& pfd : _fds)
		{
		// LOG_DEBUG("Checking pollfds...");
			if (pfd.revents & POLLIN)
			{
			// LOG_DEBUG("if POLLIN");
				handleRead(pfd);
				// break ; // should it break?
			}
			if (pfd.revents & POLLOUT)
			{
			// LOG_DEBUG("if POLLOUT");
				handleWrite(pfd.fd);
				// break ; // should it break?
			}
			// if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			if (pfd.revents & (POLLERR | POLLHUP))
			{
				LOG_DEBUG("Error or hangup on fd: ", pfd.fd);
				if (pfd.revents & POLLERR )
					LOG_DEBUG("POLLERR");
				if (pfd.revents & POLLHUP)
					LOG_DEBUG("POLLHUP");
				if (pfd.revents & POLLNVAL)
					LOG_DEBUG("POLLNVAL");
				removeClientByFd(pfd.fd);
				removeFromPollfd(pfd.fd);
				if (close(pfd.fd) == -1) {
					LOG_DEBUG("Failed to close fd: ", pfd.fd, " Error: ", strerror(errno));
				} else {
					LOG_DEBUG("Closed fd: ", pfd.fd);
				}
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
			// _fds.push_back({clientSockfd, POLLIN, 0}); // Only POLLIN? or both?
			// _fds.push_back({clientSockfd, POLLIN | POLLERR | POLLHUP | POLLNVAL, 0}); // Onlu POLLIN? or both?
			_fds.push_back({clientSockfd, POLLIN | POLLERR | POLLHUP, 0}); // Onlu POLLIN? or both?
			break ;
		}
		for (t_client& client : server->getClients())
		{
			if (pfdReadyForRead.fd == client.fd && client.state == READING)
			{
				// client.request = server->receiveRequest(pfdReadyForRead.fd);
				server->handler(server, client);
				if (client.state == READY_TO_WRITE)
				{
					// pfdReadyForRead.events = POLLOUT;
					// pfdReadyForRead.events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
					pfdReadyForRead.events = POLLOUT | POLLERR | POLLHUP;
				}
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
				if (client.state == READY_TO_WRITE)
				{
					server->responder(client, *server);
					client.responseString = Response::buildResponse(*client.response);
					LOG_DEBUG("response: ", client.responseString.substr(0, 500), "\n...\n"); // Can be really huge for huge files and can interrupt the Terminal
					client.state = WRITING;
				}
				if (client.state == WRITING)
				{
					// write(client.fd, response.c_str(), response.size());
					if (server->sendResponse(client))
						client.state = FINISHED_WRITING;
				}
				if (client.state == FINISHED_WRITING)
				{
				{
						server->finalizeResponse(client);
						LOG_INFO("Response sent and connection closed (socket fd: ", client.fd, ")");
				}
				}
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

void	ServersManager::removeClientByFd(int currentFd)
{
	for (Server*& server : _servers)
	{
		std::vector<t_client>& clients = server->getClients();
		for (auto it = clients.begin(); it != clients.end(); ++it)
		{
		if (it->fd == currentFd)
		{
			clients.erase(it);
			break ;
		}
		}
	}
}
