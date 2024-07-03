/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/03 17:05:24 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define FDS 2

struct Pipe {
	int input[FDS];
	int output[FDS];
};

#include "Socket.hpp"
#include "../utils/Utility.hpp"
#include "../utils/debug.hpp"
#include "../request/Request.hpp"
#include "client.hpp"
#include <vector>
#include <string>
#include <cstring> // memset()
#include <arpa/inet.h> // htons(), inet_pton()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <unistd.h> // read(), write(), close()

class Server
{
	private:
		Socket						_serverSocket;
		struct sockaddr_in			_address;
		std::vector<t_client>		_clients;
		ServerConfig*				_config = nullptr;
		Pipe						_CGIpipes;

		int							_port;
		std::string					_ipAddr;

	public:
		Server();
		Server(const char* ipAddr, int port);
		~Server();

		void						setConfig(ServerConfig* serverConfig);
		ServerConfig*				getConfig();
		int							getServerSockfd();
		Pipe&						getPipe();
		std::vector<t_client>&		getClients();
		std::string					whoAmI() const;

		int							accepter();
		Request*					receiveRequest(int clientSockfd);
		void						responder(t_client& client);

	private:
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(t_client& client);
		const std::string			getResponse();
		
};

// #pragma once

// #include "Colors.hpp"
// #include "Request.hpp"
// #include "ServerException.hpp"

// #include <netinet/in.h>
// #include <sys/types.h>
// #include <sys/socket.h>

// #include <cstring>
// #include <iostream>
// #include <string>
// #include <cctype>

// #include <arpa/inet.h> // For inet_pton

// #include <unistd.h>

// #include <fcntl.h>

// #include <ctime>

// #define DEFAULT_ADDRESS INADDR_ANY
// #define DEFAULT_PORT	8090


// 		int					serverSocket;

// 		const int			domain = AF_INET;
// 		// __socket_type		type = SOCK_STREAM;
// 		int					type = SOCK_STREAM;
// 		int					protocol = 0;

// 		std::string			addressString = "0.0.0.0";

// 		in_addr_t			address = DEFAULT_ADDRESS;
// 		int					port = DEFAULT_PORT;
// 		struct sockaddr_in	sockAddress;

// 		int					clientSocket;
// 		ServerConfig*		config = nullptr;

// 	public:
// 		Server();
// 		Server(std::string address, int port);
// 		~Server();

// 		void initialize();
// 		void createSocket();
// 		void bindSocket();
// 		void listenConnection();
// 		void acceptConnection(); // remove later? blocking for several servers
 
// 		void handleRequest();
// 		void handleRequest2();
// 		void handleRequest3();
// 		std::string whoAmI() const;

// 		Request				receiveRequest();
// 		Location*			findLocation(Request* req);
		
// 		/* Getters */
// 		int					getSocket() const;
// 		struct sockaddr_in&	getSockAddress();
// 		int					getClientSocket() const;
// 		ServerConfig* 		getConfig();

// 		/* Setters */
// 		void				setClientSocket(int newClientSocket);
// 		void				setConfig(ServerConfig* serverConfig);
// };

