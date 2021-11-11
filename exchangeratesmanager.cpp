#include "ExchangeRatesManager.h"

#define API_ENDPOINT		"https://min-api.cryptocompare.com"
#define API_CRYPTO_ARG		"fsym"
#define API_CURRENCY_ARG	"tsyms"

#define PATH_TO_RATES		"./data"

#define SLEEP_TIME	360000L // 6 minutes

ExchangeRatesManager::ExchangeRatesManager()
{
	try
	{
		this->_map_primary = this->_read_rates();
	/*
	 * std::map's copy ctor will copy everything
	 */
		this->_map_secondary = this->_map_primary;
	}
	catch (std::exception e)
	{
		std::cerr << "ERM ctor: Failed to read exchange rates: " << e.what() << std::endl;
	}
}

ExchangeRatesManager::~ExchangeRatesManager()
{
}

void ExchangeRatesManager::start()
{
/*
 * We only want this method to be running
 * once by one thread. So place this here
 * in the event that somehow start() was
 * called twice.
 */
	this->running_mutex.lock();

	if (this->_running)
	{
		this->running_mutex.unlock();
		return;
	}

	this->running = true;
	this->running_mutex.unlock();

	portfolius::ApplicationSettings *settings = portfolius::ApplicationSettings::instance();

	while (true)
	{
		if (!this->_map_secondary.empty())
		{
			std::map<std::string,std::vector<Rate>>::iterator iter = this->_map_secondary.begin();

			while (iter != this->_map_secondary.end())
			{
				std::string key = iter->first;
				std::vector<Rate> vec = iter->second;

				/*
				 * Let the API think we are a web browser
				 * (some servers will ignore anything that isn't)
				 */
				httplib::Headers headers = {
					{ "User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0" },
					{ "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" }
				};

				httplib::SSLClient client(API_ENDPOINT, headers);
				auto result = client.Get("/data/price?" + API_CRYPTO_ARG + "=" + key + "&" + API_CURRENCY_ARG + "=GBP");

				std::string body = result->body;

			/*
			 * Response from API is of format:
			 *
			 * { "GBP" : <value> }
			 */
				rapidjson::Document d2;
				d2.Parse(body);
				rapidjson::Value& v2 = d2["GBP"];

				double fresh_rate = v2.GetDouble();
				std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

				vec.push_back(new Rate(now, fresh_rate));
			}
		}

		this->rates_mutex.lock();

	/*
	 * Copy over updated map
	 */
		this->_map_primary = this->_map_secondary;
		this->_write_rates();

		this->rates_mutex.unlock();

		sleep(SLEEP_TIME);
	}
}

/**
 * Reads rates into map
 */
std::map<std::string,std::vector<Rate*>> ExchangeRatesManager::_read_rates()
{
	ApplicationSettings *settings = ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
		return nullptr;

	std::map<std::string,std::vector<Rate>> map;

	for (int i = 0; currencies[i]; ++i)
	{
		std::string path = PATH_TO_DATA + "/" + currencies[i] + ".json";
		FileHandler fh = new FileHandler(path);
		std::string json = fh.read_all();

		rapidjson::Document d;

		d.Parse(json);
		rapidjson::Value& v = d["data"];

		assert(v.IsArray());

		std::vector<Rate> vec;

		for (rapidjson::SizeType k, n = v.Size(); k < n; ++k)
		{
			rapidjson::Value& v_ts = v[i]["timestamp"];
			rapidjson::Value& v_val = v[i]["value"];

			vec.push_back(new Rate(v_ts.GetDouble(), v_val.GetDouble()));
		}

		std::string key = currencies[i];
		map[key] = vec;
	}

	return map;
}


/**
 * Writes rates for each currency to its respective file.
 *
 * e.g.,
 *
 * BTC.json
 * ETH.json
 */
void ExchangeRatesManager::_write_rates()
{
	std::map<std::string,std::vector<Rate*>> map = this->_primary_map;
	std::map<std::string,std::vector<Rate*>>::iterator iter = map.begin();

	while (iter != map.end())
	{
		std::string key = iter->first;
		std::vector<Rate*> vec = iter->second;

		rapidjson::Document d;
		rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
		d.SetObject();
		rapidjson::Value arr(rapidjson::kArrayType);

		for (int i = 0, n = vec.size(); i < n; ++i)
		{
			rapidjson::Value val(rapidjson::kObjectType);
			val.AddMember("timestamp", vec[i]->get_timestamp());
			val.AddMember("value", vec[i]->get_value());
			arr.PushBack(val, allocator):
		}

		d.AddMember("data", arr, allocator);

		std::string path = PATH_TO_DATA + "/" + key + ".json";
		char *buffer = (char *)calloc(FILESTREAMWRITER_BUFSIZE);
		assert(buffer);

		FILE *fp = fopen(path, "w");
		rapidjson::FileWriteStream os(fp, buffer, FILESTREAMWRITER_BUFSIZE);
		rapidjson::Writer<rapidjson::FileWriteStream> writer(os);

	/*
	 * This automatically writes the JSON to the file
	 */
		d.Accept(writer);

		fclose(fp);
		fp = nullptr;

		free(buffer);
		buffer = nullptr;

		++iter;
	}
}

/**
 * This method can be called from another thread, so
 * a mutex to protect the data structures is necessary
 */
Rate& ExchangeRatesManager::get_rate_for_currency(std::string currency)
{
	this->rates_mutex.lock();
/*
 * Critical section
 */

	std::map<std::string,std::vector<Rate*>> rates = this->_primary_map;
	std::vector<Rate*> vec = rates.get(currency);
	if (!vec)
	{
		this->rates_mutex.unlock();
		throw std::exception("No rates for currency \"" + currency + "\"");
	}


	Rate *rate = vec[vec.size()-1];

	this->rates_mutex.unlock();

	Rate& r = rate;
	return r;
}

/**
 * This method can be called from another thread, so
 * a mutex to protect the data structures is necessary
 */
std::vector<Rate*> ExchangeRatesManager::get_rates_history_for_currency(std::string)
{
	this->rates_mutex.lock();
/*
 * Critical section
 */

	std::map<std::string,std::vector<Rate*>> rates = this->_primary_map;
	std::vector<Rate*> vec = rates.get(currency);
	if (!vec)
	{
		this->rates_mutex.unlock();
		throw std::exception("No rates for currency \"" + currency + "\"");
	}

	this->rates_mutex.unlock();

	return vec;
}
