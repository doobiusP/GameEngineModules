#include "logging.h"
#include <fstream>

#include <boost/core/null_deleter.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/json.hpp>
namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", std::uint_least32_t)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(timeline, "Timeline", attrs::timer::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type)

namespace Dbg {
	namespace Log {
		class Terminal {
		public:
			static constexpr const char* reset() { return "\033[0m"; }
			static constexpr const char* red() { return "\033[31m"; }
			static constexpr const char* yellow() { return "\033[33m"; }
			static constexpr const char* blue() { return "\033[34m"; }
			static constexpr const char* white() { return "\033[37m"; }
		};

		const char* getSeverityColor(severity_level sev) {
			switch (sev) {
			case severity_level::trace:
			case severity_level::debug:
			case severity_level::info:
				return Terminal::white();
			case severity_level::warning:
				return Terminal::yellow();
			case severity_level::error:
			case severity_level::fatal:
				return Terminal::red();
			default:
				return Terminal::reset();
			}
		}

		const std::string& getConfigString() {
#if defined(DBG_CONFIG)
			static const std::string configStr = "dbg";
#elif defined(REL_DEV_CONFIG)
			static const std::string configStr = "rel-dev";
#elif defined(REL_CONFIG)
			static const std::string configStr = "rel";
#else
			static const std::string configStr;
			assert(false && "Using non-standard build configuration");
#endif
			return configStr;
		}

		static struct LogFileSetting {
			severity_level minSeverity = severity_level::trace;
			severity_level minConsoleLogSeverity = severity_level::info;
			std::string logFilePrefix = "default-dbg";
			int rotationSizeInMb = 10;
		} logFileSetting;

		/**
		 * \brief Note that this function modifies the severity string in-place to become all lowercase.
		 *
		 * \param sevStr
		 * \return
		 */
		severity_level parseSev(const std::string_view& sevStr) {
			if (sevStr == "trace") {
				return severity_level::trace;
			}
			else if (sevStr == "debug") {
				return severity_level::debug;
			}
			else if (sevStr == "info") {
				return severity_level::info;
			}
			else if (sevStr == "warning") {
				return severity_level::warning;
			}
			else if (sevStr == "error") {
				return severity_level::error;
			}
			else if (sevStr == "fatal") {
				return severity_level::fatal;
			}
			else {
				BOOST_LOG_TRIVIAL(info) << "Received invalid severity string = " << sevStr;
				assert(false && "Attempting to parse severity string not present in trivial::severity_level. Ensure all lowercase");
				return severity_level::fatal;
			}
		}

		std::string getBuildEnvironmentString() {
			std::string buildConfigStr = "Runtime Environment: [CONFIG = ";
#if defined(DBG_CONFIG)
			buildConfigStr += "DBG]";
#elif defined(REL_DEV_CONFIG)
			buildConfigStr += "REL-DEV]";
#elif defined(REL_CONFIG)
			buildConfigStr += "REL]";
#else
			buildConfigStr += "UNKNOWN]";
#endif
			buildConfigStr += "[ARCHITECTURE = ";
#if defined(_WIN64)
			buildConfigStr += "x64]";
#else
			buildConfigStr += "x86]";
#endif
			return buildConfigStr;
		}

		void readSettingsFromJson(json::value const& logConfigJson) {
			BOOST_LOG_TRIVIAL(info) << "Now reading configuration present in the config file...";
			json::object const& root = logConfigJson.as_object();

			// Minimum Log Severity
			json::object const& minSev = root.at("min_severity").as_object();
			if (auto minSevPtr = minSev.if_contains(getConfigString())) {
				logFileSetting.minSeverity = parseSev(minSevPtr->as_string());
			}
			else {
				BOOST_LOG_TRIVIAL(warning) << "Couldn't find min_severity for config=" << getConfigString() << ". Using default.";
			}

			// Minimum Log Severity for Console Logging
			json::object const& cMinSev = root.at("console_min_severity").as_object();
			if (auto cMinSevPtr = cMinSev.if_contains(getConfigString())) {
				logFileSetting.minConsoleLogSeverity = parseSev(cMinSevPtr->as_string());
			}
			else {
				BOOST_LOG_TRIVIAL(warning) << "Couldn't find console_min_severity for config=" << getConfigString() << ". Using default.";
			}

			// Log File Prefix for the log file into which records are stored
			json::object const& logFilePrefix = root.at("log_file_prefix").as_object();
			if (auto lfpPtr = logFilePrefix.if_contains(getConfigString())) {
				logFileSetting.logFilePrefix = lfpPtr->as_string();
			}
			else {
				BOOST_LOG_TRIVIAL(warning) << "Couldn't find log_file_prefix for config=" << getConfigString() << ". Using default.";
			}

			// Rotation Size (MB) of the log file
			json::object const& rotationSize = root.at("rotation_size").as_object();
			if (auto rotSizePtr = rotationSize.if_contains(getConfigString())) {
				logFileSetting.rotationSizeInMb = rotSizePtr->as_int64(); // TODO: Compiler gives warning here due to narrowing conversion. Figure out a better way to do this.
			}
			else {
				BOOST_LOG_TRIVIAL(warning) << "Couldn't find rotation_size for config=" << getConfigString() << ". Using default.";
			}

			BOOST_LOG_TRIVIAL(info) << "Done parsing config file";
		}

