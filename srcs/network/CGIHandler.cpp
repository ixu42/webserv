/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/13 14:02:25 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

const std::string CGIServer::_python_interpr = "/usr/bin/python3";
const std::string CGIServer::_php_interpr = "/usr/bin/php";

void CGIServer::handleCGI(t_client& client, Server& server)
{
	LOG_INFO(TEXT_GREEN, "Running CGI", RESET);
	LOG_DEBUG("handleCGI function started");
	std::string interpreter = determineInterpreter(client.request->getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(client.request);
	LOG_DEBUG("Enviroment has been set");
	handleProcesses(client, server, interpreter, envVars);
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

void CGIServer::handleChildProcess(Server& server, const std::string& interpreter,
	const std::string& filePath, const std::vector<std::string>& envVars)
{
	LOG_DEBUG("Duplicating stdin and stdout");
	if (dup2(server.getPipe().input[_in], STDIN_FILENO) < 0 ||
		dup2(server.getPipe().output[_out], STDOUT_FILENO) < 0)
	{
		closeFds(server);
		throw ResponseError(500, {}, "Exception (dup2) has been thrown in handleChildProcess() "
			"method of CGIServer class");
	}

	close(server.getPipe().input[_out]);
	close(server.getPipe().output[_in]);

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
	close(server.getPipe().input[_in]);
	close(server.getPipe().output[_out]);
	throw ResponseError(500, {}, "Exception (execve) has been thrown in handleChildProcess() "
		"method of CGIServer class");
}

void CGIServer::handleParentProcess(Server& server, Response* response, const std::string& body)
{
	close(server.getPipe().input[_in]);
	close(server.getPipe().output[_out]);

	LOG_DEBUG("Writing body of the request inside the pipe");
	write(server.getPipe().input[_out], body.c_str(), body.size());
	close(server.getPipe().input[_out]);

	char buffer[1024];
	ssize_t bytesRead;
	std::ostringstream oss;

	while ((bytesRead = read(server.getPipe().output[_in], buffer, sizeof(buffer))) > 0)
	{
		LOG_DEBUG("Populating response body");
		oss.write(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		close(server.getPipe().output[_in]);
		throw ResponseError(500, {}, "Exception has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}
	checkResponseHeaders(oss.str(), response);
	close(server.getPipe().output[_in]);
}

void CGIServer::handleProcesses(t_client& client, Server& server,
	const std::string& interpreter, const std::vector<std::string>& envVars)
{
	if (pipe(server.getPipe().input) == -1 || pipe(server.getPipe().output) == -1)
	{
		throw ResponseError(500, {}, "Exception (pipe) has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		closeFds(server);
		throw ResponseError(500, {}, "Exception (fork) has been thrown in handleParentProcess() "
			"method of CGIServer class");
	}
	else if (pid == 0)
	{
		LOG_DEBUG("Child started");
		handleChildProcess(server, interpreter,
			client.request->getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		LOG_DEBUG("Parent started");
		handleParentProcess(server, client.response, client.request->getBody());
		waitpid(pid, nullptr, 0);
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

void CGIServer::closeFds(Server& server)
{
	close(server.getPipe().output[_in]);
	close(server.getPipe().output[_out]);
	close(server.getPipe().input[_in]);
	close(server.getPipe().input[_out]);
}
