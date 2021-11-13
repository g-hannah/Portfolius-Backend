#include "applicationsettings.h"

portfolius::ApplicationSettings *portfolius::ApplicationSettings::_instance = 0;

portfolius::ApplicationSettings::ApplicationSettings()
{
	try
	{
		this->_read_config_file();
	}
	catch (std::runtime_error e)
	{
		std::cerr << e.what() << std::endl;
	}
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

	fd = ::open(PATH_CONFIG_FILE, O_RDONLY);
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

	rapidjson::Document d;
	d.Parse(buffer);
	rapidjson::Value& v = d["currencies"];

	assert(v.IsArray());

	this->currencies = (char **)calloc(v.Size()+1, sizeof(char *));
	assert(this->currencies);
	this->currencies[v.Size()] = nullptr;

	for (rapidjson::SizeType i = 0, n = v.Size(); i < n; ++i)
	{
		std::string currency = v[i].GetString();
		this->currencies[i] = (char *)calloc(currency.length()+1, 1);
		std::memcpy(this->currencies[i], currency.c_str(), currency.length());

		char *key = currency.c_str();
		std::vector<portfolius::Rate*> *vec1 = new std::vector<portfolius::Rate*>;
		std::vector<portfolius::Rate*> *vec2 = new std::vector<portfolius::Rate*>;
		this->_map_primary[key] = vec1;
		this->_map_secondary[key] = vec2;
	}
}

char **portfolius::ApplicationSettings::get_currencies()
{
	return this->currencies;
}
