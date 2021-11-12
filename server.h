#ifndef __SERVER_h__
#define __SERVER_h__ 1

#include <iostream.h>
#include "<unistd.h>"
#include <cstddef>
#include "socketobj.h"
#include "rapidjson/document.h"

#define LISTENING_PORT	34567

#define REQUEST_SUCCESS	0x01
#define REQUEST_ERROR	0xff

#define REQUEST_TYPE_SINGLE_RATE	"rate"
#define REQUEST_TYPE_HISTORIC		"historic"

#define CLIENT_REQUEST_BUFSIZE		2048

/**
 * A singleton class for running the server
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class Server
	{
		public:
			static Server *instance()
			{
				if (!_instance)
					_instance = new Server

				return _instance;
			}

			void run(std::string);

		private:
			static Server _instance = 0;
			ListeningSocket *listening_socket = 0; // owned pointer

			void client_error(portfolius::Client&, std::string);
			void send_response(portfolius::Client&, std::vector<Rate*>);
			bool _is_valid_type(std::string);
			bool _is_valid_currency(std::string);
	};
}

#endif
