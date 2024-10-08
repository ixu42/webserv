/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:56 by ixu               #+#    #+#             */
/*   Updated: 2024/08/13 10:46:50 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::initServer(const char *ipAddr, int port)
{
	_port = port;
	std::string str(ipAddr);
	_ipAddr = str;

	_serverSocket.create();

	memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_INET; // Use AF_INET6 for IPv6
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
	
	std::string folder = "cgi-bin/";
	_CGIBinFolder = _webservConfig->normalizeFilePath(folder, true);
	
	if (isCGIBinExistAndReadable())
		listCGIFiles();
}

Server::Server() : _serverSocket(Socket()), _webservConfig(nullptr)
{
	LOG_DEBUG("Server default constructor called");

	initServer(nullptr, 8080);
}

Server::Server(const char *ipAddr, int port, std::shared_ptr<Config> webservConfig) : _serverSocket(Socket()), _webservConfig(webservConfig)
{
	
	LOG_DEBUG("Server parameterized constructor called");
	LOG_DEBUG("Server (port: ", port, ", ipAddr: ", ipAddr, ") created");

	initServer(ipAddr, port);
}

Server::~Server()
{
	LOG_DEBUG("Server destructor called");

	for (Client &client : _clients)
	{
		close(client.getFd());
	}
}

int Server::accepter()
{
	LOG_DEBUG("Server::accepter() called");

	struct sockaddr_in clientAddr;
	int clientSockfd = _serverSocket.acceptConnection(clientAddr);
		
	LOG_INFO("Connection established with client (socket fd: ", clientSockfd, ")");

	Client newClient;
	newClient.setFd(clientSockfd);
	_clients.push_back(newClient);

	return clientSockfd;
}

int Server::findContentLength(std::string request)
{
	std::string contentLength = "content-length:";

	request = Utility::strToLower(request);
	unsigned long contentLengthPos = request.find(contentLength);
	if (contentLengthPos != std::string::npos)
	{
		std::string contentLengthValue = request.substr(contentLengthPos + contentLength.length());
		int contentLengthValueEnd = contentLengthValue.find("\r\n");
		contentLengthValue = contentLengthValue.substr(0, contentLengthValueEnd);
		return std::stoi(Utility::trim(contentLengthValue));
	}
	return 0;
}

/* When headers and start line are read in receiveRequest, maxClientBodySize can be found */
size_t Server::findMaxClientBodyBytes(std::shared_ptr<Request> request)
{

	ServerConfig *serverConfig = findServerConfig(request);
	std::string sizeString = serverConfig->clientMaxBodySize;

	// Parse the numeric part of the string
	size_t multiplier = 1;
	int numericValue = std::stoi(sizeString);

	// Determine multiplier based on the suffix
	char suffix = std::toupper(sizeString.back());
	switch (suffix)
	{
	case 'G':
		multiplier *= 1024;
		[[fallthrough]];
	case 'M':
		multiplier *= 1024;
		[[fallthrough]];
	case 'K':
		multiplier *= 1024;
		[[fallthrough]];
	case 'B':
		break;
	default:
		break;
	}

	return static_cast<size_t>(numericValue * multiplier);
}

void Server::receiveHeaders(Client &client, std::regex pattern)
{
	// Check if the request is complete (ends with "\r\n\r\n")
	if (!client.getIsHeadersRead() && client.getRequestString().find("\r\n\r\n") != std::string::npos)
	{
		client.setEmptyLinePos(client.getRequestString().find("\r\n\r\n"));
		client.setEmptyLinesSize(4);
		client.setIsHeadersRead(true);
		client.setContentLengthNum(findContentLength(client.getRequestString()));
		if (!std::regex_search(client.getRequestString(), pattern) && client.getContentLengthNum() == 0)
		{
			client.setState(Client::ClientState::READY_TO_WRITE);
		}
		
		// Find maxClientBodySize
		// only calculate if the value is initial
		if (client.getMaxClientBodyBytes() == std::numeric_limits<size_t>::max())
			client.setMaxClientBodyBytes(findMaxClientBodyBytes(std::make_shared<Request>(client)));
	}
}

