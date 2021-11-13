#ifndef _ExchangeRatesManager_h_
#define _ExchangeRatesManager_h_ 1

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <assert.h>
#include <signal.h>
#include <vector>
#include <mutex>
#include <ctime>
#include <chrono>
#include <cstdio>
#include <cstddef>
#include "httplib/httplib.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/memorystream.h"
#include "applicationsettings.h"
#include "filehandler.h"
#include "rate.h"

#define FILESTREAMWRITER_BUFSIZE	32768

#define API_ENDPOINT		"https://min-api.cryptocompare.com"
#define API_CRYPTO_ARG		"fsym"
#define API_CURRENCY_ARG	"tsyms"

#define PATH_TO_RATES		"./data"

#define SLEEP_TIME	360000L // 6 minutes

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
		friend class ApplicationSettings;

		public:

			static ExchangeRatesManager *instance()
			{
				if (!_instance)
					_instance = new ExchangeRatesManager();

				return _instance;
			}

			void start();
			portfolius::Rate* get_rate_for_currency(std::string);
			std::vector<portfolius::Rate*> *get_rates_history_for_currency(std::string);

		private:

			static ExchangeRatesManager *_instance;
			ExchangeRatesManager();
			virtual ~ExchangeRatesManager();

			bool _running = false;
			void _synchronise_maps();
			void _write_rates();
			void _read_rates();

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
			std::map<std::string,std::vector<portfolius::Rate*>*> _map_secondary;
			std::map<std::string,std::vector<portfolius::Rate*>*> _map_primary;
			std::mutex rates_mutex;
			std::mutex running_mutex;
	};
}


#endif
