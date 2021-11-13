#include "exchangeratesmanager.h"

portfolius::ExchangeRatesManager *portfolius::ExchangeRatesManager::_instance = 0;

static inline std::time_t _now()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

portfolius::ExchangeRatesManager::ExchangeRatesManager()
{
	try
	{
		this->_map_primary = this->_read_rates();
	/*
	 * std::map's copy ctor will copy everything
	 */
		this->_map_secondary = this->_map_primary;
	}
	catch (std::runtime_error e)
	{
		std::cerr << "ERM ctor: Failed to read exchange rates: " << e.what() << std::endl;
	}
}

portfolius::ExchangeRatesManager::~ExchangeRatesManager()
{
}

/**
 * Synchronise the primary and secondary maps after
 * adding fresh exchange rates to the secondary map.
 */
void portfolius::ExchangeRatesManager::_synchronise_maps()
{
	portfolius::ApplicationSettings *settings = portfolius::ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	for (int i = 0; currencies[i]; ++i)
	{
		char *key = currencies[i];
		std::vector<portfolius::Rate*> *updated = this->_map_secondary[key];
		std::vector<portfolius::Rate*> *outdated = this->_map_primary[key];

		for (int k = 0, n = outdated->size(); k < n; ++k)
		{
			portfolius::Rate *r = outdated[k];
			delete r;
		}

		outdated->clear();

		for (int k = 0, n = updated->size(); k < n; ++k)
		{
			portfolius::Rate *r = updated[k];
			outdated->push_back(new Rate(r->get_timestamp(), r->get_value()));
		}
	}
}

void portfolius::ExchangeRatesManager::start()
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

	this->_running = true;
	this->running_mutex.unlock();

	portfolius::ApplicationSettings *settings = portfolius::ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
	{
		std::cerr << "No currencies for which to get rates" << std::endl;
		return;
	}

	while (true)
	{
		for (int i = 0; currencies[i]; ++i)
		{
			const char *key = currencies[i];
			std::map<std::string,std::vector<portfolius::Rate*>*>::iterator it = this->_map_secondary.find(key);

			if (it == this->_map_secondary.end())
				continue;

			std::vector<portfolius::Rate*> *vec = it->second;

				/*
				 * Let the API think we are a web browser
				 * (some servers will ignore anything that isn't)
				 */
				httplib::Headers headers = {
					{ "User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0" },
					{ "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" }
				};


				std::string endpoint = API_ENDPOINT;
				httplib::SSLClient client(endpoint);

			/*
			 * According to the httplib documentation, when using an SSL client,
			 * there is no way to avoid a SIGPIPE signal occurring. So we simply
			 * have to ignore it here.
			 *
			 * SIGPIPE happens with a "write on a pipe with no one to read it", and
			 * the default action taken on this signal is abnormal termination
			 * of the process - IEEE POSIX Std 1003.1 p. 334
			 */

				struct sigaction sa_old = {0};
				struct sigaction sa_new = {0};

			/*
			 * SIG_IGN - requests that the signal be ignored - IEEE POSIX Std 1003.1 p. 332
			 */
				sa_new.sa_handler = SIG_IGN;
				sigaction(SIGPIPE, &sa_new, &sa_old);

				char *buf = (char *)malloc(128);
				assert(buf);

				snprintf(buf, 128, "/data/price?%s=%s&%s=GBP", API_CRYPTO_ARG, key.c_str(), API_CURRENCY_ARG);

				auto result = client.Get(buf);

				free(buf);
				buf = NULL;

				sigaction(SIGPIPE, &sa_old, NULL);

				std::string body = result->body;

				std::cerr << body << std::endl;

			/*
			 * Response from API is of format:
			 *
			 * { "GBP" : <value> }
			 */
				rapidjson::Document d2;
				d2.Parse(body.c_str());
				rapidjson::Value& v2 = d2["GBP"];

				double fresh_rate = v2.GetDouble();
				//std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				std::time_t now = _now();

				vec.push_back(new portfolius::Rate(now, fresh_rate));
		}

		this->rates_mutex.lock();


		this->_synchronise_maps();
		this->_write_rates();


		this->rates_mutex.unlock();

		sleep(SLEEP_TIME);
	}
}

