#include "rate.h"

std::time_t portfolius::Rate::get_timestamp()
{
	return this->timestamp;
}

double portfolius::Rate::get_value()
{
	return this->value;
}
