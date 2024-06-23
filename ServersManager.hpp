#pragma once
#include "Server.hpp"
#include "Config.hpp"
#include <vector>
#include <poll.h>
#include <csignal>

/* Can be only instatiated once */
class ServersManager
{
	private:
		static ServersManager* instance;
		static std::vector<Server*> servers;
		static Config* webservConfig;
		std::vector<pollfd> poll_fds;

		ServersManager();
		// ServersManager(Config& webservConfig);
		ServersManager(const ServersManager&) = delete;
		ServersManager& operator=(const ServersManager&) = delete;
		
		static void signalHandler(int signal);

	public:
		~ServersManager();
		static ServersManager* getInstance(char *configFileName);

		void run();
};
