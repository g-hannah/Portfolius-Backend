#include "rate.h"

std::time_t Rate::get_timestamp()
{
	return this->timestamp;
}

double Rate::get_value()
{
	return this->value;
}
