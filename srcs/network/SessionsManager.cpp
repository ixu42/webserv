/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 15:02:01 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/15 12:53:34 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SessionsManager.hpp"

const std::string SessionsManager::_filename = "sessions";
std::string SessionsManager::_session;

void SessionsManager::setSession(std::string session)
{
	_session = session;
}

std::string& SessionsManager::getSession()
{
	return _session;
}

const std::string SessionsManager::getFilename()
{
	return _filename;
}

void SessionsManager::generateSession()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

	std::mt19937 generator(static_cast<unsigned long>(microseconds));

	std::uniform_int_distribution<int> distribution(0, 35);

	std::stringstream sessionStream;
	sessionStream << "sessionid=";
	for (int i = 0; i < 64; ++i)
	{
		int randomValue = distribution(generator);
		if (randomValue < 10)
		{
			sessionStream << static_cast<char>('0' + randomValue);
		}
		else
		{
			sessionStream << static_cast<char>('A' + (randomValue - 10));
		}
	}
	sessionStream << "; expires=";
	Utility::strToLower(sessionStream.str());
	
	auto expirationTime = std::chrono::system_clock::now() + std::chrono::hours(24 * 365 * 25);
	std::time_t expirationTimeT = std::chrono::system_clock::to_time_t(expirationTime);
	char expirationBuffer[100];
	std::strftime(expirationBuffer, sizeof(expirationBuffer), "%a, %d-%b-%Y %H:%M:%S GMT", std::gmtime(&expirationTimeT));
	
	sessionStream << expirationBuffer;
	
	setSession(sessionStream.str());
}

void SessionsManager::checkIfFileExist()
{
	std::ifstream infile(_filename);
	
	if (!infile.good())
	{
		std::ofstream outfile(_filename);
		if (outfile.is_open())
		{
			outfile.close();
		}
		else
		{
			throw ResponseError(500, {}, "Exception has been thrown in checkIfFileExist() "
			"method of SessionsManager class");
		}
	}
	infile.close();
}

bool SessionsManager::sessionExistsCheck(std::string& sessionData)
{
	std::ifstream infile(_filename);
	std::string line;
	
	if (!infile.is_open())
	{
		throw ResponseError(500, {}, "Exception has been thrown in writeSessionToFile() "
			"method of SessionsManager class");
	}

	while (std::getline(infile, line))
	{
		if (line.find(sessionData) != std::string::npos)
		{
			infile.close();
			return true;
		}
	}

	infile.close();
	return false;
}

void SessionsManager::addSessionToFile(std::string& sessionData)
{
	std::deque<std::string> sessions;
	std::ifstream infile(_filename);
	std::string line;
	
	while (std::getline(infile, line))
	{
		sessions.push_back(line);
	}
	infile.close();
	
	sessions.push_back(sessionData);
	
	manageSessions(sessions);
	
	std::ofstream outfile(_filename, std::ios_base::trunc);
	
	if (outfile.is_open())
	{
		for (const auto& session : sessions)
		{
			outfile << session << std::endl;
		}
		outfile.close();
	}
	else
	{
		throw ResponseError(500, {}, "Exception has been thrown in addSession() "
			"method of SessionsManager class");
	}
}

void SessionsManager::manageSessions(std::deque<std::string>& sessions)
{
	while (sessions.size() > _MAX_SESSIONS)
	{
		sessions.pop_front();
	}
}

void SessionsManager::setSessionToResponse(Response* response, std::string& sessionData)
{
	response->setHeader("Set-Cookie", sessionData);
}

void SessionsManager::handleSessions(t_client& client)
{
	std::string cookie = client.request->getHeaders()["Cookie"];

	if (cookie.empty())
	{
		generateSession();
	}
	else
	{
		setSession(cookie);
	}
	checkIfFileExist();

	if (!sessionExistsCheck(getSession()))
	{
		addSessionToFile(getSession());
	}

	setSessionToResponse(client.response, getSession());
}
