#pragma once
#include "dbg/logging.h"

#ifndef NDEBUG
#include <chrono>
#endif

namespace CommonUtil {
	namespace Perf {
		class CodeTimer {
#ifdef NDEBUG
		public:
			CodeTimer(const char*) {};
			long long end() const { return 0; };
#else
		private:
			std::string m_timerName;
			std::chrono::high_resolution_clock::time_point m_start;
		public:
			CodeTimer(const char* name);
			long long end() const;
#endif
		};
	};
};
