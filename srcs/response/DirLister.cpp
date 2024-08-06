/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirLister.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 23:15:53 by vshchuki          #+#    #+#             */
/*   Updated: 2024/08/05 13:17:14 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DirLister.hpp"

bool DirLister::generateSymbolicLinkRow(std::stringstream& htmlStream, fs::path filePath)
{
	if (fs::is_symlink(filePath))
	{
		fs::path targetPath = fs::read_symlink(filePath); // Correctly read the symlink target
		htmlStream << "<div class=\"file-row\">\n";
		htmlStream << "<div class=\"file-cell name\"><a href=\"" << targetPath.string();
		htmlStream << (targetPath.string() != "/" ? "/" : "");
		htmlStream << "\">" << filePath.filename().string() << " -> " << targetPath.string() << "</a></div>\n";
		htmlStream << "<div class=\"file-cell date\">-</div>\n";
		htmlStream << "<div class=\"file-cell size\">-</div>\n";
		htmlStream << "</div>\n";
		return true;
	}
	return false;
}

std::stringstream DirLister::generateDirectoryListingHtml(const std::string& root)
{
	std::stringstream htmlStream;
	
	htmlStream << "<div class=\"file-list\">\n";
	htmlStream << "<div class=\"file-row\">\n";
	htmlStream << "<div class=\"file-cell\"><a href=\"../\">../</a></div>\n";
	htmlStream << "<div class=\"file-cell\"></div>\n";
	htmlStream << "<div class=\"file-cell\"></div>\n";
	htmlStream << "</div>\n";
	// Iterate over all the entries in the directory
	for (const auto& entry : fs::directory_iterator(root)) {
		fs::path filePath = entry.path();
		std::string fileName = filePath.filename().string();
		if (generateSymbolicLinkRow(htmlStream, filePath))
			continue;
		// Calculate last write time
		auto ftime = fs::last_write_time(filePath);
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now()
				+ std::chrono::system_clock::now());
		std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

		htmlStream << "<div class=\"file-row\">\n";
		// Output name
		htmlStream << "<div class=\"file-cell name\"><a href=\"" << UrlEncoder::encode(fileName);
		if (entry.is_directory())
			htmlStream << "/";
		htmlStream << "\">" << fileName;
		if (entry.is_directory())
			htmlStream << "/"; 
		htmlStream << "</a></div>\n";
		// Output last write time
		htmlStream << "<div class=\"file-cell date\">" << std::put_time(std::localtime(&cftime), "%d-%b-%Y %T") << "</div>";
		// Output size
		htmlStream << "<div class=\"file-cell size\">";
		htmlStream << (!entry.is_directory() ? std::to_string(fs::file_size(filePath)) : "-");
		htmlStream << "</div></div>\n";
	}
	htmlStream << "</div>\n";
	
	return htmlStream;
}

/**
 * Creates dynamic body for Response using, current location and html template
 * html template should have 
 */
std::shared_ptr<Response> DirLister::createDirListResponse(Location& location, std::string requestPath)
{
	std::string pathShortCode = "[path]";
	std::string bodyShortCode = "[body]";
	std::shared_ptr<Response> listingResponse = std::make_shared<Response>();
	std::string fileString = Utility::readFile(location.defaultListingTemplate);
	std::stringstream htmlStream;

	// Find the root for requstPath
	std::string root = location.root + requestPath.substr(location.path.length());

	try
	{
		htmlStream = generateDirectoryListingHtml(root);
	}
	catch (const fs::filesystem_error& e)
	{
		LOG_ERROR("Error accessing directory: ", e.what());
		throw ProcessingError(403, {}, "Exception has been thrown in createDirListResponse() "
			"method of Server class");
	}

	// Replace [body]
	size_t replaceStringPos = fileString.find(bodyShortCode);
	if (replaceStringPos != std::string::npos)
		fileString.replace(replaceStringPos,  bodyShortCode.length(), htmlStream.str());
	// Replace [title]
	while ((replaceStringPos = fileString.find(pathShortCode)) != std::string::npos)
		fileString.replace(replaceStringPos, pathShortCode.length(), requestPath);

	listingResponse->setBody(fileString);
	listingResponse->setTypeFromFormat("html");
	listingResponse->setStatusFromCode(200);

	return listingResponse;
}
