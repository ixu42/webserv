/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/06/25 09:50:22 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Socket.hpp"
# include <vector>

class Server
{
	private:
		Socket						_serverSocket;
		struct sockaddr_in			_address;
		static volatile bool		_running;
		std::vector<int>			_clientSockfds;
		std::vector<struct pollfd>	_fds;

	public:
		Server();
		Server(char* ipAddr, int port);
		~Server();
		bool						run();

	private:
		void						initServer(char* ipAddr, int port);
		static void					signalHandler(int signum);
		bool						accepter();
		void						handler(int clientSockfd);
		void						responder(int clientSockfd);
		void						removeClientSocket(int clientSockfd);
		const std::string			getResponse();
};

#endif
