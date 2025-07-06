#include "pch.hpp"
#include "Log.h"

#include <imgui.h>

#include "CodIcons.h"

namespace CTDLog {

	LogEntry::LogEntry(std::string itext, ImVec4 icolor) {
		text = itext;
		color = icolor;
	}

	Log& Log::getInstance() {
		static Log instance;
		return instance;
	}
	
	std::string Log::GetTime() {
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

	void Log::LogNote(std::string text) {

		logEntries.push_back(new LogEntry(std::string(ICON_CI_INFO) + GetTime() + text));
	}

	void Log::LogWarning(std::string text) {
		logEntries.push_back(new LogEntry(std::string(ICON_CI_WARNING) + GetTime() + text, { 1, 1, 0, 1 }));
	}

	void Log::LogError(std::string text) {
		logEntries.push_back(new LogEntry(std::string(ICON_CI_ERROR) + GetTime() + text, { 1, 0, 0.329f, 1 }));
	}

	void Log::LogUpdate() {
		logEntries.push_back(new LogEntry("update", { 0, 1, 0, 1 }));
	}
}