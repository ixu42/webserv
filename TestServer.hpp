/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:20:59 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 17:23:41 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTSERVER_HPP
# define TESTSERVER_HPP

# include "Server.hpp"
# include <string>

class TestServer : public Server
{
	private:
		int							_clientSocketFd;
		char 						_buffer[30000];
		static const std::string	_hello;

	public:
		TestServer();
		~TestServer() override;

		bool						launch() override;

	private:
		int							accepter() override;
		void						handler() override;
		void						responder() override;
};

#endif
