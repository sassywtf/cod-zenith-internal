#pragma once
#include "utility.h"
#include "../address.h"

//void log_shit(const char* str, ...);
//
void log_shit(const char* str, ...)
{
	va_list ap;
	char buf[256];
	va_start(ap, str);
	vsprintf_s(buf, str, ap);
	va_end(ap);
	strcat_s(buf, "\n");

	// Get the current time
	std::time_t now = std::time(nullptr);
	char timestamp[20];
	std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	// Append the timestamp to the log message
	std::string logMessage = "[" + std::string(timestamp) + "] " + std::string(buf);

	// Output the log message
	std::cout << logMessage;

	// Send the log message to the desired output, e.g., Notepad
	HWND notepad, edit;
	notepad = FindWindowA(NULL, "Untitled - Notepad");
	if (!notepad)
		notepad = FindWindowA(NULL, "*Untitled - Notepad");

	edit = FindWindowEx(notepad, NULL, L"EDIT", NULL);
	SendMessageA(edit, EM_REPLACESEL, 0, (LPARAM)logMessage.c_str());
}

namespace utility
{
	//DWORD GetProcId(const wchar_t* procName);
	//uintptr_t PatternScanEx(uintptr_t start, uintptr_t size, const char* sig, const char* mask);
	//uintptr_t FindOffset(uintptr_t start, uintptr_t size, const char* sig, const char* mask, uintptr_t base_offset, uintptr_t pre_base_offset, uintptr_t rindex, bool addRip = true);
	address_t find_ida_sig(const char* mod, const char* sig);
}