/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:16 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/23 18:03:42 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include <regex>
#include <iostream>

#include "../utils/Colors.hpp"
#include "../utils/Utility.hpp"
#include "Config.hpp"

struct ServerConfig;

class ConfigValidator
{
	private:
		ConfigValidator() = delete;
		ConfigValidator(const ConfigValidator& other) = delete;
		ConfigValidator& operator=(const ConfigValidator& other) = delete;
		static int	checkUnique(std::string line);
		static int	matchLinePattern(std::string& line, std::string field, std::regex pattern2);
		static int	validateServerNamePerIpPort(std::vector<std::string> serverStrings, size_t i, std::map<std::string, std::regex> patterns);
		static int	countMatchInRegex(std::string str, std::regex pattern);
		static int	validateMandatoryFields(std::string str, std::vector<std::string> mandatoryFields,
						std::map<std::string, std::regex> patterns);

	public:
		static int	validateMainConfig(std::string mainConfig);
		static int	validateGeneralConfig(std::string generalConfig, std::vector<std::string> serverStrings, size_t i);
		static int	validateLocationConfig(std::string locationString);
		// static std::pair<std::string, int>	contructCgiString();
};
