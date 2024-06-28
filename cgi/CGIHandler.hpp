#pragma once

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

class CGIServer {
	private:
		std::vector<std::string> envVariables;
		std::string interpreter;
		
	public:

};