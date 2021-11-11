#ifndef __RATE_h__
#define __RATE_h__ 1

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
			Rate(long t, double v) : timestamp(t), value(v) {}
			virtual ~Rate() {}

			long get_timestamp();
			double get_value();

		private:
			long timestamp;
			double value;
	};
}

#endif
