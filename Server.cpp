#include "Server.hpp"

Server::Server()
{
	initialize();
}

Server::Server(std::string ipAddress, int port)
{
	this->addressString = ipAddress;

	if (inet_pton(this->domain, ipAddress.c_str(), &(this->address)) != 1)
		throw ServerException("Conversion failed");
	// this->address = inet_addr(ipAddress.c_str());
	this->port = port;
	initialize();

	std::cout << "Server with address: " << whoAmI() << " was created" << std::endl;
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

	// Reuse address after interruption even if ports are in the TIME_WAIT state
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(this->serverSocket);
		throw ServerException("Failed to set SO_REUSEADDR option");
	}
}

void Server::bindSocket()
{
	std::memset(this->sockAddress.sin_zero, '\0', sizeof(this->sockAddress.sin_zero));

	if (bind(this->serverSocket, (struct sockaddr *)&this->sockAddress, sizeof(this->sockAddress)) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket (" + whoAmI() + ") failed binding");
	}
}

void Server::listenConnection()
{
	if (listen(this->serverSocket, 10) < 0)
	{
		close(this->serverSocket);
		throw ServerException("Server socket (" + whoAmI() + ") failed listening");
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

void Server::handleRequest2()
{

	/* Dummy response start */
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body;
	/* Dummy response end */


	const int bufferSize = 30000;
	char buffer[bufferSize] = {0};
	int bytesRead;
	std::string request;


	bytesRead = read(this->clientSocket, buffer, bufferSize);
	std::cout << "Reading in chunks bytes: " << bytesRead << std::endl;
	std::cout << "=== Input read! ===" << std::endl;
	std::cout << "bytesRead: " << bytesRead << std::endl;
	request += std::string(buffer, bytesRead);

	std::cout << "=== Request read ===" << std::endl;
	std::cout << TEXT_YELLOW << request << RESET << std::endl;



	std::cout << TEXT_GREEN;
	std::cout << "=== Response message sent ===" << std::endl;
	std::cout << response.c_str() << std::endl;
	std::cout << RESET;
	write(this->clientSocket, response.c_str(), response.length());

}

int findContentLength(std::string request)
{
	std::string contentLength = "content-length: ";

	Utility::strToLower(request);
	unsigned long contentLengthPos = request.find(contentLength);
	if (contentLengthPos != std::string::npos)
	{
		std::string contentLengthValue = request.substr(contentLengthPos + contentLength.length());
		int contentLengthValueEnd = contentLengthValue.find("\r\n");
		contentLengthValue = contentLengthValue.substr(0, contentLengthValueEnd);
		return std::stoi(contentLengthValue);
	}
	return -1;
}


void Server::handleRequest3()
{
	/* Dummy response start */
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body;
	/* Dummy response end */


	const int bufferSize = 10;
	char buffer[bufferSize] = {0};
	int bytesRead;
	std::string request;
	int emptyLinePos = -1;

	bool isHeadersRead = false;
	std::size_t contentLengthNum = std::string::npos;
	while (1)
	{
		bytesRead = read(this->clientSocket, buffer, bufferSize);
		std::cout << "=== Reading in chunks bytes: " << bytesRead << std::endl;
		if (bytesRead <= 0)
			continue ;
		request += std::string(buffer, bytesRead);
		std::cout << "Request at the moment read: " << request << std::endl;

		// Check if the request is complete (ends with "\r\n\r\n")
		if (!isHeadersRead && request.find("\r\n\r\n") != std::string::npos)
		{
			emptyLinePos = request.find("\r\n\r\n");
			isHeadersRead = true;
			contentLengthNum = findContentLength(request);
			if (contentLengthNum == std::string::npos)
				break;
		}
		if (isHeadersRead && contentLengthNum != -std::string::npos)
		{
			if (request.length() - emptyLinePos - 4 >= contentLengthNum)
				break;
		}
	}

	std::cout << "=== Request read ===" << std::endl;
	std::cout << TEXT_YELLOW << request << RESET << std::endl;

	Request req(request);
	std::cout << req.getStartLine()["method"] << std::endl;
	std::cout << req.getStartLine()["path"] << std::endl;
	std::cout << req.getStartLine()["version"] << std::endl;

	std::cout << TEXT_GREEN;
	std::cout << "=== Response message sent ===" << std::endl;
	std::cout << response.c_str() << std::endl;
	std::cout << RESET;
	write(this->clientSocket, response.c_str(), response.length());
}

void Server::handleRequest()
{

	/* Dummy response start */
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body;
	/* Dummy response end */


	int bufferSize = 1024;
	char buffer[bufferSize];
	int bytesRead;
	std::string request;
	buffer[bufferSize - 1] = '\0';
	int count = 0;

	while (request.empty())
	{
		if (count == 0)
			std::cout << "=== Waiting for request ===" << std::endl;
		count++;

		// If the request was never read
		if (count >1000)
			break;

		// Waiting capturing the request even if the first read fails
		while (1)
		{
			bytesRead = read(this->clientSocket, buffer, bufferSize);
			std::cout << "Reading in chunks bytes: " << bytesRead << std::endl;
			if (bytesRead > 0)
			{
				std::cout << "=== Input read! ===" << std::endl;
				std::cout << "bytesRead: " << bytesRead << std::endl;
				request += std::string(buffer, bytesRead);
			}
			if (bytesRead <= 0)
				// std::cout << "bytesRead: " << bytesRead << std::endl;
				break;
		}
	}
	std::cout << "Last bytesRead: " << bytesRead << std::endl;
	std::cout << "=== Request read in chunks ===" << std::endl;
	std::cout << TEXT_YELLOW << request << RESET << std::endl;

	Request req(request);

/*	// Testing request
	std::cout << TEXT_CYAN;
	std::cout << req.getStartLine()["method"] << std::endl;
	std::cout << req.getStartLine()["path"] << std::endl;
	std::cout << req.getStartLine()["version"] << std::endl;
	std::cout << RESET; */

	// std::cout << "bytesRead" << bytesRead << std::endl; // message length
	std::cout << "=== Buffer ===" << std::endl;
	std::cout << buffer << std::endl;


/* 	
	// Sending dummy response
	write(clientSocket, response.c_str(), response.length());
	std::cout << "Response message sent: " <<  response.c_str() << std::endl;
 */


	// int fileFd = open("test.html", O_RDONLY);
/* 	int fileFd = open((std::string("." + req.getStartLine()["path"])).c_str(), O_RDONLY);

	std::cout << TEXT_CYAN;
	std::cout << "filename: " << req.getStartLine()["path"] << std::endl;
	std::cout << "fd for file: " << fileFd << std::endl;
	std::cout << RESET; */

/* 	if (fileFd < 0)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		// throw ServerException("Error: " + std::string(strerror(errno)));
	}
	else
	{
		std::cout << "File opened" << std::endl;
		std::string header = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
		std::string body;

		while (1)
		{
			bytesRead = read(fileFd, buffer, bufferSize);
			if (bytesRead > 0)
			{
				std::cout << "=== File read! ===" << std::endl;
				std::cout << "bytesRead: " << bytesRead << std::endl;
				body += std::string(buffer, bytesRead);
			}
			if (bytesRead <= 0)
				// std::cout << "bytesRead: " << bytesRead << std::endl;
				break;
		}
		std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
		response = header + contentLength + "\r\n" + body;
	} */

	std::cout << TEXT_GREEN;
	std::cout << "=== Response message sent ===" << std::endl;
	std::cout << response.c_str() << std::endl;
	std::cout << RESET;
	write(this->clientSocket, response.c_str(), response.length());

	// close(this->clientSocket);
	// std::cout << "Connection closed" << std::endl;
}

void Server::shutdown()
{
	if (close(this->serverSocket) == 0)
		std::cout << "Server with address: " + whoAmI() + " was closed" << std::endl;
	else
		throw ServerException("Server with address: " + this->addressString + ":"  + std::to_string(this->port) + " could not be closed");
}

std::string Server::whoAmI() const
{
	return this->addressString + ":" + std::to_string(this->port);
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
