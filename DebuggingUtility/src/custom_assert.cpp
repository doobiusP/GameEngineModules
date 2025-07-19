#pragma once
#include "stacktrace.h"
#include <iostream>

namespace boost
{
#if defined(BOOST_ASSERT_HANDLER_IS_NORETURN)
	BOOST_NORETURN
#endif
	void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line) {
		std::cout << COLOR_RED;
		std::cout << "---------------- ASSERT TRIGGERED ----------------\n";
		std::cout << COLOR_RESET;
		std::cout << "[EXPR]: " << expr << '\n';
		std::cout << "[MSG]: " << msg << '\n';
		std::cout << "[FUNC]: " << function << '\n';
		std::cout << "[FILE]: " << file << '\n';
		std::cout << "[LINE]: " << line << '\n';

		PRINT_STACKTRACE(FATAL);
	}
}

