#pragma once

#include "Colors.hpp"
#include "Request.hpp"
#include "ServerException.hpp"

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h> // For inet_pton

#include <unistd.h>

#include <fcntl.h>

#include <ctime>

#define DEFAULT_ADDRESS INADDR_ANY
#define DEFAULT_PORT	8090

class Server
{
	private:

		int					serverSocket;

		const int			domain = AF_INET;
		// __socket_type		type = SOCK_STREAM;
		int					type = SOCK_STREAM;
		int					protocol = 0;

		std::string			addressString = "0.0.0.0";

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
		int					getSocket() const;
		struct sockaddr_in&	getSockAddress();
		int					getClientSocket() const;

		/* Setters */
		void setClientSocket(int newClientSocket);
};
