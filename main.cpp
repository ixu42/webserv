/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:04:36 by ixu               #+#    #+#             */
/*   Updated: 2024/06/20 12:41:48 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Socket.hpp"

int	main()
{
	Socket	socket;
	socket.create();
	std::cout << socket.getSocketFd() << std::endl;
	socket.bindToSocket();
	socket.listenOnSocket();

	return (0);
}
