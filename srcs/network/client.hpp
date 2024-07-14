/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 21:59:14 by ixu               #+#    #+#             */
/*   Updated: 2024/07/14 03:02:00 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "../response/Response.hpp"

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
	Request*	request;
	Response*	response;
	ClientState	state = READING;
	std::string	requestString;
	std::string	responseString;
	size_t		totalBytesWritten = 0;
}	t_client;
