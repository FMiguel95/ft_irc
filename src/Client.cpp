#include "../inc/Client.hpp"

Client::Client() : isRegistered(false) {}

Client::Client(const Client& src) : isRegistered(src.isRegistered) {}

Client::~Client()
{

}

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
	double difference = std::difftime(lastActivityTime, currentTime);
	return difference;
}

void Client::updateActivityTime()
{
	lastActivityTime = std::time(NULL);
}
