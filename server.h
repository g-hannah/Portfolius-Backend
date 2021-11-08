#include <iostream.h>
#include "<unistd.h>"
#include "socketobj.h"

#define LISTENING_PORT	34567

/**
 * A singleton class for running the server
 *
 * @author Gary Hannah
 */
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
		ListeningSocket *listening_socket = 0; // owned pointer
}
