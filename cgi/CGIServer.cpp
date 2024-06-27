#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

class CGIServer {
public:
    CGIServer(const std::string& scriptPath) : scriptPath(scriptPath) {}

    void handleRequest(const std::string& method, const std::string& queryString, const std::string& body, const std::string& filePath) {
        std::vector<std::string> envStrings = setEnvironmentVariables(method, queryString, filePath);

        int inputPipe[2], outputPipe[2];
        if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            handleChildProcess(inputPipe, outputPipe, filePath, envStrings);
        } else {
            handleParentProcess(inputPipe, outputPipe, method, body);
            waitpid(pid, nullptr, 0);
        }
    }

private:
    std::string scriptPath;

    std::vector<std::string> setEnvironmentVariables(const std::string& method, const std::string& queryString, const std::string& filePath) {
        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("QUERY_STRING=" + queryString);
        env.push_back("PATH_INFO=" + filePath);
        env.push_back("SCRIPT_FILENAME=" + filePath);
        return env;
    }

    void handleChildProcess(int inputPipe[2], int outputPipe[2], const std::string& filePath, const std::vector<std::string>& envStrings) {
        dup2(inputPipe[0], STDIN_FILENO);
        dup2(outputPipe[1], STDOUT_FILENO);

        close(inputPipe[1]);
        close(outputPipe[0]);

        std::vector<char*> args;
        args.push_back(const_cast<char*>(scriptPath.c_str()));
        args.push_back(const_cast<char*>(filePath.c_str()));
        args.push_back(nullptr);

        std::vector<char*> envp;
        for (const auto& var : envStrings) {
            envp.push_back(const_cast<char*>(var.c_str()));
        }
        envp.push_back(nullptr);

        execve(scriptPath.c_str(), args.data(), envp.data());
        perror("execve");
        exit(1);
    }

    void handleParentProcess(int inputPipe[2], int outputPipe[2], const std::string& method, const std::string& body) {
        close(inputPipe[0]);
        close(outputPipe[1]);

        if (method == "POST") {
            write(inputPipe[1], body.c_str(), body.size());
        }
        close(inputPipe[1]);

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
            std::cout.write(buffer, bytesRead);
        }
        close(outputPipe[0]);
    }
};

int main() {

    std::string scriptPath = "/usr/bin/python3";
    std::string method = "POST";
    std::string queryString = "";
    std::string body = "";
    std::string filePath = "script.py";

    CGIServer server(scriptPath);
    server.handleRequest(method, queryString, body, filePath);

    return 0;
}
