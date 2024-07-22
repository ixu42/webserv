/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/07/22 19:41:45 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"
#include "CGIHandler.hpp"
#include "SessionsManager.hpp"
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

		// Request*					receiveRequest(int clientSockfd);
		bool						receiveRequest(Client& client);
		bool						sendResponse(Client& client);
		void						finalizeResponse(Client& client);
		ServerConfig*				findServerConfig(Request* req);

	private:
		std::string					whoAmI() const;
		void						initServer(const char* ipAddr, int port);
		void						removeFromClients(Client& client);


		void						validateRequest(Client& client);
		// bool						formRequestErrorResponse(t_client& client);
		bool						formCGIConfigAbsenceResponse(Client& client, Server &server);
		void						handleUpload(Client& client, Location& foundLocation);
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
