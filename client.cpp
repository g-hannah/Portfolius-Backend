#include "client.h"

portfolius::Client::Client()
{
	this->sock = -1;
}

portfolius::Client::Client(Client& c)
{
	this->sock = c.sock;
	this->sin = c.sin;
}

portfolius::Client::~Client()
{
	if (STDERR_FILENO < this->sock)
		close(this->sock);
}

void portfolius::Client::set_socket(int s)
{
	this->sock = s;
}

int portfolius::Client::get_socket()
{
	return this->sock;
}

void portfolius::Client::set_sin(struct sockaddr_in *sin)
{
	std::memcpy(&this->sin, sin, sizeof(*sin));
}

struct sockaddr_in& portfolius::Client::get_sin()
{
	return this->sin;
}

std::size_t portfolius::Client::send(std::string data)
{
	std::size_t to_send = data.length();
	std::size_t num = 0;

	char *copy = (char *)malloc(data.length()+1);
	assert(copy);
	std::memcpy(copy, data.c_str(), data.length());
	copy[data.length()] = 0;

	char *p = copy;

	while (to_send > 0 && (num = ::send(this->sock, p, to_send, 0)) > 0)
	{
		to_send -= num;
		p += num;
	}

	free(copy);
	copy = NULL;

	return data.length();
}
