/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:50 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/19 15:25:00 by dnikifor         ###   ########.fr       */
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
			if (server->getIpAddress() == serverConfigs[0].ipAddress
				&& server->getPort() == serverConfigs[0].port)
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
	{
		server->setFds(&_fds);
		_fds.push_back({server->getServerSockfd(), POLLIN, 0});
	}

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
	// if config is not initialized with initConfig, DEFAULT_CONFIG will be used
	if (_webservConfig == nullptr)
		_webservConfig = new Config(DEFAULT_CONFIG); 
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
				LOG_DEBUG("if POLLIN for fd: ", pfd.fd);
				handleRead(pfd);
				// break ; // should it break?
			}
			if (pfd.revents & POLLOUT)
			{
				LOG_DEBUG("if POLLOUT for fd: ", pfd.fd);
				handleWrite(pfd.fd);
				// break ; // should it break?
			}
			// if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
			// if the page refreshed, device has been disconnected or an error has occurred on the file descriptor
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
			// Onlu POLLIN? or both?
			// _fds.push_back({clientSockfd, POLLIN | POLLERR | POLLHUP | POLLNVAL, 0});
			// Onlu POLLIN? or both?
			_fds.push_back({clientSockfd, POLLIN | POLLOUT | POLLERR | POLLHUP, 0}); 
			break ;
		}
		for (Client& client : server->getClients())
		{
			// LOG_DEBUG("client.state: ", client.getState());
			// LOG_DEBUG("client.stateCGI: ", client.getCGIState());
			if (pfdReadyForRead.fd == client.getFd()
				&& client.getState() == Client::ClientState::READING)
			{
				// client.request = server->receiveRequest(pfdReadyForRead.fd);
				server->handler(server, client);
				if (client.getState() == Client::ClientState::READY_TO_WRITE)
				{
					// Check cgi path and create pipes
					// Set non blocking
					// Add to pollfd
					if (client.getRequest()->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
					{
						CGIServer::InitCGI(client, *server);
					}
					// pfdReadyForRead.events = POLLOUT;
					// pfdReadyForRead.events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
					// pfdReadyForRead.events = POLLOUT | POLLERR | POLLHUP;
				}

				fdFound = true;
				break ;
			}
			// LOG_DEBUG("client.stateCGI ", client.getCGIState());
			if (ifCGIsFd(client, pfdReadyForRead.fd) && client.getCGIState() == Client::CGIState::FORKED)
			{
				LOG_DEBUG("Now forked and reading");
				// after forking reading from the file and set lockers for CGIHandler
				if (CGIServer::readScriptOutput(client, server)) // read in CGI
				{
					client.setState(Client::ClientState::BUILDING);
					client.setCGIState(Client::CGIState::FINISHED_SET);
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
		for (Client& client : server->getClients())
		{
			if (fdReadyForWrite == client.getFd() || (ifCGIsFd(client, fdReadyForWrite)))
			{
				if (client.getState() == Client::ClientState::READY_TO_WRITE && !ifCGIsFd(client, fdReadyForWrite))
				{
					server->responder(client, *server); // for CGI only fork, execve, child stuff
					if (client.getChildPipe(0) == -1)
					{
						client.setState(Client::ClientState::BUILDING);
						LOG_DEBUG("client switched to building");
					}
				}
				if (ifCGIsFd(client, fdReadyForWrite) && client.getCGIState() == Client::CGIState::INIT)
				{
					server->responder(client, *server);
				}

				if ((!ifCGIsFd(client, fdReadyForWrite) && client.getState() == Client::ClientState::BUILDING)
					|| (ifCGIsFd(client, fdReadyForWrite) && client.getCGIState() == Client::CGIState::FINISHED_SET))
				{
					client.setResponseString(Response::buildResponse(*client.getResponse()));
					// Can be really huge for huge files and can interrupt the Terminal
					LOG_DEBUG("response: ", client.getResponseString().substr(0, 500), "\n...\n");
					client.setState(Client::ClientState::WRITING);
				}
				if (fdReadyForWrite == client.getFd() && client.getState() == Client::ClientState::WRITING)
				{
					LOG_DEBUG("Sending the response now");
					if (server->sendResponse(client))
						client.setState(Client::ClientState::FINISHED_WRITING);
				}
				if (client.getState() == Client::ClientState::FINISHED_WRITING
					&& (client.getChildPipe(0) == -1 || client.getCGIState() == Client::CGIState::FINISHED_SET))
				{
					server->finalizeResponse(client);
					LOG_DEBUG("Response sent and connection closed (socket fd: ", client.getFd(), ")");
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
		std::vector<Client>& clients = server->getClients();
		for (auto it = clients.begin(); it != clients.end(); ++it)
		{
		if (it->getFd() == currentFd)
		{
			clients.erase(it);
			break ;
		}
		}
	}
}

bool ServersManager::ifCGIsFd(Client& client, int fd)
{
	return fd == client.getChildPipe(0);
}

// find pollfd by fd
pollfd* ServersManager::findPollfdByFd(int fd)
{
	for (struct pollfd& pfd : _fds)
	{
		if (pfd.fd == fd)
			return &pfd;
	}
	return nullptr;
}
