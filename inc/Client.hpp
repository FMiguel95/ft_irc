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
	std::string userAtHost;
	std::string realname;

	bool isRegistered;
	bool passOk;
	bool nickOk;
	bool userOk;

	bool pendingPong;

	Client(const int& socket);
	Client(const Client& src);
	~Client();
	Client& operator =(const Client& src);

	// returns the difference in seconds
	double getTimeSinceLastActivity() const;
	double getTimeSinceLastPing() const;
	void updateActivityTime();
	void updatePingTime();

	std::string getUserInfo(char choice) const;

private:
	Client();
	
	std::time_t _lastActivityTime;
	std::time_t _lastPingTime;

};
