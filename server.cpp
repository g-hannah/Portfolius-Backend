#include "server.h"

#define BUFFER_SIZE 2048

Server::Server()
{
	this->listening_socket = new ListeningSocket;
	this->set_port_no(LISTENING_PORT);
}

Server::~Server()
{
	if (this->listening_socket)
		delete this->listening_socket;
}

void Server::run()
{
	this->listening_socket->listen();

	while (1)	
	{
		Client& client = this->listening_socket->wait_for_client_request();

		const int client_socket = client.get_socket();
		const struct sockaddr_in& client_sin = client.get_sin();

		if (STDERR_FILENO >= client_socket)
			continue;

	/*
	 * Fork a new process to handle the client's request
	 */
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
			/*
				Parent process simply continues
				with another iteration of the loop,
				waiting for the next client request.
			 */
			continue;
		}
	}
}
