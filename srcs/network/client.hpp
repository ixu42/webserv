/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 21:59:14 by ixu               #+#    #+#             */
/*   Updated: 2024/07/17 12:49:22 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include <limits>

#define FDS 2

enum ClientState
{
	READING,
	READY_TO_WRITE,
	WRITING,
	FINISHED_WRITING
};

typedef struct s_client
{
	int			fd;
	int			parentPipe[FDS];
	int			childPipe[FDS];
	std::string	CGIString;
	Request*	request = nullptr;
	Response*	response = nullptr;
	ClientState	state = READING;
	std::string	requestString;
	std::size_t maxClientBodyBytes = std::numeric_limits<std::size_t>::max();
	std::string	responseString;
	size_t		totalBytesWritten = 0;
}	t_client;
