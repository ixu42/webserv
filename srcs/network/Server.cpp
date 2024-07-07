/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/07/08 00:56:10 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::initServer(const char* ipAddr, int port)
{
	_port = port;
	std::string str(ipAddr);
	_ipAddr = str;
	
	_serverSocket.create();

	memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_INET;			 	// Use AF_INET6 for IPv6
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE;			// For wildcard IP address	
	int status = getaddrinfo(nullptr, std::to_string(port).c_str(), &_hints, &_res);
	if (status != 0) {
		std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		throw ServerException("getaddrinfo error");
	}

	_serverSocket.bindAddress(_res);
	freeaddrinfo(_res);

	_serverSocket.listenForConnections(10);

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

	for (t_client& client : _clients)
	{
		close(client.fd);
		if (client.request)
			delete client.request;
	}
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

	_clients.push_back((t_client){clientSockfd, nullptr, nullptr});
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

/* When headers and start line are read in receiveRequest, maxClientBodySize can be found */
size_t Server::findMaxClientBodyBytes(Request request)
{
	 // 104857600 bytes; // = 100M

	ServerConfig* serverConfig = findServerConfig(&request);
	std::string sizeString = serverConfig->clientMaxBodySize;

	// if (sizeString.empty())
	// 	return 0;

	// Parse the numeric part of the string
	size_t multiplier = 1;
	int numericValue = std::stoi(sizeString);

	// Determine multiplier based on the suffix
	char suffix = std::toupper(sizeString.back());
	switch (suffix) {
		case 'G':
			multiplier *= 1024;
		case 'M':
			multiplier *= 1024;
		case 'K':
			multiplier *= 1024;
		case 'B':
			break;
		default:
			break;
	}

	return static_cast<std::size_t>(numericValue * multiplier);
}

Request* Server::receiveRequest(int clientSockfd)
{
	DEBUG("Server::receiveRequest called");
	const int bufferSize = 10;
	char buffer[bufferSize] = {0};
	int bytesRead;
	std::string request;
	int emptyLinePos = -1;
	int emptyLinesSize = 0;

	bool isHeadersRead = false;
	std::size_t contentLengthNum = std::string::npos;

	std::size_t maxClientBodyBytes = std::numeric_limits<std::size_t>::max();
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
			emptyLinesSize = request.find("\r\n\r\n") ? 4 : 2;
			isHeadersRead = true;
			contentLengthNum = findContentLength(request);
			if (contentLengthNum == std::string::npos)
				break;

			// Find maxClientBodySize
			maxClientBodyBytes = findMaxClientBodyBytes(Request(request));
		}

		if (isHeadersRead && contentLengthNum != -std::string::npos)
		{
			size_t currRequestBodyBytes = request.length() - emptyLinePos - emptyLinesSize;

			if (currRequestBodyBytes > maxClientBodyBytes)
				throw ResponseError(407);

			if (currRequestBodyBytes >= contentLengthNum)
				break;
		}
	}

	std::cout << "=== Request read ===" << std::endl;
	std::cout << TEXT_YELLOW << request << RESET << std::endl;

	return new Request(request);
}

void	Server::handler(Server*& server, t_client& client)
{
	try 
	{
		client.request = server->receiveRequest(client.fd);
	}
	catch (ResponseError& e)
	{
		std::cerr << BG_RED << TEXT_WHITE << "Request can not be handled: " << e.what() << ": " << e.getCode() << RESET << std::endl;
		client.response = new Response(e.getCode());
	}
}

void Server::sendResponse(std::string& response, t_client& client)
{
	size_t totalBytesWritten = 0;
	size_t bytesToWrite = response.length();
	const char* buffer = response.c_str();

	while (totalBytesWritten < bytesToWrite)
	{
		ssize_t bytesWritten = write(client.fd, buffer + totalBytesWritten, bytesToWrite - totalBytesWritten);
		if (bytesWritten == -1) // We can not use EAGAIN or EWOULDBLOCK here
			continue;
		else if (bytesWritten == 0) // Handle case where write returns 0 (should not happen with regular sockets)
			break;
		else {
			totalBytesWritten += bytesWritten;
		}
		std::cout << TEXT_GREEN << "Bytes written: " << bytesWritten  << RESET << std::endl;
	}
	std::cout << TEXT_GREEN << "response.length(): " << response.length()  << RESET << std::endl;
}

