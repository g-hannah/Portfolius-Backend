#include "rate.h"

/**
 * Copy ctor
 */
portfolius::Rate::Rate(const portfolius::Rate &other)
{
	this->timestamp = other.timestamp;
	this->value = other.value;
}

/**
 * Move ctor
 */
portfolius::Rate::Rate(portfolius::Rate &&other)
{
	this->timestamp = other.timestamp;
	this->value = other.value;
}

/**
 * Copy assignment operator overload
 */
portfolius::Rate portfolius::Rate::operator=(portfolius::Rate other)
{
	this->timestamp = other.timestamp;
	this->value = other.value;
}

/**
 * Move semantics assignment operator overload
 */
portfolius::Rate& portfolius::Rate::operator=(portfolius::Rate &&other)
{
	this->timestamp = other.timestamp;
	this->value = other.value;
	return *this;
}

std::time_t portfolius::Rate::get_timestamp()
{
	return this->timestamp;
}

double portfolius::Rate::get_value()
{
	return this->value;
}
