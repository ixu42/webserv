/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/30 20:02:20 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
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
