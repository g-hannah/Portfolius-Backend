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

void Server::client_error(portfolius::Client& client, std::string message)
{
	rapidjson::Document d;
	rapidjson::Document::AllocatorType& a = d.GetAllocator();

	d.SetObject();

	d.AddMember("status", "error", a);
	d.AddMember("message", message, a);

/*
 * Write the JSON to a string
 */
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	d.Accept(writer);

	client.send(d.GetString());

	std::string out = d.GetString();
}

void Server::send_response(portfolius::Client& client, std::vector<Rate*> vec)
{
	rapidjson::Document d;
	rapidjson::Document::AllocatorType& a = d.GetAllocator();

	d.SetObject();

	rapidjson::Value arr(rapidjson::kArrayType);
	d.AddMember("status", "ok");

	for(int i = 0, n = vec.size(); i < n; ++i)
	{
		Rate *r = vec[i];
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

	client.send(d.GetString());
}

bool Server::_is_valid_type(std::string type)
{
	bool ret = false;

	ret = (!strcmp(type.c_str(), REQUEST_TYPE_SINGLE_RATE) || !strcmp(type.c_str(), REQUEST_TYPE_HISTORIC));

	return ret;
}

bool Server::_is_valid_currency(std::string currency)
{
	portfolius::ApplicationSettings *settings = portfolius::ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
		return false;

	for (int i = 0; currencies[i]; ++i)
	{
		if (!strcmp(currency, currencies[i]))
			return true;
	}

	return false;
}

void Server::run()
{
	this->listening_socket->listen();

	while (1)	
	{
		portfolius::Client& client = this->listening_socket->wait_for_client_request();

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
			portfolius::ExchangeRatesManager *manager = portfolius::ExchangeRatesManager::instance();
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
					"currency" : "ETH"
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

			if (!this->_is_valid_type(value_type))
			{
				this->client_error(client, "invalid request type");
			}
			else
			if (!this->_is_valid_currency(value_currency))
			{
				this->client_error(client, "invalid currency");
			}
			else
			{
				if (!strcmp(type.c_str(), REQUEST_TYPE_SINGLE_RATE))
				{
					Rate& rate = manager->get_rate_for_currency(currency);
					std::vector<Rate*> vec;
					vec.push_back(new Rate(rate.get_timestamp(), rate.get_value()));

					this->send_response(client, vec);
				}
				else
				if (!strcmp(type.c_str(), REQUEST_TYPE_HISTORIC))
				{
					std::vector<Rate*> vec = manager->get_rates_history_for_currency(currency);
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
