#include "../inc/Channel.hpp"

Channel::Channel() :
channelName(""),
channelKey(""),
topic(""),
channelMode(0),
userLimit(9999)
{}

Channel::Channel(const std::string& name, const std::string& key) :
channelName(name),
channelKey(key),
topic(""),
channelMode(0),
userLimit(9999)
{}

Channel::Channel(const Channel& src) :
channelName(src.channelName),
channelKey(src.channelKey),
topic(src.topic),
channelMode(src.channelMode),
userList(src.userList),
userLimit(src.userLimit)
{}

Channel::~Channel() {}

Channel& Channel::operator =(const Channel& src)
{
	if (this != &src)
	{
		channelName = src.channelName;
		channelKey = src.channelKey;
		topic = src.topic;
		channelMode = src.channelMode;
		userList = src.userList;
		userLimit = src.userLimit;
	}
	return *this;
}

std::map<Client*,char>::iterator Channel::getClientInChannel(const std::string& nick)
{
	for (std::map<Client*,char>::iterator it = userList.begin(); it != userList.end(); ++it)
	{
		if (it->first->nick == nick && it->first->isRegistered)
			return it;
	}
	return userList.end();
}
