# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile_debug                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ixu <ixu@student.hive.fi>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/08 17:44:55 by ixu               #+#    #+#              #
#    Updated: 2024/06/19 14:10:54 by ixu              ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Program name
NAME := webserv

# Directory for object files
OBJS_DIR := objs/

# Source files
SRCS = $(addsuffix .cpp, main Socket)

# Object files derived from sources
OBJS = $(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))

# Compiler and flags
COMPILER := c++
FLAGS := -Wall -Wextra -Werror -Wshadow -std=c++11
DEBUG_FLAGS := -DDEBUG_MODE

# Color scheme for terminal output
BRIGHT_YELLOW := \033[0;93m
YELLOW := \033[0;33m
COLOR_RESET := \033[0m

# Default target
all: $(NAME)

$(NAME): $(OBJS_DIR) $(OBJS)
	@$(COMPILER) -o $(NAME) $(OBJS)
	@echo "$(BRIGHT_YELLOW)Built $(NAME)$(COLOR_RESET)"

# Debug target
debug: FLAGS += $(DEBUG_FLAGS)
debug: .debug

.debug: $(OBJS_DIR) $(OBJS)
	@$(COMPILER) -o $(NAME) $(OBJS)
	@echo "$(BRIGHT_YELLOW)Built $(NAME) (DEBUG_MODE)$(COLOR_RESET)"
	@touch .debug

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	@echo "$(YELLOW)Built object directory$(COLOR_RESET)"

$(OBJS_DIR)%.o: %.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

.flags: flags
	@echo 'FLAGS: $(FLAGS)' > .flags_current
	@cmp -s .flags_current $@ || mv -f .flags_current $@

clean:
	@rm -fr $(OBJS_DIR) .debug .flags .flags_current
	@echo "$(YELLOW)Removed object files$(COLOR_RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(YELLOW)Removed executable(s)$(COLOR_RESET)"

re: fclean all

.PHONY: flags
