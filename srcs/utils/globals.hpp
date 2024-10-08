/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   globals.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 17:16:12 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/13 10:45:35 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <atomic>

extern std::vector<pid_t>	g_childPids;
extern std::atomic<bool>	g_signalReceived;
extern const size_t			g_bufferSize;
extern const float			g_timeout;