#include <iostream.h>
#include "socketobj.h"

#define LISTENING_PORT	34567

class Server
{
	public:
		static Server *instance()
		{
			if (!server_)
				server_ = new Server

			return server_
		}

		void run(std::string);

	private:
		static Server server_ = 0;

		Server() {}
		~Server() {}
		SocketObj listening_socket;
}
