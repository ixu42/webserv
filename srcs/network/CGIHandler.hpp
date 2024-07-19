/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/19 15:38:29 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "../response/Response.hpp"
#include "../utils/logUtils.hpp"
#include "../utils/globals.hpp"

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
		static	void						handleProcesses(Client& client, const std::string& interpreter,
												const std::vector<std::string>& envVars);
		static	void						handleChildProcess(Client& client, const std::string& interpreter,
												const std::string& filePath, const std::vector<std::string>& envVars);
		static	void						handleParentProcess(Client& client, const std::string& body);
		static	std::string					readErrorPage(const std::string& errorPagePath);
		static	void						checkResponseHeaders(const std::string& result, Response* response);
		static	void						closeFds(Client& client);
		static	void						registerCGIPollFd(Server& server, int fd, short events);
		static	void						unregisterCGIPollFd(Server& server, int fd);
		static	void						fcntlSet(int fd);

	public:
		CGIServer()							= delete;
		static void							handleCGI(Client& client);
		static	void						InitCGI(Client& client, Server& server);
		static	bool						readScriptOutput(Client& client, Server*& server);
};
