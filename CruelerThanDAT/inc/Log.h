#pragma once
#include "pch.hpp"

#include <imgui.h>

#include "CodIcons.h"

namespace CTDLog {

    struct LogEntry {
        std::string text;
        ImVec4 color;

        LogEntry(std::string itext, ImVec4 icolor = { 1.0f, 1.0f, 1.0f, 1.0f });
    };

    class Log {
    private:
        Log() = default;
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

    public:
        static Log& getInstance();

        std::vector<LogEntry*> logEntries;
        
        std::string GetTime();
        void LogNote(std::string text);
        void LogWarning(std::string text);
        void LogError(std::string text);
        void LogUpdate();
    };
}