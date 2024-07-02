/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/07/01 19:09:40 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "debug.hpp"
#include <cstring> // memset()
#include <arpa/inet.h> // htons(), inet_pton()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <string>

void	Server::initServer(const char* ipAddr, int port)
{
	_port = port;
	std::string str(ipAddr);
	_ipAddr = str;

	std::memset((char*)&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_port = htons(port);
	if (ipAddr == nullptr)
		_address.sin_addr.s_addr = INADDR_ANY;
	else
	{
		int ret = inet_pton(AF_INET, ipAddr, &_address.sin_addr);
		if (ret <= 0)
		{
			std::cerr << "inet_pton() error\n";
			close(_serverSocket.getSockfd());
			exit(EXIT_FAILURE);
		}
	}

	if (!_serverSocket.create() ||
		!_serverSocket.bindAddress(_address) ||
		!_serverSocket.listenForConnections(10))
	{
		std::cerr << "Failed to construct server\n";
		exit(EXIT_FAILURE);
	}
}

Server::Server() : _serverSocket(Socket())
{
	DEBUG("Server default constructor called");

	initServer(nullptr, 8080);
}

Server::Server(const char* ipAddr, int port) : _serverSocket(Socket())
{
	DEBUG("Server parameterized constructor called");

	initServer(ipAddr, port);
}

Server::~Server()
{

	DEBUG("Server destructor called");

	for (t_client client : _clients)
		close(client.fd);
}

int	Server::accepter()
{
	DEBUG("Server::accepter() called");

	struct sockaddr_in clientAddr;
	int clientSockfd = _serverSocket.acceptConnection(clientAddr);
	if (clientSockfd == -1)
		return -1;
	std::cout << "\n=== CONNECTION ESTABLISHED WITH CLIENT (SOCKET FD: "
				<< clientSockfd << ") ===\n";

	_clients.push_back((t_client){clientSockfd, nullptr});
	return clientSockfd;
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

Request* Server::receiveRequest(int clientSockfd)
{
	DEBUG("Server::receiveRequest called");
	const int bufferSize = 10;
	char buffer[bufferSize] = {0};
	int bytesRead;
	std::string request;
	int emptyLinePos = -1;

	bool isHeadersRead = false;
	std::size_t contentLengthNum = std::string::npos;
	while (1)
	{
		bytesRead = read(clientSockfd, buffer, bufferSize);
		std::cout << "=== Reading in chunks bytes: " << bytesRead << std::endl;
		for (int i = 0; i < bytesRead; i++)
			std::cout << buffer[i]<< "(" << int(buffer[i]) << ")," ;
		std::cout << std::endl;

		if (bytesRead < 0)
			continue;
		else if (bytesRead == 0)
			break;
		request += std::string(buffer, bytesRead);
		// std::cout << "Request at the moment read: " << request << std::endl;

		// Check if the request is complete (ends with "\r\n\r\n")
		if (!isHeadersRead && (request.find("\r\n\r\n") != std::string::npos || request.find("\n\n") != std::string::npos))
		{
			emptyLinePos = request.find("\r\n\r\n") ? request.find("\r\n\r\n") : request.find("\n\n");
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

	return new Request(request);
}

void	Server::responder(int clientSockfd)
{
	DEBUG("Server::responder() called");

	const std::string& response = getResponse();
	write(clientSockfd, response.c_str(), response.length());

	// after writing, close the connection
	close(clientSockfd);
	removeFromClients(clientSockfd);
	std::cout << "\n=== RESPONSE SENT AND CONNECTION CLOSED (SOCKET FD: "
				<< clientSockfd << ") ===\n";
	DEBUG("response: " << response);
}

void	Server::removeFromClients(int clientSockfd)
{
	// remove from _clients vector
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		if ((*it).fd == clientSockfd)
		{
			_clients.erase(it);
			break ;
		}
	}
}

const std::string	Server::getResponse()
{
	/* Dummy response start */
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body + "\r\n";
	// /* Dummy response end */



	return response;
}


std::string Server::whoAmI() const
{
	return _ipAddr + ":" + std::to_string(_port);
}

/**
 * Getters
*/

ServerConfig* Server::getConfig()
{
	return _config;
}

int Server::getServerSockfd()
{
	return _serverSocket.getSockfd();
}

std::vector<t_client> Server::getClients()
{
	return _clients;
}

/**
 * Setters
 */

void Server::setConfig(ServerConfig* serverConfig)
{
	if (serverConfig == nullptr)
		return ;
	_config = serverConfig;
}


// void	Server::handler(int clientSockfd)
// {
// 	DEBUG("Server::handler() called");

// 	char buffer[1024];
// 	int bytesRead = read(clientSockfd, buffer, sizeof(buffer));
// 	if (bytesRead == -1)
// 		std::cerr << "read() error\n";
// 	else if (bytesRead == 0)
// 	{
// 		// connection closed by client
// 		close(clientSockfd);
// 		removeFromClientSockfds(clientSockfd);
// 		std::cout << "\n=== CLIENT DISCONNECTED (SOCKET FD: " 
// 					<< clientSockfd << ")===\n\n";
// 	}
// 	else
// 	{
// 		// handle received data (e.g. process HTTP request)
// 		std::cout << "\n--- Received data from client ---\n\n"
// 					<< std::string(buffer, bytesRead);
// 	}
// }


// Vladimir's Server.cpp
/* #include "Server.hpp"

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

} */

// void Server::handleRequest2()
// {

// 	/* Dummy response start */
// 	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
// 	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
// 	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
// 	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
// 	// std::cout << contentLength << std::endl;
// 	response = response + contentLength + "\r\n" + body;
// 	/* Dummy response end */


// 	const int bufferSize = 30000;
// 	char buffer[bufferSize] = {0};
// 	int bytesRead;
// 	std::string request;


// 	bytesRead = read(this->clientSocket, buffer, bufferSize);
// 	std::cout << "Reading in chunks bytes: " << bytesRead << std::endl;
// 	std::cout << "=== Input read! ===" << std::endl;
// 	std::cout << "bytesRead: " << bytesRead << std::endl;
// 	request += std::string(buffer, bytesRead);

// 	std::cout << "=== Request read ===" << std::endl;
// 	std::cout << TEXT_YELLOW << request << RESET << std::endl;



// 	std::cout << TEXT_GREEN;
// 	std::cout << "=== Response message sent ===" << std::endl;
// 	std::cout << response.c_str() << std::endl;
// 	std::cout << RESET;
// 	write(this->clientSocket, response.c_str(), response.length());

// }




// Location* Server::findLocation(Request* req)
// {
// 	(void)req;
// 	std::cout << "== Finding location for current request ==" << std::endl;
// 	// int i = 0;
// 	std::cout << "Locations vector size: " << this->config->locations.size() << std::endl;
// 	if (this->config->locations.empty())
// 		return nullptr;
// 		// throw ServerException("No location specified in the server config");

// 	Location* foundLocation = nullptr;
// 	size_t locationLength = 0;
// 	std::string requestPath = req->getStartLine()["path"];

// 	std::cout << "Let's find location for request path: " << requestPath << std::endl;
// 	for (Location& location : this->config->locations)
// 	{
// 		std::cout << "Path: " << location.path << " RequestPath: " << requestPath << std::endl;
// 		if (location.path == requestPath)
// 		{
// 			std::cout << "Location found, perfect match: " << location.path << std::endl;
// 			foundLocation = &location;
// 			break;
// 		}
// 		else if (requestPath.rfind(location.path, 0) == 0 && location.path[location.path.length() - 1] == '/')
// 		{
// 			std::cout << "Location found: " << location.path << std::endl;
// 			if (location.path.length() > locationLength)
// 			{
// 				locationLength = location.path.length();
// 				foundLocation = &location;
// 			}
// 		}

// 		// std::cout << "Server config and location: " << location.path << std::endl;
// 	}
// 	return foundLocation;
// }



// 	std::cout << "=== Request read ===" << std::endl;
// 	std::cout << TEXT_YELLOW << request << RESET << std::endl;

// 	return Request(request);
// }

// void Server::handleRequest3()
// {
// 	/* Dummy response start */
// 	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
// 	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
// 	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
// 	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
// 	// std::cout << contentLength << std::endl;
// 	response = response + contentLength + "\r\n" + body;
// 	/* Dummy response end */


// 	Request req = receiveRequest();

// 	// std::cout << "Locations vector size after request generated: " << this->config->locations.size() << std::endl;
// 	// for (Location location : this->config->locations)
// 	// {
// 	// 	std::cout << "Server config and location from handleRequest: " << location.path << std::endl;
// 	// }
// 	Location* foundLocation = findLocation(&req);
// 	if (foundLocation == nullptr)
// 	{
// 		// throw ServerException("Location not found");
// 		std::cout << nullptr << std::endl;
// 		response = "HTTP/1.1 404 Not Found\r\nServer: webserv\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
// 		// response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
// 	}
// 	else
// 	{
// 		std::cout << "Great this is your location: " << foundLocation->path << std::endl;
// 		// Handling redirect
// 		if (foundLocation->redirect != "")
// 		{
// 			std::string redirectUrl = foundLocation->redirect;

// 			size_t requestUriPos = foundLocation->redirect.find("$request_uri");

// 			std::string pagePath = req.getStartLine()["path"];
// 			pagePath.replace(0, foundLocation->path.length(), "");

// 			redirectUrl = redirectUrl.substr(0, requestUriPos);

// 			if (requestUriPos != std::string::npos)
// 				redirectUrl.append(pagePath);

// 			std::cout << "Redirect URL: " << redirectUrl << std::endl;
// 			std::cout << "Page path: " << pagePath << std::endl;
// 			response = "HTTP/1.1 307 Temporary Redirect\r\nServer: webserv\r\nLocation: " + redirectUrl + "\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
// 		}
// 	}

// 	// Testing request
// 	std::cout << TEXT_CYAN;
// 	std::cout << req.getStartLine()["method"] << std::endl;
// 	std::cout << req.getStartLine()["path"] << std::endl;
// 	std::cout << req.getStartLine()["version"] << std::endl;
// 	std::cout << RESET;

// 	std::cout << TEXT_GREEN;
// 	std::cout << "=== Response message sent ===" << std::endl;
// 	std::cout << response.c_str() << std::endl;
// 	std::cout << RESET;
// 	write(this->clientSocket, response.c_str(), response.length());
// }


// std::string Server::whoAmI() const
// {
// 	return this->addressString + ":" + std::to_string(this->port);
// }

// /**
//  * Getters
//  */

// int Server::getSocket() const
// {
// 	return this->serverSocket;
// }

// struct sockaddr_in &Server::getSockAddress()
// {
// 	return this->sockAddress;
// }

// int Server::getClientSocket() const
// {
// 	return this->clientSocket;
// }

// ServerConfig* Server::getConfig()
// {
// 	return this->config;
// }

// /**
//  * Setters
//  */

// void Server::setClientSocket(int newClientSocket)
// {
// 	this->clientSocket = newClientSocket;
// }

// void Server::setConfig(ServerConfig* serverConfig)
// {
// 	if (serverConfig == nullptr)
// 		return ;
// 	// for (Location location : serverConfig->locations)
// 	// {
// 	// 	std::cout << "Server config and location: " << location.path << std::endl;
// 	// }
// 	this->config = serverConfig;
// 	// for (Location location : this->config->locations)
// 	// {
// 	// 	std::cout << "Server config and location now: " << location.path << std::endl;
// 	// }
// }
