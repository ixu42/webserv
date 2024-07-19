/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/19 17:18:33 by dnikifor         ###   ########.fr       */
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

	public:
		static int			handleUpload(Client& client, Location& foundLocation);
};
