#pragma once

#include <string>

class Client
{
public:
	Client(int clientSocket);
private:
	std::string _nick;
	std::string _user;
	std::string _hostname;
	std::string _servername;
	std::string _realname;
	
	std::string _messageBuffer;

	Client();
};
