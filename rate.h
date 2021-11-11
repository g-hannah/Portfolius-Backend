#ifndef __RATE_h__
#define __RATE_h__ 1

#include <ctime>

/**
 * A class for handling rates data
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class Rate
	{
		public:
			Rate(std::time_t t, double v) : timestamp(t), value(v) {}
			virtual ~Rate() {}

			std::time_t get_timestamp();
			double get_value();

		private:
			std::time_t timestamp;
			double value;
	};
}

#endif
