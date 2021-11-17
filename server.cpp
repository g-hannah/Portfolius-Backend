#include "server.h"

portfolius::Server *portfolius::Server::_instance = nullptr;

portfolius::Server::Server()
{
	this->listening_socket = new portfolius::ListeningSocket;
}

portfolius::Server::~Server()
{
	if (this->listening_socket)
		delete this->listening_socket;
}

void portfolius::Server::client_error(portfolius::Client *client, int type)
{
	if (!client)
		return;

	rapidjson::Document d;
	rapidjson::Document::AllocatorType& a = d.GetAllocator();

	d.SetObject();

	d.AddMember("status", "error", a);

	switch (type)
	{
		case CLIENT_ERROR_INVALID_TYPE:

			d.AddMember("message", "invalid request type", a);
			break;

		case CLIENT_ERROR_INVALID_CURRENCY:

			d.AddMember("message", "invalid currency", a);
			break;

		default:

			d.AddMember("message", "unknown error", a);
	}

/*
 * Write the JSON to a string
 */
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	d.Accept(writer);

	client->send(d.GetString());
	std::cerr << "sent response to client" << std::endl;
	std::cerr << d.GetString() << std::endl;
}

void portfolius::Server::send_response(portfolius::Client *client, std::vector<portfolius::Rate*> *vec)
{
	rapidjson::Document d;
	rapidjson::Document::AllocatorType& a = d.GetAllocator();

	d.SetObject();

	rapidjson::Value arr(rapidjson::kArrayType);
	d.AddMember("status", "ok", a);

	for(int i = 0, n = vec->size(); i < n; ++i)
	{
		portfolius::Rate *r = vec->at(i);
		rapidjson::Value v(rapidjson::kObjectType);
		v.AddMember("timestamp", r->get_timestamp(), a);
		v.AddMember("value", r->get_value(), a);

		arr.PushBack(v, a);
	}

/*
 * Write the JSON to a string
 */
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	d.Accept(writer);

	client->send(d.GetString());
	std::cerr << "sent response to client" << std::endl;
	std::cerr << d.GetString() << std::endl;
}

bool portfolius::Server::_is_valid_type(std::string type)
{
	bool ret = false;

	ret = (!strcmp(type.c_str(), REQUEST_TYPE_SINGLE_RATE) || !strcmp(type.c_str(), REQUEST_TYPE_HISTORIC));

	return ret;
}

bool portfolius::Server::_is_valid_currency(std::string currency)
{
	portfolius::ApplicationSettings *settings = portfolius::ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
		return false;

	for (int i = 0; currencies[i]; ++i)
	{
		if (!strcmp(currency.c_str(), currencies[i]))
			return true;
	}

	return false;
}

void portfolius::Server::run()
{
	this->listening_socket->listen();

	while (1)
	{
		std::cerr << "Waiting for client request" << std::endl;
		portfolius::Client *client = this->listening_socket->wait_for_client_request();
		const int client_socket = client->get_socket();

		if (STDERR_FILENO >= client_socket)
			continue;

		std::cerr << "Received request from client" << std::endl;

	/*
	 * Fork a new process to handle the client's request
	 */
		pid_t pid = fork();

		if (0 > pid)
		{
			std::cerr << "error forking new process..." << std::endl;
			//throw std::runtime_error("Failed to fork new process");
		}
		else
		if (0 == pid)
		{
			// child process executes this block
			portfolius::ExchangeRatesManager *manager = portfolius::ExchangeRatesManager::instance();
			char *buffer = (char *)malloc(CLIENT_REQUEST_BUFSIZE);
			assert(buffer);

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
					"currency" : "ETH"
				}
			 */
			std::size_t bytes_received = read(client_socket, buffer, CLIENT_REQUEST_BUFSIZE);
			std::cerr << "read " << bytes_received << " bytes from client socket" << std::endl;

			if (0 >= bytes_received)
			{
				delete client;
				exit(1);
			}

			std::cerr << buffer << std::endl;

		/*
		 * Parses the JSON data into a DOM
		 */
			rapidjson::Document d;
			d.Parse(buffer);

			rapidjson::Value& type = d["type"];
			std::string value_type = type.GetString();

			rapidjson::Value& currency = d["currency"];
			std::string value_currency = currency.GetString();

			if (!this->_is_valid_type(value_type))
			{
				this->client_error(client, CLIENT_ERROR_INVALID_TYPE);
			}
			else
			if (!this->_is_valid_currency(value_currency))
			{
				this->client_error(client, CLIENT_ERROR_INVALID_CURRENCY);
			}
			else
			{
				if (!strcmp(value_type.c_str(), REQUEST_TYPE_SINGLE_RATE))
				{
					portfolius::Rate *rate = manager->get_rate_for_currency(value_currency);
					std::vector<portfolius::Rate*> *vec = new std::vector<portfolius::Rate*>;
					vec->push_back(rate);

					this->send_response(client, vec);

					delete vec;
				}
				else
				if (!strcmp(value_type.c_str(), REQUEST_TYPE_HISTORIC))
				{
					std::vector<portfolius::Rate*> *vec = manager->get_rates_history_for_currency(value_currency);
					this->send_response(client, vec);
				}
			}

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
