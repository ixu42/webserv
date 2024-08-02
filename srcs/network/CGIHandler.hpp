/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/02 16:16:52 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"
#include "../utils/globals.hpp"

#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include <chrono>
#include <ctime> 

class Server;

class CGIHandler {
	private:
		const static std::string			_python_interpr;
		const static std::string			_php_interpr;
		const static int					_in = 0;
		const static int					_out = 1;

		static std::string					determineInterpreter(Client& client, const std::string& filePath, Server& server);
		static std::vector<std::string>		setEnvironmentVariables(Request* request);
		static void							handleProcesses(Client& client, const std::string& interpreter,
												const std::vector<std::string>& envVars, Server& server);
		static void							handleChildProcess(Client& client, const std::string& interpreter,
												const std::string& filePath, const std::vector<std::string>& envVars, Server& server);
		static void							handleParentProcess(Client& client, const std::string& body);
		static void							checkResponseHeaders(const std::string& result, Response* response);
		static void							registerCGIPollFd(int fd, short events, std::vector<pollfd>& new_fds);
		static void							unregisterCGIPollFd(Server& server, int fd);

	public:
		CGIHandler()						= delete;
		static void							changeToErrorState(Client& client);
		static void							handleCGI(Client& client, Server& server);
		static void							InitCGI(Client& client, std::vector<pollfd>& new_fds);
		static bool							readScriptOutput(Client& client, Server*& server);
		static void							closeFds(Client& client);
		static void							setToInit(Client& client);
		static void							removeFromPids(pid_t pid);
};
