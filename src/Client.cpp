#include "../inc/Client.hpp"

Client::Client() :
isRegistered(false),
passOk(false),
nickOk(false),
userOk(false)
{}

Client::Client(const Client& src) :
isRegistered(src.isRegistered),
passOk(src.passOk),
nickOk(src.nickOk),
userOk(src.userOk),
_nick(src._nick),
_user(src._user),
_lastActivityTime(src._lastActivityTime)
{}

Client::~Client() {}

Client& Client::operator =(const Client& src)
{
	if (this != &src)
	{
		isRegistered = src.isRegistered;
	}
	return *this;
}

double Client::getTimeSinceLastActivity() const
{
	std::time_t currentTime = std::time(NULL);
	double difference = std::difftime(_lastActivityTime, currentTime);
	return difference;
}

void Client::updateActivityTime()
{
	_lastActivityTime = std::time(NULL);
}
