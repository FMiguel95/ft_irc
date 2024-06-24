#include "../inc/Channel.hpp"

Channel::Channel() {}

Channel::Channel(const Channel& src)
{
	(void) src;
}

Channel::~Channel() {}

Channel& Channel::operator =(const Channel& src)
{
	if (this != &src)
	{

	}
	return *this;
}
