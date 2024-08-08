#pragma once

#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

#define SERVER_NAME				"ft_irc"

#define TIMEOUT_TIME			10
#define PING_TIMEOUT_TIME		10

#define RPL_WELCOME				"001"
#define RPL_YOURHOST			"002"
#define RPL_CREATED				"003"
#define RPL_MYINFO				"004"
#define RPL_ISUPPORT			"005"
#define RPL_UMODEIS				"221"
#define RPL_WHOISUSER			"311"
#define RPL_WHOISSERVER			"312"
#define RPL_WHOISOPERATOR		"313"
#define RPL_ENDOFWHO			"315"
#define RPL_WHOISIDLE			"317"
#define RPL_ENDOFWHOIS			"318"
#define RPL_WHOISCHANNELS		"319"
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
#define RPL_MOTD				"372"
#define RPL_MOTDSTART			"375"
#define RPL_ENDOFMOTD			"376"
#define RPL_WHOISHOST			"378"

#define ERR_NOSUCHNICK			"401"
#define ERR_NOSUCHCHANNEL		"403"
#define ERR_CANNOTSENDTOCHAN	"404"
#define ERR_NOORIGIN			"409"
#define ERR_NORECIPIENT			"411"
#define ERR_NOTEXTTOSEND		"412"
#define ERR_UNKNOWNCOMMAND		"421"
#define ERR_NOMOTD				"422"
#define ERR_NONICKNAMEGIVEN		"431"
#define ERR_ERRONEUSNICKNAME	"432"
#define ERR_NICKNAMEINUSE		"433"
#define ERR_USERNOTINCHANNEL	"441"
#define ERR_NOTONCHANNEL		"442"
#define ERR_USERONCHANNEL		"443"
#define ERR_NEEDMOREPARAMS		"461"
#define ERR_ALREADYREGISTRED	"462"
#define ERR_PASSWDMISMATCH		"464"
#define ERR_CHANNELISFULL		"471"
#define ERR_INVITEONLYCHAN		"473"
#define ERR_BADCHANNELKEY		"475"
#define ERR_CHANOPRIVSNEEDED	"482"
#define ERR_UMODEUNKNOWNFLAG	"501"
#define ERR_INVALIDKEY			"525"

// https://datatracker.ietf.org/doc/html/rfc2812#section-2.3
// mensagens em irc são compostas por prefixo(opcional), comando, e argumentos (até 15)
// todos separados por um space character
typedef struct
{
	std::string	raw;
	std::string	prefix;
	std::string	command;
	std::string	arguments[15];
} t_message;

class Server
{
	public:
		static bool	run;

		Server();
		Server(const int& serverPort, const std::string& serverPassword);
		Server(const Server& src);
		~Server();

		Server& operator =(const Server& src);

		int runServer();

	private:
		int						_serverPort;
		std::string				_serverPassword;
		std::string				_serverHostname;
		time_t					_serverCreationTime;
		std::string				_messageOfTheDay;
		bool					_hasMOTD;
		std::map<int,Client>	_clients;
		std::list<Channel>		_channels;
		t_message				_message;

		void		getMOTD();
		void		getHostname();
		Channel*	getChannelByName(const std::string& name);
		Client*		getClientByNick(const std::string& nick);

		void		sendMessage(const int& socket, const std::string& message);
		void		broadcastMessage(Channel& channel, const std::string& message);
		void		receiveMessage(const int& socket, std::string& stream);
		t_message*	parseMessage(std::string& stream);
		void		handleMessage(const int& socket, t_message* message);

		bool		isChannelNameValid(const std::string& name) const;

		void		checkRegistration(Client& client);

		void		unregisterClient(Client& client, const std::string& reason);

		void		checkTimeouts(std::vector<pollfd>& fds);
		
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

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.4.1
		// Command: MOTD
		// Parameters: [ <target> ]
		void cmdMOTD(const int& socket, const t_message* message);
		void sendMOTD(const Client& client);

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

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.2
		// Command: NOTICE
		// Parameters: <msgtarget> <text>
		void cmdNOTICE(const int& socket, const t_message* message);

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

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.2
		// Command: PING
		// Parameters: <server1> [ <server2> ]
		void cmdPING(const int& socket, const t_message* message);

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.3
		// Command: PONG
		// Parameters: <server> [ <server2> ]
		void cmdPONG(const int& socket, const t_message* message);

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.2
		// Command: PART
		// Parameters: <channel> *( "," <channel> ) [ <Part Message> ]
		void cmdPART(const int& socket, const t_message* message);

		// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.7
		// Command: QUIT
		// Parameters: [ <Quit Message> ]
		void cmdQUIT(const int& socket, const t_message* message);
};
