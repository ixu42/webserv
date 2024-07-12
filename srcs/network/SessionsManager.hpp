/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 12:23:32 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/12 12:35:03 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <sstream>

#define SESSIONS_FILE "sessions.txt"

class SessionsManager {
private:
    const std::string sessionFile = SESSIONS_FILE;
    const int sessionLifeSpan = 10; // years
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sessions;
    
    static std::string generateSessionId();
    static void saveSessionToFile(const std::string &sessionId);
    static bool loadSessionFromFile(const std::string &sessionId);

public:
    static std::string createSession(const std::string &domain);
    static bool loadSession(const std::string &sessionId);
    static void addDataToSession(const std::string &sessionId, const std::string &key, const std::string &value);
    static std::unordered_map<std::string, std::string> getSessionData(const std::string &sessionId);
    static void printSessionsToFile();
};
