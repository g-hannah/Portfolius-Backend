#include "applicationsettings.h"

ApplicationSettings::ApplicationSettings()
{
	try
	{
		this->read_config_file();
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
}

ApplicationSettings::~ApplicationSettings()
{
	if (this->currencies)
	{
		for (int i = 0; this->currencies[i]; ++i)
			free(this->currencies[i]);

		free(this->currencies);
	}
}

ApplicationSettings::read_config_file()
{
	int fd = -1;

	fd = open(PATH_CONFIG_FILE, O_RDONLY);
	if (0 > fd)
		throw std::exception("Failed to open config file");

	struct stat st;

	if (lstat(fd, &st) < 0)
		throw std::exception("Failed to get config file stats");

	char *buffer = malloc(st.st_size+1);
	assert(nullptr != buffer);

	buffer[st.st_size] = 0;
	size_t to_read = st.st_size;
	size_t bytes_read = 0;
	char *p = buffer;

	while (0 < to_read && (bytes_read = read(fd, p, to_read)))
	{
		p += bytes_read;
		to_read -= bytes_read;
	}

	rapidjson::Document d;
	d.Parse(buffer);
	rapidjson::Value& v = d["currencies"];

	assert(v.IsArray());

	this->currencies = calloc(v.Size()+1, sizeof(char *));
	assert(nullptr != this->currencies);
	this->currencies[v.Size()] = nullptr;

	for (rapidjson::SizeType i = 0, n = v.Size(); i < n; ++i)
	{
		std::string currency = v[i].GetString();
		this->currencies[i] = calloc(currency.length()+1, 1);
		memcpy(this->currencies[i], currency.data(), currency.length());
	}
}

bool ApplicationSettings::is_valid_currency(std::string currency)
{
	if (!this->currencies)
		return false;

	for (int i = 0; this->currencies[i]; ++i)
	{
		if (!strcmp(currency, this->currencies[i]))
			return true;
	}

	return false;
}

char **ApplicationSettings::get_currencies()
{
	return this->currencies;
}
