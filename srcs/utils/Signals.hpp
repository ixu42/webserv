/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 19:35:24 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/01 17:02:14 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "colors.hpp"
#include "logUtils.hpp"
#include "globals.hpp"

#include <csignal>
#include <iostream>
#include <atomic>
#include <vector>
#include <unistd.h>

class Signals
{
	private:
		static void signalHandler(int signal);
		
	public:
		static void trackSignals();
		static void killAllChildrenPids();
};