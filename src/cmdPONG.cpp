#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.3
// Command: PONG
// Parameters: <server1> [ <server2> ]

void Server::cmdPONG(const int& socket, const t_message* message)
{
    Client& client = clients.at(socket);

    // validate if the user is registered
    if (!client.isRegistered)
        return;

    // validate if it has arguments
    if (message->arguments[0].empty())
    {
        // reply ERR_NOORIGIN
        sendMessage(socket, std::string(":") + SERVER_NAME + " " + ERR_NOORIGIN + " " + client.nick + " PONG :No origin specified\r\n");
        return;
    }

    // reply PONG
    sendMessage(socket, std::string(":") + SERVER_NAME + " PONG " + SERVER_NAME + " :" + message->arguments[0] + "\r\n");
}