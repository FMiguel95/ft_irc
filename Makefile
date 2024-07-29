NAME = ircserv

NAME_BONUS = ircserv_bonus

BOT = dealer

SRC_LIST = main.cpp \
			Server.cpp \
			Client.cpp \
			Channel.cpp \
			cmdPASS.cpp \
			cmdNICK.cpp \
			cmdUSER.cpp \
			cmdMOTD.cpp \
			cmdLIST.cpp \
			cmdJOIN.cpp \
			cmdPART.cpp \
			cmdPRIVMSG.cpp \
			cmdNOTICE.cpp \
			cmdTOPIC.cpp \
			cmdWHO.cpp \
			cmdWHOIS.cpp \
			cmdMODE.cpp \
			cmdKICK.cpp \
			cmdINVITE.cpp \
			cmdPING.cpp \
			cmdPONG.cpp \
			cmdQUIT.cpp

SRC_DIR = src/
SRC = $(addprefix $(SRC_DIR), $(SRC_LIST))

BIN_DIR = bin/
BIN = $(addprefix $(BIN_DIR), $(SRC_LIST:.cpp=.o))

CXX = c++
CXXFLAGS = -g -std=c++98# -Wall -Werror -Wextra 
RM = rm -rf

.PHONY: all bonus clean fclean re

all: $(NAME)

$(NAME): $(BIN)
	$(CXX) $(CXXFLAGS) $(BIN) -o $(NAME)

$(BIN_DIR)%.o: $(SRC_DIR)%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir $(BIN_DIR)

bonus: $(NAME_BONUS) $(BOT)

$(NAME_BONUS): $(BIN_BONUS)%.cpp | $(BIN_DIR_BONUS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BOT): $(BOT_BIN)%.cpp | $(BOT_BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(BIN_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all
