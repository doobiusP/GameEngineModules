#include "logging.h"
#include <filesystem>

namespace DebugUtil {
    const char* get_glog_color(google::LogSeverity severity) {
        switch (severity) {
        case google::GLOG_INFO: return COLOR_WHITE;
        case google::GLOG_WARNING: return COLOR_YELLOW;
        case google::GLOG_ERROR: return COLOR_RED;
        case google::GLOG_FATAL: return COLOR_RED;
        default: return COLOR_WHITE;
        }
    }

    void setup_glog(const char* argv0)
    {
        std::filesystem::path exe_path = std::filesystem::absolute(argv0).parent_path();
        std::filesystem::path log_dir = exe_path / "logs"; // TODO: Obtain from config file
        std::filesystem::create_directories(log_dir);

        google::InitGoogleLogging(argv0);

        google::SetLogDestination(google::GLOG_INFO, (log_dir / "info_").string().c_str());
        google::SetLogDestination(google::GLOG_WARNING, (log_dir / "warning_").string().c_str());
        google::SetLogDestination(google::GLOG_ERROR, (log_dir / "error_").string().c_str());
        google::SetLogDestination(google::GLOG_FATAL, (log_dir / "fatal_").string().c_str());

        google::SetStderrLogging(google::GLOG_ERROR); // TODO: Obtain from config file
        LOG(INFO) << "GLOG system setup\n";
    }

    void shutdown_glog()
    {
        LOG(INFO) << "Shutting GLOG system down\n";
        google::ShutdownGoogleLogging();
    }

};