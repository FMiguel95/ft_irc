#pragma once

#include <string>
#include <ctime>

class Client
{
public:
	std::string _messageBuffer;
	bool isRegistered;

	Client();
	Client(const Client& src);
	~Client();
	Client& operator =(const Client& src);

	// returns the difference in seconds
	double getTimeSinceLastActivity() const;
	void updateActivityTime();

private:
	std::string _nick;
	std::string _user;
	std::string _hostname;
	//std::string _servername;
	std::string _realname;
	

	std::time_t lastActivityTime;

};