void Server::receiveBody(Client &client, std::regex pattern)
{
	if (client.getIsHeadersRead() && (client.getContentLengthNum() != std::string::npos || std::regex_search(client.getRequestString(), pattern)))
	{
		size_t currRequestBodyBytes = client.getRequestString().length() - client.getEmptyLinePos() - client.getEmptyLinesSize();

		if (currRequestBodyBytes > client.getMaxClientBodyBytes())
			throw ProcessingError(413, {}, "Exception has been thrown in receiveRequest() "
											"method of Server class");
		// Find end of chunked body
		size_t endOfChunkedBody = client.getRequestString().find("\r\n0\r\n\r\n");

		if ((client.getContentLengthNum() != 0
				&& currRequestBodyBytes >= client.getContentLengthNum()) || endOfChunkedBody != std::string::npos)
		{
			client.setState(Client::ClientState::READY_TO_WRITE);
			client.setIsBodyRead(true);
		}
	}
}

bool Server::receiveRequest(Client &client)
{
	LOG_DEBUG("Server::receiveRequest called for fd: ", client.getFd());
	char buffer[g_bufferSize];
	int bytesRead;
	std::fill(buffer, buffer + g_bufferSize, 0);
	std::regex pattern(R"(\s*transfer-encoding:\s*chunked\s*)", std::regex_constants::icase);

	bytesRead = read(client.getFd(), buffer, sizeof(buffer));
	LOG_DEBUG(TEXT_YELLOW, "bytesRead in receiveRequest())): ", bytesRead, RESET);

	if (bytesRead < 0)
		throw ProcessingError(500, {}, "receiveRequest() reading failed");
	if (bytesRead == 0)
		client.setState(Client::ClientState::READY_TO_WRITE);

	if (client.getState() == Client::ClientState::READING)
	{
		client.setRequestString(client.getRequestString() + std::string(buffer, bytesRead));

		receiveHeaders(client, pattern);
		receiveBody(client, pattern);
		if (client.getState() != Client::ClientState::READY_TO_WRITE)
			return false;
	}

	LOG_INFO("Request read");
	LOG_DEBUG(TEXT_YELLOW, client.getRequestString().substr(0, 1000), "\n...\n", RESET, "\n");
	return true;
}

bool Server::handler(std::shared_ptr<Server> &server, Client &client)
{
	try
	{
		if (server->receiveRequest(client))
			client.setRequest(std::make_shared<Request>(client));
	}
	catch (ProcessingError &e) // For example, maxClientBodySize exceeded
	{
		if (e.getCode() == 500)
			return false;
		LOG_ERROR("Request can not be handled: ", e.what(), ": ", e.getCode());
		client.setRequest(std::make_shared<Request>(client));
		client.setResponse(createResponse(client.getRequest(), e.getCode()));
		LOG_DEBUG("Response set for ProcessingError catch");
		CGIHandler::changeToErrorState(client);
	}
	catch (std::exception &e)
	{
		client.setRequest(std::make_shared<Request>(client));
		LOG_ERROR("Request handle threw an exception");
		LOG_DEBUG("host: ", client.getRequest()->getHeaders()["host"]);
		client.getRequest()->setHeader("host", getIpAddress()+ ":" + std::to_string(getPort())); // Fallback to default host
		client.setResponse(createResponse(client.getRequest(), 400));
		CGIHandler::changeToErrorState(client);
	}
	return true;
}

std::shared_ptr<Response> Server::createResponse(std::shared_ptr<Request> request, int code, std::map<std::string, std::string> optionalHeaders)
{
	LOG_DEBUG("createResponse() called");
	return std::make_shared<Response>(code, findServerConfig(request), optionalHeaders);
}

bool Server::sendResponse(Client &client)
{
	size_t bytesToWrite = client.getResponseString().length();

	// Calculate the remaining bytes to write
	size_t remainingBytes = bytesToWrite - client.getTotalBytesWritten();
	// Limit the chunk size to the remaining bytes
	size_t bytesToWriteNow = remainingBytes < g_bufferSize ? remainingBytes : g_bufferSize;

	ssize_t bytesWritten = write(client.getFd(), client.getResponseString().c_str() + client.getTotalBytesWritten(), bytesToWriteNow);
	LOG_DEBUG(TEXT_GREEN, "Bytes written: ", bytesWritten, RESET);

	if (bytesWritten == -1)
		throw ProcessingError(500, {}, "sendResponse() writing failed");
	client.setTotalBytesWritten(client.getTotalBytesWritten() + bytesWritten);
	LOG_DEBUG(TEXT_GREEN, "client.totalBytesWritten: ", client.getTotalBytesWritten(), RESET);
	
	// Handle case where write returns 0 (should not happen with regular sockets)
	if (bytesWritten == 0 || client.getTotalBytesWritten() == bytesToWrite)
	{
		LOG_INFO(TEXT_GREEN, "Response written with length: ", client.getTotalBytesWritten(), RESET);
		return true;
	}

	return false;
}

