#pragma once

#include "../request/Request.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <cstring>
#include <string>
#include <vector>

#define IN 0
#define OUT 1
#define FDS 2

class CGIServer {
	private:
		static					int inputPipe[FDS];
		static					int outputPipe[FDS];
		static					std::string result;

		static					std::string determineInterpreter(const std::string& filePath);
		static					std::vector<std::string> setEnvironmentVariables(Request& request);
		static void				handleProcesses(Request& request, const std::string& interpreter, const std::vector<std::string>& envVars);
		static void				handleChildProcess(const std::string& interpreter, const std::string& filePath,
									const std::vector<std::string>& envVars);
		static void				handleParentProcess(const std::string& method, const std::string& body);

	public:
		CGIServer()				= delete;
		static std::string		handleCGI(Request& request);
};