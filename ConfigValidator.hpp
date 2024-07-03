/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:08:16 by vshchuki          #+#    #+#             */
/*   Updated: 2024/07/03 17:53:24 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include <regex>
#include <iostream>

#include "Colors.hpp"
#include "Utility.hpp"
#include "Config.hpp"

struct ServerConfig;

class ConfigValidator
{
	public:
		static int	checkUnique(std::string line);
		static int	matchLinePattern(std::string& line, std::string field, std::regex pattern2);
		// static int	validateGeneralConfig(std::string generalConfig, std::vector<ServerConfig>& servers);
		static int	validateGeneralConfig(std::string generalConfig);
		static int	validateLocationConfig(std::string locationString);
};
