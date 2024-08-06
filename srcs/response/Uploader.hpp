/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/06 17:47:11 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../network/Client.hpp"
#include <string>
#include "../config/Config.hpp"
#include "../response/Response.hpp"

class Uploader
{
	private:
		static std::string	extractFromMultiValue(std::string value, std::string field);
		static std::string	findUploadFormBoundary(Client& client);
		static std::string	removeQuotes(const std::string& str);
		static size_t		processForm(std::string headers, std::string body, Location& foundLocation);

	public:
		static int			handleUpload(Client& client, Location& foundLocation);
};
