/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/07/18 03:17:36 by vshchuki         ###   ########.fr       */
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
	_hints.ai_family = AF_INET;	// Use AF_INET6 for IPv6
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE; // For wildcard IP address
	int status = getaddrinfo(nullptr, std::to_string(port).c_str(), &_hints, &_res);
	if (status != 0)
	{
		LOG_DEBUG("getaddrinfo error: ", gai_strerror(status));
		throw ServerException("getaddrinfo error");
	}

	_serverSocket.bindAddress(_res);
	freeaddrinfo(_res);

	_serverSocket.listenForConnections(10);

}

Server::Server() : _serverSocket(Socket())
{
	LOG_DEBUG("Server default constructor called");

	initServer(nullptr, 8080);
}

Server::Server(const char* ipAddr, int port) : _serverSocket(Socket())
{
	LOG_DEBUG("Server parameterized constructor called");
	LOG_DEBUG("Server (port: ", port, ", ipAddr: ", ipAddr, ") created");

	initServer(ipAddr, port);
}

Server::~Server()
{
	LOG_DEBUG("Server destructor called");

	for (t_client& client : _clients)
	{
		close(client.fd);
		if (client.request)
			delete client.request;
		if (client.response)
			delete client.response;
	}
}

int	Server::accepter()
{
	LOG_DEBUG("Server::accepter() called");

	struct sockaddr_in clientAddr;
	int clientSockfd = _serverSocket.acceptConnection(clientAddr);
	if (clientSockfd == -1)
		return -1;
	LOG_INFO("Connection established with client (socket fd: ", clientSockfd, ")");

	t_client newClient;
    newClient.fd = clientSockfd;

	_clients.push_back(newClient);
	// _clients.push_back((t_client){clientSockfd,
	// 								nullptr, nullptr,
	// 								READING,
	// 								"", 
	// 								std::numeric_limits<std::size_t>::max(),
	// 								"", 0});
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
	ServerConfig* serverConfig = findServerConfig(&request);
	std::string sizeString = serverConfig->clientMaxBodySize;
	// std::string sizeString = serverConfig ? serverConfig->clientMaxBodySize : "100M";

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

// Request* Server::receiveRequest(int clientSockfd)
bool Server::receiveRequest(t_client& client)
{
	LOG_DEBUG("Server::receiveRequest called for fd: ", client.fd);
	const int bufferSize = 1024;
	char buffer[bufferSize] = {0};
	int bytesRead;
	// std::string request;
	int emptyLinePos = -1;
	int emptyLinesSize = 0;

	bool isHeadersRead = false;
	std::size_t contentLengthNum = std::string::npos;

	// std::size_t maxClientBodyBytes = std::numeric_limits<std::size_t>::max();
	
		bytesRead = read(client.fd, buffer, bufferSize);
		LOG_DEBUG(TEXT_YELLOW, "bytesRead in receiveRequest())): ", bytesRead, RESET);
		// LOG_DEBUG("=== Reading in chunks bytes: ", bytesRead); // too much debug lol
		// LOG_DEBUG_RAW("[DEBUG] ");
		// for (int i = 0; i < bytesRead; i++)
		// 	LOG_DEBUG_RAW(buffer[i], "(", int(buffer[i]), "),");
		// LOG_DEBUG_RAW("\n");

	if (bytesRead < 0)
		return false;
	if (bytesRead == 0)
		client.state = READY_TO_WRITE;

	if (client.state == READING)
	{
		client.requestString += std::string(buffer, bytesRead);

		// Check if the request is complete (ends with "\r\n\r\n")
		if (!isHeadersRead && (client.requestString.find("\r\n\r\n") != std::string::npos || client.requestString.find("\n\n") != std::string::npos))
		{
			emptyLinePos = client.requestString.find("\r\n\r\n") ? client.requestString.find("\r\n\r\n") : client.requestString.find("\n\n");
			emptyLinesSize = client.requestString.find("\r\n\r\n") ? 4 : 2;
			isHeadersRead = true;
			contentLengthNum = findContentLength(client.requestString);
			if (contentLengthNum == std::string::npos)
			{
				client.state = READY_TO_WRITE;
			}

			// Find maxClientBodySize
			if (client.maxClientBodyBytes == std::numeric_limits<std::size_t>::max()) // only calculate if the value is initial
				client.maxClientBodyBytes = findMaxClientBodyBytes(Request(client.requestString));
		}

		if (isHeadersRead && contentLengthNum != std::string::npos)
		{
			size_t currRequestBodyBytes = client.requestString.length() - emptyLinePos - emptyLinesSize;

			if (currRequestBodyBytes > client.maxClientBodyBytes)
				throw ResponseError(413, {}, "Exception has been thrown in receiveRequest() "
			"method of Server class");

			if (currRequestBodyBytes >= contentLengthNum)
			{
				client.state = READY_TO_WRITE;
			}
		}
		if (client.state != READY_TO_WRITE)
			// std::cout << TEXT_CYAN << client.requestString.substr(0, 500) << "\n...\n" << RESET << std::endl;
			return false;
	}

	LOG_INFO("Request read");
	std::cout << TEXT_YELLOW << client.requestString.substr(0, 1000) << "\n...\n" << RESET << std::endl;
	// std::cout << TEXT_YELLOW << client.requestString << RESET << std::endl;
	// LOG_DEBUG_MULTILINE(TEXT_YELLOW, client.requestString, RESET);

	return true;
}

void	Server::handler(Server*& server, t_client& client)
{
	try
	{
		// client.request = server->receiveRequest(client);
		if (server->receiveRequest(client))
			client.request = new Request(client.requestString);
	}
	catch (ResponseError& e)
	{
		LOG_ERROR("Request can not be handled: ", e.what(), ": ", e.getCode());
	}
}

Response* Server::createResponse(Request* request, int code, 
						std::map<std::string, std::string> optionalHeaders)
{
	LOG_DEBUG("createResponse() called");
	return new Response(code, findServerConfig(request), optionalHeaders);
}

bool Server::sendResponse(t_client& client)
{
	// size_t totalBytesWritten = 0;
	size_t		chunkSize = 10*1024;
	size_t		bytesToWrite = client.responseString.length(); // maybe copy to struct to optimize
	// const char*	buffer = client.responseString.c_str(); // maybe copy to struct to optimize

	// Calculate the remaining bytes to write
	size_t remainingBytes = bytesToWrite - client.totalBytesWritten;
	// Limit the chunk size to the remaining bytes
	size_t bytesToWriteNow = remainingBytes < chunkSize ? remainingBytes : chunkSize;

	// ssize_t bytesWritten = write(client.fd, buffer + client.totalBytesWritten, bytesToWrite - client.totalBytesWritten);
	ssize_t bytesWritten = write(client.fd, client.responseString.c_str() + client.totalBytesWritten, bytesToWriteNow);
	LOG_DEBUG(TEXT_GREEN, "Bytes written: ", bytesWritten, RESET);
	if (bytesWritten == -1) // We can not use EAGAIN or EWOULDBLOCK here
		return false;
	client.totalBytesWritten += bytesWritten;
	LOG_DEBUG(TEXT_GREEN, "client.totalBytesWritten: ", client.totalBytesWritten, RESET);
	if (bytesWritten == 0 || client.totalBytesWritten == bytesToWrite) // Handle case where write returns 0 (should not happen with regular sockets)
	{
		LOG_INFO(TEXT_GREEN, "Response written total: ", client.totalBytesWritten, RESET);
		return true;
	}	
	// else if (client.totalBytesWritten < bytesToWrite)
	return false;
	// LOG_DEBUG(TEXT_GREEN, "client.responseString.length(): ", client.responseString.length(), RESET);
}

bool	Server::formCGIConfigAbsenceResponse(t_client& client, Server& server)
{
	if (!server.findServerConfig(client.request)->cgis["php"]
		&& !server.findServerConfig(client.request)->cgis["py"]
		&& client.request->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
	{
		client.response = createResponse(client.request, 404);
		// finalizeResponse(client);
		return true;
	}
	return false;
}

void	Server::handleNonCGIResponse(t_client& client, Server &server)
{
	Location foundLocation = server.findLocation(client.request);
	LOG_DEBUG(TEXT_GREEN, "Location: ", foundLocation.path, RESET);
	
	checkIfMethodAllowed(client, foundLocation);
	if (foundLocation.redirect != "")
		handleRedirect(client, foundLocation);
	else if (foundLocation.upload && client.request->getStartLine()["method"] == "POST")
		handleUpload(client, foundLocation);
	else
		handleStaticFiles(client, foundLocation);
}

void	Server::checkIfMethodAllowed(t_client& client, Location& foundLocation)
{
	if (!foundLocation.methods[Utility::strToLower(client.request->getStartLine()["method"])])
	{
		std::string allowedMethods;
		for (auto& [methodName, methodBool] : foundLocation.methods)
		{
			if (methodBool)
				allowedMethods += allowedMethods.empty() ? Utility::strToUpper(methodName) : ", " + Utility::strToUpper(methodName);
		}
		throw ResponseError(405, {{"Allowed", allowedMethods}}, "Exception has been thrown in checkIfAllowed() "
			"method of Server class");
	}
}

std::string findUplooadFormBoundary(t_client& client)
{
	std::string boundary;
	std::string contentTypeValue = client.request->getHeaders().at("content-type");

	if (contentTypeValue.find("multipart/form-data") != std::string::npos)
	{
		
	}

	return boundary;
}

void Server:: handleUpload(t_client& client, Location& foundLocation)
{
	LOG_INFO("Handling upload...");
	// // Handle upload from API app (Thunder Client for example)
	// if (client.request->getHeaders().at("content-type") == "application/octet-stream")
	// {
		
	// }
	// // Handle upload from the HTML form
	// else if (client.request->getHeaders())
	// {
	// 	/* code */
	// }
	
	(void)foundLocation;
	(void)client;
}

void	Server::handleRedirect(t_client& client, Location& foundLocation)
{
	std::string pagePath = client.request->getStartLine()["path"].substr(foundLocation.path.length());
	size_t requestUriPos = foundLocation.redirect.find("$request_uri");
	std::string redirectUrl = foundLocation.redirect.substr(0, requestUriPos);

	if (requestUriPos != std::string::npos)
		redirectUrl.append(pagePath);

	LOG_DEBUG("Redirect URL: ", redirectUrl);
	LOG_DEBUG("Page path: ", pagePath);
	client.response = createResponse(client.request, 307, {{"Location", redirectUrl}});
}

void	Server::handleStaticFiles(t_client& client, Location& foundLocation)
{
	std::string requestPath = client.request->getStartLine()["path"];
	std::string filePath = foundLocation.root + requestPath.substr(foundLocation.path.length());
	if (access(filePath.c_str(), F_OK) == -1)
		throw ResponseError(404, {}, "Exception has been thrown in handleStaticFiles() "
			"method of Server class");
	else if (access(filePath.c_str(), R_OK) == -1)
		throw ResponseError(403, {}, "Exception has been thrown in handleStaticFiles() "
			"method of Server class");

	// If path ends with /, check for index file and directory listing, otherwise throw 403
	Response* locationResp = nullptr;
	if (requestPath.back() == '/')
	{
		if (access((filePath + foundLocation.index).c_str(), F_OK) == 0)
		{
			filePath += foundLocation.index;
			if (access((filePath).c_str(), R_OK) != 0)
				throw ResponseError(403);
		}
		else if (foundLocation.autoindex)
			locationResp = DirLister::createDirListResponse(foundLocation, requestPath);
		else
			throw ResponseError(404, {}, "Exception has been thrown in handleStaticFiles() "
			"method of Server class"); // 
	}
	
	// Checks if location response was formed, otherwise creates Response from filePath
	client.response = !locationResp ? new Response(200, filePath) : locationResp; 
}

void	Server::finalizeResponse(t_client& client)
{
	delete client.request;
	delete client.response;
	client.request = nullptr;
	client.response = nullptr;
	close(client.fd);
	removeFromClients(client);
}

void Server::validateRequest(t_client& client)
{
	LOG_DEBUG("validateRequest()");
	if (!client.request)
		throw ResponseError(400, {}, "Request is nullptr");
	if (client.request->getStartLine()["method"].empty() ||
		client.request->getStartLine()["path"].empty() ||
		client.request->getStartLine()["version"].empty() ||
		client.request->getHeaders()["host"].empty())
		throw ResponseError(400, {}, "Request does not have mandatory fields");
	if (client.request->getStartLine()["version"] != "HTTP/1.1")
		throw ResponseError(505, {}, "Wrong HTTP version in the start line");
	// try
	// {
	// 	client.request->getStartLine().at("content-length");
	// }
	// catch(const std::exception& e)
	// {
	// 	throw ResponseError(411, {}, "No content length provided");
	// }
}

void	Server::responder(t_client& client, Server& server)
{
	LOG_DEBUG("Server::responder() called");
	// if (formRequestErrorResponse(client)) return; // TODO: remove this line later
	if ((client.response && !client.response->getBody().empty()) || formCGIConfigAbsenceResponse(client, server))
	{
		client.state = FINISHED_WRITING;
		return;	
	}
	try
	{
		validateRequest(client);
		client.request->printRequest(); // can flood the Terminal if a file is uploaded
		if (client.request->getStartLine()["path"].find("/cgi-bin") != std::string::npos && client.stateCGI == INIT)
		{
			CGIServer::handleCGI(client);
			client.stateCGI = FORKED;
			LOG_INFO("cgi switched to forked");
		}
		else
			handleNonCGIResponse(client, server);
		SessionsManager::handleSessions(client);
	}
	catch (ResponseError& e)
	{
		delete client.response;
		LOG_ERROR("Responder caught an error: ", e.what(), ": ", e.getCode());
		client.response = createResponse(client.request, e.getCode(), e.getHeaders());
	}
	catch (const std::exception& e)
	{
		delete client.response;
		LOG_ERROR("Responder caught an exception: ", e.what());
		client.response = createResponse(client.request, 500);
	}
	if (!client.response)
		client.response = createResponse(client.request, 500);
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

std::string Server::whoAmI() const
{
	return _ipAddr + ":" + std::to_string(_port);
}

/** If no match found, the first config will be used */
ServerConfig* Server::findServerConfig(Request* req)
{
	// If request host is an ip address:port or if the ip is not specified for current server, the first config for the server is used
/* 	if(!req)
		throw ResponseError(400, {}, "Exception has been thrown in findServerConfig() "
			"method of Server class");
	if (req->getHeaders()["host"].empty())
		throw ResponseError(400, {}, "Exception (empty host) has been thrown in findServerConfig() "
			"method of Server class"); // Bad request */


	if(!req || req->getHeaders()["host"].empty())
		return &_configs[0];

	std::vector<std::string> hostSplit = Utility::splitString(req->getHeaders()["host"], ":");

	std::string reqHost = Utility::trim(hostSplit[0]);
	std::string reqPort = Utility::trim(hostSplit[1]);

	if (reqPort.empty())
		reqPort = "80";

	if (whoAmI() == req->getHeaders()["host"] ||
	// Also additional check can be needed for the port 80. The port might be not specified in the request. Check with sudo ./webserv
		(_ipAddr.empty() && std::to_string(_port) == reqPort)) 
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
	LOG_INFO("Searching for server for current location...");
	if (!req)
		throw ResponseError(400, {}, "Exception (no request) has been thrown in findLocation() "
			"method of Server class");

	ServerConfig* namedServerConfig = findServerConfig(req);
	// This block might be redundant as we always have a server config???f
	if (!namedServerConfig)
	{
		throw ResponseError(404, {}, "Exception (no ServerConfig) has been thrown in findLocation() "
			"method of Server class");
	}
	if (namedServerConfig->locations.empty())
	{
		LOG_ERROR("No locations found for server: ", whoAmI());
		throw ResponseError(404, {}, "Exception has been thrown in findLocation() "
			"method of Server class");
	}

	LOG_INFO("Server found. Searching for location...");
	// Find the longest matching location
	Location foundLocation;
	size_t locationLength = 0;
	std::string requestPath = req->getStartLine()["path"];

	LOG_INFO("Let's find location for request path: ", requestPath);
	LOG_INFO("We have locations to check: ", namedServerConfig->locations.size());
	for (Location& location : namedServerConfig->locations)
	{
		LOG_INFO("Path: ", location.path, " RequestPath: ", requestPath);
		if (location.path == requestPath)
		{
			LOG_INFO("Location found, perfect match: ", location.path);
			foundLocation = location;
			break;
		}
		else if (requestPath.rfind(location.path, 0) == 0 && location.path[location.path.length() - 1] == '/')
		{
			LOG_INFO("Location found: ", location.path);
			if (location.path.length() > locationLength)
			{
				locationLength = location.path.length();
				foundLocation = location;
			}
		}
	}
	return foundLocation;
}

/**
 * Getters
*/

std::vector<ServerConfig> Server::getConfigs()
{
	return _configs;
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

std::vector<struct pollfd>* Server::getFds()
{
	return _managerFds;
}

/**
 * Setters
 */

void Server::setConfig(std::vector<ServerConfig> serverConfigs)
{
	_configs = serverConfigs;
}

void Server::setFds(std::vector<struct pollfd>* fds)
{
	_managerFds = fds;
}
