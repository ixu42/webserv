/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:10:43 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/03 15:33:54 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <stdexcept>
#include <exception>

class ServerException : public std::runtime_error
{
	public:
		explicit ServerException(const std::string &message)
		: std::runtime_error(message) {}
};
