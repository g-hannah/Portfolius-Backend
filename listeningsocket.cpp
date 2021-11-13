#include "listeningsocket.h"

portfolius::ListeningSocket::ListeningSocket()
{
}

portfolius::ListeningSocket::~ListeningSocket()
{
}

void portfolius::ListeningSocket::listen()
{
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
		throw std::runtime_error("Failed to create socket");

	struct sockaddr_in sin = {0,};

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(this->sock, (struct sockaddr *)&sin, sizeof(sin)))
		throw std::runtime_error("Failed to bind socket");

	if (::listen(this->sock, BACKLOG_VALUE) < 0)
		throw std::runtime_error("Failed to listen on socket");
}

portfolius::Client *portfolius::ListeningSocket::wait_for_client_request()
{
	portfolius::Client *client = new Client;
	socklen_t socklen = 0;
	
	client->sock = accept(this->sock, (struct sockaddr *)&client->sin, &socklen);

	if (0 > client->sock)
		throw std::runtime_error("Error accepting client request");

	return client;
}

void portfolius::ListeningSocket::send(std::string data)
{
}
