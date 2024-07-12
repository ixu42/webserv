/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 12:23:32 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/12 12:31:20 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <sstream>

class SessionsManager {
private:
    std::string sessionDir;
    const int sessionLifeSpan = 10; // years
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sessions;
    
    std::string generateSessionId();
    void saveSessionToFile(const std::string &sessionId);
    bool loadSessionFromFile(const std::string &sessionId);

public:
    SessionsManager();
    SessionsManager(const std::string &sessionFile);
    
    std::string createSession(const std::string &domain);
    bool loadSession(const std::string &sessionId);
    void addDataToSession(const std::string &sessionId, const std::string &key, const std::string &value);
    std::unordered_map<std::string, std::string> getSessionData(const std::string &sessionId);
    void printSessionsToFile();
};
