# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/08 17:44:55 by ixu               #+#    #+#              #
#    Updated: 2024/07/01 19:08:02 by vshchuki         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Program name
NAME := webserv

# Directory for object files
OBJS_DIR := objs/

# Source files
SRCS = $(addsuffix .cpp, main Socket Server Request Utility ServersManager Config ConfigValidator CGIHandler)

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

# COMPILER = c++
# FLAGS = -Wall -Wextra -Werror -std=c++17 -g

# NAME = webserv

# SRCS = main.cpp \
# 		Server.cpp \
# 		ServersManager.cpp \
# 		Request.cpp \
# 		Config.cpp \
# 		Utility.cpp

# OBJ_DIR = objs/
# OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.cpp=.o))

# all: $(NAME)

# $(NAME): $(OBJ_DIR) $(OBJS)
# 	$(COMPILER) $(FLAGS) $(OBJS) -o $(NAME)

# $(OBJ_DIR):
# 	mkdir -p $(OBJ_DIR)

# $(OBJ_DIR)%.o: %.cpp %.hpp | $(OBJ_DIR)
# 	$(COMPILER) $(FLAGS) -c $< -o $@

# $(OBJ_DIR)%.o: %.cpp | $(OBJ_DIR)
# 	$(COMPILER) $(FLAGS) -c $< -o $@

# clean:
# 	rm -rf $(OBJ_DIR)

# fclean: clean
# 	rm -f $(NAME)

# re: fclean all

# .PHONY: all clean fclean re

