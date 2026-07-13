.SILENT:

NAME =	ircserv
SRCS =	src/main.cpp \
		src/Channel/Channel.cpp \
		src/Channel/Channel_members.cpp \
		src/Channel/Channel_ChanOps.cpp \
		src/Channel/Channel_get_set.cpp \
		src/Client/Client.cpp \
		src/Client/Client_get_set.cpp \
		src/Message/Message_extract.cpp \
		src/Message/Message_get_set.cpp \
		src/Message/Message_parser.cpp \
		src/Message/Message.cpp \
		src/Server/Server_action.cpp \
		src/Server/Server_error.cpp \
		src/Server/Server_exec.cpp \
		src/Server/Server_exec_invite.cpp \
		src/Server/Server_exec_mode.cpp \
		src/Server/Server_get_set.cpp \
		src/Server/Server_utilities.cpp \
		src/Server/Server.cpp \
		src/Signals/Signals_handling.cpp \
		src/utils/Utils.cpp

COLOUR_GREEN = $(shell tput setaf 2)
COLOUR_END = $(shell tput sgr0)

OBJ_DIR = obj
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

CXX = c++
CXXFLAGS = -Wall  -Wextra -g -std=c++98
INCLUDES = -I./includes

CXXFLAGS += $(DEBUG_FLAGS)

all:
	@if $(MAKE) --no-print-directory -q $(NAME); then \
		echo "$(COLOUR_GREEN)$(NAME) is already up to date$(COLOUR_END)"; \
	else \
		$(MAKE) --no-print-directory $(NAME); \
	fi

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) -o $(NAME)
	@echo "$(COLOUR_GREEN)compilation done$(COLOUR_END)"

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
clean:
	@/usr/bin/rm -rf $(OBJ_DIR)
	@echo "$(COLOUR_GREEN)clean obj files done$(COLOUR_END)"

fclean: clean
	@/usr/bin/rm -rf $(NAME)
	@echo "$(COLOUR_GREEN)all clean complete$(COLOUR_END)" 

re: fclean all

.PHONY: all clean fclean re