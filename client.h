#ifndef __CLIENT_h__
#define __CLIENT_h__ 1

#include <assert.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * A class to represent a client
 * that has sent a request to the
 * server
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class Client
	{
		friend class ListeningSocket;

		public:
			Client();
			Client(Client&);
			virtual ~Client();

			void set_socket(int);
			void set_sin(struct sockaddr_in *);
			int get_socket();
			struct sockaddr_in& get_sin();
			std::size_t send(std::string);

		private:
			int sock;
			struct sockaddr_in sin = {0};
	};
}

#endif
