#include "doobius/common/code_timer.h"

namespace Doobius {
	namespace Perf {
		CodeTimer::CodeTimer(const char* name) : m_timerName(name), m_start(std::chrono::high_resolution_clock::now()) {
			BOOST_LOG_NAMED_SCOPE("Timing");
			DOOBIUS_CLOG(debug) << "Started timer " << m_timerName;
		}

		long long CodeTimer::end() const {
			BOOST_LOG_NAMED_SCOPE("Timing");
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
			long long int microsecs = duration.count(), millisecs = 0, secs = 0;
			if (microsecs >= 1e3) {
				millisecs = microsecs / 1e3;
				microsecs -= millisecs * 1e3;
			}
			if (millisecs >= 1e3) {
				secs = millisecs / 1e3;
				millisecs -= secs * 1e3;
			}
			DOOBIUS_CLOG(debug) << m_timerName << " took " << secs << "s " << millisecs << "ms " << microsecs << "us to complete";

			return duration.count();
		}
	};
};