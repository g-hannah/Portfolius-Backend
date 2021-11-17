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
			explicit FileHandler(std::string);
			FileHandler(FileHandler& other);
			FileHandler(FileHandler&& other);
			FileHandler& operator=(FileHandler& other);
			FileHandler& operator=(FileHandler&& other);
			virtual ~FileHandler();

			std::string read_all();
			void open_file();
			bool is_error();

		private:
			std::string path;
			int fd;
			size_t size;
			bool error = false;
			std::string reason;
	};
}
#endif
