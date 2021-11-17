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

			explicit Rate(std::time_t t, double v) : timestamp(t), value(v) {}
			Rate(const Rate& other);
			Rate(Rate&& other); // rvalue copy ctor
			Rate& operator=(Rate& other);
			Rate& operator=(Rate&& other); // rvalue ref copy assignment ctor
			virtual ~Rate() {}

			std::time_t get_timestamp();
			double get_value();

		private:

			std::time_t timestamp;
			double value;
	};
}

#endif
