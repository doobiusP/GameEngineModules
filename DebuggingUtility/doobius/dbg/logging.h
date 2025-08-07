#pragma once
#include <string>
#include <source_location>
#include <filesystem>

#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/stacktrace/stacktrace.hpp>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

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

// Don't know how useful these defines are going to be, so I'll leave them here for now
#if defined(_WIN32)
#define DOOBIUS_WIN32
#else
#define DOOBIUS_WIN64
#endif

#if defined(_DEBUG)
#define DOOBIUS_DEBUG
#else
#define DOOBIUS_NDEBUG
#endif

#define DOOBIUS_LOG_MNG() Doobius::Log::LogManager::get()

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

namespace Doobius {
	namespace Log {
		class LogManager {
		private:
			LogManager();
			~LogManager() = default;

			const std::filesystem::path m_nameOfLogConfigFile;
			std::filesystem::path m_fullLogDir;
			bool m_setup;
		public:
			using ModuleLogger = src::severity_channel_logger_mt< severity_level, std::string >;

			LogManager(const LogManager&) = delete;
			LogManager& operator=(const LogManager&) = delete;
			LogManager(LogManager&&) = delete;
			LogManager& operator=(LogManager&&) = delete;

			static LogManager& get();

			void initLogging(const std::filesystem::path& logDir);

			// TODO: Implement createSubsystemLogger
			ModuleLogger createSubsystemLogger(const std::string& subsystemName) const;

			inline const std::filesystem::path& getLogDir() const { return m_fullLogDir; }
		};
	}
}
