#include "server.h"
#include "rapidjson/document.h"

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
		 * Parses the JSON data into a DOM
		 */
			rapidjson::Document d;
			d.Parse(buffer);

			rapidjson::Value& type = d["type"];
			std::string value_type = type.GetString();

			rapidjson::Value& currency = d["currency"];
			std::string value_currency = currency.GetString();

			ApplicationSettings *settings = ApplicationSettings::instance();

			if (!settings->is_valid_currency(value_currency))
			{
				this->send_response(client, "invalid currency", REQUEST_ERROR);
				goto child_exit;
			}
			else
			if (!settings->is_valid_request_type(value_type))
			{
				this->send_response(client, "invalid request type", REQUEST_ERROR);
				goto child_exit;
			}

			ExchangeRatesManager *manager = ExchangeRatesManager::instance();

			double rate = manager->get_rate_for_currency(currency);

			// write json data and send it
			

		child_exit:
			exit(0); // we won't be checking child process's return value, so just return 0 no matter what
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
