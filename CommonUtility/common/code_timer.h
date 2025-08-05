#pragma once
#include "dbg/logging.h"

namespace CommonUtil {
	namespace Perf {
		class CodeTimer {
		private:
			std::string m_timerName;
			std::chrono::high_resolution_clock::time_point m_start;
		public:
			CodeTimer(const char* name);
			long long end() const;
		};
	};
};
