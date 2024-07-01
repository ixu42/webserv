#include "CGIHandler.hpp"

int CGIServer::inputPipe[FDS];
int CGIServer::outputPipe[FDS];
std::string CGIServer::result;

std::string CGIServer::handleCGI(Request& request)
{
	std::string interpreter = determineInterpreter(request.getStartLine()["path"]);
	std::vector<std::string> envVars = setEnvironmentVariables(request);
	handleProcesses(request, interpreter, envVars);

	return result;
}

std::string CGIServer::determineInterpreter(const std::string& filePath)
{
	if (filePath.substr(filePath.find_last_of(".") + 1) == "py")
	{
		return "/usr/bin/python3";
	}
	else if (filePath.substr(filePath.find_last_of(".") + 1) == "php")
	{
		return "/usr/bin/php";
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
	env.push_back("SCRIPT_NAME=" + request.getStartLine()["path"]);
	env.push_back("SERVER_PROTOCOL=" + request.getStartLine()["version"]);

	if (request.getStartLine()["method"] == "POST")
	{
		env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
		env.push_back("CONTENT_LENGTH=" + std::to_string(request.getBody().size()));
	}

	return env;
}

void CGIServer::handleChildProcess(const std::string& interpreter, const std::string& filePath,
	const std::vector<std::string>& envVars)
{
	dup2(inputPipe[IN], STDIN_FILENO);
	dup2(outputPipe[OUT], STDOUT_FILENO);

	close(inputPipe[OUT]);
	close(outputPipe[IN]);

	std::vector<char*> args;
	args.push_back(const_cast<char*>(interpreter.c_str()));
	args.push_back(const_cast<char*>(filePath.c_str()));
	args.push_back(nullptr);

	std::vector<char*> envp;
	for (const auto& var : envVars)
	{
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);

	execve(interpreter.c_str(), args.data(), envp.data());
	throw ServerException("Error occured while execve() function was called");
}

void CGIServer::handleParentProcess(const std::string& method, const std::string& body)
{
	close(inputPipe[IN]);
	close(outputPipe[OUT]);

	if (method == "POST")
	{
		write(inputPipe[1], body.c_str(), body.size());
	}
	close(inputPipe[OUT]);

	char buffer[10];
	ssize_t bytesRead;

	while ((bytesRead = read(outputPipe[IN], buffer, sizeof(buffer))) > 0)
	{
		result.append(buffer, bytesRead);
	}
	close(outputPipe[IN]);
}

void CGIServer::handleProcesses(Request& request, const std::string& interpreter, const std::vector<std::string>& envVars)
{
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
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
		handleChildProcess(interpreter, request.getStartLine()["path"], envVars);
	}
	else
	{
		handleParentProcess(request.getStartLine()["method"], request.getBody());
		waitpid(pid, nullptr, 0);
	}
}

// int main() {
// 	Request request("POST cgi-bin/script.py?name=alex&surname=johnson HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nsec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google Chrome\";v=\"126\"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: \"macOS\"\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7\r\nCookie: wp-settings-1=libraryContent%3Dbrowse%26posts_list_mode%3Dlist; wp-settings-time-1=1697667275; adminer_permanent=c2VydmVy--cm9vdA%3D%3D-bG9jYWw%3D%3AWdDaEmjuEAY%3D\r\n\r\n");
// 	std::string response = CGIServer::handleCGI(request);
// 	std::cout << "Response from CGI script:\n\n\n" << response << std::endl;

// 	return 0;
// }