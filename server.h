#ifndef __SERVER_h__
#define __SERVER_h__ 1

#include <iostream>
#include <cstddef>
#include <unistd.h>
#include <vector>
#include "exchangeratesmanager.h"
#include "listeningsocket.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "applicationsettings.h"
#include "client.h"
#include "rate.h"

#define LISTENING_PORT	34567

#define CLIENT_ERROR_INVALID_TYPE		0x01
#define CLIENT_ERROR_INVALID_CURRENCY	0x02

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
					_instance = new Server;

				return _instance;
			}

			void run();

		private:
			Server();
			virtual ~Server();

			static Server *_instance;
			portfolius::ListeningSocket *listening_socket = 0;

			void client_error(portfolius::Client *, int);
			void send_response(portfolius::Client *, std::vector<portfolius::Rate*> *);
			bool _is_valid_type(std::string);
			bool _is_valid_currency(std::string);
	};
}

#endif
