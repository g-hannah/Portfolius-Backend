#include <thread>
#include "exchangeratesmanager.h"
#include "server.h"

int
main(void)
{
	/*
	 * Create separate thread within which the
	 * exchange rates manager will run
	 */
	std::thread t_exchangeratesmanager(&portfolius::ExchangeRatesManager::start, portfolius::ExchangeRatesManager::instance());

	/*
	 * Server runs in main thread
	 */
	portfolius::Server *server = portfolius::Server::instance();
	server->run();

	return 0;
}
