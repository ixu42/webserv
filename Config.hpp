#include "ServerException.hpp"
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

struct Location
{
	std::string path;
	std::string redirect;
	std::string root;
	std::vector<std::string> index;
	std::vector<std::string> methods;
};

class Config
{
	private:
		std::string configString;

		std::string ipAddress = "127.0.0.1";
		int port = 8080;
		std::string serverName = "localhost";
		std::string clientMaxBodySize = "100M";

		std::map<int, std::string> errorPages = {{404, "404.html"}, {500, "500.html"}};
		std::map<std::string, bool> cgi = {{"php", false}, {"py", false}};
		std::vector<std::string> cgi;

		std::vector<Location> locations;

	public:
		void readFile(char *filePath);
		void parse();
};
