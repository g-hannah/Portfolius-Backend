#include "filehandler.h"

portfolius::FileHandler::FileHandler(std::string path)
	: path(path)
{
}

portfolius::FileHandler::FileHandler(FileHandler& other)
{
	this->fd = other.fd;
	this->path = other.path;
	this->size = other.size;
	this->error = other.error;
	this->reason = other.reason;
}

portfolius::FileHandler::FileHandler(FileHandler&& other)
{
	this->fd = other.fd;
	this->path = std::move(other.path);
	this->size = other.size;
	this->error = other.error;
	this->reason = std::move(other.reason);
}

portfolius::FileHandler& operator=(portfolius::FileHandler& other)
{
	this->fd = other.fd;
	this->path = other.path;
	this->size = other.size;
	this->error = other.error;
	this->reason = other.reason;

	return *this;
}

portfolius::FileHandler& operator=(portfolius::FileHandler&& other)
{
	this->fd = other.fd;
	this->path = std::move(other.path);
	this->size = other.size;
	this->error = other.error;
	this->reason = std::move(other.reason);

	return *this;
}

portfolius::FileHandler::~FileHandler()
{
	::close(this->fd);
	this->fd = -1;
}

bool portfolius::FileHandler::is_error()
{
	bool err = this->error;
	this->error = false;
	return err;
}

void portfolius::FileHandler::open_file()
{
	//static char buf[128];

	this->fd = ::open(this->path.c_str(), O_RDWR);

	if (0 > this->fd)
	{
		//snprintf(buf, 128, "Failed to open file \"%s\"", this->path.c_str());
		//goto end;
		//throw std::runtime_error(buf);
		this->error = true;
		this->reason = "failed to open file";
		return;
	}

	struct stat st;
	if (fstat(this->fd, &st) < 0)
	{
		//snprintf(buf, 128, "Failed to lstat() file \"%s\"", this->path.c_str());
		//goto end;
		//throw std::runtime_error(buf);
		this->error = true;
		this->reason = "failed to lstat() file";
		return;
	}

	this->size = st.st_size;
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
