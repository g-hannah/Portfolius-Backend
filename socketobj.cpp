#include <assert.h>
#include "socketobh.h"

#define clear_struct(s) memset((s), 0, sizeof(*(s)))

SocketObj::SocketObj()
{
	this->sock = -1;
}


SocketObj::~SocketObj()
{
}

bool SocketObj::connect()
{
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (0 > this->sock)
		return false;

	struct sockaddr_in *sin = _get_remote_host_ip(this->remote_host_FQDN);

	clear_struct(&sin);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(this->port);
}

/*
 * Private methods
 */
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
