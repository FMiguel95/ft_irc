# mandatory
NAME = ircserv

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

SRC_DIR = mandatory/srcs/
SRC = $(addprefix $(SRC_DIR), $(SRC_LIST))

BIN_DIR = mandatory/bin/
BIN = $(addprefix $(BIN_DIR), $(SRC_LIST:.cpp=.o))

# bonus
NAME_BONUS = ircserv_bonus

SRC_LIST_BONUS = main.cpp \
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

SRC_DIR_BONUS = bonus/srcs/
SRC_BONUS = $(addprefix $(SRC_DIR_BONUS), $(SRC_LIST_BONUS))

BIN_DIR_BONUS = bonus/bin/
BIN_BONUS = $(addprefix $(BIN_DIR_BONUS), $(SRC_LIST_BONUS:.cpp=.o))

# bot
BOT = dealer

SRC_LIST_BOT = main.cpp \
			Bot.cpp \
			GameInstance.cpp \
			Deck.cpp

SRC_DIR_BOT = bonus/bot/srcs/
SRC_BOT = $(addprefix $(SRC_DIR_BOT), $(SRC_LIST_BOT))

BIN_DIR_BOT = bonus/bot/bin/
BIN_BOT = $(addprefix $(BIN_DIR_BOT), $(SRC_LIST_BOT:.cpp=.o))

CXX = c++
CXXFLAGS = -g -std=c++98 -Wall -Werror -Wextra 
RM = rm -rf

.PHONY: all bonus clean fclean re

all: $(NAME)

$(NAME): $(BIN)
	$(CXX) $(CXXFLAGS) $(BIN) -o $(NAME)

$(BIN_DIR)%.o: $(SRC_DIR)%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

bonus: $(NAME_BONUS) $(BOT)

$(NAME_BONUS): $(BIN_BONUS)
	$(CXX) $(CXXFLAGS) $(BIN_BONUS) -o $(NAME_BONUS)

$(BIN_DIR_BONUS)%.o: $(SRC_DIR_BONUS)%.cpp | $(BIN_DIR_BONUS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR_BONUS):
	mkdir -p $(BIN_DIR_BONUS)

$(BOT): $(BIN_BOT)
	$(CXX) $(CXXFLAGS) $(BIN_BOT) -o $(BOT)

$(BIN_DIR_BOT)%.o: $(SRC_DIR_BOT)%.cpp | $(BIN_DIR_BOT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR_BOT):
	mkdir -p $(BIN_DIR_BOT)

clean:
	$(RM) $(BIN_DIR) $(BIN_DIR_BONUS) $(BIN_DIR_BOT)

fclean: clean
	$(RM) $(NAME) $(NAME_BONUS) $(BOT)

re: fclean all
