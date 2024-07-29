/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/29 14:30:54 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

const std::string CGIServer::_python_interpr = "/usr/bin/python3";
const std::string CGIServer::_php_interpr = "/usr/bin/php";

void CGIServer::handleCGI(Client& client, Server& server)
{
	LOG_INFO(TEXT_GREEN, "Running CGI", RESET);
	LOG_DEBUG("handleCGI function started");
	std::string interpreter = determineInterpreter(client, client.getRequest()->getStartLine()["path"], server);
	std::vector<std::string> envVars = setEnvironmentVariables(client.getRequest());
	LOG_DEBUG("Enviroment has been set");
	handleProcesses(client, interpreter, envVars);
	LOG_DEBUG("handleCGI function ended");
}

std::string CGIServer::determineInterpreter(Client& client, const std::string& filePath, Server& server)
{
	if (server.getcgiBinFiles().size() == 0)
	{
		changeToErrorState(client);
		throw ResponseError(404, {}, "cgi-bin/ folder is empty or does not exist");
	}

	size_t cgiBinPos = filePath.find(server.getCGIBinFolder());
	size_t fileStart = cgiBinPos + sizeof(server.getCGIBinFolder());
	size_t fileEnd = filePath.find('/', fileStart);
	std::string fileName = (fileEnd == std::string::npos) ?
							filePath.substr(fileStart) :
							filePath.substr(fileStart, fileEnd - fileStart);

	std::string fullPath = static_cast<std::string>(server.getCGIBinFolder()) + fileName;

	// check if the file exists
	if (access(fullPath.c_str(), F_OK) != 0)
	{
		changeToErrorState(client);
		throw ResponseError(404, {}, "File does not exist");
	}
	// check if the file has read permissions
	if (access(fullPath.c_str(), R_OK) != 0)
	{
		changeToErrorState(client);
		throw ResponseError(403, {}, "File is not readable");
	}

	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
	std::string cgiPath = (*(server.findServerConfig(client.getRequest())->cgis))[extension];
	
	if (cgiPath == "")
	{
		changeToErrorState(client);
		throw ResponseError(500, {}, "Unknown file extension");
	}

	return cgiPath;
}

std::vector<std::string> CGIServer::setEnvironmentVariables(Request* request)
{
	std::vector<std::string> env;

	// setting the enviroment for cgi
	env.push_back("REQUEST_METHOD=" + request->getStartLine()["method"]);
	env.push_back("QUERY_STRING=" + request->getStartLine()["query"]);
	env.push_back("SCRIPT_NAME=" + request->getStartLine()["path"].erase(0, 1));
	env.push_back("SERVER_PROTOCOL=" + request->getStartLine()["version"]);
	env.push_back("PATH_INFO=" + request->getStartLine()["path_info"]);
	
	if (request->getStartLine()["method"] == "POST")
	{
		env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		env.push_back("CONTENT_LENGTH=" + std::to_string(request->getBody().size()));
	}
	return env;
}

void CGIServer::handleChildProcess(Client& client, const std::string& interpreter,
	const std::string& filePath, const std::vector<std::string>& envVars)
{
	LOG_DEBUG("Duplicating stdin and stdout");
	if (dup2(client.getParentPipe(_in), STDIN_FILENO) < 0 ||
		dup2(client.getChildPipe(_out), STDOUT_FILENO) < 0)
	{
		closeFds(client);
		changeToErrorState(client);
		throw ResponseError(500, {}, "Exception (dup2) has been thrown in handleChildProcess() "
			"method of CGIServer class");
	}

	close(client.getParentPipe(_out));
	close(client.getChildPipe(_in));

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	args.push_back(const_cast<char*>(filePath.c_str()));
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
	close(client.getChildPipe(_out));
	changeToErrorState(client);
	throw ResponseError(500, {}, "Exception (execve) has been thrown in handleChildProcess() "
		"method of CGIServer class");
}

void CGIServer::handleParentProcess(Client& client, const std::string& body)
{
	close(client.getParentPipe(_in));
	close(client.getChildPipe(_out));

	LOG_DEBUG("Writing body of the request inside the pipe");
	write(client.getParentPipe(_out), body.c_str(), body.size());
	close(client.getParentPipe(_out));
	LOG_DEBUG("Wrote body of the request and closed the pipe");
}

