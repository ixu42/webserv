/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/12 14:54:08 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

void CGIHandler::handleCGI(Client& client, Server& server)
{
	LOG_INFO(TEXT_GREEN, "Running CGI", RESET);
	
	// Save starting time
	client.setCgiStart(std::chrono::system_clock::now());
	std::time_t start_time = std::chrono::system_clock::to_time_t(client.getCgiStart());
	LOG_DEBUG("Cgi started at: ", std::ctime(&start_time));
	
	LOG_DEBUG("handleCGI function started");
	std::string interpreter = determineInterpreter(client, client.getRequest()->getStartLine()["path"], server);
	std::vector<std::string> envVars = setEnvironmentVariables(client.getRequest());
	LOG_DEBUG("Enviroment has been set");
	handleProcesses(client, interpreter, envVars, server);
	LOG_DEBUG("handleCGI function ended");
}

std::string CGIHandler::determineInterpreter(Client& client, const std::string& filePath, Server& server)
{
	if (server.getcgiBinFiles().size() == 0)
	{
		changeToErrorState(client);
		throw ProcessingError(404, {}, "cgi-bin/ folder is empty or does not exist");
	}

	size_t cgiBinPos = filePath.find("/cgi-bin/");
	size_t fileStart = cgiBinPos + 9;
	size_t fileEnd = filePath.find('/', fileStart);
	std::string fileName = (fileEnd == std::string::npos) ?
							filePath.substr(fileStart) :
							filePath.substr(fileStart, fileEnd - fileStart);

	std::string fullPath = static_cast<std::string>(server.getCGIBinFolder().c_str()) + fileName;

	// check if the file exists
	if (access(fullPath.c_str(), F_OK) != 0)
	{
		changeToErrorState(client);
		throw ProcessingError(404, {}, "File does not exist");
	}
	// check if the file has read permissions
	if (access(fullPath.c_str(), R_OK) != 0)
	{
		changeToErrorState(client);
		throw ProcessingError(403, {}, "File is not readable");
	}

	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
	std::string cgiPath = (*(server.findServerConfig(client.getRequest())->cgis))[extension];
	
	if (cgiPath == "")
	{
		changeToErrorState(client);
		throw ProcessingError(502, {}, "Unknown file extension");
	}

	return cgiPath;
}

std::vector<std::string> CGIHandler::setEnvironmentVariables(std::shared_ptr<Request> request)
{
	std::vector<std::string> env;

	// setting the enviroment for cgi
	env.push_back("REQUEST_METHOD=" + request->getStartLine()["method"]);
	env.push_back("QUERY_STRING=" + request->getStartLine()["query"]);
	env.push_back("SCRIPT_NAME=" + request->getStartLine()["path"].erase(0, 1));
	env.push_back("SERVER_PROTOCOL=" + request->getStartLine()["version"]);
	env.push_back("SERVER_NAME=" + request->getHeaders()["host"]);
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("PATH_INFO=" + request->getStartLine()["path_info"]);
	env.push_back("HTTP_ACCEPT=" + request->getHeaders()["accept"]);
	env.push_back("HTTP_USER_AGENT=" + request->getHeaders()["user-agent"]);
	
	if (request->getStartLine()["method"] == "POST")
	{
		env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		env.push_back("CONTENT_LENGTH=" + std::to_string(request->getBody().size()));
	}
	return env;
}