/**
 * Creates dynamic body for Response using, current location and html template
 * html template should have 
 */
Response Server::createDirListResponse(Location& location, std::string requestPath)
{
	std::string pathShortCode = "[path]";
	std::string bodyShortCode = "[body]";
	Response listingResponse = Response();
	std::string fileString = Utility::readFile(location.defaultListingTemplate);
	std::stringstream htmlStream;

	// Find the root for requstPath
	std::string root = location.root + requestPath.substr(location.path.length());

	try
	{
		htmlStream << "<div class=\"file-list\">\n";
		htmlStream << "<div class=\"file-row\">\n";
		htmlStream << "<div class=\"file-cell\"><a href=\"../\">../</a></div>\n";
		htmlStream << "<div class=\"file-cell\"></div>\n";
		htmlStream << "<div class=\"file-cell\"></div>\n";
		htmlStream << "</div>\n";
		// Iterate over all the entries in the directory
		for (const auto& entry : fs::directory_iterator(root)) {
			fs::path filePath = entry.path();
			std::string fileName = filePath.filename().string();

			// Calculate last write time
			auto ftime = fs::last_write_time(filePath);
			auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now()
					+ std::chrono::system_clock::now());
			std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

			htmlStream << "<div class=\"file-row\">\n";
			// Output name
			htmlStream << "<div class=\"file-cell name\"><a href=\"" << fileName;
			if (entry.is_directory())
				htmlStream << "/";
			htmlStream << "\">" << fileName;
			if (entry.is_directory())
				htmlStream << "/"; 
			htmlStream << "</a></div>\n";
			// Output last write time
			htmlStream << "<div class=\"file-cell date\">" << std::put_time(std::localtime(&cftime), "%d-%b-%Y %T") << "</div>";
			// Output size
			htmlStream << "<div class=\"file-cell size\">";
			htmlStream << (!entry.is_directory() ? std::to_string(fs::file_size(filePath)) : "-");
			htmlStream << "</div></div>\n";
		}
		htmlStream << "</div>\n";
	}
	catch (const fs::filesystem_error& e)
	{
		std::cerr << "Error accessing directory: " << e.what() << "\n";
		throw ResponseError(403);
	}

	// Replace [body]
	size_t replaceStringPos = fileString.find(bodyShortCode);
	if (replaceStringPos != std::string::npos)
		fileString.replace(replaceStringPos,  bodyShortCode.length(), htmlStream.str());
	// Replace [title]
	while ((replaceStringPos = fileString.find(pathShortCode)) != std::string::npos)
		fileString.replace(replaceStringPos, pathShortCode.length(), requestPath);

	listingResponse.setBody(fileString);
	listingResponse.setTypeFromFormat("html");
	listingResponse.setStatusFromCode(200);
	return listingResponse;
}

