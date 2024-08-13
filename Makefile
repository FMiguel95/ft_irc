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

SRC_DIR = srcs/
SRC = $(addprefix $(SRC_DIR), $(SRC_LIST))

BIN_DIR = bin/
BIN = $(addprefix $(BIN_DIR), $(SRC_LIST:.cpp=.o))


BOT = the_bot

SRC_LIST_BOT = main.cpp \
			Bot.cpp

SRC_DIR_BOT = bot/srcs/
SRC_BOT = $(addprefix $(SRC_DIR_BOT), $(SRC_LIST_BOT))

BIN_DIR_BOT = bot/bin/
BIN_BOT = $(addprefix $(BIN_DIR_BOT), $(SRC_LIST_BOT:.cpp=.o))

CXX = c++
CXXFLAGS = -g -std=c++98 -Wall -Werror -Wextra 
RM = rm -rf

.PHONY: all bot bonus clean fclean re re_bonus

all: $(NAME)

$(NAME): $(BIN)
	$(CXX) $(CXXFLAGS) $(BIN) -o $(NAME)

$(BIN_DIR)%.o: $(SRC_DIR)%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

bot: $(BOT)

$(BOT): $(BIN_BOT)
	$(CXX) $(CXXFLAGS) $(BIN_BOT) -o $(BOT)

$(BIN_DIR_BOT)%.o: $(SRC_DIR_BOT)%.cpp | $(BIN_DIR_BOT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR_BOT):
	mkdir -p $(BIN_DIR_BOT)

bonus: all bot

clean:
	$(RM) $(BIN_DIR) $(BIN_DIR_BOT)

fclean: clean
	$(RM) $(NAME) $(BOT)

re: fclean all

re_bonus: fclean bonus
