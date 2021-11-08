#ifndef __CLIENT_h__
#define __CLIENT_h__ 1

/**
 * A class to represent a client
 * that has sent a request to the
 * server
 *
 * @author Gary Hannah
 */
class Client
{
	friend class ListeningSocket;

	public:

		Client();
		virtual ~Client();

		void set_socket(int);
		void set_sin(struct sockaddr_in&);

	private:

		int sock;
		struct sockaddr_in& sin;
}

#endif
