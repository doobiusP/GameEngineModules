#pragma once
#include "logging.h"
#include <windows.h>
#include <string>
#include <vector>

#define PRINT_STACKTRACE(SEV) DebugUtil::print_stacktrace(__FILE__, __LINE__, google::GLOG_##SEV);

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
};