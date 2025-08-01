#pragma once
#include <string>
#include <string_view>
#include <source_location>
#include <filesystem>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/stacktrace/stacktrace.hpp>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
using severity_level = logging::trivial::severity_level;

/**
 * C in CLOG stands for custom
 */
#define DOOBIUS_CLOG(SEV) \
	BOOST_LOG_TRIVIAL(severity_level::SEV) \
	<< logging::add_value("Line", std::source_location::current().line()) \
	<< logging::add_value("File", std::filesystem::path(std::source_location::current().file_name()).filename().string())

#define DOOBIUS_CLOG_TAG(SEV, TAG) \
	DOOBIUS_CLOG(SEV) \
	<< logging::add_value("Tag", TAG)

#define DOOBIUS_LOG(LOGGER, SEV) \
	BOOST_LOG_SEV(LOGGER, severity_level::SEV) \
	<< logging::add_value("Line", std::source_location::current().line()) \
	<< logging::add_value("File", std::filesystem::path(std::source_location::current().file_name()).filename().string())

#define DOOBIUS_LOG_TAG(LOGGER, SEV, TAG) \
	DOOBIUS_LOG(LOGGER, SEV) \
	<< logging::add_value("Tag", TAG)

#define DOOBIUS_CLOG_STACKTRACE(SEV) \
	{ \
		BOOST_LOG_NAMED_SCOPE("Stacktrace"); \
		DOOBIUS_CLOG(SEV) << boost::stacktrace::basic_stacktrace(); \
	}

namespace Dbg {
	namespace Log {
		using cLoggerRef = src::severity_logger< severity_level >;

		/**
		 * \brief Unused for now. Using the trivial logger with my own formatter for now. In case I start needing a custom
		 * logger in the future, I can edit this one.
		 */
		cLoggerRef& getLogger();

		src::severity_channel_logger_mt< severity_level, std::string > createSubsystemLogger(const std::string& subsystemName);

		void initLogging(std::filesystem::path const& logDir);
	}
}
