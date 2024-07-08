/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/08 10:45:19 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define FDS 2

struct Pipe {
	int input[FDS];
	int output[FDS];
};

#include "Socket.hpp"
#include "CGIHandler.hpp"
#include "../response/Response.hpp"
#include "../utils/Utility.hpp"
#include "../utils/debug.hpp"
#include "../request/Request.hpp"
#include "../utils/ServerException.hpp"
#include "client.hpp"
#include <vector>
#include <string>
#include <cstring> // memset()
// #include <arpa/inet.h> // htons(), inet_pton()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <unistd.h> // read(), write(), close()

#include <limits> // for max size_t

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <fstream> //open file

#include <filesystem> // for createDirListResp()

class Server
{
	private:
		Socket						_serverSocket;
		struct addrinfo				_hints;
		struct addrinfo*			_res;
		std::vector<t_client>		_clients;
		std::vector<ServerConfig>	_configs;
		Pipe						_CGIpipes;

		int							_port;
		std::string					_ipAddr;

	public:
		Server();
		Server(const char* ipAddr, int port);
		~Server();

		// void						setConfig(ServerConfig* serverConfig);
		void						setConfig(std::vector<ServerConfig> serverConfigs);
		// std::vector<ServerConfig>	getConfig();
		int							getServerSockfd();
		Pipe&						getPipe();
		std::vector<t_client>&		getClients();
		std::string					getIpAddress();
		int							getPort();
		std::string					whoAmI() const;

		int							accepter();
		void						handler(Server*& server, t_client& client);
		void						responder(t_client& client, Server &server);

		Request*					receiveRequest(int clientSockfd);
		void						sendResponse(std::string& response, t_client& client);
		Location					findLocation(Request* req);

		Response*					createDirListResponse(Location& location, std::string requestPath);

	private:
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(t_client& client);
		const std::string			getResponse();

		ServerConfig*				findServerConfig(Request* req);
		size_t						findMaxClientBodyBytes(Request request);
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

