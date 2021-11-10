#include "ExchangeRatesManager.h"

#define API_ENDPOINT		"https://min-api.cryptocompare.com"
#define API_CRYPTO_ARG		"fsym"
#define API_CURRENCY_ARG	"tsyms"

#define PATH_TO_RATES		"./data"

#define SLEEP_TIME	360000L // 6 minutes

ExchangeRatesManager::ExchangeRatesManager()
{
}

ExchangeRatesManager::~ExchangeRatesManager()
{
	if (this->_sock)
		delete this->_sock;
}

void ExchangeRatesManager::start()
{
	ApplicationSettings *settings = ApplicationSettings::instance();

	while (true)
	{
		char **currencies = settings->get_currencies();

		if (currencies)
		{
			for (int i = 0; currencies[i]; ++i)
			{
				std::string = PATH_TO_DATA + "/" + currencies[i] + ".json";
				FileHandler fh = new FileHandler(PATH_TO_DATA);
			//	std::string json = this->read_rates(PATH_TO_DATA);


				std::string json;

				try
				{
					json = fh.read_all();
				}
				catch (std::exception e)
				{
					continue;
				}

				rapidjson::Document d;
				d.Parse(json);
				rapidjson::Value& v = d["data"];

				if (!v.IsArray()) // it should be...
					continue;

				//std::vector<Rate> vector_rates;

				//for (rapidjson::SizeType i = 0, n = v.Size(); i < n; ++i)
				//	vector_rates.push_back(new Rate(v[i]["timestamp"], v[i]["value"]));

				std::string api_url = API_ENDPOINT API_CRYPTO_ARG + "=" + currencies[i] + "&" + API_CURRENCY_ARG + "=GBP";

			/*
			 * Let the API think we are a web browser
			 * (some servers will ignore anything that isn't)
			 */
				httplib::Headers headers = {
					{ "User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0" },
					{ "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" }
				};

				httplib::SSLClient client(API_ENDPOINT, headers);
				auto result = client.Get("/data/price?" + API_CRYPTO_ARG + "=" + currencies[i] + "&" + API_CURRENCY_ARG + "=GBP");

				std::string body = result->body;

				rapidjson::Document d2;
				d2.Parse(body);
				rapidjson::Value& v2 = d2["GBP"];
				double rate_value = v2.GetDouble();

				std::time_t now = std::chrono::system_clock::now();

				//Rate new_rate = new Rate((long)now, rate_value);
				//vector_rates.push_back(new_rate);


			/*
			 * Add new object to the d["data"] array
			 * with new rate:
			 *
			 * { "timetamp" : <timestamp>, "value" : <value> }
			 */
				rapidjson::Document d3;

				d3.setObject();
				rapidjson::Value object(rapidjson::kObjectType);
				object.AddMember("timestamp", now);
				object.AddMember("value", rate_value);
				d.PushBack(object, d3.GetAllocator());

				this->write_rates(d);
			}
		}

		/*
		 * Retrieve rates from the endpoint
		 *
		 *	- parse JSON data
		 *
		 * Add to data structure
		 *
		 *	- Acquire mutex that protects the data structure
		 *	- A hashmap (currency_key -> list_of_data)
		 *	- Unlock mutex
		 *
		 * Write to disk
		 *
		 */
		sleep(SLEEP_TIME);
	}


	pthread_mutex_lock(&this->init_mutex);

	if (true == _initialised)
	{
		pthread_mutex_unlock(&this->init_mutex);
		return;
	}

	// initialise the manager

	_initialised = true;
	pthread_mutex_unlock(&this->init_mutex);
}

std::string ExchangeRatesManager::read_rates(std::string path)
{
	std::string s = "nothing";
	return s;
}

void ExchangeRatesManager::write_rates(rapidjson::Document d)
{
}

double ExchangeRatesManager::getRateForCurrency(std::string currency)
{
	/*
		Acquire mutex for data structure

		Check for existence of data for given currency

		if no such currency
			unlock mutex
			return error

		else
			retrieve most up-to-date rate for the currency
			unlock mutex
			return the rate
	*/
}

std::list<double> ExchangeRatesManager::getRatesHistoryForCurrency(std::string)
{
	/*
		Acquire mutex

		Get list of rates for the currency if it exists

		unlock mutex

		return result (list of rates or nullptr)
	*/
}
