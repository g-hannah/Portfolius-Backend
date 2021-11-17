#include "applicationsettings.h"

portfolius::ApplicationSettings *portfolius::ApplicationSettings::_instance = nullptr;

portfolius::ApplicationSettings::ApplicationSettings()
{
}

portfolius::ApplicationSettings::~ApplicationSettings()
{
	if (this->currencies)
	{
		for (int i = 0; this->currencies[i]; ++i)
			free(this->currencies[i]);

		free(this->currencies);
	}
}

void portfolius::ApplicationSettings::_read_config_file()
{
	int fd = -1;

	fd = open(PATH_CONFIG_FILE, O_RDONLY);
	std::cerr << "fd: " << fd << std::endl;
	if (0 > fd)
		throw std::runtime_error("Failed to open config file");

	struct stat st;

	if (fstat(fd, &st) < 0)
		throw std::runtime_error("Failed to get config file stats");

	char *buffer = (char *)malloc(st.st_size+1);
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

	*p = 0;
	std::cerr << buffer << std::endl;

	rapidjson::Document d;
	std::cerr << "parsing json in buffer" << std::endl;
	d.Parse(buffer);
	rapidjson::Value& v = d["currencies"];

	assert(v.IsArray());
	std::cerr << "currencies maps to array" << std::endl;

	this->currencies = (char **)calloc(v.Size()+1, sizeof(char *));
	assert(this->currencies);
	this->currencies[v.Size()] = nullptr;

	portfolius::ExchangeRatesManager *manager = portfolius::ExchangeRatesManager::instance();

	for (rapidjson::SizeType i = 0, n = v.Size(); i < n; ++i)
	{
		std::cerr << "IsObject(): " << v[i].IsObject() << std::endl;
		rapidjson::Value& v_currency = v[i]["name"];
		std::cerr << "IsObject(): " << v_currency.IsObject() << std::endl;
		std::cerr << "IsString(): " << v_currency.IsString() << std::endl;
		std::string currency = v_currency.GetString();

		std::cerr << "Got currency \"" << currency << "\"" << std::endl;
		this->currencies[i] = (char *)calloc(currency.length()+1, 1);
		std::memcpy(this->currencies[i], currency.c_str(), currency.length());

		const char *key = currency.c_str();
		std::vector<portfolius::Rate*> *vec1 = new std::vector<portfolius::Rate*>;
		std::vector<portfolius::Rate*> *vec2 = new std::vector<portfolius::Rate*>;

		manager->_map_primary[key] = vec1;
		manager->_map_secondary[key] = vec2;
	}

	std::cerr << "finished reading config file" << std::endl;
}

void portfolius::ApplicationSettings::_init()
{
	this->mutex.lock();
	if (!this->initialised)
	{
		try
		{
			this->_read_config_file();
		}
		catch (std::runtime_error e)
		{
			this->mutex.unlock();
			return;
		}

		this->initialised = true;
	}
	this->mutex.unlock();
}

char **portfolius::ApplicationSettings::get_currencies()
{
	this->_init();

	return this->currencies;
}
