/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 21:59:14 by ixu               #+#    #+#             */
/*   Updated: 2024/07/19 11:12:03 by dnikifor         ###   ########.fr       */
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
	BUILDING,
	WRITING,
	FINISHED_WRITING
};

enum CGIState
{
	INIT,
	FORKED,
	FINISHED_SET,
	FINISHED
};

typedef struct s_client
{
	int			fd;
	pid_t		pid;
	int			parentPipe[FDS] = {-1, -1};
	int			childPipe[FDS] = {-1, -1};
	std::string	CGIString;
	Request*	request = nullptr;
	Response*	response = nullptr;
	ClientState	state = READING;
	CGIState	stateCGI = INIT;
	std::string	requestString;
	std::size_t maxClientBodyBytes = std::numeric_limits<std::size_t>::max();
	std::string	responseString;
	size_t		totalBytesWritten = 0;
}	t_client;
