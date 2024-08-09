#pragma once

#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include "Client.hpp"

enum ChannelMode
{
	MODE_i = 0b00000001, // makes the channel invite-only
	MODE_t = 0b00000010, // only ops can change the topic
	MODE_k = 0b00000100, // requires a key to join the channel
	MODE_l = 0b00001000  // sets a user limit
};

enum UserMode
{
	MODE_o = 0b00000001
};

class Channel
{
	public:
		std::string				channelName;
		std::string				channelKey;
		std::string				topic;
		int						userLimit;
		char					channelMode;
		
		std::map<Client*,char>	userList; // key: pointer to the client, value: user mode flags
		std::list<Client*>		invitedUsers;

		Channel(const std::string& name, const std::string& key);
		Channel(const Channel& src);
		~Channel();
		
		Channel& operator =(const Channel& src);

		std::map<Client*,char>::iterator getClientInChannel(const std::string& nick);
	
	private:
		Channel();
};
