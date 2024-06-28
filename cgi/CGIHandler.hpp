#pragma once

#include "Request.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

class CGIServer {
	private:
		Request request;
		int inputPipe[2];
		int outputPipe[2];
		std::string interpreter;
		std::vector<std::string>& envVars;

		static std::string determineInterpreter(const std::string& filePath);
		static std::vector<std::string> setEnvironmentVariables(Request& request);
		static void handleChildProcess(int inputPipe[2], int outputPipe[2], const std::string& interpreter, const std::string& filePath, const std::vector<std::string>& envStrings);
		static void handleParentProcess(int inputPipe[2], int outputPipe[2], const std::string& method, const std::string& body);

	public:
		CGIServer() = delete;
		static void handleRequest(Request& request);
};