bool Server::formCGIConfigAbsenceResponse(Client &client, Server &server)
{
	if (server.findServerConfig(client.getRequest())->cgis->size() < 1
		&& client.getRequest()->getStartLine()["path"].rfind("/cgi-bin/", 0) == 0)
	{
		client.setResponse(createResponse(client.getRequest(), 500));
		return true;
	}
	return false;
}

void Server::handleNonCGIResponse(Client &client, Server &server)
{
	Location foundLocation = server.findLocation(client.getRequest());
	LOG_DEBUG(TEXT_GREEN, "Location: ", foundLocation.path, RESET);

	checkIfMethodAllowed(client, foundLocation);

	if (foundLocation.redirect != "")
		handleRedirect(client, foundLocation);
	else if (foundLocation.upload && client.getRequest()->getStartLine()["method"] == "POST")
	{
		LOG_INFO("Handling file upload...");
		client.setResponse(createResponse(client.getRequest(), Uploader::handleUpload(client, foundLocation)));
	}
	else if (client.getRequest()->getStartLine()["method"] == "DELETE")
	{
		LOG_INFO("Handling file deletion...");
		client.setResponse(createResponse(client.getRequest(), handleDelete(client, foundLocation)));
	}
	else
		handleStaticFiles(client, foundLocation);
}

void Server::checkIfMethodAllowed(Client &client, Location &foundLocation)
{
	if (!foundLocation.methods[Utility::strToLower(client.getRequest()->getStartLine()["method"])])
	{
		std::string allowedMethods;
		for (auto &[methodName, methodBool] : foundLocation.methods)
		{
			if (methodBool)
				allowedMethods += allowedMethods.empty() ? Utility::strToUpper(methodName) : ", " + Utility::strToUpper(methodName);
		}
		throw ProcessingError(405, {{"Allowed", allowedMethods}}, "Exception has been thrown in checkIfAllowed() "
																"method of Server class");
	}
}

void Server::handleRedirect(Client &client, Location &foundLocation)
{
	std::string pagePath = client.getRequest()->getStartLine()["path"].substr(foundLocation.path.length());
	size_t requestUriPos = foundLocation.redirect.find("$request_uri");
	std::string redirectUrl = foundLocation.redirect.substr(0, requestUriPos);

	if (requestUriPos != std::string::npos)
		redirectUrl.append(pagePath);

	LOG_DEBUG("Redirect URL: ", redirectUrl);
	LOG_DEBUG("Page path: ", pagePath);
	client.setResponse(createResponse(client.getRequest(), 307, {{"Location", redirectUrl}}));
}

int Server::handleDelete(Client &client, Location &foundLocation)
{
	std::string filePathString = foundLocation.root + client.getRequest()->getStartLine()["path"].substr(foundLocation.path.length());
	std::filesystem::path filePath = filePathString;

	if (access(filePathString.c_str(), F_OK) != 0)
		return 404;
	else if (access(filePathString.c_str(), W_OK) != 0)
		return 403;
	else if (std::filesystem::remove(filePath))
	{
		LOG_INFO("File ", filePathString, " deleted successfully.");
		return 204;
	}
	return 500;
}

void Server::handleStaticFiles(Client &client, Location &foundLocation)
{
	std::string requestPath = client.getRequest()->getStartLine()["path"];
	std::string filePath = foundLocation.root + requestPath.substr(foundLocation.path.length());
	if (access(filePath.c_str(), F_OK) == -1)
		throw ProcessingError(404, {}, "Exception has been thrown in handleStaticFiles() "
									 "method of Server class");
	else if (access(filePath.c_str(), R_OK) == -1)
		throw ProcessingError(403, {}, "Exception has been thrown in handleStaticFiles() "
									 "method of Server class");

	// If path ends with /, check for index file and directory listing, otherwise throw 403
	std::shared_ptr<Response> locationResp = nullptr;
	if (requestPath.back() == '/')
	{
		if (access((filePath + foundLocation.index).c_str(), F_OK) == 0)
		{
			filePath += foundLocation.index;
			if (access((filePath).c_str(), R_OK) != 0)
				throw ProcessingError(403);
		}
		else if (foundLocation.autoindex)
			locationResp = DirLister::createDirListResponse(foundLocation, requestPath);
		else
			throw ProcessingError(404, {}, "Exception has been thrown in handleStaticFiles() "
										 "method of Server class"); //
	}

	// Checks if location response was formed, otherwise creates Response from filePath
	client.setResponse(locationResp ? locationResp : std::make_shared<Response>(200, filePath));
}

