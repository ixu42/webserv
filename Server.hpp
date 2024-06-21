#pragma once
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdexcept>
#include <exception>

#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h> // For inet_pton

#include <unistd.h>

#include <fcntl.h>

#define DEFAULT_ADDRESS INADDR_ANY
#define DEFAULT_PORT	8090

class ServerException : public std::runtime_error
{
	public:
		explicit ServerException(const std::string &message)
		: std::runtime_error(message) {}
};

// class ServerException : public std::exception
// {
// 	private:
// 		std::string message;

// 	public:
// 		virtual const char* what() const noexcept;
// };

class Server
{
	private:

		int			serverSocket;

		const int			domain = AF_INET;
		// __socket_type		type = SOCK_STREAM;
		int		type = SOCK_STREAM;
		int					protocol = 0;

		in_addr_t			address = DEFAULT_ADDRESS;
		int					port = DEFAULT_PORT;
		struct sockaddr_in	sockAddress;

		int					clientSocket;

	public:
		Server();
		Server(std::string address, int port);
		~Server();

		void initialize();
		void createSocket();
		void bindSocket();
		void listenConnection();
		void acceptConnection(); // remove later? blocking for several servers
 
		void handleRequest();
		void shutdown();
		
		/* Getters */
		int getSocket() const;
		struct sockaddr_in& getSockAddress();
		int getClientSocket() const;

		/* Setters */
		void setClientSocket(int newClientSocket);
};
