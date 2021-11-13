#ifndef __LISTENING_SOCKET_h__
#define __LISTENING_SOCKET_h__ 1

#include <assert.h>
#include "socketobj.h"
#include "client.h"

#define BACKLOG_VALUE	16

/**
 * A class specifically for listening for client
 * requests that is derived from base class SocketObj
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class ListeningSocket : SocketObj
	{
		public:
			ListeningSocket();
			virtual ~ListeningSocket();

			void listen();
			portfolius::Client *wait_for_client_request();
			void send(std::string);
	};
}
#endif
