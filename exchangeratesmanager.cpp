#include "ExchangeRatesManager.h"

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
	while (true)
	{

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
