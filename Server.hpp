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
#include <cctype>

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
		ServerConfig*		config = nullptr;

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
		void handleRequest2();
		void handleRequest3();
		void shutdown();
		std::string whoAmI() const;

		Request				receiveRequest();
		Location*			findLocation(Request* req);
		
		/* Getters */
		int					getSocket() const;
		struct sockaddr_in&	getSockAddress();
		int					getClientSocket() const;
		ServerConfig* 		getConfig();

		/* Setters */
		void				setClientSocket(int newClientSocket);
		void				setConfig(ServerConfig* serverConfig);
};
