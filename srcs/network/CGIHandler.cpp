/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/02 15:50:01 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

void CGIServer::handleCGI(Request& request, Server& server, Response& response)
{
	DEBUG("\n===handleCGI function started===");
	std::string interpreter = determineInterpreter(request.getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(request);
	DEBUG("===Enviroment has been set===");
	handleProcesses(request, server, response, interpreter, envVars);
	DEBUG("===handleCGI function ended===");
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
		throw ServerException("Unsupported type passed to CGIHandler");
	}
}

std::vector<std::string> CGIServer::setEnvironmentVariables(Request& request)
{
	std::vector<std::string> env;

	env.push_back("REQUEST_METHOD=" + request.getStartLine()["method"]);
	env.push_back("QUERY_STRING=" + request.getStartLine()["query"]);
	env.push_back("SCRIPT_NAME=" + request.getStartLine()["path"].erase(0, 1));
	env.push_back("SERVER_PROTOCOL=" + request.getStartLine()["version"]);

	if (request.getStartLine()["method"] == "POST")
	{
		env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		env.push_back("CONTENT_LENGTH=" + std::to_string(request.getBody().size()));
	}
	return env;
}

void CGIServer::handleChildProcess(Server& server, const std::string& interpreter,
	const std::string& filePath,const std::vector<std::string>& envVars)
{
	DEBUG("===Duplicating stdin and stdout===");
	dup2(server.getPipe().input[IN], STDIN_FILENO);
	dup2(server.getPipe().output[OUT], STDOUT_FILENO);

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
	throw ServerException("Error occured while execve() function was called");
}

void CGIServer::handleParentProcess(Server& server, Response& response, const std::string& method,
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

	char buffer[10];
	ssize_t bytesRead;

	while ((bytesRead = read(server.getPipe().output[IN], buffer, sizeof(buffer))) > 0)
	{
		DEBUG("Populating response body");
		response.appendToBody(buffer, bytesRead);
	}
	std::cerr << response.getBody() << std::endl;
	close(server.getPipe().output[IN]);
}

void CGIServer::handleProcesses(Request& request, Server& server, Response& response,
	const std::string& interpreter, const std::vector<std::string>& envVars)
{
	if (pipe(server.getPipe().input) == -1 || pipe(server.getPipe().output) == -1)
	{
		throw ServerException("Error occured while pipe() function was called");
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		throw ServerException("Error occured while fork() function was called");
	}
	else if (pid == 0)
	{
		DEBUG("===Child started===");
		handleChildProcess(server, interpreter, request.getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		DEBUG("===Parent started===");
		handleParentProcess(server, response, request.getStartLine()["method"], request.getBody());
		waitpid(pid, nullptr, 0);
	}
}