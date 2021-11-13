#ifndef __FILE_HANDLER_h__
#define __FILE_HANDLER_h__ 1

#include <assert.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

namespace portfolius
{
	class FileHandler
	{
		public:
			FileHandler(std::string);
			virtual ~FileHandler();
			std::string read_all();

		private:
			std::string path;
			int fd;
			size_t size;
	};
}
#endif
