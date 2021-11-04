#ifndef _ExchangeRatesManager_h_
#define _ExchangeRatesManager_h_ 1

#include <pthread.h>

#define API_ENDPOINT ""

/**
 * A singleton class to get exchange rates
 * from an API endpoint.
 *
 * @author Gary Hannah
 */
class ExchangeRatesManager
{
	public:

		static ExchangeRatesManager *instance()
		{
			if (!_instance)
				_instance = new ExchangeRatesManager();

			return _instance;
		}

		double getRateForCurrency(std::string);
		std::list<double> getRatesHistoryForCurrency(std::string);

	private:

		static ExchangeRatesManager *_instance = 0;
		static bool _initialised = false;
		ExchangeRatesManager();
		virtual ~ExchangeRatesManager();

		SocketObj *_sock = 0;


		pthread_mutex_t init_mutex;
}


#endif