		void consoleLogRecordFormat(logging::record_view const& rec, logging::formatting_ostream& strm)
		{
			strm << getSeverityColor(*rec[severity]);
			strm << "<" << rec[severity] << "> ";

			strm << "[" << rec[file] << ":" << rec[line] << "]"; // TODO: Make rec[line] and rec[file] default to "?" in case its not present
			strm << "[" << rec[thread_id] << "] ";

			strm << "|";
			if (auto channelPtr = rec[channel])
				strm << *channelPtr;
			strm << ":";
			if (auto scopePtr = rec[scope])
				strm << *scopePtr;
			strm << ":";
			if (auto tagPtr = rec[tag_attr])
				strm << *tagPtr;
			strm << ":";
			if (auto timelinePtr = rec[timeline])
				strm << *timelinePtr;
			strm << "| ";

			strm << "\t";
			strm << rec[expr::smessage];
			strm << Terminal::reset();
		}

		void fileLogRecordFormat(logging::record_view const& rec, logging::formatting_ostream& strm)
		{
			strm << "<" << rec[severity] << "> ";
			strm << "[" << rec[line_id] << "]";

			strm << "[" << rec[file] << ":" << rec[line] << "]"; // TODO: Make rec[line] and rec[file] default to "?" in case its not present
			strm << "[" << rec[thread_id] << "]";
			auto ts = rec[timestamp];
			strm << "[" << boost::posix_time::to_simple_string(*ts) << "] ";

			strm << "|";
			if (auto channelPtr = rec[channel])
				strm << *channelPtr;
			strm << ":";
			if (auto scopePtr = rec[scope])
				strm << *scopePtr;
			strm << ":";
			if (auto tagPtr = rec[tag_attr])
				strm << *tagPtr;
			strm << ":";
			if (auto timelinePtr = rec[timeline])
				strm << *timelinePtr;
			strm << "| ";

			strm << "\t";
			strm << rec[expr::smessage];
		}

		void setupConsoleSink()
		{
			// I am aware add_console_log() exists but I'm doing this because I want to understand how making sinks works
			typedef sinks::text_ostream_backend text_stream;
			typedef sinks::synchronous_sink< text_stream > sink_t;

			boost::shared_ptr< logging::core > core = logging::core::get();
			core->add_global_attribute("Scope", attrs::named_scope());
			logging::add_common_attributes();

			boost::shared_ptr< sink_t > clSink = boost::make_shared< sink_t >();
			clSink->locked_backend()->add_stream(boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

			clSink->set_filter(severity >= logFileSetting.minConsoleLogSeverity);
			clSink->set_formatter(&consoleLogRecordFormat);

			BOOST_LOG_TRIVIAL(info) << "Switching to new console logger";
			core->add_sink(clSink);
		}

		void setupFileSink(std::filesystem::path const& logDir) {
			BOOST_LOG_NAMED_SCOPE("SetupFileSink");
			DOOBIUS_CLOG(info) << "Resolved directory to store log files at " << logDir.string();

			bool newlyCreatedLogDir = std::filesystem::create_directories(logDir);
			if (newlyCreatedLogDir) {
				DOOBIUS_CLOG(info) << "Directory to store log files not previously created. Created new directory";
			}
			else {
				DOOBIUS_CLOG(info) << "Directory to store log files previously created. Using existing directory";
			}
			assert(std::filesystem::exists(logDir) && "logDir provided still does not exist after creation");

			std::string logFileName = logFileSetting.logFilePrefix + "_%N.log";
			std::filesystem::path logFilePath = logDir / logFileName;
			DOOBIUS_CLOG(info) << "Log file(s) will be generated as " << logFilePath.string();

			logging::add_file_log
			(
				keywords::file_name = logFilePath,
				keywords::rotation_size = logFileSetting.rotationSizeInMb * 1024 * 1024,
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
				keywords::format = &fileLogRecordFormat,
				keywords::filter = severity >= logFileSetting.minSeverity
			);
			DOOBIUS_CLOG(info) << "Finished setting up file sink";
		}

		cLoggerRef& Dbg::Log::getLogger()
		{
			static cLoggerRef _cLogger;
			return _cLogger;
		}

		// TODO: Also accept some kind of module name so that you can have <config>_<module>_%N.log
		void initLogging(std::filesystem::path const& logDir)
		{
			const std::filesystem::path nameOfLogConfigFile = "log_config.json";
			std::filesystem::path logConfigPath = PATH_TO_CONFIGS_DIR / nameOfLogConfigFile;

			std::ifstream logConfigStream(logConfigPath);
			if (logConfigStream.fail()) {
				BOOST_LOG_TRIVIAL(warning) << "Couldn't find/open " << logConfigPath.string() << ". Reverting to default log configuration";
			}
			else {
				BOOST_LOG_TRIVIAL(info) << "Found " << logConfigPath.string() << ".";

				std::ostringstream tempFileContents;
				tempFileContents << logConfigStream.rdbuf();

				json::value logConfigJson = json::parse(tempFileContents.str());
				readSettingsFromJson(logConfigJson);
			}
			// At this point, logFileSetting is valid and up-to-date
			setupConsoleSink();
			DOOBIUS_CLOG(info) << getBuildEnvironmentString();

			setupFileSink(logDir);
		}
	}
}