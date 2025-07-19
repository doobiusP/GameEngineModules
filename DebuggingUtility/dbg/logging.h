#pragma once
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>

#include <iostream>

constexpr auto COLOR_RESET = "\033[0m";
constexpr auto COLOR_RED = "\033[31m";
constexpr auto COLOR_YELLOW = "\033[33m";
constexpr auto COLOR_BLUE = "\033[34m";
constexpr auto COLOR_WHITE = "\033[37m";

#define COLOR_LOG(SEV, MSG) \
    std::cout << DebugUtil::get_glog_color(google::GLOG_##SEV); \
    LOG(SEV) << MSG; \
    std::cout << COLOR_WHITE;

namespace DebugUtil {
    const char* get_glog_color(google::LogSeverity severity);
    void setup_glog(const char* argv0);
    void shutdown_glog();
};