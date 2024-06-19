/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 11:05:45 by ixu               #+#    #+#             */
/*   Updated: 2024/06/19 12:06:53 by ixu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEBUG_HPP
# define DEBUG_HPP

# include <iostream>

# ifdef DEBUG_MODE
    // variadic macro for debugging messages
    # define DEBUG(...) \
        do { \
            std::cout << "[DEBUG] "; \
            std::cout << __VA_ARGS__; \
            std::cout << std::endl; \
        } while (0)
# else
    // define empty macro
    # define DEBUG(...)
# endif

#endif
