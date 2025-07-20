#include "observer.h"

namespace CommonUtil {
	namespace Notification {
		void ConfigNotifiee::onNotify(const json& config, const std::string& notifierName)
		{
			LOG(INFO) << notifieeName << " got notified by " << notifierName << '\n';

			// Print all entries recursively
			for (auto it = config.begin(); it != config.end(); ++it) {
				LOG(INFO) << "Key: " << it.key() << ", Value: " << it.value();
			}

			// Optional: pretty-print full JSON
			LOG(INFO) << "Full JSON payload:\n" << config.dump(4); // 4 = indentation
		}

		void ConfigNotifiee::onNotify(const int& val, const std::string& notifierName)
		{
			LOG(INFO) << notifieeName << " got notified by " << notifierName << " with value of " << val << '\n';
		}

		ConfigNotifiee::ConfigNotifiee(const char* moduleName) : NotifieeCommon(moduleName)
		{
			LOG(INFO) << "ConfigNotifiee " << moduleName << " now created\n";
		}

	};
};