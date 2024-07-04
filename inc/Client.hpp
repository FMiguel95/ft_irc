#pragma once

#include <string>
#include <ctime>

class Client
{
public:
	int socket;
	std::string messageBuffer;
	std::string nick;
	std::string user;
	std::string hostname;
	std::string realname;

	bool isRegistered;
	bool passOk;
	bool nickOk;
	bool userOk;

	Client(const int& socket);
	Client(const Client& src);
	~Client();
	Client& operator =(const Client& src);

	// returns the difference in seconds
	double getTimeSinceLastActivity() const;
	void updateActivityTime();

	std::string getUserInfo(char choice) const;

private:
	Client();
	// std::string _servername;
	

	std::time_t _lastActivityTime;

};
