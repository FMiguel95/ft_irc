#include "../inc/Client.hpp"

Client::Client() :
socket(0),
isRegistered(false),
passOk(false),
nickOk(false),
userOk(false)
{}

Client::Client(const int& socket) :
socket(socket),
isRegistered(false),
passOk(false),
nickOk(false),
userOk(false)
{}

Client::Client(const Client& src) :
socket(src.socket),
isRegistered(src.isRegistered),
passOk(src.passOk),
nickOk(src.nickOk),
userOk(src.userOk),
nick(src.nick),
user(src.user),
hostname(src.hostname),
_lastActivityTime(src._lastActivityTime)
{}

Client::~Client() {}

Client& Client::operator =(const Client& src)
{
	if (this != &src)
	{
		socket = src.socket;
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

std::string Client::getUserInfo(char choice) const
{
	std::string info;
	if (choice == 'n')
		info = nick;
	else if (choice == 'u')
		info = user;
	else if (choice == 'h')
		info = hostname;
	else if (choice == 'r')
		info = realname;
	else
		info = "Invalid choice";
	return (info);
}
