#include <assert.h>
#include "socketobh.h"

#define clear_struct(s) memset((s), 0, sizeof(*(s)))

SocketObj::SocketObj()
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

SocketObj::~SocketObj()
{
}

void SocketObj::set_port_no(const uint16_t port)
{
	this->port = port;
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

uint16_t SocketObj::get_port_no()
{
	return this->port_no;
}

void SocketObj::listen()
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

struct sockaddr *SocketObj::wait_for_client_request()
{
	int client_socket = -1;
	struct sockaddr_in *sin = NULL;
	socklen_t socklen = 0;

	sin = malloc(sizeof(struct sockaddr_in));
	assert(NULL != sin);

	client_socket = accept(this->sock, std::static_cast<struct sockaddr *>(sin), &socklen);
	if (0 > client_socket)
		throw std::exception("Error accepting client request");

	return sin;
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
