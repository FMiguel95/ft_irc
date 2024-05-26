NAME = ircserv

SRC_LIST =  main.cpp

SRC_DIR = src/
SRC = $(addprefix $(SRC_DIR), $(SRC_LIST))

BIN_DIR = bin/
BIN = $(addprefix $(BIN_DIR), $(SRC_LIST:.cpp=.o))

CXX = c++
CXXFLAGS = #-Wall -Werror -Wextra -std=c++98 -g 
RM = rm -rf

all: $(NAME)

$(NAME): $(BIN) $(LIBFT)
	$(CXX) $(CXXFLAGS) $(BIN) -o $(NAME)

$(BIN_DIR)%.o: $(SRC_DIR)%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	$(RM) $(BIN_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all
