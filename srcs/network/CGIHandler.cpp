/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 13:17:21 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/08 11:13:10 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

void CGIServer::handleCGI(Request& request, Server& server, Response& response)
{
	LOG_DEBUG("\n===handleCGI function started===");
	std::string interpreter = determineInterpreter(request.getStartLine()["path"], response);
	std::vector<std::string> envVars = setEnvironmentVariables(request);
	LOG_DEBUG("===Enviroment has been set===");
	handleProcesses(request, server, response, interpreter, envVars);
	LOG_DEBUG("===handleCGI function ended===");
}

void CGIServer::setResponse(Response& response, std::string status,
	std::string type, std::string page)
{
	response.setStatus(status);
	response.setType(type);
	response.setBody(readErrorPage(page));
}

std::string CGIServer::readErrorPage(const std::string& errorPagePath)
{
	std::ifstream file(errorPagePath);
	
	if (!file)
	{
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

std::string CGIServer::determineInterpreter(const std::string& filePath, Response& response)
{
	if (filePath.substr(filePath.find_last_of(".") + 1) == "py")
	{
		LOG_DEBUG("===Python specificator found===");
		return PYTHON_INTERPRETER;
	}
	else if (filePath.substr(filePath.find_last_of(".") + 1) == "php")
	{
		LOG_DEBUG("===PHP specificator found===");
		return PHP_INTERPRETER;
	}
	else
	{
		setResponse(response, "404 Page Not Found", "text/html", "pages/404.html");
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

void CGIServer::handleChildProcess(Response& response, Server& server, const std::string& interpreter,
	const std::string& filePath, const std::vector<std::string>& envVars)
{
	LOG_DEBUG("===Duplicating stdin and stdout===");
	if (dup2(server.getPipe().input[IN], STDIN_FILENO) < 0 ||
		dup2(server.getPipe().output[OUT], STDOUT_FILENO) < 0)
	{
		setResponse(response, "500 Internal Server error", "text/html", "pages/500.html");
		throw ServerException("Duplication error occured in CGIHandler module");
	}

	close(server.getPipe().input[OUT]);
	close(server.getPipe().output[IN]);

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	args.push_back(const_cast<char*>(filePath.c_str()));
	args.push_back(nullptr);
	LOG_DEBUG("===Agruments set===");

	std::vector<char*> envp;
	for (const auto& var : envVars)
	{
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);
	LOG_DEBUG("===Environment casted===");

	LOG_DEBUG("===About to start execve===");
	execve(interpreter.c_str(), args.data(), envp.data());
	setResponse(response, "500 Internal Server error", "text/html", "pages/500.html");
	throw ServerException("Error occured while execve() function was called");
}

void CGIServer::handleParentProcess(Server& server, Response& response, const std::string& method,
	const std::string& body)
{
	close(server.getPipe().input[IN]);
	close(server.getPipe().output[OUT]);

	if (method == "POST")
	{
		LOG_DEBUG("===Writing body of the request inside the pipe===");
		write(server.getPipe().input[OUT], body.c_str(), body.size());
	}
	close(server.getPipe().input[OUT]);

	char buffer[10];
	ssize_t bytesRead;

	while ((bytesRead = read(server.getPipe().output[IN], buffer, sizeof(buffer))) > 0)
	{
		LOG_DEBUG("Populating response body");
		response.appendToBody(buffer, bytesRead);
	}
	if (bytesRead < 0)
	{
		setResponse(response, "500 Internal Server error", "text/html", "pages/500.html");
		throw ServerException("Error occured while read() from pipe to form the response body");
	}
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
		LOG_DEBUG("===Child started===");
		handleChildProcess(response, server, interpreter,
			request.getStartLine()["path"].erase(0, 1), envVars);
	}
	else
	{
		LOG_DEBUG("===Parent started===");
		handleParentProcess(server, response, request.getStartLine()["method"], request.getBody());
		waitpid(pid, nullptr, 0);
	}
}
