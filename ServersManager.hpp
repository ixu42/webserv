#pragma once
#include "Server.hpp"
#include "Config.hpp"
#include <vector>
#include <poll.h>
#include <csignal>
#include "debug.hpp"

#define DEFAULT_CONFIG "config/default.conf"

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
		void						handleRead(int fdReadyForRead);
		void						handleWrite(int fdReadyForWrite);
		void						removeFromPollfd(int fd);

	public:
		~ServersManager();
		static ServersManager*		getInstance();

		void						run();
		static void					initConfig(char *fileNameString);
};
