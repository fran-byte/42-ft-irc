NAME := ircserv
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g
INC := -Iinclude

SRC := \
	src/main.cpp \
	src/core/Server.cpp \
	src/core/Intro.cpp \
	src/handlers/Auth.cpp \
	src/handlers/Misc.cpp \
	src/handlers/ChannelJoin.cpp \
	src/handlers/Message.cpp \
	src/handlers/Mode.cpp \
	src/handlers/Topic.cpp \
	src/handlers/InviteKick.cpp

OBJ := $(SRC:.cpp=.o)

# Build executable
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

all: $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Valgrind RULE
val: $(NAME)
	valgrind --leak-check=full \
	         --show-leak-kinds=definite,indirect,possible \
	         --track-origins=yes \
	         --track-fds=yes \
	         --trace-children=yes \
	         --errors-for-leak-kinds=definite,indirect \
	         --error-exitcode=1 \
	         ./ircserv 6667 mypass

.PHONY: all clean fclean re val
