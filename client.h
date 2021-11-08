#ifndef __CLIENT_h__
#define __CLIENT_h__ 1

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
