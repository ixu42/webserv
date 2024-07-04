/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 00:51:19 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/04 19:29:10 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"

ServerException::ServerException(const std::string message) : _message(message) {}

const char* ServerException::what() const noexcept
{
	return _message.c_str();
}