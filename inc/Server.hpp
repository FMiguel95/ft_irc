#pragma once

#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <sstream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

#define RPL_WELCOME				"001"
#define RPL_YOURHOST			"002"
#define RPL_CREATED				"003"
#define RPL_MYINFO				"004"
#define RPL_UMODEIS				"221"
#define RPL_ENDOFWHO			"315"
#define RPL_LISTSTART			"321"
#define RPL_LIST				"322"
#define RPL_LISTEND				"323"
#define RPL_CHANNELMODEIS		"324"
#define RPL_NOTOPIC				"331"
#define RPL_TOPIC				"332"
#define RPL_INVITING			"341"
#define RPL_WHOREPLY			"352"
#define RPL_NAMREPLY			"353"
#define RPL_ENDOFNAMES			"366"

#define ERR_NOSUCHNICK			"401"
#define ERR_NOSUCHCHANNEL		"403"
#define ERR_CANNOTSENDTOCHAN	"404"
#define ERR_NORECIPIENT			"411"
#define ERR_NOTEXTTOSEND		"412"
#define ERR_UNKNOWNCOMMAND		"421"
#define ERR_NONICKNAMEGIVEN		"431"
#define ERR_ERRONEUSNICKNAME	"432"
#define ERR_NICKNAMEINUSE		"433"
#define ERR_NOTONCHANNEL		"442"
#define ERR_USERONCHANNEL		"443"
#define ERR_NEEDMOREPARAMS		"461"
#define ERR_ALREADYREGISTRED	"462"
#define ERR_PASSWDMISMATCH		"464"
#define ERR_CHANOPRIVSNEEDED	"482"
#define ERR_UMODEUNKNOWNFLAG	"501"

// https://datatracker.ietf.org/doc/html/rfc2812#section-2.3
// mensagens em irc são compostas por prefixo(opcional), comando, e argumentos (até 15)
// todos separados por um space character
typedef struct {
	std::string prefix;
	std::string command;
	std::string arguments[15];
} t_message;

class Server
{
public:
	Server();
	Server(const int& serverPort, const std::string& serverPassword);
	Server(const Server& src);
	~Server();
	Server& operator =(const Server& src);

	int run(); // will be called in main

private:
	int serverPort;
	std::string serverPassword;
	std::map<int,Client> clients;	// socket/client pair
	std::list<Channel> channels;	// list of channels
	t_message message;

	void sendMessage(const int& socket, const std::string& message);

	// recebe uma mensagem de um client e adiciona ao buffer - adicionar à lista de clients se ainda não existir?
	// se detetar \r\n, construir um struct t_message e remover do buffer ao estilo gnl (sorry anna)
	void receiveMessage(const int& socket, std::string& stream);

	// faz parse à string
	// retira o prefix, command e args e passa para um struct
	// retorna nulo se for invalido
	t_message* parseMessage(std::string& stream);

	// decide o que fazer com a mensagem resultante
	void handleMessage(const int& socket, t_message* message);

	Channel* getChannelByName(const std::string& name) const;

	Client* getClientByNick(const std::string& nick) const;

	// closes connection to a specific socket
	// sends appropriate message and removes client from the list, etc..
	// usar quando o client se desconectar, der timeout, não reponder a ping......
	void closeConnection(const int& socket);

	// iterates through the list of clients and pings or disconnects ones that have been inactive too long
	void checkTimeouts();

	bool isChannelNameValid(const std::string& name) const;

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.1
	// Command: PASS
	// Parameters: <password>
	void cmdPASS(const int& socket, const t_message* message);
	
	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.2
	// Command: NICK
	// Parameters: <nickname>
	void cmdNICK(const int& socket, const t_message* message);
	
	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.3
	// Command: USER
	// Parameters: <user> <mode> <unused> <realname>
	void cmdUSER(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.1
	// Command: JOIN
	// Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"
	void cmdJOIN(const int& socket, const t_message* message);
	void attempJoin(Client& client, const std::string& channelName, const std::string& channelKey);
	void addClientToChannel(Client& client, Channel& channel);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.3
	// Command: MODE
	// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )
	void cmdMODE(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.4
	// Command: TOPIC
	// Parameters: <channel> [ <topic> ]
	void cmdTOPIC(const int& socket, const t_message* message);

 	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.8
	// Command: KICK
	// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
	void cmdKICK(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.7
	// Command: INVITE
	// Parameters: Parameters: <nickname> <channel>
	void cmdINVITE(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
	// Command: PRIVMSG
	// Parameters: <msgtarget> <text to be sent>
	void cmdPRIVMSG(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.6
	// Command: LIST
	// Parameters: [ <channel> *( "," <channel> ) [ <target> ] ]
	void cmdLIST(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.1
	// Command: WHO
	// Parameters: [ <mask> [ "o" ] ]
	void cmdWHO(const int& socket, const t_message* message);

	// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.2
	// Command: WHOIS
	// Parameters: [ <target> ] <mask> *( "," <mask> )
	void cmdWHOIS(const int& socket, const t_message* message);

};
