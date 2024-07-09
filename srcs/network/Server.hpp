/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/09 16:03:36 by vshchuki         ###   ########.fr       */
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
#include "../utils/logUtils.hpp"
#include "../request/Request.hpp"
#include "../utils/ServerException.hpp"
#include "client.hpp"
#include <vector>
#include <string>
#include <cstring> // memset()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <unistd.h> // read(), write(), close()

#include <limits> // for max size_t

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <fstream> //open file

#include <filesystem> // for createDirListResp()
#include <chrono> // for createDirListResp()

namespace fs = std::filesystem;

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

		void						setConfig(std::vector<ServerConfig> serverConfigs);
		int							getServerSockfd();
		Pipe&						getPipe();
		std::vector<t_client>&		getClients();
		std::string					getIpAddress();
		int							getPort();
		std::vector<ServerConfig>	getConfig();

		int							accepter();
		void						handler(Server*& server, t_client& client);
		void						responder(t_client& client, Server &server);

		Request*					receiveRequest(int clientSockfd);

	private:
		std::string					whoAmI() const;
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(t_client& client);
		bool						formRequestErrorResponse(t_client& client);
		bool						formCGIConfigAbsenceResponse(t_client& client, Server &server);
		void						handleCGIResponse(t_client& client, Server &server);
		void						handleNonCGIResponse(t_client& client, Server &server);
		void						checkIfAllowed(t_client& client, Location& foundLocation);
		void						handleRedirect(t_client& client, Location& foundLocation);
		void						handleStaticFiles(t_client& client, Location& foundLocation);
		void						finalizeResponse(t_client& client);
		Location					findLocation(Request* req);
		void						sendResponse(std::string& response, t_client& client);
		Response*					createDirListResponse(Location& location, std::string requestPath);
		std::stringstream			generateDirectoryListingHtml(const std::string& root);
		
		ServerConfig*				findServerConfig(Request* req);
		size_t						findMaxClientBodyBytes(Request request);
};
