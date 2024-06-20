#include "Server.hpp"

Server::Server()
{
	initialize();
}


Server::Server(std::string ipAddress, int port)
{
	if (inet_pton(this->domain, ipAddress.c_str(), &(this->address)) != 1)
		throw ServerException("Conversion failed");
	// this->address = inet_addr(ipAddress.c_str());
	this->port = port;
	initialize();
}

void Server::initialize()
{
	createSocket();

	this->sockAddress.sin_family = this->domain;
	this->sockAddress.sin_addr.s_addr = this->address;
	this->sockAddress.sin_port = htons(this->port);

	bindSocket();
	listenConnection();
	while (1)
	{
		acceptConnection();
	}
}

void Server::createSocket()
{
	this->serverSocket = socket(this->domain, this->type, this->protocol);
	if (serverSocket == 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}
}

void Server::bindSocket()
{
	std::memset(this->sockAddress.sin_zero, '\0', sizeof(this->sockAddress.sin_zero));

	if (bind(this->serverSocket, (struct sockaddr *)&this->sockAddress, sizeof(this->sockAddress)) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}
}

void Server::listenConnection()
{
	if (listen(this->serverSocket, 10) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}
}

void Server::acceptConnection()
{
	std::cout << "Server listening on port " << this->port << std::endl;
	char hello[] = "Hello from server";
	int addrlen = sizeof(this->sockAddress);
	this->newSocket = accept(this->serverSocket, (struct sockaddr *)&this->sockAddress, (socklen_t *)&addrlen);
	if (this->newSocket < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}

	char buffer[30000] = {0};
	int valread = read(this->newSocket, buffer, 30000);
	std::cout << valread << std::endl;
	std::cout << buffer << std::endl;
	write(newSocket, (char *)hello, std::strlen(hello));
	std::cout << "Hello message sent" << std::endl;
	close(this->newSocket);
	std::cout << "Connection closed" << std::endl;
}