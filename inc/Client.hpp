#pragma once

#include <string>
#include <ctime>

class Client
{
public:
	std::string _messageBuffer;

	bool isRegistered;
	bool passOk;
	bool nickOk;
	bool userOk;

	Client();
	Client(const Client& src);
	~Client();
	Client& operator =(const Client& src);

	// returns the difference in seconds
	double getTimeSinceLastActivity() const;
	void updateActivityTime();

	std::string getUserInfo(char choice) const;

private:
	std::string _nick;
	std::string _user;
	std::string _hostname;
	std::string _realname;
	// std::string _servername;
	

	std::time_t _lastActivityTime;

};
