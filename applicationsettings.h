#include "rapidjson/document.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define PATH_CONFIG_FILE	"./settings.json"

/**
 * A singleton class that manages
 * application settings
 *
 * @author Gary Hannah
 */
namespace portfolius
{
	class ApplicationSettings
	{
		public:
			static ApplicationSettings *instance()
			{
				if (!_instance)
					_instance = new ApplicationSettings;

				return _instance;
			}

			ApplicationSettings();
			virtual ~ApplicationSettings();
			bool is_valid_currency(std::string);
			char **get_currencies();

		private:
			static ApplicationSettings *_instance;

			void read_config_file();

			char **currencies = 0;
	};
}
