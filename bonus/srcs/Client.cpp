#include "../incs/Client.hpp"

// Constructors, Destructor and Assignment operator -----------------------------------------------

Client::Client() :
		socket(0),
		isRegistered(false),
		passOk(false),
		nickOk(false),
		userOk(false),
		pendingPong(false),
		_lastActivityTime(std::time(NULL)),
		_lastPingTime(std::time(NULL)) {}

Client::Client(const int& socket) :
		socket(socket),
		isRegistered(false),
		passOk(false),
		nickOk(false),
		userOk(false),
		pendingPong(false),
		_lastActivityTime(std::time(NULL)),
		_lastPingTime(std::time(NULL)) {}

Client::Client(const Client& src) :
		socket(src.socket),
		nick(src.nick),
		user(src.user),
		hostname(src.hostname),
		userAtHost(src.userAtHost),
		realname(src.realname),
		isRegistered(src.isRegistered),
		passOk(src.passOk),
		nickOk(src.nickOk),
		userOk(src.userOk),
		pendingPong(src.pendingPong),
		_lastActivityTime(src._lastActivityTime),
		_lastPingTime(src._lastPingTime) {}

Client::~Client() {}

Client& Client::operator =(const Client& src)
{
	if (this != &src)
	{
		socket = src.socket;
		isRegistered = src.isRegistered;
		passOk = src.passOk;
		nickOk = src.nickOk;
		userOk = src.userOk;
		nick = src.nick;
		user = src.user;
		pendingPong = src.pendingPong;
		userAtHost = src.userAtHost;
		realname = src.realname;
		hostname = src.hostname;
		_lastActivityTime = src._lastActivityTime;
		_lastPingTime = src._lastPingTime;
	}
	return *this;
}

// Member functions -------------------------------------------------------------------------------

double Client::getTimeSinceLastActivity() const
{
	std::time_t currentTime = std::time(NULL);
	double difference = std::difftime(currentTime, _lastActivityTime);
	return difference;
}

double Client::getTimeSinceLastPing() const
{
	std::time_t currentTime = std::time(NULL);
	double difference = std::difftime(currentTime, _lastPingTime);
	return difference;
}

void Client::updateActivityTime()
{
	_lastActivityTime = std::time(NULL);
}

void Client::updatePingTime()
{
	_lastPingTime = std::time(NULL);
}
