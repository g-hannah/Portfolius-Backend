#ifndef _ExchangeRatesManager_h_
#define _ExchangeRatesManager_h_ 1

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <assert.h>
#include "httplib/httplib.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/memorystream.h"
#include <vector>
#include <mutex>
#include <ctime>
#include <chrono>
#include <cstdio>

#define API_ENDPOINT ""

#define FILESTREAMWRITER_BUFSIZE	32768

/**
 * A singleton class to get exchange rates
 * from an API endpoint.
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class ExchangeRatesManager
	{
		public:

			static ExchangeRatesManager *instance()
			{
				if (!_instance)
					_instance = new ExchangeRatesManager();

				return _instance;
			}

			void start();
			Rate& get_rate_for_currency(std::string);
			std::vector<Rate*> get_rates_history_for_currency(std::string);

		private:

			static ExchangeRatesManager *_instance = 0;
			ExchangeRatesManager();
			virtual ~ExchangeRatesManager();

			bool _running = false;
			void write_rates(rapidjson::Document);
			std::string read_rates(std::string);

		/*
		 * When we are iterating through the map's keys in order
		 * to get fresh rates from the API and add the fresh rates
		 * to each vector, we don't want to have the mutex locked
		 * for the entire time the rates are being updated because
		 * if a client request comes within that time to get rates,
		 * the latency could be high since we are waiting for a
		 * response from the API for each currency before the
		 * mutex would be unlocked.
		 *
		 * So have two maps. This secondary map will be updated
		 * when getting fresh rates. Once we are done, lock the
		 * mutex for the primary one and copy over the data.
		 */
			std::map<std::string,std::vector<Rate*>> _map_secondary;
			std::map<std::string,std::vector<Rate*>> _map_primary;
			std::mutex rates_mutex;
			std::mutex running_mutex;
	};
}


#endif
