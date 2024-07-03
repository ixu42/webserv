/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 15:53:37 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/03 17:05:06 by ixu              ###   ########.fr       */
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

class CGIServer {
	private:
		static			std::string determineInterpreter(const std::string& filePath);
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