NAME := ircserv
CXX  := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
INC := -Iinclude

SRC := \
src/main.cpp \
src/core/Server.cpp \
src/core/Client.cpp \
src/core/Channel.cpp \
src/handlers/Auth.cpp \
src/handlers/Misc.cpp \
src/handlers/ChannelJoin.cpp \
src/handlers/Message.cpp \
src/handlers/Mode.cpp \
src/handlers/Topic.cpp \
src/handlers/InviteKick.cpp

$(NAME): $(SRC)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $(SRC)

all: $(NAME)
clean:
	@true
fclean: clean
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
