#include "exchangeratesmanager.h"
#include "server.h"

int
main(void)
{
	/*
	 * Create separate thread within which the
	 * exchange rates manager will run
	 */
	std::thread t_exchangeratesmanager(&ExchangeRatesManager::start, ExchangeRatesManager::instance());

	/*
	 * Server runs in main thread
	 */
	Server *server = Server::instance();
	server->run();

	return 0;
}
