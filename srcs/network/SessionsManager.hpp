/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 12:23:32 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/16 11:42:51 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <random>

#include "client.hpp"
#include "../utils/Utility.hpp"
#include "../request/Request.hpp"

class SessionsManager {
	private:
		static std::string _session;
		const static std::string _filename;
		const static size_t _MAX_SESSIONS = 500;
		
		static bool sessionExistsCheck(std::string& sessionData);
		static void generateSession(Request* request);
		static void addSessionToFile(std::string& sessionData);
		static void manageSessions(std::deque<std::string>& sessions);
		static void setSessionToResponse(Response* response, std::string& sessionData);
		static bool isHTMLRequest(t_client& client);
		static void checkPermissions();
		
	public:
		static void setSession(std::string session);
		static std::string& getSession();
		const std::string getFilename();
		
		static void handleSessions(t_client& client);
};
