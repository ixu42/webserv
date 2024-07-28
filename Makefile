# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dnikifor <dnikifor@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/08 17:44:55 by ixu               #+#    #+#              #
#    Updated: 2024/07/28 20:35:46 by dnikifor         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Program name
NAME := webserv

# Directories
OBJS_DIR := ./objs/
SRCS_DIR := ./srcs/
CONFIG_DIR := $(SRCS_DIR)config/
NETWORK_DIR := $(SRCS_DIR)network/
REQUEST_DIR := $(SRCS_DIR)request/
RESPONSE_DIR := $(SRCS_DIR)response/
UTILS_DIR := $(SRCS_DIR)utils/

# Source files
SRCS = $(addsuffix .cpp, main DirLister Uploader Socket Server Client ServerException \
			ServersManager Request Response Utility Config ConfigValidator CGIHandler \
			SessionsManager UrlEncoder Signals)

# Object files
OBJS = $(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))

# Compiler and flags
COMPILER := c++
FLAGS := -Wall -Wextra -Werror -Wshadow -std=c++17 -g
DEBUG_FLAGS := -DDEBUG_MODE

# Color scheme for terminal output
BRIGHT_YELLOW := \033[0;93m
RED := \033[31m
YELLOW := \033[0;33m
GREEN := \033[32m
COLOR_RESET := \033[0m

# Default target
all: $(NAME)

$(NAME): $(OBJS_DIR) $(OBJS)
	@$(COMPILER) -o $(NAME) $(OBJS)
	@echo "$(GREEN)Built $(NAME)$(COLOR_RESET)"

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

$(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

$(OBJS_DIR)%.o: $(CONFIG_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

$(OBJS_DIR)%.o: $(NETWORK_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

$(OBJS_DIR)%.o: $(REQUEST_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

$(OBJS_DIR)%.o: $(RESPONSE_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

$(OBJS_DIR)%.o: $(UTILS_DIR)%.cpp .flags
	@$(COMPILER) $(FLAGS) -c $< -o $@
	@echo "$(YELLOW)Built $@$(COLOR_RESET)"

.flags: flags
	@echo 'FLAGS: $(FLAGS)' > .flags_current
	@cmp -s .flags_current $@ || mv -f .flags_current $@

clean:
	@rm -fr $(OBJS_DIR) .debug .flags .flags_current
	@echo "$(RED)Removed object files$(COLOR_RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)Removed executable(s)$(COLOR_RESET)"

re: fclean all

.PHONY: flags