void CGIServer::handleProcesses(Client& client, const std::string& interpreter,
	const std::vector<std::string>& envVars)
{
	pid_t childPid = fork();
	client.setPid(childPid);
	LOG_DEBUG("forked in handleProcesses");
	if (client.getPid() == -1)
	{
		closeFds(client);
		changeToErrorState(client);
		throw ResponseError(500, {}, "Exception (fork) has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}
	else if (client.getPid() == 0)
	{
		LOG_DEBUG("Child started");
		handleChildProcess(client, interpreter,
			client.getRequest()->getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		LOG_DEBUG("Parent started");
		g_childPids.push_back(client.getPid());
		handleParentProcess(client, client.getRequest()->getBody());
	}
	LOG_INFO(TEXT_GREEN, "CGI script executed", RESET);
}

void CGIServer::checkResponseHeaders(const std::string& result, Response* response)
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

void CGIServer::closeFds(Client& client)
{
	close(client.getChildPipe(_in));
	close(client.getChildPipe(_out));
	close(client.getParentPipe(_in));
	close(client.getParentPipe(_out));
}

void CGIServer::registerCGIPollFd(Server& server, int fd, short events)
{
	LOG_DEBUG("CGIServer::registerCGIPollFd() called");
	server.getFds()->push_back({fd, events, 0});
}

void CGIServer::unregisterCGIPollFd(Server& server, int fd)
{
	server.getFds()->erase(std::remove_if(server.getFds()->begin(),
		server.getFds()->end(), [fd](const pollfd& pfd)
	{
		return pfd.fd == fd;
	}), server.getFds()->end());
}

void CGIServer::InitCGI(Client& client, Server& server)
{
	LOG_DEBUG("Initializing CGI");
	if (client.getRequest()->getStartLine()["path"].find("/cgi-bin") != std::string::npos)
	{
		Response* response = new Response();
		client.setResponse(response);
		if (pipe(client.getParentPipeWhole()) == -1 || pipe(client.getChildPipeWhole()) == -1)
		{
			changeToErrorState(client);
			throw ResponseError(500, {}, "Exception (pipe) has been thrown in InitCGI() "
				"method of CGIServer class");
		}

		LOG_DEBUG("Pipes numbers: ",client.getParentPipe(0)," ",client.getParentPipe(1),
			" ", client.getChildPipe(0)," ",client.getChildPipe(1));
		
		registerCGIPollFd(server, client.getChildPipe(0), POLLIN);
		LOG_DEBUG("Finished InitCGI()");
	}
}

bool CGIServer::readScriptOutput(Client& client, Server*& server)
{
	LOG_DEBUG("readScriptOutput() called");
	char buffer[g_bufferSize];
	ssize_t bytesRead;
	std::ostringstream oss;

	std::fill(buffer, buffer + g_bufferSize, 0);
	while ((bytesRead = read(client.getChildPipe(0), buffer, sizeof(buffer))) > 0)
	{
		LOG_DEBUG(TEXT_GREEN, "Populating response body with: ", bytesRead, RESET);
		LOG_DEBUG(TEXT_GREEN, "Response body: ", buffer, RESET);
		oss.write(buffer, bytesRead);
	}

	LOG_INFO(TEXT_YELLOW, "bytesRead in readScriptOutput: ", bytesRead, RESET);

	if (bytesRead != 0)
	{
		LOG_DEBUG("Still reading from pipe in CGI");
		return false;
	}
	
	LOG_DEBUG(TEXT_YELLOW, "readScriptOutput read the whole body", RESET);
	
	checkResponseHeaders(oss.str(), client.getResponse());
	close(client.getChildPipe(0));
	unregisterCGIPollFd(*server, client.getChildPipe(0));
	
	auto it = std::find(g_childPids.begin(), g_childPids.end(), client.getPid());
	if (it != g_childPids.end())
		g_childPids.erase(it);
	return true;
}

void CGIServer::changeToErrorState(Client& client)
{
	client.setState(Client::ClientState::BUILDING);
	client.setCGIState(Client::CGIState::FINISHED_SET);
}
