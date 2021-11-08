#include "listeningsocket.h"

ListeningSocket::ListeningSocket()
{
}

ListeningSocket::ListeningSocket(ListeningSocket *other)
{
	this->sock = other->sock;

	if (other->client_sin)
	{
		this->client_sin = malloc(sizeof(struct sockaddr_in));
		assert(nullptr != this->client_sin);
		memcpy(this->client_sin, other->client_sin, sizeof(*other->client_sin));
	}
}

ListeningSocket::~ListeningSocket()
{
	if (this->client_sin)
		free(this->client_sin);
}

void ListeningSocket::listen()
{
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
		throw std::exception("Failed to create socket");

	struct sockaddr_in sin;

	clear_struct(&sin);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(this->sock, std::static_cast<struct sockaddr *>(&sin), sizeof(sin)))
		throw std::exception("Failed to bind socket");

	if (listen(this->sock, this->value_backlog) < 0)
		throw std::exception("Failed to listen on socket");
}

Client& ListeningSocket::wait_for_client_request()
{
	Client client;
	socklen_t socklen = 0;
	
	client.sock = accept(this->sock, std::static_cast<struct sockaddr *>(&client.sin), &socklen);

	if (0 > client.sock)
		throw std::exception("Error accepting client request");

	return client;
}

void ListeningSocket::send(std::string data)
{
	assert(nullptr != this->client_sin);
}
