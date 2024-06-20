/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:53 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 13:29:49 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <netinet/in.h> // struct sockaddr_in, struct in_addr
# include <string>

class Socket
{
	private:
		int					_socketFd;

	public:
		Socket();
		~Socket();

		int					getSocketFd();
		bool				create();
		bool				bindAddress(struct sockaddr_in addr);
		bool				listenForConnections(int backlog);
		int					acceptConnection(struct sockaddr_in addr);

	private:
		bool				isValid(int funcReturn);
		void				printError(const std::string& msg);
};

#endif
