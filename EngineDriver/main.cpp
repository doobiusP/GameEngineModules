#include "dbg/custom_assert.h"

int main(int argc, char* argv[]) {
	BOOST_LOG_NAMED_SCOPE("EntryPoint");
	BOOST_LOG_SEV(Dbg::Log::getLogger(), severity_level::info) << "Hi!";
	std::filesystem::path logDir = std::filesystem::absolute(argv[0]).parent_path() / "EngineDriverLogs";
	Dbg::Log::initLogging(logDir);
	DOOBIUS_CLOG(trace) << "Trace msg";
	DOOBIUS_CLOG(debug) << "Debug msg";
	DOOBIUS_CLOG(info) << "Info msg";
	DOOBIUS_CLOG(warning) << "Warning msg";
	DOOBIUS_CLOG(error) << "Error msg";
	DOOBIUS_CLOG(fatal) << "Fatal msg";
	BOOST_LOG_TRIVIAL(info) << "Hi!";

	DOOBIUS_CLOG_STACKTRACE(info);
	BOOST_ASSERT_MSG(false, "This should never pass");
}
