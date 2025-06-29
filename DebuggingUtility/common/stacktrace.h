#pragma once
#define GLOG_NO_ABBREVIATED_SEVERITIES
#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <glog/logging.h>
#include <boost/assert.hpp>
#include <windows.h>
#include <string>
#include <vector>

constexpr auto COLOR_RESET = "\033[0m";
constexpr auto COLOR_RED = "\033[31m";
constexpr auto COLOR_YELLOW = "\033[33m";
constexpr auto COLOR_BLUE = "\033[34m";
constexpr auto COLOR_WHITE = "\033[37m";

#define PRINT_STACKTRACE(severity) DebugUtil::print_stacktrace(__FILE__, __LINE__, severity);

namespace DebugUtil
{
    struct StackFrame {
        DWORD64 address;
        std::string name;
        std::string module;
        unsigned int line;
        std::string file;
    };

    std::vector<StackFrame> stack_trace();
    void print_stacktrace(const char* filename, int line, google::LogSeverity severity = google::GLOG_INFO);
    void setup_glog(const char* argv0);
};