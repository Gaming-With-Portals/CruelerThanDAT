#pragma once
#include "string"
#include "vector"
#include "../imgui.h"
#include "../Assets/CodIcons.h"

namespace CTDLog {

    struct LogEntry {
        std::string text;
        ImVec4 color;

        LogEntry(std::string itext, ImVec4 icolor = { 1.0f, 1.0f, 1.0f, 1.0f }) {
            text = itext;
            color = icolor;
        }
    };

    class Log {
    private:
        Log() = default;
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

    public:
        static Log& getInstance() {
            static Log instance;
            return instance;
        }

        std::vector<LogEntry*> logEntries;
        
        std::string GetTime() {
            SYSTEMTIME st;

            GetLocalTime(&st);

            std::string hour = std::to_string(st.wHour);
            std::string ticker = "AM";
            if (st.wHour > 12) {
                hour = std::to_string(st.wHour - 12);
                ticker = "PM";
            }


            return " [" + hour + ":" + std::to_string(st.wMinute) + ":" + std::to_string(st.wSecond) + " " + ticker + "] ";

        }

        void LogNote(std::string text) {

            logEntries.push_back(new LogEntry(std::string(ICON_CI_INFO) + GetTime() + text));
        }

        void LogWarning(std::string text) {
            logEntries.push_back(new LogEntry(std::string(ICON_CI_WARNING) + GetTime() + text, { 1, 1, 0, 1 }));
        }

        void LogError(std::string text) {
            logEntries.push_back(new LogEntry(std::string(ICON_CI_ERROR) + GetTime() + text, { 1, 0, 0.329f, 1 }));
        }

    };
}