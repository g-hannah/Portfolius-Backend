#include "filehandler.h"

portfolius::FileHandler::FileHandler(std::string path)
	: path(path)
{
	static char buf[128];

	this->fd = ::open(this->path.c_str(), O_RDWR);

	if (0 > this->fd)
	{
		snprintf(buf, 128, "Failed to open file \"%s\"", this->path.c_str());
		throw std::runtime_error(buf);
	}

	struct stat st;
	if (fstat(this->fd, &st) < 0)
	{
		snprintf(buf, 128, "Failed to lstat() file \"%s\"", this->path.c_str());
		throw std::runtime_error(buf);
	}

	this->size = st.st_size;
}

portfolius::FileHandler::~FileHandler()
{
	::close(this->fd);
	this->fd = -1;
}

std::string portfolius::FileHandler::read_all()
{
	size_t to_read = this->size;
	ssize_t bytes_read = 0;
	char *buffer = (char *)malloc(to_read+1);

	assert(buffer);

	char *p = buffer;

	while (to_read && (bytes_read = ::read(this->fd, p, to_read)) > 0)
	{
		to_read -= bytes_read;
		p += bytes_read;
	}

	*p = 0;

	std::string ret = buffer;

	free(buffer);
	buffer = p = nullptr;

	return ret;
}
