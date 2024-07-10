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

std::map<Client*,char>::const_iterator Channel::getClientInChannel(const std::string& nick) const
{
	for (std::map<Client*,char>::const_iterator it = userList.begin(); it != userList.end(); ++it)
	{
		if (it->first->nick == nick)
			return it;
	}
	return userList.end();
}
