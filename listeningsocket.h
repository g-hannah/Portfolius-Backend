#ifndef __LISTENING_SOCKET_h__
#define __LISTENING_SOCKET_h__ 1

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
			Client& wait_for_client_request();
			void send(std::string);

		private:
			struct sockaddr_in *client_sin = 0;
	};
}
#endif
