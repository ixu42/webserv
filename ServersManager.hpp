/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:53 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/02 15:08:52 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Server.hpp"
#include "Config.hpp"
#include <vector>
#include <poll.h>
#include <csignal>
#include "debug.hpp"

#define DEFAULT_CONFIG "default/config.conf"

/* Can be only instatiated once */
class ServersManager
{
	private:
		static ServersManager*		_instance;
		static std::vector<Server*>	_servers;
		static Config*				_webservConfig;
		std::vector<struct pollfd>	_fds;

		ServersManager();
		// ServersManager(Config& webservConfig);
		ServersManager(const ServersManager&) = delete;
		ServersManager& operator=(const ServersManager&) = delete;
		

		static void					signalHandler(int signal);
		void						handleRead(struct pollfd& pfdReadyForRead);
		void						handleWrite(int fdReadyForWrite);
		void						removeFromPollfd(int fd);

	public:
		~ServersManager();
		static ServersManager*		getInstance();

		void						run();
		static void					initConfig(const char *fileNameString);
};
