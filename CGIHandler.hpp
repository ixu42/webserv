/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/03 19:15:09 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "debug.hpp"

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
		static			std::string determineInterpreter(const std::string& filePath, Response& response);
		static			std::vector<std::string> setEnvironmentVariables(Request& request);
		static void		handleProcesses(Request& request, Server& server, Response& response,
							const std::string& interpreter, const std::vector<std::string>& envVars);
		static void		handleChildProcess(Server& server, const std::string& interpreter, const std::string& filePath,
							const std::vector<std::string>& envVars);
		static void		handleParentProcess(Server& server, Response& response, const std::string& method,
							const std::string& body);

	public:
		CGIServer()		= delete;
		static void		handleCGI(Request& request, Server& server, Response& response);
};