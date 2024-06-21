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

Server::~Server()
{
	shutdown();
	close(clientSocket);
}

void Server::initialize()
{
	createSocket();

	this->sockAddress.sin_family = this->domain;
	this->sockAddress.sin_addr.s_addr = this->address;
	this->sockAddress.sin_port = htons(this->port);

	bindSocket();
	listenConnection();
	// while (1)
	// {
	// 	acceptConnection();
	// }
}

void Server::createSocket()
{
	this->serverSocket = socket(this->domain, this->type, this->protocol);
	if (this->serverSocket == 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed");
	}

	// Set socket to non-blocking mode
	int flags = fcntl(this->serverSocket, F_GETFL, 0);
	if (flags == -1)
	{
		close(this->serverSocket);
		throw ServerException("Failed to get socket flags");
	}
	if (fcntl(this->serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(this->serverSocket);
		throw ServerException("Failed to set non-blocking mode on socket");
	}
}

void Server::bindSocket()
{
	std::memset(this->sockAddress.sin_zero, '\0', sizeof(this->sockAddress.sin_zero));

	if (bind(this->serverSocket, (struct sockaddr *)&this->sockAddress, sizeof(this->sockAddress)) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed binding");
	}
}

void Server::listenConnection()
{
	if (listen(this->serverSocket, 10) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed listening");
	}
}

/* void Server::acceptConnection()
{
	std::cout << "Server listening on port " << this->port << std::endl;
	// char hello[] = "Hello from server";
	char hello[] = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n\r\n<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>Document</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	int addrlen = sizeof(this->sockAddress);
	this->clientSocket = accept(this->serverSocket, (struct sockaddr *)&this->sockAddress, (socklen_t *)&addrlen);
	if (this->clientSocket < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket failed accepting");
	}

	char buffer[30000] = {0};
	int valread = read(this->clientSocket, buffer, 30000);
	std::cout << valread << std::endl;
	std::cout << buffer << std::endl;
	write(clientSocket, (char *)hello, std::strlen(hello));
	std::cout << "Hello message sent" << std::endl;
	close(this->clientSocket);
	std::cout << "Connection closed" << std::endl;
} */

void Server::handleRequest()
{
	// char response[] = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n\r\n<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body;
	char buffer[300000] = {0};
	int valread = read(this->clientSocket, buffer, 300000);
	std::cout << "Valread" << valread << std::endl; // message length
	std::cout << "Buffer: " << buffer << std::endl;
	// if (valread != -1)
	// {
		write(clientSocket, response.c_str(), response.length());
		std::cout << "Response message sent: " <<  response.c_str() << std::endl;
	// }
		close(this->clientSocket);
		std::cout << "Connection closed" << std::endl;
}

void Server::shutdown()
{
	close(this->serverSocket);
}

int Server::getSocket() const
{
	return this->serverSocket;
}

struct sockaddr_in &Server::getSockAddress()
{
	return this->sockAddress;
}

int Server::getClientSocket() const
{
	return this->clientSocket;
}

void Server::setClientSocket(int newClientSocket)
{
	this->clientSocket = newClientSocket;
}
