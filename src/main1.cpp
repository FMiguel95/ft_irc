#include <iostream>
#include <vector>
#include <csignal>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT		6667
#define BUFFER_SIZE	1024

bool run = true;

int error_exit(const char *message)
{
	std::cerr << message << '\n';
	return 1;
}

void handle_sigint(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << std::endl;
		run = false;
	}
}

int main(void)
{
	int				serverSocket, clientSocket, opt;
	sockaddr_in		serverAddress, clientAddress;
	socklen_t		clientAddressSize = sizeof(clientAddress);
	
	char			buffer[BUFFER_SIZE];

	// CREATE A SOCKET ---------------------------------------------------------
	
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return (error_exit("Error creating socket"));
	opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return (error_exit("Error setting socket options"));

	// BIND SOCKET TO PORT -----------------------------------------------------

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(PORT);

	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		return (error_exit("Error binding socket to port"));

	// LISTEN FOR CONNECTIONS --------------------------------------------------

	if (listen(serverSocket, SOMAXCONN) == -1)
		return (error_exit("Error listening for connections"));
	std::cout << "Server listening on port " << PORT << '\n';

	// CLIENT CONNECTION -------------------------------------------------------
	
	std::vector<pollfd>	fds;
	std::vector<int>	clients;
	
	fds.push_back({serverSocket, POLLIN, 0});
	std::signal(SIGINT, handle_sigint);
	while (run)
	{
		int pollResult = poll(fds.data(), fds.size(), -1);
		if (!run || pollResult == -1)
		{
			if (pollResult == -1)
				error_exit("Error polling sockets");
			break;
		}
		for (int i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == serverSocket)
				{
					clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
					if (clientSocket == -1)
					{
						error_exit("Error accepting connection");
						continue;
					}
					else
					{
						fds.push_back({clientSocket, POLLIN | POLLHUP | POLLERR, 0});
						clients.push_back(clientSocket);
						std::cout << "New client connected: " << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << '\n';
					}
				}
				else
				{
					clientSocket = fds[i].fd;
					int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
					if (bytesRead <= 0)
					{
						if (bytesRead == 0)
							std::cout << "Client disconnected\n";
						else
							error_exit("Error receiving data");
						close(clientSocket);
						fds.erase(fds.begin() + i);
						clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
						i--;
					}
					else
					{
						std::string clientMessage = std::string(buffer, 0, bytesRead);
						std::cout << "Received from client: " << clientMessage << "\n";

						std::string reply;
						if (clientMessage == "CAP LS 302\r\n")
							reply = ":verycoolserver CAP * LS :\r\n";
						else
							reply = "hi!\r\n";
						send(fds[i].fd, reply.c_str(), reply.length(), 0);
						std::cout << "Replied with > " << reply << "\n";
					}
				}
			}
		}
	}

	// CLOSE CONNECTION --------------------------------------------------------

	if (close(serverSocket) == -1)
		return (error_exit("Error closing server socket"));
	std::cout << "Server socket closed\n";
}
