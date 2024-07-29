/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:50 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/29 20:26:52 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServersManager.hpp"

std::vector<Server*> ServersManager::_servers;
ServersManager* ServersManager::_instance = nullptr;
Config* ServersManager::_webservConfig = nullptr;
std::vector<struct pollfd> ServersManager::_fds;

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
				catch (const ServerException& e)
				{
					LOG_ERROR("Failed to launch server: ", e.what());
					if (e.getErrno() == EADDRINUSE)
					{
						moveServerConfigsToNoIpServer(serverConfigs[0].port, serverConfigs);
						LOG_INFO("Config for the server will be moved");
					}
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

Server* ServersManager::findNoIpServerByPort(int port)
{
	LOG_DEBUG("ServersManager::findNoIpServerByPort() called");
	for (Server*& server : _servers)
	{
		LOG_DEBUG("ipAddr: ", server->getIpAddress());
		if (server->getIpAddress().empty() && server->getPort() == port)
			return server;
	}
	return nullptr;
}


bool ServersManager::checkUniqueNameServer(ServerConfig& serverConfig, std::vector<ServerConfig>& targetServerconfigs)
{
	for (ServerConfig& targetConfig : targetServerconfigs)
	{
		if (targetConfig.serverName == serverConfig.serverName)
			return false;
	}
	return true;

}
void ServersManager::moveServerConfigsToNoIpServer(int port, std::vector<ServerConfig>& serverConfigs)
{
	LOG_INFO("Address already in use. All the configs will be moved to the no ip server.");
	Server* noIpServer = findNoIpServerByPort(port);
	if (noIpServer == nullptr)
		return ;
	for (ServerConfig& serverConfig : serverConfigs)
	{
		if (checkUniqueNameServer(serverConfig, noIpServer->getConfigs()))
			noIpServer->getConfigs().push_back(serverConfig);
	}
}

void ServersManager::initConfig(const char *fileNameString, const char*argv0)
{
	_webservConfig = new Config(fileNameString, argv0);
}

ServersManager* ServersManager::getInstance(const char* argv0)
{
	// if config is not initialized with initConfig, DEFAULT_CONFIG will be used
	if (_webservConfig == nullptr)
		_webservConfig = new Config(DEFAULT_CONFIG, argv0);
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
		LOG_DEBUG("fds size:", _fds.size());
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
			}
			if (pfd.revents & POLLOUT)
			{
				LOG_DEBUG("if POLLOUT for fd: ", pfd.fd);
				handleWrite(pfd.fd);
			}
			
			// if the page refreshed, device has been disconnected or an error has occurred on the file descriptor
			if (pfd.revents & (POLLERR | POLLHUP)) 
			{
				LOG_DEBUG("Error or hangup on fd: ", pfd.fd);
				if (pfd.revents & POLLERR )
					LOG_DEBUG("POLLERR");
				if (pfd.revents & POLLHUP)
					LOG_DEBUG("POLLHUP");
				// if (pfd.revents & POLLNVAL)
				// 	LOG_DEBUG("POLLNVAL");
				if (close(pfd.fd) == -1) {
					LOG_DEBUG("Failed to close fd: ", pfd.fd, " Error: ", strerror(errno));
				} else {
					LOG_DEBUG("Closed fd: ", pfd.fd);
				}
				removeClientByFd(pfd.fd);
				removeFromPollfd(pfd.fd);
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
				
			// Add connected client fd to pollfd vector
			// _fds.push_back({clientSockfd, POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL, 0}); 
			_fds.push_back({clientSockfd, POLLIN | POLLOUT | POLLERR | POLLHUP, 0}); 
			break ;
		}
		for (Client& client : server->getClients())
		{
			if (pfdReadyForRead.fd == client.getFd()
				&& client.getState() == Client::ClientState::READING)
			{
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
				}

				fdFound = true;
				break ;
			}
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
	int limitResponseString = 500;
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
					SessionsManager::handleSessions(client);
					client.setResponseString(Response::buildResponse(*client.getResponse()));
					
					// Can be really huge for huge files and can interrupt the Terminal
					LOG_DEBUG("response: ", client.getResponseString().substr(0, limitResponseString), "\n...\n");
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
			delete it->getRequest();
			delete it->getResponse();
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
