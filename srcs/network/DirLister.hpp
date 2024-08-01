/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirLister.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 23:16:26 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/01 03:00:37 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../response/Response.hpp"
#include "../utils/ServerException.hpp"
#include "../utils/Utility.hpp"
#include "../config/Config.hpp"
#include "../utils/UrlEncoder.hpp"
#include <sstream>	// std::stringstream
#include <filesystem> // for createDirListResp()
#include <chrono> // for createDirListResp()
#include <string>

namespace fs = std::filesystem;

class DirLister
{
	public:
		static bool					generateSymbolicLinkRow(std::stringstream& htmlStream, fs::path filePath);
		static Response*			createDirListResponse(Location& location, std::string requestPath);
		static std::stringstream	generateDirectoryListingHtml(const std::string& root);
};