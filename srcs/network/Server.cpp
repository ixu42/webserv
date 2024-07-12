/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/07/13 01:34:42 by vshchuki         ###   ########.fr       */
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

Request* Server::receiveRequest(int clientSockfd)
{
	LOG_DEBUG("Server::receiveRequest called");
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
		// LOG_DEBUG("=== Reading in chunks bytes: ", bytesRead); // too much debug lol
		// LOG_DEBUG_RAW("[DEBUG] ");
		// for (int i = 0; i < bytesRead; i++)
		// 	LOG_DEBUG_RAW(buffer[i], "(", int(buffer[i]), "),");
		// LOG_DEBUG_RAW("\n");

		if (bytesRead < 0)
			continue;
		else if (bytesRead == 0)
			break;
		request += std::string(buffer, bytesRead);

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
				throw ResponseError(407, {}, "Exception has been thrown in receiveRequest() "
			"method of Server class");

			if (currRequestBodyBytes >= contentLengthNum)
				break;
		}
	}

	LOG_INFO("Request read");
	LOG_DEBUG_MULTILINE(TEXT_YELLOW, request, RESET);

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
		LOG_ERROR("Request can not be handled: ", e.what(), ": ", e.getCode());
	}
}

Response* Server::createResponse(Request* request, int code, 
						std::map<std::string, std::string> optionalHeaders)
{
	// LOG_DEBUG("createResponse() called");
	// ServerConfig* serverConfig = nullptr;
	// try
	// {
	// 	serverConfig = findServerConfig(request);
	// 	LOG_DEBUG("trying to find server config for create response");
	// }
	// catch(const ResponseError& e)
	// {
	// 	serverConfig = &getConfigs()[0];
	// 	return new Response(e.getCode(), serverConfig, optionalHeaders);
	// }
	LOG_DEBUG("createResponse() called");
	// ServerConfig* serverConfig = findServerConfig(request);
	// serverConfig = serverConfig ? serverConfig : &getConfigs()[0];

	return new Response(code, findServerConfig(request), optionalHeaders);
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
		LOG_DEBUG(TEXT_GREEN, "Bytes written: ", bytesWritten, RESET);
	}
	LOG_DEBUG(TEXT_GREEN, "response.length(): ", response.length(), RESET);
}

bool	Server::formRequestErrorResponse(t_client& client)
{
	if (client.response)
	{
		finalizeResponse(client);
		return true;
	}
	return false;
}

bool	Server::formCGIConfigAbsenceResponse(t_client& client, Server& server)
{
	if (!server.findServerConfig(client.request)->cgis["php"]
		&& !server.findServerConfig(client.request)->cgis["py"]
		&& client.request->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
	{
		client.response = createResponse(client.request, 404);
		finalizeResponse(client);
		return true;
	}
	return false;
}

void	Server::handleCGIResponse(t_client& client, Server& server)
{
	client.response = new Response();
	CGIServer::handleCGI(client, server);
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
		throw ResponseError(404);
	else if (access(filePath.c_str(), R_OK) == -1)
		throw ResponseError(403);

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
	std::string response = Response::buildResponse(*client.response);
	LOG_DEBUG("response: ", response); // Can be really huge for huge files and can interrupt the Terminal
	// write(client.fd, response.c_str(), response.size());
	sendResponse(response, client);
	delete client.request;
	delete client.response;
	client.request = nullptr;
	client.response = nullptr;
	close(client.fd);
	removeFromClients(client);
}

bool Server::validateRequest(t_client& client)
{
	try
	{
		client.request->getStartLine().at("method");
		client.request->getStartLine().at("path");
		client.request->getStartLine().at("version");
		client.request->getHeaders().at("host");
	}
	catch(const std::exception& e)
	{
		client.response = createResponse(client.request, 400);
		std::cerr << e.what() << '\n';
		return false;
	}
	return true;
}

void	Server::responder(t_client& client, Server& server)
{
	LOG_DEBUG("Server::responder() called");
	client.request->printRequest(); // can flood the Terminal if a file is uploaded
	if (formRequestErrorResponse(client)) return;
	if (formCGIConfigAbsenceResponse(client, server)) return;
	if (!validateRequest(client)) return;
	try
	{
		if (client.request->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
			handleCGIResponse(client, server);
		else
			handleNonCGIResponse(client, server);
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
	finalizeResponse(client);

	LOG_INFO("Response sent and connection closed (socket fd: ", client.fd, ")");
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