void	Server::responder(t_client& client, Server &server)
{
	/* If response was handled previously in handler (e.g. in receiveRequest) */
	if (client.response)
	{
		std::string responseString = Response::buildResponse(*client.response);
		// write(client.fd, responseString.c_str(), responseString.length());
		sendResponse(responseString, client);
		delete client.response;
		client.response = nullptr;
		return;
	}

	DEBUG("Server::responder() called");
	Response resp;
	std::string response;
	
	// uncomment the following line for checking content of request
	// client.request->printRequest();

	// replace writing a dummy response by the actual response
	// request obj can be accessed by e.g. client.request->
	
	if (client.request->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
	{
		// resp.setCGIflag(true); // wtf is this?
		try {
			CGIServer::handleCGI(*(client.request), server, resp);
		} catch (const ServerException& e) {
			std::cerr << BG_RED << e.what() << RESET_BG << std::endl;
		}
		DEBUG("response: " << response);
	}
	else
	{
		try
		{
			Location foundLocation = server.findLocation(client.request);
			std::cout << TEXT_GREEN << "Location: " << foundLocation.path << RESET << std::endl;

			/* Check if method is allowed */
			// client.request->printRequest();
			if (!foundLocation.methods[Utility::strToLower(client.request->getStartLine()["method"])])
			{
				std::string allowedMethods;
				for (auto& [methodName, methodBool] : foundLocation.methods)
				{
					if (methodBool)
						allowedMethods += allowedMethods.empty() ? methodName : ", " + methodName;
				}
				throw ResponseError(405, {{"Allowed", allowedMethods}});
			}

			/* Handle redirect */
			if (foundLocation.redirect != "")
			{
			// std::cout << TEXT_GREEN << "Redirect found: " << foundLocation.redirect << RESET << std::endl;
				std::string pagePath = client.request->getStartLine()["path"].substr(foundLocation.path.length());
				size_t requestUriPos = foundLocation.redirect.find("$request_uri");
				std::string redirectUrl = foundLocation.redirect.substr(0, requestUriPos);

				if (requestUriPos != std::string::npos)
					redirectUrl.append(pagePath);

				std::cout << "Redirect URL: " << redirectUrl << std::endl;
				std::cout << "Page path: " << pagePath << std::endl;
				resp = Response(307, {{"Location", redirectUrl}});
			}
			else
			{
				/* Handle static files */
				std::string requestPath = client.request->getStartLine()["path"];
				std::string filePath = foundLocation.root + requestPath.substr(foundLocation.path.length());
				Response locationResp;

				// If path ends with /, check for index file and directory listing, otherwise throw 403
				if (requestPath.back() == '/')
				{
					std::ifstream indexFile;
					indexFile.open(filePath + foundLocation.index);
					if (indexFile.is_open())
					{
						filePath += foundLocation.index;
						indexFile.close();
					}
					else if (foundLocation.autoindex)
						locationResp = createDirListResponse(foundLocation, requestPath);
					else
						throw ResponseError(403);
				}
				resp = locationResp.getStatus().empty() ? Response(200, filePath) : locationResp; // Checks if location response was formed, otherwise creates Response from filePath
			}
		}
		catch (ResponseError& e)
		{
			std::cerr << BG_RED << TEXT_WHITE << "Responder caught an error: " << e.what() << ": " << e.getCode() << RESET << std::endl;
			resp = Response(e.getCode(), e.getHeaders());
						// resp = Response(e.getCode(), "pages/" + std::to_string(e.getCode()) + ".html");
		}
		catch (const std::exception& e)
		{
			std::cerr << BG_RED << TEXT_WHITE << "Responder caught an exception: " << e.what() << RESET << std::endl;
			resp = Response(500);
		}
	}

	response = Response::buildResponse(resp);

	sendResponse(response, client);

	delete client.request;
	client.request = nullptr;

	// after writing, close the connection
	close(client.fd);
	removeFromClients(client);

	std::cout << "\n=== RESPONSE SENT AND CONNECTION CLOSED (SOCKET FD: "
				<< client.fd << ") ===\n\n";
}

void	Server::removeFromClients(t_client& client)
{
	// remove from _clients vector
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (&(*it) == &client)
		{
			_clients.erase(it);
			break ;
		}
	}
}
/* Dummy response */
/* const std::string	Server::getResponse()
{
	std::string response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\n";
	// std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string body = "<html lang=\"en\">\r\n<head>\r\n\t<meta charset=\"UTF-8\">\r\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\t<title>WebServ Response</title>\r\n</head>\r\n<body>\r\n\t<h1>\r\n\t\tHello world " + std::to_string(time(NULL)) + " " + whoAmI() + "\r\n\t</h1>\r\n</body>\r\n</html>";
	std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
	// std::cout << contentLength << std::endl;
	response = response + contentLength + "\r\n" + body + "\r\n";

	return response;
} */


std::string Server::whoAmI() const
{
	return _ipAddr + ":" + std::to_string(_port);
}

/** If no match found, the first config will be used */
ServerConfig* Server::findServerConfig(Request* req)
{
	// If request host is an ip address:port or if the ip is not specified for current server, the first config for the server is used
	if(!req)
		throw ResponseError(400);
	if (req->getHeaders()["host"].empty())
		throw ResponseError(400); // Bad request

	std::vector<std::string> hostSplit = Utility::splitString(req->getHeaders()["host"], ":");

	std::string reqHost = Utility::trim(hostSplit[0]);
	std::string reqPort = Utility::trim(hostSplit[1]);

	if (reqPort.empty())
		reqPort = "80";

	if (whoAmI() == req->getHeaders()["host"] ||
		(_ipAddr.empty() && std::to_string(_port) == reqPort)) // Also additional check can be needed for the port 80. The port might be not specified in the request. Check with sudo ./webserv
	{
		if (!_configs.empty())
			return &_configs[0];
	}

	// If request is a servername, find the correct servername
	if (!reqHost.empty())
	{
		for (ServerConfig& config : _configs)
		{
			if (config.serverName == reqHost)
				return &config;
		}
	}

	if (_configs.empty())
		throw ServerException("Program has no configs");

	return &_configs[0];
}


