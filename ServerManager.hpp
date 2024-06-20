#pragma once
#include "Server.hpp"
#include <vector>
#include <poll.h>

class ServerManager
{
	private:
		static std::vector<Server*> servers;

		static void signalHandler(int signal);

	public:
		ServerManager();
		~ServerManager();

		void run();
};
