/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/06/24 09:03:42 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Socket.hpp"

class Server
{
	protected:
		Socket				_serverSocket;
		int					_port;
		struct sockaddr_in	_address;
		int					_backlog;
		static bool			_running;

	public:
		Server();
		virtual ~Server();

		virtual bool		launch() = 0;

	private:
		virtual int			accepter() = 0;
		virtual void		handler() = 0;
		virtual void		responder() = 0;
		static void			signal_handler(int signum);
};

#endif
