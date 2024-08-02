/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:53 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/02 11:40:58 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include "../config/Config.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"
#include "CGIHandler.hpp"
#include <vector>
#include <poll.h>
#include <csignal>
#include <string>
#include <errno.h>

#include <exception>
#include "../utils/globals.hpp"

#define DEFAULT_CONFIG "default/config.conf"

class Server;

/* Can be only instatiated once */
class ServersManager
{
	private:
		static ServersManager*				_instance;
		static std::vector<Server*>			_servers;
		static Config*						_webservConfig;
		static std::vector<struct pollfd>	_fds;

		ServersManager();
		ServersManager(const ServersManager&) = delete;
		ServersManager& operator=(const ServersManager&) = delete;

		void								processFoundServer(Server* foundServer, std::vector<ServerConfig> serverConfigs);
		Server*								findNoIpServerByPort(int port);
		bool								checkUniqueNameServer(ServerConfig& serverConfig, std::vector<ServerConfig>& targetServerconfigs);
		void								moveServerConfigsToNoIpServer(int port, std::vector<ServerConfig>& serverConfigs);
		void								handleRead(int fdReadyForRead, std::vector<pollfd>& new_fds);
		void								processClientCycle(Server*& server, Client& client, int fdReadyForWrite);
		void								handleWrite(int fdReadyForWrite);
		void								removeClientByFd(int fd);
		bool								ifCGIsFd(Client& client, int fd);
		pollfd*								findPollfdByFd(int fd);
		static void							printServersInfo();
		void								checkRevents(std::vector<pollfd>& new_fds);

	public:
		~ServersManager();
		static ServersManager*				getInstance(const char* argv0);

		void								run();
		static void							initConfig(const char *fileNameString, const char* argv0);
		static void							removeFromPollfd(int fd);
		static void							changeStateToDeleteClient(Client& client);
};
