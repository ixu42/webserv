/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/05 13:28:39 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "Server.hpp"
#include "../response/Response.hpp"
#include "../utils/debug.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>

#define IN 0
#define OUT 1

#define PYTHON_INTERPRETER "/usr/bin/python3"
#define PHP_INTERPRETER "/usr/bin/php"

class Server;

class CGIServer {
	private:
		static				std::string determineInterpreter(const std::string& filePath, Response& response);
		static				std::vector<std::string> setEnvironmentVariables(Request& request);
		static void			handleProcesses(Request& request, Server& server, Response& response,
								const std::string& interpreter, const std::vector<std::string>& envVars);
		static void			handleChildProcess(Response& response, Server& server, const std::string& interpreter,
								const std::string& filePath, const std::vector<std::string>& envVars);
		static void			handleParentProcess(Server& server, Response& response, const std::string& method,
								const std::string& body);
		static std::string	readErrorPage(const std::string& errorPagePath);

	public:
		CGIServer()			= delete;
		static void			setResponse(Response& response, std::string status, std::string type, std::string page);
		static void			handleCGI(Request& request, Server& server, Response& response);
};