/**
 * Reads rates into map
 */
std::map<std::string,std::vector<portfolius::Rate*>> portfolius::ExchangeRatesManager::_read_rates()
{
	ApplicationSettings *settings = ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
		return this->_map_primary;

	std::map<std::string,std::vector<portfolius::Rate*>> map;

	for (int i = 0; currencies[i]; ++i)
	{
		char *buf = (char *)malloc(128);
		assert(buf);

		snprintf(buf, 128, "%s/%s.json", PATH_TO_RATES, currencies[i]);
		std::string path = buf;

		portfolius::FileHandler fh(path);
		std::string json = fh.read_all();

		free(buf);
		buf = NULL;

		rapidjson::Document d;

		d.Parse(json.c_str());
		rapidjson::Value& v = d["data"];

		assert(v.IsArray());

		std::vector<portfolius::Rate*> vec;

		for (rapidjson::SizeType k = 0, n = v.Size(); k < n; ++k)
		{
			rapidjson::Value& v_ts = v[i]["timestamp"];
			rapidjson::Value& v_val = v[i]["value"];

			vec.push_back(new portfolius::Rate(v_ts.GetDouble(), v_val.GetDouble()));
		}

		char *key = currencies[i];
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
void portfolius::ExchangeRatesManager::_write_rates()
{
	std::map<std::string,std::vector<portfolius::Rate*>> map = this->_map_primary;
	std::map<std::string,std::vector<portfolius::Rate*>>::iterator iter = map.begin();

	while (iter != map.end())
	{
		std::string key = iter->first;
		std::vector<portfolius::Rate*> vec = iter->second;

		rapidjson::Document d;
		rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
		d.SetObject();
		rapidjson::Value arr(rapidjson::kArrayType);

		for (int i = 0, n = vec.size(); i < n; ++i)
		{
			rapidjson::Value val(rapidjson::kObjectType);
			val.AddMember("timestamp", vec[i]->get_timestamp(), allocator);
			val.AddMember("value", vec[i]->get_value(), allocator);
			arr.PushBack(val, allocator);
		}

		d.AddMember("data", arr, allocator);

		char *path = (char *)malloc(128);
		assert(path);

		snprintf(path, 128, "%s/%s.json", PATH_TO_RATES, key.c_str());
		char *buffer = (char *)calloc(FILESTREAMWRITER_BUFSIZE, 1);
		assert(buffer);

		FILE *fp = fopen(path, "w");
		free(path);
		path = NULL;

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
portfolius::Rate* portfolius::ExchangeRatesManager::get_rate_for_currency(std::string currency)
{
	this->rates_mutex.lock();
/*
 * Critical section
 */

	std::map<std::string,std::vector<portfolius::Rate*>> rates = this->_map_primary;
	std::map<std::string,std::vector<portfolius::Rate*>>::iterator iter = rates.find(currency.c_str());

	if (iter == rates.end())
	{
		this->rates_mutex.unlock();
		throw std::runtime_error(currency);
	}

	std::vector<portfolius::Rate*> vec = iter->second;
	portfolius::Rate *rate = vec[vec.size()-1];

	this->rates_mutex.unlock();

	return rate;
}

/**
 * This method can be called from another thread, so
 * a mutex to protect the data structures is necessary
 */
std::vector<portfolius::Rate*> portfolius::ExchangeRatesManager::get_rates_history_for_currency(std::string currency)
{
	this->rates_mutex.lock();
/*
 * Critical section
 */

	std::map<std::string,std::vector<portfolius::Rate*>> rates = this->_map_primary;
	std::map<std::string,std::vector<portfolius::Rate*>>::iterator iter = rates.find(currency.c_str());

	if (iter == rates.end())
	{
		this->rates_mutex.unlock();
		throw std::runtime_error(currency);
	}

	this->rates_mutex.unlock();

	return iter->second;
}
