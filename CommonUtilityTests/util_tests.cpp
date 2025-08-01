#define BOOST_TEST_MODULE My Test

#include "dbg/logging.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

struct LoggingFixture {
	LoggingFixture() {
		BOOST_LOG_NAMED_SCOPE("CommonUtilityLogFixture");
		Dbg::Log::initLogging("./CommonUtilityTestLogs");
	}
	~LoggingFixture() = default;
};

BOOST_TEST_GLOBAL_FIXTURE(LoggingFixture);

BOOST_AUTO_TEST_CASE(first_test)
{
	int i = 2;
	DOOBIUS_CLOG(info) << "i=" << i;
	BOOST_TEST(i == 2);
	BOOST_TEST(i == 1);
}