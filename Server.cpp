#include "Server.hpp"

class SocketException : public std::runtime_error
{
public:
	explicit SocketException(const std::string &message)
		: std::runtime_error(message) {}
};

Server::Server()
{
	createSocket();

	this->sockAddress.sin_family = AF_INET;
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
		throw SocketException("Server socket failed");
	}
}

void Server::bindSocket()
{
	std::memset(this->sockAddress.sin_zero, '\0', sizeof(this->sockAddress.sin_zero));

	if (bind(this->serverSocket, (struct sockaddr *)&this->sockAddress, sizeof(this->sockAddress)) < 0)
	{
		close(this->serverSocket);
		throw SocketException("Server socket failed");
	}
}

void Server::listenConnection()
{
	if (listen(this->serverSocket, 10) < 0)
	{
		close(this->serverSocket);
		throw SocketException("Server socket failed");
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
		throw SocketException("Server socket failed");
	}

	char buffer[30000] = {0};
	int valread = read(this->newSocket, buffer, 30000);
	std::cout << valread << std::endl;
	std::cout << buffer << std::endl;
	write(newSocket, (char *)hello, std::strlen(hello));
	std::cout << "Hello message sent" << std::endl;
	close(this->newSocket);
}