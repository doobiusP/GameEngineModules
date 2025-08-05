#include "observer.h"

namespace CommonUtil {
	namespace Notification {
		void ConfigNotifiee::onNotify(const json::value& config, const std::string& notifierName)
		{
			DOOBIUS_CLOG(trace) << notifieeName << " got notified by " << notifierName;

			// Print all entries recursively
			//for (auto it = config.begin(); it != config.end(); ++it) {
			//	DOOBIUS_CLOG(trace) << "Key: " << it.key() << ", Value: " << it.value();
			//}

			// Optional: pretty-print full JSON
			//DOOBIUS_CLOG(trace) << "Full JSON payload:\n" << config.dump(4); // 4 = indentation
		}

		void ConfigNotifiee::onNotify(const int& val, const std::string& notifierName)
		{
			DOOBIUS_CLOG(trace) << notifieeName << " got notified by " << notifierName << " with value of " << val;;
		}

		ConfigNotifiee::ConfigNotifiee(const char* moduleName) : NotifieeCommon(moduleName)
		{
			DOOBIUS_CLOG(trace) << "ConfigNotifiee " << moduleName << " now created";
		}
	};
};