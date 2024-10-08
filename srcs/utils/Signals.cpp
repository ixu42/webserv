/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 19:35:22 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/06 15:15:34 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Signals.hpp"

void Signals::killAllChildrenPids()
{
	for (auto pid : g_childPids)
	{
		if (pid > 0)
		{
			std::cout << TEXT_WHITE << "\n[" << getCurrentTime() << "] " << RESET;
			std::cout << TEXT_MAGENTA << "[INFO] " << RESET;
			std::cout << TEXT_MAGENTA << "Terminating the child process with pid [" << pid << "]" << RESET;
			kill(pid, SIGTERM);
		}
	}
}

void Signals::signalHandler(int signal)
{
	LOG_DEBUG("Signal ", signal, " received");
	g_signalReceived.store(true);
	killAllChildrenPids();
	std::cout << TEXT_WHITE << "\n[" << getCurrentTime() << "] " << RESET;
	std::cout << TEXT_MAGENTA << "[INFO] " << RESET;
	std::cout << TEXT_MAGENTA << "Shutting down the server(s)..." << RESET << std::endl;
}

void Signals::trackSignals()
{
	signal(SIGINT, signalHandler); /* ctrl + c */
	signal(SIGTSTP, signalHandler); /* ctrl + z */
	signal(SIGQUIT, signalHandler); /* ctrl + \ */
	signal(SIGTERM, signalHandler); /* kill -15 pid */
	signal(SIGPIPE, SIG_IGN); /* cancelling request */
}
