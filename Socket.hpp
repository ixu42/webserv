/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:53 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 12:21:40 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <netinet/in.h> // struct sockaddr_in, struct in_addr
# include <string>

class Socket
{
	private:
		int					_socket_fd;
		int					_backlog;
		struct sockaddr_in	_addr;

	public:
		Socket();
		Socket(int domain, int type, int protocol, int port, int backlog);
		~Socket();

		int					get_socket_fd();
		bool				bind_to_socket();
		bool				listen_on_socket();

	private:
		void				print_error(const std::string& msg);
};

#endif
