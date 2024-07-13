/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 12:23:32 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/12 16:14:43 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "client.hpp"
#include "../utils/Utility.hpp"
#include "../request/Request.hpp"

class SessionsManager {
private:
	static std::string _session;
	
public:
	static void setSession(std::string session);
	static std::string getSession();
	
	static void generateSession(Request& request);
	
	static void saveSessionToFile();
	static bool loadSessionFromFile(const std::string &sessionId);
	static std::string createSession(const std::string &domain);
	static bool loadSession(const std::string &sessionId);
	static void addDataToSession(const std::string &sessionId, const std::string &key, const std::string &value);
	static std::unordered_map<std::string, std::string> getSessionData(const std::string &sessionId);
	static void printSessionsToFile();
};
