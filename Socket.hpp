/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:09:53 by ixu               #+#    #+#             */
/*   Updated: 2024/06/19 15:51:35 by ixu              ###   ########.fr       */
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
		// Constructors and destructor
		Socket();
		Socket(int domain, int type, int protocol, int port, int backlog);
		~Socket();

		// Getter functions
		int					get_socket_fd();

		// Other member functions
		void				bind_to_socket();
		void				listen_on_socket();
		void				print_error_and_exit(const std::string& msg);
};

#endif
