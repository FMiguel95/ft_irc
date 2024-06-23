#include <iostream>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include <csignal>

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;
const int PORT = 6667;
bool run = true;

void	handle_sigint(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << std::endl;
		run = false;
	}
}

// modern.ircdocs.horse
// datatracker.ietf.org/doc/html/rfc2812
// ircv3.net


int main(int ac, char** av)
{
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addrLen = sizeof(struct sockaddr_in);
	char buffer[BUFFER_SIZE];

	// Create socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Error creating socket\n";
		return 1;
	}

	// Set socket options to allow reuse of the address
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error setting socket options\n";
		return 1;
	}


	// Set up server address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(PORT);

	// Bind socket
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cerr << "Error binding socket\n";
		return 1;
	}

	// Listen for incoming connections
	if (listen(serverSocket, MAX_CLIENTS) < 0)
	{
		std::cerr << "Error listening on socket\n";
		return 1;
	}

	std::cout << "Server listening on port " << PORT << std::endl;

	struct pollfd fds[MAX_CLIENTS + 1];
	std::memset(fds, 0, sizeof(fds));

	fds[0].fd = serverSocket;
	fds[0].events = POLLIN;

	int numClients = 0;

	std::signal(SIGINT, handle_sigint);
	while (run)
	{
		// Wait for activity on any of the sockets
		int activity = poll(fds, numClients + 1, -1);
		if (!run)
			break;
		if (activity < 0)
		{
			std::cerr << "Poll error\n";
			break;
		}

		// If there's a new connection request
		if (fds[0].revents & POLLIN)
		{
			// Accept the connection
			clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
			if (clientSocket < 0)
			{
				std::cerr << "Error accepting connection\n";
				continue;
			}

			// Add the new client to the array of sockets
			numClients++;
			fds[numClients].fd = clientSocket;
			fds[numClients].events = POLLIN;

			std::cout << "New connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
		}

		// Check for messages from clients
		for (int i = 1; i <= numClients; i++)
		{
			if (fds[i].revents & POLLIN)
			{
				// Read the message
				int bytesRead = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
				if (bytesRead <= 0)
				{
					// Connection closed or error
					close(fds[i].fd);
					// Remove from the poll list
					fds[i] = fds[numClients];
					numClients--;
				}
				else
				{
					// Process the message
					buffer[bytesRead] = '\0'; // Null-terminate the string
					std::string clientMessage = buffer;
					std::cout << "Received message from client " << i << ">\n" << clientMessage << std::endl;
					

					std::string reply;
					if (clientMessage == "CAP LS 302\r\n")
						reply = ":verycoolserver CAP * LS :\r\n";
					else
						reply = "Hi!\r\n";
					send(fds[i].fd, reply.c_str(), reply.length(), 0);
					std::cout << "Replied with >\n" << reply << std::endl;
				}
			}
		}
	}

	// Close server socket
	if (close(serverSocket) < 0)
	{
		std::cerr << "Close error" << std::endl;
	}
	std::cout << "quitting" << std::endl;
	return 0;
}
