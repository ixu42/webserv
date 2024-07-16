/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/16 18:21:09 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "client.hpp"
#include <string>
#include "../config/Config.hpp"
#include "../response/Response.hpp"


class Uploader
{
	private:
		static std::string	extractFromMultiValue(std::string value, std::string field);
		static std::string	findUploadFormBoundary(t_client& client);
		static std::string	removeQuotes(const std::string& str);

	public:
		static int			handleUpload(t_client& client, Location& foundLocation);
};
