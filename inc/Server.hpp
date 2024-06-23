#pragma once

#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-2.3
// mensagens em irc são compostas por prefixo(opcional), comando, e argumentos (até 15)
// todos separados por um space character
typedef struct {
	std::string prefix;
	std::string command;
	std::string arguments[15];
} t_message;
// edit: se calhar é melhor usar uma classe em vez de struct porque não podemos usar malloc


class Server
{
public:
	Server();
	Server(const int& port, const std::string& password);
	Server(const Server& src);
	~Server();
	Server& operator =(const Server& src);

	int run(); // will be called in main

private:
	int port;
	std::string password;
	std::map<std::string,Client> clients;	// socket/client pair, chave tipo 127.0.0.1:12345
	std::list<Channel> channels;	// list of channels

	// recebe uma mensagem de um client e adiciona ao buffer - adicionar à lista de clients se ainda não existir?
	// se detetar \r\n, construir um struct t_message e remover do buffer ao estilo gnl (sorry anna)
	void recieveMessage(std::string& socket, std::string& stream);

	// faz parse à string
	// retira o prefix, command e args e passa para um struct
	// retorna nulo se for invalido (?)
	t_message* parseStream(std::string& stream);

	// decide o que fazer com a mensagem resultante
	void handleMessage(std::string& socket, t_message* message);

	// funçao para validar o registo de um client
	// returns true if valid, else false
	// checks if the ip already exists in the clients list, then validates the parameters given (pass, nick, user, etc...)
	bool validateRegistration(std::string& socket, std::string& buffer);

	// closes connection to a specific socket
	// sends appropriate message and removes client from the list, etc..
	// usar quando o client se desconectar, der timeout, não reponder a ping......
	void closeConnection(std::string& socket);

	// iterates through the list of clients and pings or disconnects ones that have been inactive too long
	void checkTimeouts();
};