void CGIHandler::handleChildProcess(Client& client, const std::string& interpreter,
	const std::string& filePath, const std::vector<std::string>& envVars, Server& server)
{
	LOG_DEBUG("Duplicating stdin and stdout");
	if (dup2(client.getParentPipe(_in), STDIN_FILENO) < 0 ||
		dup2(client.getChildPipe(_out), STDOUT_FILENO) < 0)
	{
		closeFds(client);
		std::exit(EXIT_FAILURE);
	}

	close(client.getParentPipe(_out));
	client.setParentPipe(_out, -1);
	close(client.getChildPipe(_in));
	client.setChildPipe(_in, -1);

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	std::string absFilePath = server.getCGIBinFolder() + filePath;
	args.push_back(const_cast<char*>(absFilePath.c_str()));
	args.push_back(nullptr);
	LOG_DEBUG("Agruments set");

	std::vector<char*> envp;
	for (const auto& var : envVars)
	{
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);
	LOG_DEBUG("Environment casted");

	LOG_DEBUG("About to start execve");
	execve(interpreter.c_str(), args.data(), envp.data());
	close(client.getParentPipe(_in));
	client.setParentPipe(_in, -1);
	close(client.getChildPipe(_out));
	client.setChildPipe(_out, -1);
	std::exit(EXIT_FAILURE);
}

void CGIHandler::handleParentProcess(Client& client, const std::string& body)
{
	close(client.getParentPipe(_in));
	client.setParentPipe(_in, -1);
	close(client.getChildPipe(_out));
	client.setChildPipe(_out, -1);

	LOG_DEBUG("Writing body of the request inside the pipe");
	int bytesRead = write(client.getParentPipe(_out), body.c_str(), body.size());
	if (bytesRead < 0)
	{
		LOG_DEBUG("Child pid: ", client.getPid());
		kill(client.getPid(), SIGTERM);
		removeFromPids(client.getPid());
		changeToErrorState(client);
		throw ProcessingError(502, {}, "handleParentProcess() writing failed");
	}

	close(client.getParentPipe(_out));
	ServersManager::removeFromPollfd(client.getParentPipe(1));
	client.setParentPipe(_out, -1);
	
	if (bytesRead == 0)
		LOG_DEBUG("bytesRead: 0");
	LOG_DEBUG("Wrote body of the request and closed the pipe");
}

void CGIHandler::handleProcesses(Client& client, const std::string& interpreter,
	const std::vector<std::string>& envVars, Server& server)
{
	pid_t childPid = fork();
	LOG_DEBUG("forked in handleProcesses");
	client.setPid(childPid);
	if (client.getPid() == -1)
	{
		kill(client.getPid(), SIGTERM);
		removeFromPids(client.getPid());
		changeToErrorState(client);
		throw ProcessingError(502, {}, "Exception (fork) has been thrown in handleParentProcess() "
			"method of CGIHandler class");
	}
	else if (client.getPid() == 0)
	{
		LOG_DEBUG("Child started");
		handleChildProcess(client, interpreter,
			client.getRequest()->getStartLine()["path"].erase(0, 9), envVars, server);
	}
	else
	{
		LOG_DEBUG("Child pid in parent: ", childPid);
		LOG_DEBUG("Parent started");
		g_childPids.push_back(client.getPid());
		handleParentProcess(client, client.getRequest()->getBody());
	}
	LOG_INFO(TEXT_GREEN, "CGI script executed", RESET);
}

void CGIHandler::checkResponseHeaders(const std::string& result, std::shared_ptr<Response> response)
{
	LOG_DEBUG("Checking for the headers in CGI output");
	size_t headerEnd = result.find("\n\n");
	
	if (headerEnd != std::string::npos)
	{
		std::string headers = result.substr(0, headerEnd);
		
		response->setBody(result.substr(headerEnd + 2));

		std::istringstream headerStream(headers);
		std::string line;
		
		while (std::getline(headerStream, line))
		{
			size_t separator = line.find(": ");
			if (separator != std::string::npos)
			{
				const std::string& key = line.substr(0, separator);
				std::string value = line.substr(separator + 2);
				if (key == "Content-Type")
				{
					response->setType(value);
				}
				else
				{
					response->setHeader(key, value);
				}
			}
		}
	}
	else
	{
		response->setBody(result);
	}
}

void CGIHandler::closeFds(Client& client)
{
	close(client.getChildPipe(_in));
	close(client.getChildPipe(_out));
	close(client.getParentPipe(_in));
	close(client.getParentPipe(_out));
}

void CGIHandler::setToInit(Client& client)
{
	client.setChildPipe(_in, -1);
	client.setChildPipe(_out, -1);
	client.setParentPipe(_in, -1);
	client.setParentPipe(_out, -1);
}

