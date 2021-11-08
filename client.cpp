#include "client.h"

Client::Client()
{
	this->sock = -1;
}

Client::~Client()
{
	if (STDERR_FILENO < this->sock)
		close(this->sock);
}

void Client::set_socket(int s)
{
	this->sock = s;
}

int Client::get_socket()
{
	return this->sock;
}

void Client::set_sin(struct sockaddr_in& sin)
{
	this->sin = sin;
}

void struct sockaddr_in& Client::get_sin()
{
	return this->sin;
}
