#include "exchangeratesmanager.h"

portfolius::ExchangeRatesManager *portfolius::ExchangeRatesManager::_instance = nullptr;

static inline std::time_t _now()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

portfolius::ExchangeRatesManager::ExchangeRatesManager()
{
}

portfolius::ExchangeRatesManager::~ExchangeRatesManager()
{
}

void portfolius::ExchangeRatesManager::_init()
{
	this->mutex.lock();

	if (!this->_initialised)
	{
		try
		{
			this->_read_rates();
		}
		catch (std::runtime_error e)
		{
			std::cerr << "ERM _init(): caught runtime error from _read_rates()" << std::endl;
			this->mutex.unlock();
			return;
		}

		this->_initialised = true;
	}

	this->mutex.unlock();
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

	this->_init();

	std::cerr << "erm start(): Getting settings instance" << std::endl;
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
			portfolius::RatesMapIterator it = this->_map_secondary.find(key);

			if (it == this->_map_secondary.end())
				continue;

			portfolius::Rates& vec = it->second;

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

			snprintf(buf, 128, "/data/price?%s=%s&%s=GBP", API_CRYPTO_ARG, key, API_CURRENCY_ARG);

			std::cerr << "requesting \"" << buf << "\"" << std::endl;
			httplib::Result result = client.Get(buf, headers);

			assert(result);

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

			portfolius::Rate rate(now, fresh_rate);
			vec.push_back(std::move(rate));
		}

		this->rates_mutex.lock();

		this->_map_primary = this->_map_secondary;
		this->_write_rates();

		this->rates_mutex.unlock();

		sleep(SLEEP_TIME);
	}
}

/**
 * Reads rates into map
 */
void portfolius::ExchangeRatesManager::_read_rates()
{
	std::cerr << "erm _read_rates(): getting settings instance" << std::endl;
	ApplicationSettings *settings = ApplicationSettings::instance();
	char **currencies = settings->get_currencies();

	if (!currencies)
		return;

	for (int i = 0; currencies[i]; ++i)
	{
		char *buf = (char *)malloc(128);
		assert(buf);

		snprintf(buf, 128, "%s/%s.json", PATH_TO_RATES, currencies[i]);
		std::string path = buf;

		portfolius::FileHandler fh(path);
		fh.open_file();

		if (fh.is_error())
			continue;

		std::string json = fh.read_all();

		free(buf);
		buf = NULL;

		rapidjson::Document d;

		d.Parse(json.c_str());
		rapidjson::Value& v = d["data"];

		assert(v.IsArray());

		portfolius::Rates vec;
		//std::vector<portfolius::Rate*> *vec = new std::vector<portfolius::Rate*>;

		for (rapidjson::SizeType k = 0, n = v.Size(); k < n; ++k)
		{
			rapidjson::Value& v_ts = v[i]["timestamp"];
			rapidjson::Value& v_val = v[i]["value"];

			portfolius::Rate rate(v_ts.GetDouble(), v_val.GetDouble());
			vec.push_back(std::move(rate));
		}

		char *key = currencies[i];
		this->_map_secondary[key] = std::move(vec);
	}

	this->_map_primary = this->_map_secondary;
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
	portfolius::RatesMap map = this->_map_primary;
	portfolius::RatesMapIterator iter = map.begin();
	//std::map<std::string,std::vector<portfolius::Rate*>*> map = this->_map_primary;
	//std::map<std::string,std::vector<portfolius::Rate*>*>::iterator iter = map.begin();

	while (iter != map.end())
	{
		std::string key = iter->first;
		portfolius::Rates *vec = iter->second;
		//std::vector<portfolius::Rate*> *vec = iter->second;

		rapidjson::Document d;
		rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
		d.SetObject();
		rapidjson::Value arr(rapidjson::kArrayType);

		for (int i = 0, n = vec->size(); i < n; ++i)
		{
			rapidjson::Value val(rapidjson::kObjectType);
			val.AddMember("timestamp", vec->at(i)->get_timestamp(), allocator);
			val.AddMember("value", vec->at(i)->get_value(), allocator);
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

	portfolius::RatesMap rates = this->_map_primary;
	portfolius::RatesMapIterator iter = rates.find(currency.c_str());

	//std::map<std::string,std::vector<portfolius::Rate*>*> rates = this->_map_primary;
	//std::map<std::string,std::vector<portfolius::Rate*>*>::iterator iter = rates.find(currency.c_str());

	if (iter == rates.end())
	{
		this->rates_mutex.unlock();
		throw std::runtime_error(currency);
	}

	portfolius::Rates *vec = iter->second;
	//std::vector<portfolius::Rate*> *vec = iter->second;
	portfolius::Rate *rate = vec->at(vec->size()-1);

	this->rates_mutex.unlock();

	return rate;
}

/**
 * This method can be called from another thread, so
 * a mutex to protect the data structures is necessary
 */
portfolius::Rates *portfolius::ExchangeRatesManager::get_rates_history_for_currency(std::string currency)
{
	this->rates_mutex.lock();
/*
 * Critical section
 */

	portfolius::RatesMap rates = this->_map_primary;
	portfolius::RatesMapIterator iter = rates.find(currency.c_str());

	//std::map<std::string,std::vector<portfolius::Rate*>*> rates = this->_map_primary;
	//std::map<std::string,std::vector<portfolius::Rate*>*>::iterator iter = rates.find(currency.c_str());

	if (iter == rates.end())
	{
		this->rates_mutex.unlock();
		throw std::runtime_error(currency);
	}

	this->rates_mutex.unlock();

	return iter->second;
}