void CGIHandler::registerCGIPollFd(int fd, short events, std::vector<pollfd>& new_fds)
{
	LOG_DEBUG("CGIHandler::registerCGIPollFd() called");
	new_fds.push_back({fd, events, 0});
}

void CGIHandler::unregisterCGIPollFd(Server& server, int fd)
{
	server.getFds()->erase(std::remove_if(server.getFds()->begin(),
		server.getFds()->end(), [fd](const pollfd& pfd)
	{
		return pfd.fd == fd;
	}), server.getFds()->end());
}

void CGIHandler::InitCGI(Client& client, std::vector<pollfd>& new_fds)
{
	LOG_DEBUG("Initializing CGI");
	std::shared_ptr<Response> response = std::make_shared<Response>();
	client.setResponse(response);
	if (pipe(client.getParentPipeWhole()) == -1 || pipe(client.getChildPipeWhole()) == -1)
	{
		changeToErrorState(client);
		throw ProcessingError(502, {}, "Exception (pipe) has been thrown in InitCGI() "
			"method of CGIHandler class");
	}

	LOG_INFO("Pipes numbers: ",client.getParentPipe(_in)," ",client.getParentPipe(_out),
		" ", client.getChildPipe(_in)," ",client.getChildPipe(_out));
	
	registerCGIPollFd(client.getChildPipe(_in), POLLIN, new_fds);
	registerCGIPollFd(client.getParentPipe(_out), POLLOUT, new_fds);
	LOG_DEBUG("Finished InitCGI()");
}

bool CGIHandler::readScriptOutput(Client& client, std::shared_ptr<Server>& server)
{
	LOG_DEBUG("readScriptOutput() called");
	
	char buffer[g_bufferSize];
	ssize_t bytesRead;
	ssize_t currPipeSize = 0;
	
	std::fill(buffer, buffer + g_bufferSize, 0);
	while ((bytesRead = read(client.getChildPipe(_in), buffer, sizeof(buffer))) > 0)
	{
		currPipeSize += bytesRead;
		if (currPipeSize >= _pipeMaxSize)
		{
			kill(client.getPid(), SIGTERM);
			removeFromPids(client.getPid());
			throw ProcessingError(502, {}, "Pipe overflowed");
		}
		LOG_DEBUG(TEXT_GREEN, "Populating response body with: ", bytesRead, RESET);
		LOG_DEBUG(TEXT_GREEN, "Response body: ", buffer, RESET);
		client.getRespBody().append(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		kill(client.getPid(), SIGTERM);
		removeFromPids(client.getPid());
		throw ProcessingError(502, {}, "readScriptOutput() reading failed");
	}
	if (bytesRead != 0)
	{
		LOG_DEBUG("Still reading from pipe in CGI");
		return false;
	}
	
	LOG_INFO(TEXT_GREEN, "CGI script output read correctly", RESET);
	
	checkResponseHeaders(client.getRespBody(), client.getResponse());
	
	close(client.getChildPipe(_in));
	unregisterCGIPollFd(*server, client.getChildPipe(_in));
	client.setChildPipe(_in, -1);
	close(client.getParentPipe(_out));
	unregisterCGIPollFd(*server, client.getParentPipe(_out));
	client.setParentPipe(_out, -1);
	
	auto it = std::find(g_childPids.begin(), g_childPids.end(), client.getPid());
	if (it != g_childPids.end())
		g_childPids.erase(it);
	return true;
}

void CGIHandler::changeToErrorState(Client& client)
{
	client.setState(Client::ClientState::BUILDING);
	client.setCGIState(Client::CGIState::FINISHED_SET);
}

void CGIHandler::removeFromPids(pid_t pid)
{
	// remove from g_childPids vector
	for (auto it = g_childPids.begin(); it != g_childPids.end(); ++it)
	{
		if (*it == pid)
		{
			g_childPids.erase(it);
			break;
		}
	}
}