Location Server::findLocation(Request* req)
{
	std::cout << "== Finding server for current location ==" << std::endl;
	if (!req)
		throw ResponseError(400);

	ServerConfig* namedServerConfig = findServerConfig(req);
	// This block might be redundant as we always have a server config???
	if (!namedServerConfig)
	{
		throw ResponseError(404);
	}
	if (namedServerConfig->locations.empty())
	{
		std::cerr << "No locations found for server: " << whoAmI() << std::endl;
		throw ResponseError(404);
	}

	std::cout << "== Server found. Finding location... ==" << std::endl;
	// Find the longest matching location
	Location foundLocation;
	size_t locationLength = 0;
	std::string requestPath = req->getStartLine()["path"];

	std::cout << "Let's find location for request path: " << requestPath << std::endl;
	std::cout << "We have locations to check: " << namedServerConfig->locations.size() << std::endl;
	for (Location& location : namedServerConfig->locations)
	{
		std::cout << "Path: " << location.path << " RequestPath: " << requestPath << std::endl;
		if (location.path == requestPath)
		{
			std::cout << "Location found, perfect match: " << location.path << std::endl;
			foundLocation = location;
			break;
		}
		else if (requestPath.rfind(location.path, 0) == 0 && location.path[location.path.length() - 1] == '/')
		{
			std::cout << "Location found: " << location.path << std::endl;
			if (location.path.length() > locationLength)
			{
				locationLength = location.path.length();
				foundLocation = location;
			}
		}

		// std::cout << "Server config and location: " << location.path << std::endl;
	}
	return foundLocation;
}


/**
 * Getters
*/

// std::vector<ServerConfig> Server::getConfig()
// {
// 	return _configs;
// }

Pipe& Server::getPipe()
{
	return _CGIpipes;
}

int Server::getServerSockfd()
{
	return _serverSocket.getSockfd();
}

std::vector<t_client>& Server::getClients()
{
	return _clients;
}

std::string Server::getIpAddress()
{
	return _ipAddr;
}

int Server::getPort()
{
	return _port;
}

/**
 * Setters
 */

void Server::setConfig(std::vector<ServerConfig> serverConfigs)
{
	_configs = serverConfigs;
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

	// std::cout << "Locations vector size after request generated: " << this->config->locations.size() << std::endl;
	// for (Location location : this->config->locations)
	// {
	// 	std::cout << "Server config and location from handleRequest: " << location.path << std::endl;
	// }
	// Location* foundLocation = findLocation(&req);
	// if (foundLocation == nullptr)
	// {
	// 	// throw ServerException("Location not found");
	// 	std::cout << nullptr << std::endl;
	// 	response = "HTTP/1.1 404 Not Found\r\nServer: webserv\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	// 	// response = "HTTP/1.1 200 OK\r\nServer: webserv\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	// }
	// else
	// {
	// 	std::cout << "Great this is your location: " << foundLocation.path << std::endl;
	// 	// Handling redirect
	// 	if (foundLocation.redirect != "")
	// 	{
	// 		std::string redirectUrl = foundLocation.redirect;

	// 		size_t requestUriPos = foundLocation.redirect.find("$request_uri");

	// 		std::string pagePath = req.getStartLine()["path"];
	// 		pagePath.replace(0, foundLocation.path.length(), "");

	// 		redirectUrl = redirectUrl.substr(0, requestUriPos);

	// 		if (requestUriPos != std::string::npos)
	// 			redirectUrl.append(pagePath);

	// 		std::cout << "Redirect URL: " << redirectUrl << std::endl;
	// 		std::cout << "Page path: " << pagePath << std::endl;
	// 		response = "HTTP/1.1 307 Temporary Redirect\r\nServer: webserv\r\nLocation: " + redirectUrl + "\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
	// 	}
	// }

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