void Server::finalizeResponse(Client &client)
{
	client.setRequest(nullptr);
	client.setResponse(nullptr);
	LOG_DEBUG("closing fd: ", client.getFd());
	close(client.getFd());
	if (client.getChildPipe(0) != -1)
		CGIHandler::closeFds(client);
	LOG_DEBUG("removing from poll fd: ", client.getFd());
	ServersManager::removeFromPollfd(client.getFd());
	if (client.getChildPipe(0) != -1)
		CGIHandler::setToInit(client);
	client.setFd(-1);
	removeFromClients(client);
}

void Server::validateRequest(Client &client)
{
	LOG_DEBUG("validateRequest()");
	if (!client.getRequest())
		throw ProcessingError(400, {}, "Request is nullptr");
	if (client.getRequest()->getStartLine()["method"].empty() ||
		client.getRequest()->getStartLine()["path"].empty() ||
		client.getRequest()->getStartLine()["version"].empty() ||
		client.getRequest()->getHeaders()["host"].empty())
		throw ProcessingError(400, {}, "Request does not have mandatory fields");
	if (client.getRequest()->getStartLine()["version"] != "HTTP/1.1")
		throw ProcessingError(505, {}, "Wrong HTTP version in the start line");
}

void Server::handleCGITimeout(Client &client)
{
	LOG_DEBUG("handleCGITimeout() called");

	auto cgiEnd = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = cgiEnd - client.getCgiStart();

	if (elapsed_seconds.count() >= g_timeout && client.getCGIState() == Client::CGIState::FORKED)
	{
		LOG_WARNING("Cgi has been timeouted");
		kill(client.getPid(), SIGTERM);
		CGIHandler::removeFromPids(client.getPid());
		CGIHandler::changeToErrorState(client);
		
		close(client.getChildPipe(0));
		ServersManager::removeFromPollfd(client.getChildPipe(0));
		client.setChildPipe(0, -1);
		
		client.setResponse(createResponse(client.getRequest(), 504));
	}
}

void Server::responder(Client &client, Server &server)
{
	LOG_DEBUG("Server::responder() called");
	if ((client.getResponse() && !client.getResponse()->getBody().empty()) || formCGIConfigAbsenceResponse(client, server))
	{
		CGIHandler::changeToErrorState(client);
		return ;
	}
	try
	{
		validateRequest(client);
		handleCGITimeout(client);

		if (client.getRequest()->getStartLine()["path"].rfind("/cgi-bin/", 0) == 0 && client.getCGIState() == Client::CGIState::INIT)
		{
			CGIHandler::handleCGI(client, server);
			client.setCGIState(Client::CGIState::FORKED);
			LOG_DEBUG("cgi switched to forked");
		}
		else if (client.getCGIState() == Client::CGIState::INIT)
			handleNonCGIResponse(client, server);
	}
	catch (ProcessingError &e)
	{
		if (e.getCode() == 500 && std::strcmp(e.what(), "handleParentProcess() writing failed") == 0)
		{
			ServersManager::changeStateToDeleteClient(client);
			return ;
		}
		LOG_ERROR("Responder caught an error: ", e.what(), ": ", e.getCode());
		client.setResponse(createResponse(client.getRequest(), e.getCode(), e.getHeaders()));
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("Responder caught an exception: ", e.what());
		client.setResponse(createResponse(client.getRequest(), 500));
	}
	if (!client.getResponse())
		client.setResponse(createResponse(client.getRequest(), 500));
}

void Server::removeFromClients(Client &client)
{
	// remove from _clients vector
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (&(*it) == &client)
		{
			_clients.erase(it);
			break;
		}
	}
}

std::string Server::whoAmI() const
{
	return _ipAddr + ":" + std::to_string(_port);
}

