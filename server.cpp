#include "server.h"

#define BUFFER_SIZE 2048

Server::Server()
{
	this->listening_socket = new SocketObj();
	this->set_port_no(LISTENING_PORT);
}

Server::~Server()
{
	delete this->listening_socket;
}

void Server::run()
{
	this->listening_socket->listen();

	while (1)	
	{
		int client_socket = this->listening_socket->wait_for_client_request();
		if (STDERR_FILENO >= client_socket)
			continue;

		pid_t pid = fork();
		if (0 > pid)
		{
			throw std::exception("Failed to fork new process");
		}
		else
		if (0 == pid)
		{
			// child process executes this block
			ExchangeRatesManager manager = ExchangeRatesManager::instance();
			char *buffer = malloc(BUFFER_SIZE);
			assert(NULL != buffer);

			/*
			 * Should receive a JSON-encoded request
			 * from the client, such as:
			 *
			 * {
					"type" : "rate",
					"currency" : "BTC"
			   }

				OR

				{
					"type" : "historic",
					"currency" : [
						"BTC",
						"ETH"
					]
				}
			 */
			read(client_socket, buffer, BUFFER_SIZE);

			/*
				TODO

				Use external json lib to parse request,
				obtain the required data from the
				exchange rates manager and send a JSON
				encoded response to the client.


				e.g.,

				JsonObj jsonobj = Json::read_data(buffer);
				std::string type = jsonobj->get_node("type");
				if (IS_SINGLE_RATE_REQUEST(type))
					// get the rate
				else
					// get array of rates

				JsonObj outjson = new JsonObj;
				outjson.put("type", type);
				outjson.put("data", Json::encode_data(the_data));
				std::string stringified = outjson.get_string();

				size_t sent_len = send(client_socket, stringified, stringified.length());
				if (stringified.length() != sent_len)
					throw std::exception("Failed to send response to client");

				exit(EXIT_SUCCESS);
			 */

			// child process has to exit
			exit(EXIT_SUCCESS);
		}
		else
		{
			// parent process executes this block
			continue;
		}
	}
}
