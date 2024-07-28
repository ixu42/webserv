/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UrlEncoder.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 14:32:13 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/28 20:40:56 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <sstream>

class UrlEncoder
{
	public:
		static std::string encode(const std::string& str);
		static std::string decode(const std::string& str);
};
