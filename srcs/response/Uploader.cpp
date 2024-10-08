/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/30 20:12:10 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Uploader.hpp"

/**
 * Example:
 * For Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryTZKquQIqOzprsPRf
 * value = "multipart/form-data; boundary=----WebKitFormBoundaryTZKquQIqOzprsPRf"
 * field = "boundary"
 * return -> "----WebKitFormBoundaryTZKquQIqOzprsPRf"
 * 
 * For Content-Disposition: form-data; name="file1"; filename="favicon.ico"
 * pass value and field as "filename", it will extract: "favicon.ico"
*/
std::string Uploader::extractFromMultiValue(const std::string value, const std::string field)
{
	std::string extract;
	std::regex regex(field + R"(=\"([^\"]*)\")"); // Regex to find field="value" pattern

	std::smatch match;
	if (std::regex_search(value, match, regex)) {
		if (match.size() == 2) {
			extract = match[1].str(); // Extract the value inside the quotes
		}
	} else {
		// If not found with quotes, try without quotes
		std::regex regex_no_quotes(field + R"(=([^;]*))"); // Regex to find field=value pattern without quotes
		if (std::regex_search(value, match, regex_no_quotes)) {
			if (match.size() == 2) {
				extract = match[1].str();
			}
		}
	}

	return extract;
}

std::string Uploader::findUploadFormBoundary(Client& client)
{
	std::string contentTypeValue = Utility::replaceWhiteSpaces(client.getRequest()->getHeaders().at("content-type"), ' ');
	std::string boundary = extractFromMultiValue(contentTypeValue, "boundary");

	return boundary;
}

std::string Uploader::removeQuotes(const std::string& str)
{
	std::string result = str;
	if (!result.empty() && (result.front() == '"' || result.front() == '\'') && result.front() == result.back()) {
		result.erase(result.begin());
		result.pop_back();
	}
	return result;
}

size_t Uploader::processForm(std::string headers, std::string body, Location& foundLocation)
{
	std::istringstream stream(headers);
	std::string line;
	std::string filename;
	size_t filesCreated = 0;

	// Process headers
	while (std::getline(stream, line))
	{
		size_t colonPos = line.find(":");
		if (colonPos != std::string::npos)
		{
			std::string headerKey = Utility::trim(line.substr(0, colonPos));
			std::string headerValue = Utility::trim(line.substr(colonPos + 1));
			LOG_INFO(TEXT_GREEN, "headerKey: ", headerKey, RESET);
			LOG_INFO(TEXT_GREEN, "headerValue: ", headerValue, RESET);

			if (Utility::strToLower(headerKey) == "content-disposition")
			{
				filename = removeQuotes(extractFromMultiValue(headerValue, "filename"));
				LOG_INFO(TEXT_GREEN, "filename: ", filename, RESET);
				break;
			}
		}
	}
	
	// Process body
	if (!filename.empty())
	{
		LOG_INFO(TEXT_YELLOW, "File will be created here: ", foundLocation.root, RESET);
		Utility::createFile(foundLocation.root + filename, body);
		filesCreated++;
	}
	return filesCreated;
}

int Uploader::handleUpload(Client& client, Location& foundLocation)
{
	LOG_DEBUG("handleUpload() called");
	size_t filesCreated = 0;
	
	// Handle upload from the HTML form
	if (client.getRequest()->getHeaders().at("content-type").find("multipart/form-data") != std::string::npos)
	{
		LOG_INFO(TEXT_CYAN, "HTML Form upload...", RESET);
		std::string boundary = findUploadFormBoundary(client);
		LOG_DEBUG(TEXT_GREEN, boundary, RESET);
		std::string requestBody = Utility::replaceStrInStr(client.getRequest()->getBody(), "--" + boundary + "--", "");
		std::vector<std::string> multipartVec = Utility::splitStr(requestBody, "--" + boundary);

		for (std::string& part : multipartVec)
		{
			LOG_DEBUG(TEXT_GREEN, part, RESET);

			size_t emptyLinePos = part.find("\r\n\r\n");
			if (emptyLinePos == std::string::npos)
				continue;
			size_t emptyLinesSize = 4;

			std::string headers = part.substr(0, emptyLinePos);
			std::string body = emptyLinePos + emptyLinesSize != part.size() ? 
								part.substr(emptyLinePos + emptyLinesSize) :
								"";

			LOG_DEBUG("body: \n", body);
			filesCreated += processForm(headers, body, foundLocation);
		}
	}
	if (filesCreated)
		return 201;
	return 400;
}
