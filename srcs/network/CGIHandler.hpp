/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/18 03:25:23 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "Server.hpp"
#include "client.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"
#include "../utils/pids.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

class Server;

class CGIServer {
	private:
		const static std::string			_python_interpr;
		const static std::string			_php_interpr;
		const static int					_in = 0;
		const static int					_out = 1;

		static	std::string					determineInterpreter(const std::string& filePath);
		static	std::vector<std::string>	setEnvironmentVariables(Request* request);
		static	void						handleProcesses(t_client& client, const std::string& interpreter,
												const std::vector<std::string>& envVars);
		static	void						handleChildProcess(t_client& client, const std::string& interpreter,
												const std::string& filePath, const std::vector<std::string>& envVars);
		static	void						handleParentProcess(t_client& client, const std::string& body);
		static	std::string					readErrorPage(const std::string& errorPagePath);
		static	void						checkResponseHeaders(const std::string& result, Response* response);
		static	void						closeFds(t_client& client);

	public:
		CGIServer()							= delete;
		static void							handleCGI(t_client& client);

		static	void						registerCGIPollFd(Server& server, int fd, short events);
		static	void						unregisterCGIPollFd(Server& server, int fd);
		static	void						InitCGI(t_client& client, Server& server);
		static	void						fcntlSet(int fd);
		static	bool						readScriptOutput(t_client& client, Server*& server);
};
