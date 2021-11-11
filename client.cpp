#include "client.h"

Client::Client()
{
	this->sock = -1;
}

Client::Client(Client& c)
{
	this->sock = c.sock;
	this->sin = c.sin;
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

std::size_t send(std::string data)
{
	std::size_t to_send = data.length();
	std::size_t num = 0;
	char *p = data.c_str();

	while (to_send > 0 && (num = send(this->sock, p, to_send)) > 0)
	{
		to_send -= num;
		p += num;
	}
}
