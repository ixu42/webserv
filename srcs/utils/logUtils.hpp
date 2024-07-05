/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 11:05:45 by ixu               #+#    #+#             */
/*   Updated: 2024/07/05 14:12:01 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

/**
 * Utilities for logging four levels of messages: DEBUG, INFO, WARNING, ERROR
 * When compile-time DEBUG_MODE is on (by running make debug with Makefile), 
 * messages at all levels are displayed on terminal. 
 * When compile-time DEBUG_MODE is off (by running make with Makefile), 
 * messages at INFO, WARNING, ERROR levels are displayed on terminal.
 */

#ifdef DEBUG_MODE
	// macros for printing contextual info mostly used for problem diagnosis.
	# define LOG_DEBUG(...) std::cerr << "[DEBUG] " << __VA_ARGS__ << std::endl;
	# define LOG_DEBUG_MULTILINE(...) std::cerr << "[DEBUG] " << std::endl << __VA_ARGS__ << std::endl;
	# define LOG_DEBUG_RAW(...) std::cerr << __VA_ARGS__;
#else
	// define empty macros.
	# define LOG_DEBUG(...) do {} while (false)
	# define LOG_DEBUG_MULTILINE(...) do {} while (false)
	# define LOG_DEBUG_RAW(...) do {} while (false)
#endif

// macro printing key info for tracing program execution in a production environment.
#define LOG_INFO(...) std::cout << TEXT_CYAN << "[INFO] " << __VA_ARGS__ << RESET << std::endl;

// macro for printing warning messages indicating a potential issue in the system.
#define LOG_WARNING(...) std::cout << TEXT_YELLOW << "[WARNING] " << __VA_ARGS__ << RESET << std::endl;

// macro for printing error messages indicating an error has occurred.
#define LOG_ERROR(...) std::cout << TEXT_RED << "[ERROR] " << __VA_ARGS__ << RESET << std::endl;