/** If no match found, the first config will be used */
ServerConfig *Server::findServerConfig(std::shared_ptr<Request> req)
{
	// If request host is an ip address:port or if the ip is not specified for current server,
	// the first config for the server is used
	if (!req || req->getHeaders()["host"].empty())
		return &_configs[0];

	std::vector<std::string> hostSplit = Utility::splitStr(req->getHeaders()["host"], ":");
	std::string reqPort = "80"; // Default port for HTTP
	std::string reqHost;

	if (hostSplit.size() > 0) {
		reqHost = Utility::trim(hostSplit.at(0));
		if (hostSplit.size() > 1)
					reqPort = Utility::trim(hostSplit.at(1));
	}

	// If request is a servername, find the correct servername
	if (!reqHost.empty())
	{
		for (ServerConfig &config : _configs)
		{
			LOG_DEBUG(config.serverName, ", ", reqHost);
			if (config.serverName == reqHost)
			{
				LOG_DEBUG("config match!");
				return &config;
			}
		}
	}

	if (whoAmI() == req->getHeaders()["host"] ||
		(_ipAddr.empty() && std::to_string(_port) == reqPort))
	{
		if (!_configs.empty())
			return &_configs[0];
	}

	if (_configs.empty())
		throw ServerException("Program has no configs");

	return &_configs[0];
}

ServerConfig* Server::processNamedServerConfig(std::shared_ptr<Request> req)
{
	LOG_INFO("Searching for server for current location...");
	if (!req)
		throw ProcessingError(400, {}, "Exception (no request) has been thrown in findLocation() "
									 "method of Server class");

	ServerConfig* namedServerConfig = findServerConfig(req);
	
	// This block might be redundant as we always have a server config
	if (!namedServerConfig)
		throw ProcessingError(404, {}, "Exception (no ServerConfig) has been thrown in findLocation() "
									 "method of Server class");
	if (namedServerConfig->locations.empty())
	{
		LOG_ERROR("No locations found for server: ", whoAmI());
		throw ProcessingError(404, {}, "Exception has been thrown in findLocation() "
									 "method of Server class");
	}
	LOG_INFO("Server found. Searching for location...");
	return namedServerConfig;
}

Location Server::findLocation(std::shared_ptr<Request> req)
{
	ServerConfig* namedServerConfig = processNamedServerConfig(req);
	
	// Find the longest matching location
	Location foundLocation;
	size_t locationLength = 0;
	std::string requestPath = req->getStartLine()["path"];

	LOG_DEBUG("Let's find location for request path: ", requestPath);
	LOG_DEBUG("We have locations to check: ", namedServerConfig->locations.size());
	for (Location &location : namedServerConfig->locations)
	{
		LOG_DEBUG("Path: ", location.path, " RequestPath: ", requestPath);
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

bool Server::isCGIBinExistAndReadable()
{
	if (access(_CGIBinFolder.c_str(), F_OK) != 0 || access(_CGIBinFolder.c_str(), R_OK) != 0)
	{
		LOG_WARNING("cgi-bin/ directory doesn't exist or forbidden to access");
		return false;
	}

	return true;
}

void Server::listCGIFiles()
{
	struct dirent *entry;

	DIR *dp = opendir(_CGIBinFolder.c_str());
	if (dp == NULL)
	{
		throw ProcessingError(500, {}, "Error occured on opendir() function");
	}

	while ((entry = readdir(dp)))
	{
		if (entry->d_name[0] != '.')
			_cgiBinFiles.push_back(entry->d_name);
	}

	closedir(dp);
}

/**
 * Getters
 */

std::vector<ServerConfig> &Server::getConfigs()
{
	return _configs;
}

int Server::getServerSockfd()
{
	return _serverSocket.getSockfd();
}

std::vector<Client> &Server::getClients()
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

std::vector<struct pollfd> *Server::getFds()
{
	return _managerFds;
}

std::string Server::getCGIBinFolder()
{
	return _CGIBinFolder;
}

std::vector<std::string> Server::getcgiBinFiles()
{
	return _cgiBinFiles;
}

/**
 * Setters
 */

void Server::setConfig(std::vector<ServerConfig> serverConfigs)
{
	_configs = serverConfigs;
}

void Server::setFds(std::vector<struct pollfd> *fds)
{
	_managerFds = fds;
}
