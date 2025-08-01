#ifndef NDEBUG
#include "code_timer.h"

namespace CommonUtil {
	namespace Perf {
		CodeTimer::CodeTimer(const char* name) : m_timerName(name), m_start(std::chrono::high_resolution_clock::now()) {
		}

		long long CodeTimer::end() const {
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

			std::ostringstream oss;
			oss << m_timerName << ": " << duration.count() << " ms\n";
			LOG(INFO) << oss.str();

			return duration.count();
		}
	};
};

#endif /* NDEBUG */