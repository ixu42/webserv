/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 19:35:24 by dnikifor          #+#    #+#             */
/*   Updated: 2024/07/28 20:25:17 by dnikifor         ###   ########.fr       */
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
};