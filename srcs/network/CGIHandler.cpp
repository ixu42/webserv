/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/19 15:50:00 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

const std::string CGIServer::_python_interpr = "/usr/bin/python3";
const std::string CGIServer::_php_interpr = "/usr/bin/php";

void CGIServer::handleCGI(Client& client)
{
	LOG_INFO(TEXT_GREEN, "Running CGI", RESET);
	LOG_DEBUG("handleCGI function started");
	std::string interpreter = determineInterpreter(client.getRequest()->getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(client.getRequest());
	LOG_DEBUG("Enviroment has been set");
	handleProcesses(client, interpreter, envVars);
	LOG_DEBUG("handleCGI function ended");
}

std::string CGIServer::readErrorPage(const std::string& errorPagePath)
{
	std::ifstream file(errorPagePath);
	
	if (!file)
	{
		throw ResponseError(404, {}, "Exception has been thrown in readErrorPage() "
			"method of CGIServer class");
	}
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string CGIServer::determineInterpreter(const std::string& filePath)
{
	if (filePath.substr(filePath.find_last_of(".") + 1) == "py")
	{
		LOG_DEBUG("Python specificator found");
		return _python_interpr;
	}
	else if (filePath.substr(filePath.find_last_of(".") + 1) == "php")
	{
		LOG_DEBUG("PHP specificator found");
		return _php_interpr;
	}
	else
	{
		throw ResponseError(404, {}, "Exception has been thrown in determineInterpreter() "
			"method of CGIServer class");
	}
}

std::vector<std::string> CGIServer::setEnvironmentVariables(Request* request)
{
	std::vector<std::string> env;

	env.push_back("REQUEST_METHOD=" + request->getStartLine()["method"]);
	env.push_back("QUERY_STRING=" + request->getStartLine()["query"]);
	env.push_back("SCRIPT_NAME=" + request->getStartLine()["path"].erase(0, 1));
	env.push_back("SERVER_PROTOCOL=" + request->getStartLine()["version"]);

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
		throw ResponseError(500, {}, "Exception (dup2) has been thrown in handleChildProcess() "
			"method of CGIServer class");
	}

	client.getParentPipe(_out);
	client.getChildPipe(_in);

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
		// waitpid(pid, nullptr, 0);
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
			throw ResponseError(500, {}, "Exception (pipe) has been thrown in InitCGI() "
				"method of CGIServer class");
		}

		LOG_DEBUG("Pipes numbers: ",client.getParentPipe(0)," ",client.getParentPipe(1)," ",client.getChildPipe(0)," ",client.getChildPipe(1));
		
		// fcntlSet(client.childPipe[_in]); // works without non-blocking ??
		
		registerCGIPollFd(server, client.getChildPipe(0), POLLIN);
		// registerCGIPollFd(server, client.parentPipe[_out], POLLOUT);
		LOG_DEBUG("Finished InitCGI()");
	}
}

void CGIServer::fcntlSet(int fd)
{
	LOG_DEBUG("Setting CGI as non-blocking with fd: ", fd);
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		throw ResponseError(500, {}, "Exception  has been thrown in fcntlSet() "
			"method of CGIServer class");
	}

	int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0)
	{
		throw ResponseError(500, {}, "Exception  has been thrown in fcntlSet() "
			"method of CGIServer class");
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
		oss.write(buffer, bytesRead);
	}
	
	LOG_DEBUG(TEXT_YELLOW, "bytesRead in readScriptOutput: ", bytesRead, RESET);

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
