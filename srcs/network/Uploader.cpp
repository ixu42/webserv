/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uploader.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 17:53:36 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/16 18:20:06 by vshchuki         ###   ########.fr       */
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
std::string Uploader::extractFromMultiValue(std::string value, std::string field)
{
	std::string extract;

	value = Utility::replaceWhiteSpaces(value, ' ');
	std::vector<std::string> splitVec = Utility::splitString(value, " ");

	auto pos = std::find_if(splitVec.begin(), splitVec.end(), [field](const std::string s) {
		return s.compare(0, 9, field + "=") == 0;
	});

	if (pos != splitVec.end())
	{
		size_t fieldPos = value.find(field);
		value = value.substr(fieldPos, value.size() - fieldPos);
		std::vector<std::string> extractSplitVec = Utility::splitString(value, "=");
		if (extractSplitVec.size() >= 2)
			extract = extractSplitVec[1];
	}

	return extract;
}

std::string Uploader::findUploadFormBoundary(t_client& client)
{
	std::string contentTypeValue = Utility::replaceWhiteSpaces(client.request->getHeaders().at("content-type"), ' ');
	std::string boundary = extractFromMultiValue(contentTypeValue, "boundary");

	return boundary;
}

std::string Uploader::removeQuotes(const std::string& str) {
	std::string result = str;
	if (!result.empty() && (result.front() == '"' || result.front() == '\'') && result.front() == result.back()) {
		result.erase(result.begin());
		result.pop_back();
	}
	return result;
}

int Uploader::handleUpload(t_client& client, Location& foundLocation)
{
	LOG_DEBUG("handleUpload() called");
	size_t filesCreated = 0;
	// Handle upload from API app (Thunder Client for example)
	if (client.request->getHeaders().at("content-type") == "application/octet-stream")
	{
		// check query string for filename
		LOG_INFO(TEXT_CYAN, "API Client upload...", RESET);
	}
	// Handle upload from the HTML form
	else if (client.request->getHeaders().at("content-type").find("multipart/form-data") != std::string::npos)
	{
		LOG_INFO(TEXT_CYAN, "HTML Form upload...", RESET);
		std::string boundary = findUploadFormBoundary(client);
		LOG_DEBUG(TEXT_GREEN, boundary, RESET);
		std::string requestBody = Utility::replaceStrInStr(client.request->getBody(), "--" + boundary + "--", "");
		std::vector<std::string> multipartVec = Utility::splitString(requestBody, "--" + boundary);

		for (std::string& part : multipartVec)
		{
			LOG_DEBUG(TEXT_GREEN, part, RESET);
			std::string filename;

			size_t emptyLinePos = part.find("\r\n\r\n") != std::string::npos ? part.find("\r\n\r\n") : part.find("\n\n");
			if (emptyLinePos == std::string::npos)
				continue;
			size_t emptyLinesSize = part.find("\r\n\r\n") != std::string::npos ? 4 : 2;

			std::string headers = part.substr(0, emptyLinePos);
			std::string body = emptyLinePos + emptyLinesSize != part.size() ? 
								part.substr(emptyLinePos + emptyLinesSize) :
								"";

			// LOG_DEBUG("body: \n", body);
			std::istringstream stream(headers);
			std::string line;
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
		}
	}
	if (filesCreated)
		return 201;
	else
		return 500;
}