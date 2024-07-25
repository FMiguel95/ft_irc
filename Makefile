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

SRC_DIR = src/
SRC = $(addprefix $(SRC_DIR), $(SRC_LIST))

BIN_DIR = bin/
BIN = $(addprefix $(BIN_DIR), $(SRC_LIST:.cpp=.o))

CXX = c++
CXXFLAGS = -g -std=c++98# -Wall -Werror -Wextra 
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
