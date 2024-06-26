/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/06/26 11:16:20 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Socket.hpp"
# include <vector>
# include "Utility.hpp"
# include "Request.hpp"


class Server
{
	private:
		Socket						_serverSocket;
		struct sockaddr_in			_address;
		static volatile bool		_running;
		std::vector<int>			_clientSockfds;
		// std::vector<struct pollfd>	_fds;
		ServerConfig*				_config = nullptr;

		int							_port;
		std::string					_ipAddr;

	public:
		Server();
		Server(const char* ipAddr, int port);
		~Server();
		// bool						run();
		void						setConfig(ServerConfig* serverConfig);
		ServerConfig*				getConfig();
		int							getServerSockfd();
		std::vector<int>			getClientSockfds();
		std::string					whoAmI() const;

		bool						accepter();
		Request						receiveRequest(int clientSockfd);
		void						handler(int clientSockfd); // remove later
		void						responder(int clientSockfd);

	private:
		void						initServer(const char* ipAddr, int port);
		static void					signalHandler(int signum); // remove later
		void						removeClientSocket(int clientSockfd);
		const std::string			getResponse();
		
};

#endif




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

