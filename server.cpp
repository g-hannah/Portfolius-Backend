#include "server.h"

Server::Server()
{
	this->listening_socket = new SocketObj();
	this->set_port_no(LISTENING_PORT);
}

Server::~Server()
{
	delete this->listening_socket;
}

void Server::run()
{
	this->listening_socket->listen();

	while (1)	
	{
		struct sockaddr_in *sock = this->listening_socket->wait_for_client_request();
		if (NULL == sock)
			continue;

		ExchangeRatesManager manager = ExchangeRatesManager::instance();
		manager->handle_client_request(sock);
	}
}
