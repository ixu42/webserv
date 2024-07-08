/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/07 14:53:12 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

void CGIServer::handleCGI(t_client& client, Server& server)
{
	DEBUG("\n===handleCGI function started===");
	std::string interpreter = determineInterpreter(client.request->getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(client.request);
	DEBUG("===Enviroment has been set===");
	handleProcesses(client, server, interpreter, envVars);
	DEBUG("===handleCGI function ended===");
}

std::string CGIServer::readErrorPage(const std::string& errorPagePath)
{
	std::ifstream file(errorPagePath);
	
	if (!file)
	{
		throw ResponseError(404);
	}
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string CGIServer::determineInterpreter(const std::string& filePath)
{
	if (filePath.substr(filePath.find_last_of(".") + 1) == "py")
	{
		DEBUG("===Python specificator found===");
		return PYTHON_INTERPRETER;
	}
	else if (filePath.substr(filePath.find_last_of(".") + 1) == "php")
	{
		DEBUG("===PHP specificator found===");
		return PHP_INTERPRETER;
	}
	else
	{
		throw ResponseError(404);
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
	DEBUG("===Duplicating stdin and stdout===");
	if (dup2(server.getPipe().input[IN], STDIN_FILENO) < 0 ||
		dup2(server.getPipe().output[OUT], STDOUT_FILENO) < 0)
	{
		throw ResponseError(500);
	}

	close(server.getPipe().input[OUT]);
	close(server.getPipe().output[IN]);

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	args.push_back(const_cast<char*>(filePath.c_str()));
	args.push_back(nullptr);
	DEBUG("===Agruments set===");

	std::vector<char*> envp;
	for (const auto& var : envVars)
	{
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);
	DEBUG("===Environment casted===");

	DEBUG("===About to start execve===");
	execve(interpreter.c_str(), args.data(), envp.data());
	throw ResponseError(500);
}

void CGIServer::handleParentProcess(Server& server, Response* response, const std::string& method,
	const std::string& body)
{
	close(server.getPipe().input[IN]);
	close(server.getPipe().output[OUT]);

	if (method == "POST")
	{
		DEBUG("===Writing body of the request inside the pipe===");
		write(server.getPipe().input[OUT], body.c_str(), body.size());
	}
	close(server.getPipe().input[OUT]);

	char buffer[1024];
	ssize_t bytesRead;

	while ((bytesRead = read(server.getPipe().output[IN], buffer, sizeof(buffer))) > 0)
	{
		DEBUG("Populating response body");
		response->appendToBody(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		throw ResponseError(500);
	}
	close(server.getPipe().output[IN]);
	std::cout<<response->getBody()<<std::endl;
}

void CGIServer::handleProcesses(t_client& client, Server& server,
	const std::string& interpreter, const std::vector<std::string>& envVars)
{
	if (pipe(server.getPipe().input) == -1 || pipe(server.getPipe().output) == -1)
	{
		throw ResponseError(500);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		throw ResponseError(500);
	}
	else if (pid == 0)
	{
		DEBUG("===Child started===");
		handleChildProcess(server, interpreter,
			client.request->getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		DEBUG("===Parent started===");
		handleParentProcess(server, client.response, client.request->getStartLine()["method"], client.request->getBody());
		waitpid(pid, nullptr, 0);
	}
	std::cout<<client.response->getBody()<<std::endl;
}
