SRCFILES =	main.cpp				\
			utils/Logger.cpp 		utils/ParsingUtils.cpp \
			server/Server.cpp		server/Client.cpp			server/Socket.cpp \
			request/Request.cpp 	request/Response.cpp		request/RequestUtils.cpp \
			config/ParsingFile.cpp	config/FillBlock.cpp 		config/ServerContext.cpp \
			exec/DirectoryListing.cpp 	config/ConfigUtils.cpp		CGI/CGI.cpp		\
			exec/set_content_type.cpp 	exec/redir.cpp	\

NAME = webserver

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -g3
INCLUDES = -Isrcs


SRC_DIR = srcs
OBJ_DIR = objs

SRC = $(addprefix $(SRC_DIR)/, $(SRCFILES))
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
ERASE = \033[1A\033[K
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(YELLOW)Linking $(NAME)$(RESET)"
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "$(GREEN)Linking $(NAME) Done !$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(YELLOW)Compiling $<$(RESET)"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo "$(GREEN)Compiling $< Done !$(RESET)"

clean:
	@echo "$(RED)Cleaning $(OBJ_DIR)$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(ERASE)$(GREEN)Cleaning $(OBJ_DIR) Done !$(RESET)"

fclean: clean
	@echo "$(RED)Removing $(NAME)$(RESET)"
	@rm -f $(NAME)
	@echo "$(ERASE)$(GREEN)Removing $(NAME) Done !$(RESET)"

re: fclean all

.PHONY: clean fclean re
