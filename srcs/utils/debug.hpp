/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 11:05:45 by ixu               #+#    #+#             */
/*   Updated: 2024/07/04 19:38:18 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

#ifdef DEBUG_MODE
	// variadic macro for printing debugging messages with newline
	# define DEBUG(...) \
		do { \
			std::cerr << "[DEBUG] "; \
			std::cerr << __VA_ARGS__; \
			std::cerr << std::endl; \
		} while (0)

	// variadic macro for printing debugging messages without newline
	# define DEBUG_NO_NL(...) \
		do { \
			std::cerr << "[DEBUG] "; \
			std::cerr << __VA_ARGS__; \
		} while (0)
#else
	// define empty macros
	# define DEBUG(...)
	# define DEBUG_NO_NL(...)
#endif
