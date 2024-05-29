#pragma once

#include <string>

class Client
{
public:
	Client();
	Client(const Client& src);
	~Client();
	Client& operator =(const Client& src);

private:
	std::string _nick;
	std::string _user;
	std::string _hostname;
	std::string _servername;
	std::string _realname;
	
	std::string _messageBuffer;

};
