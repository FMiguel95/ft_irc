#pragma once

#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include "Client.hpp"

enum ChannelMode
{
	MODE_i = 0b00000001, // needs invite to join
	MODE_t = 0b00000010, // channel privileges are required to set the topic
	MODE_k = 0b00000100, // needs key to join
	MODE_l = 0b00001000  // user limit is set
};

enum UserMode
{
	MODE_o = 0b00000001
};

class Channel
{
public:
	std::string channelName;
	std::string channelKey;
	std::string topic;
	char channelMode; // each bit corresponds to a flag as set in enum ChannelMode
	std::map<Client*,char> userList; // key: pointer to the client, value: user mode flags
	std::list<Client*> invitedUsers;
	int userLimit;

	Channel(const std::string& name, const std::string& key);
	Channel(const Channel& src);
	~Channel();
	Channel& operator =(const Channel& src);

	std::map<Client*,char>::const_iterator getClientInChannel(const std::string& nick) const;
	void broadcastMessage(); // send message to everyone in the channel

private:
	Channel();
};
