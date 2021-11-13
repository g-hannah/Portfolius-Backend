#include <assert.h>
#include "socketobj.h"

#define clear_struct(s) memset((s), 0, sizeof(*(s)))

portfolius::SocketObj::SocketObj()
{
	this->sock = -1;
/*
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in sin;

	clear_struct(&sin);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (bind(this->sock, std::static_cast<struct sockaddr *>(&sin), sizeof(sin)))
		throw std::exception("Failed to bind socket");
*/
}

portfolius::SocketObj::SocketObj(SocketObj *obj)
{
	this->sock = obj->sock;
	
}

portfolius::SocketObj::~SocketObj()
{
}

void portfolius::SocketObj::set_port(const uint16_t port)
{
	this->port = port;
}

uint16_t portfolius::SocketObj::get_port()
{
	return this->port;
}

void portfolius::SocketObj::set_socket(const int sock)
{
	this->sock = sock;
}

int portfolius::SocketObj::get_socket()
{
	return this->sock;
}

/*
 * Private methods
 *
struct sockaddr_in *SocketObj::_get_remote_host_ip(std::string host_addr)
{
	struct addrinfo *ainf = nullptr;
	struct addrinfo *aip = nullptr;
	struct sockaddr_in *sin = malloc(sizeof(struct sockaddr_in));

	assert(sin);

	if (getaddrinfo(host_addr, NULL, NULL, &ainf) < 0)
	{
	}

	for (aip = ainf; aip; aip = aip->ai_next)
	{
		if (AF_INET == aip->ai_family && SOCK_STREAM == aip->ai_socktype)
		{
			memcpy(&sin, aip->ai_addr, aip->ai_addrlen);
			break;
		}
	}

	if (nullptr == aip)
		goto fail;


	

fail:
	freeaddrinfo(ainf);
	return nullptr;
}
*/
