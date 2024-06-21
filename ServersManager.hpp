#pragma once
#include "Server.hpp"
#include <vector>
#include <poll.h>
#include <csignal>

/* Can be only instatiated once */
class ServersManager
{
	private:
		static ServersManager* instance;
		static std::vector<Server*> servers;
		std::vector<pollfd> poll_fds;

		ServersManager();
		ServersManager(const ServersManager&) = delete;
		ServersManager& operator=(const ServersManager&) = delete;
		
		static void signalHandler(int signal);

	public:
		~ServersManager();
		static ServersManager* getInstance();

		void run();
};
