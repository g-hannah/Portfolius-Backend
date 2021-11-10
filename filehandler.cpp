#include "filehandler.h"

FileHandler::FileHandler(std::string path)
	: path(path)
{
	this->fd = open(this->path, O_RDWR);
	if (0 > this->fd)
		throw std::exception("Failed to open file " + this->path);

	struct stat st;
	if (fstat(this->fd, &st) < 0)
		throw std::exception("Failed to lstat() file");

	this->size = st.st_size;
}

FileHandler::~FileHandler()
{
	close(this->fd);
	this->fd = -1;
}

std::string FileHandler::read_all()
{
	size_t to_read = this->size;
	ssize_t bytes_read = 0;
	char *buffer = malloc(to_read+1);

	assert(buffer);

	char *p = buffer;

	while (to_read && (bytes_read = read(this->fd, p, to_read)) > 0)
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
