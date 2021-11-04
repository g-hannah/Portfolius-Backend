#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#define HTTP_PORT	80
#define HTTPS_PORT	443

/**
 * A class to manage sockets to a remote host
 *
 * @author Gary Hannah
 */
class SocketObj
{
	public:
		SocketObj();
		virtual ~SocketObj();
		bool connect();

	private:

/*
 * Methods
 */
		inaddr_t _get_remote_host_ip();

/*
 * Vars
 */
		std::string remote_host_FQDN;
		in_addr_t remote_in_addr_;
		struct sockaddr *sa_;
		int socket_;
}
