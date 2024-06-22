#include "Config.hpp"

void Config::readFile(char *filePath)
{
	int fd = open(filePath, O_RDONLY);
	if (fd == -1)
	{
		// std::cerr << "Error: " << strerror(errno) << std::endl;
		throw ServerException("Error: " + std::string(strerror(errno)));
	}

	int stringSize = 1024;
	char buffer[stringSize];

	int bytesRead = read(fd, buffer, stringSize);
	while (bytesRead != 0)
	{
		this->configString.append(buffer, stringSize);
		if (bytesRead == -1)
			throw ServerException("Error: " + std::string(strerror(errno)));
		bytesRead = read(fd, buffer, stringSize);
	}

	std::cout << this->configString << std::endl;

	close(fd);
}


void Config::parse()
{

}
