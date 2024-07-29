/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/29 16:24:04 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"
#include "CGIHandler.hpp"
#include "SessionsManager.hpp"
#include "ServersManager.hpp"
#include "../response/Response.hpp"
#include "../utils/Utility.hpp"
#include "../utils/logUtils.hpp"
#include "../request/Request.hpp"
#include "../utils/ServerException.hpp"
#include "DirLister.hpp"
#include "Uploader.hpp"
#include <vector>
#include <string>
#include <cstring> // memset()
#include <signal.h> // signal()
#include <poll.h> // poll()
#include <unistd.h> // read(), write(), close()
#include <dirent.h> // opendir(), readdir(), closedir()

#include <limits> // for max size_t

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <filesystem>

#include <fstream> //open file

#include <regex> // to match header of chunked request

class ServersManager;

class Server
{
	private:
		Socket						_serverSocket;
		struct addrinfo				_hints;
		struct addrinfo*			_res;
		std::vector<Client>			_clients;
		std::vector<ServerConfig>	_configs;
		std::vector<struct pollfd>*	_managerFds;
		std::vector<std::string>	_cgiBinFiles;

		const char *				_CGIBinFolder = "cgi-bin/";
		int							_port;
		std::string					_ipAddr;

	public:
		Server();
		Server(const char* ipAddr, int port);
		~Server();

		void						setConfig(std::vector<ServerConfig> serverConfigs);
		void						setFds(std::vector<struct pollfd>* fds);
		
		int							getServerSockfd();
		std::vector<Client>&		getClients();
		std::string					getIpAddress();
		int							getPort();
		std::vector<ServerConfig>&	getConfigs();
		std::vector<struct pollfd>*	getFds();
		const char *				getCGIBinFolder();
		std::vector<std::string>	getcgiBinFiles();

		int							accepter();
		void						handler(Server*& server, Client& client);
		void						responder(Client& client, Server &server);

		bool						receiveRequest(Client& client);
		bool						sendResponse(Client& client);
		void						finalizeResponse(Client& client);
		ServerConfig*				findServerConfig(Request* req);

	private:
		std::string					whoAmI() const;
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(Client& client);


		void						validateRequest(Client& client);
		int							findContentLength(std::string request);
		bool						formCGIConfigAbsenceResponse(Client& client, Server &server);
		void						handleNonCGIResponse(Client& client, Server &server);
		void						checkIfMethodAllowed(Client& client, Location& foundLocation);
		void						handleRedirect(Client& client, Location& foundLocation);
		int							handleDelete(Client& client, Location& foundLocation);
		void						handleStaticFiles(Client& client, Location& foundLocation);
		Location					findLocation(Request* req);
		void						listCGIFiles();
		bool						isCGIBinExistAndReadable();
		
		size_t						findMaxClientBodyBytes(Request request);

		Response*					createResponse(Request* request, int code, std::map<std::string,
										std::string> optionalHeaders = {});
};
