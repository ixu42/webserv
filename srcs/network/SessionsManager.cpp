/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionsManager.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/12 15:02:01 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/12 16:14:34 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SessionsManager.hpp"

void SessionsManager::setSession(std::string session)
{
	_session = session;
}

std::string SessionsManager::getSession()
{
	return _session;
}

void SessionsManager::generateSession(Request& request)
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
	
	sessionStream << expirationBuffer << "; domain=";
	sessionStream << request.getHeaders()["host"];
	
	setSession(sessionStream.str());
}

void SessionsManager::saveSessionToFile()
{
	
}