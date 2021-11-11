#ifndef __SOCKETOBJ_h__
#define __SOCKETOBJ_h__ 1

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdexcept>

#define HTTP_PORT	80
#define HTTPS_PORT	443

/**
 * A class to manage sockets to a remote host
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class SocketObj
	{
		public:
			SocketObj();
			SocketObj(SocketObj *obj);
			virtual ~SocketObj();

			void set_port(uint16_t);
			uint16_t get_port();
			int get_socket();
			void set_socket(int);

		private:
			int sock;
			in_port_t port;
	};
}

#endif
