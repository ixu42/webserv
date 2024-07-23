/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 11:05:45 by ixu               #+#    #+#             */
/*   Updated: 2024/07/23 12:17:04 by dnikifor         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "Colors.hpp"

/**
 * Utilities for logging four levels of messages: DEBUG, INFO, WARNING, ERROR
 * When compile-time DEBUG_MODE is on (by running make debug with Makefile), 
 * messages at all levels are displayed on terminal. 
 * When compile-time DEBUG_MODE is off (by running make with Makefile), 
 * messages at INFO, WARNING, ERROR levels are displayed on terminal.
 * 
 * Usage:
 * LOG_DEBUG(arg_1, arg_2 ... arg_9); up to 9 args separated by comma are accepted. 
 * LOG_INFO(arg_1, arg_2 ... arg_n); no limitation on the number of arguments passed 
 * to LOG_INFO(), LOG_WARNING(), LOG_ERROR().
 * Simple data types or objects can be passed as args, not function template such as std::endl
 */

inline std::string getCurrentTime()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm;
	
	localtime_r(&now_time_t, &now_tm);
	
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
	
	return oss.str();
}

template<typename... Args>
void LogDebug(Args... args)
{
	std::ostringstream oss;
	// Fold expression to concatenate all arguments
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	(oss << ... << args);
	std::cerr << "[DEBUG] " << oss.str() << std::endl;
}

template<typename... Args>
void LogDebugMultiline(Args... args)
{
	std::ostringstream oss;
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	(oss << ... << args);
	std::cerr << "[DEBUG] " << std::endl << oss.str() << std::endl;
}

template<typename... Args>
void LogDebugRaw(Args... args)
{
	std::ostringstream oss;
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	// Fold expression to print all arguments
	(std::cerr << ... << args);
}

template<typename... Args>
void LogInfo(Args... args)
{
	std::ostringstream oss;
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	(oss << ... << args);
	std::cout << TEXT_CYAN << "[INFO] " << oss.str() << RESET << std::endl;
}

template<typename... Args>
void LogWarning(Args... args)
{
	std::ostringstream oss;
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	(oss << ... << args);
	std::cout << TEXT_BRIGHT_YELLOW << "[WARNING] " << oss.str() << RESET << std::endl;
}

template<typename... Args>
void LogError(Args... args)
{
	std::ostringstream oss;
	oss << TEXT_GREY << "[" << getCurrentTime() << "] " << RESET;
	(oss << ... << args);
	std::cout << TEXT_RED << "[ERROR] " << oss.str() << RESET << std::endl;
}

// Define a macro to cast a single element to void
#define CAST_TO_VOID(x) static_cast<void>(x)

// Define a set of macros to apply CAST_TO_VOID to each argument
#define APPLY_1(func, a) func(a)
#define APPLY_2(func, a, b) func(a); func(b)
#define APPLY_3(func, a, b, c) func(a); func(b); func(c)
#define APPLY_4(func, a, b, c, d) func(a); func(b); func(c); func(d)
#define APPLY_5(func, a, b, c, d, e) func(a); func(b); func(c); func(d), func(e)
#define APPLY_6(func, a, b, c, d, e, f) func(a); func(b); func(c); func(d), func(e), func(f)
#define APPLY_7(func, a, b, c, d, e, f, g) func(a); func(b); func(c); func(d), func(e), func(f), func(g)
#define APPLY_8(func, a, b, c, d, e, f, g, h) func(a); func(b); func(c); func(d), func(e), func(f), func(g), func(h)
#define APPLY_9(func, a, b, c, d, e, f, g, h, i) func(a); func(b); func(c); func(d), func(e), func(f), func(g), func(h), func(i)
// Current implementation accepts 9 args passed to variadic macros such as LOG_DEBUG(). Add more as needed

// Helper macros to count the number of arguments
#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
#define APPLY_TO_EACH(func, ...) GET_MACRO(__VA_ARGS__, APPLY_9, APPLY_8, APPLY_7, APPLY_6, APPLY_5, APPLY_4, APPLY_3, APPLY_2, APPLY_1)(func, __VA_ARGS__)

#ifdef DEBUG_MODE
	// Macros for printing contextual info mostly used for problem diagnosis
	# define LOG_DEBUG(...) LogDebug(__VA_ARGS__)
	# define LOG_DEBUG_MULTILINE(...) LogDebugMultiline(__VA_ARGS__)
	# define LOG_DEBUG_RAW(...) LogDebugRaw(__VA_ARGS__)
#else
	// Define empty macros (casting variables passed as args to void to prevent unused variable warning from -Werror in Makefile)
	# define LOG_DEBUG(...) do { APPLY_TO_EACH(CAST_TO_VOID, __VA_ARGS__); } while (false)
	# define LOG_DEBUG_MULTILINE(...) do { APPLY_TO_EACH(CAST_TO_VOID, __VA_ARGS__); } while (false)
	# define LOG_DEBUG_RAW(...) do { APPLY_TO_EACH(CAST_TO_VOID, __VA_ARGS__); } while (false)
#endif

// Macro for printing key info for tracing program execution in a production environment
#define LOG_INFO(...) LogInfo(__VA_ARGS__);

// Macro for printing a warning message indicating a potential issue in the system
#define LOG_WARNING(...) LogWarning(__VA_ARGS__);

// Macro for printing an error message indicating an error has occurred
#define LOG_ERROR(...) LogError(__VA_ARGS__);
