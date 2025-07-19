#include "stacktrace.h"
#include <dbghelp.h>
#include <sstream>
#include <filesystem>

#pragma comment(lib, "dbghelp.lib")

namespace DebugUtil
{
    static std::string basename(const std::string& file)
    {
        size_t i = file.find_last_of("\\/");
        return (i == std::string::npos) ? file : file.substr(i + 1);
    }

    std::vector<StackFrame> stack_trace()
    {
#if _WIN64
        DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
        DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        std::vector<StackFrame> frames;

        if (!SymInitialize(process, NULL, TRUE)) {
            return frames;
        }

        SymSetOptions(SYMOPT_LOAD_LINES);

        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&context);

        STACKFRAME frame = {};
#if _WIN64
        frame.AddrPC.Offset = context.Rip;
        frame.AddrFrame.Offset = context.Rbp;
        frame.AddrStack.Offset = context.Rsp;
#else
        frame.AddrPC.Offset = context.Eip;
        frame.AddrFrame.Offset = context.Ebp;
        frame.AddrStack.Offset = context.Esp;
#endif
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Mode = AddrModeFlat;

        bool skipFirst = true;

        while (StackWalk(machine, process, thread, &frame, &context, NULL,
            SymFunctionTableAccess, SymGetModuleBase, NULL)) {
            if (skipFirst) { skipFirst = false; continue; }

            StackFrame f = {};
            f.address = frame.AddrPC.Offset;

            DWORD64 moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);
            char moduleName[MAX_PATH] = "";
            if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduleName, MAX_PATH)) {
                f.module = basename(moduleName);
            }
            else {
                f.module = "Unknown";
            }

            char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
            auto* symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
            symbol->MaxNameLength = 254;

            DWORD64 offset = 0;
            if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol)) {
                f.name = symbol->Name;
            }
            else {
                f.name = "Unknown";
            }

            IMAGEHLP_LINE line;
            DWORD offset_ln = 0;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
            if (SymGetLineFromAddr(process, frame.AddrPC.Offset, &offset_ln, &line)) {
                f.file = line.FileName;
                f.line = line.LineNumber;
            }
            else {
                f.line = 0;
                f.file = "Unknown";
            }

            frames.push_back(f);
        }

        SymCleanup(process);
        return frames;
    }

    void print_stacktrace(const char* filename, int line, google::LogSeverity severity)
    {
        auto stack = stack_trace();
        auto& usefulFrame = stack[1];
        const char* sevColor = get_glog_color(severity);

        std::ostringstream oss;
        oss << "-----------STACKTRACE-----------\n"
            << "Caller: " << usefulFrame.name << ", File: "
            << usefulFrame.file << ", Line: " << std::dec
            << usefulFrame.line << ", Target: " << usefulFrame.module << '\n';

        for (const auto& frame : stack) {
            oss << "0x" << std::hex << frame.address
                << ": " << frame.name
                << " (" << std::dec << frame.line << ") in " << frame.module
                << " from " << frame.file << '\n';
        }

        std::cout << sevColor;
        if (severity < FLAGS_stderrthreshold) {
            std::cout << oss.str();
        }
        google::LogMessage(filename, line, severity).stream() << oss.str(); // Not using LOG() because want to pass in
                                                                            // caller's filename and line.
        std::cout << COLOR_WHITE;
    }

}