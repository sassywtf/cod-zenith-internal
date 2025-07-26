#include "utility.h"
#include "../globals.h"
#include "../spoofer/callstack.h"

//void log_shit(const char* str, ...)
//{
//	va_list ap;
//	char buf[256];
//	va_start(ap, str);
//	vsprintf_s(buf, str, ap);
//	va_end(ap);
//	strcat_s(buf, "\n");
//
//	// Get the current time
//	std::time_t now = std::time(nullptr);
//	char timestamp[20];
//	std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
//
//	// Append the timestamp to the log message
//	std::string logMessage = "[" + std::string(timestamp) + "] " + std::string(buf);
//
//	// Output the log message
//	std::cout << logMessage;
//
//	// Send the log message to the desired output, e.g., Notepad
//	HWND notepad, edit;
//	notepad = FindWindowA(NULL, "Untitled - Notepad");
//	if (!notepad)
//		notepad = FindWindowA(NULL, "*Untitled - Notepad");
//
//	edit = FindWindowEx(notepad, NULL, XOR(L"EDIT"), NULL);
//	SendMessageA(edit, EM_REPLACESEL, 0, (LPARAM)logMessage.c_str());
//}

//bool MemCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
//	for (; *szMask; ++szMask, ++bData, ++bMask) {
//		if (*szMask == 'x' && *bData != *bMask) {
//			return false;
//		}
//	}
//	return (*szMask == NULL);
//}

//DWORD utility::GetProcId(const wchar_t* procName)
//{
//	DWORD procId = 0;
//	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//	if (hSnap != INVALID_HANDLE_VALUE)
//	{
//		PROCESSENTRY32 procEntry;
//		procEntry.dwSize = sizeof(procEntry);
//
//		if (Process32First(hSnap, &procEntry))
//		{
//			do
//			{
//				if (!_wcsicmp(procEntry.szExeFile, procName))
//				{
//					procId = procEntry.th32ProcessID;
//					break;
//				}
//			} while (Process32Next(hSnap, &procEntry));
//		}
//	}
//	CloseHandle(hSnap);
//	return procId;
//}

//uintptr_t utility::PatternScanEx(uintptr_t start, uintptr_t size, const char* sig, const char* mask)
//{
//	BYTE* data = reinterpret_cast<BYTE*>(start);
//
//	for (uintptr_t i = 0; i < size; i++)
//	{
//		if (MemCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
//			return start + i;
//		}
//	}
//
//	return NULL;
//}
//
//uintptr_t utility::FindOffset(uintptr_t start, uintptr_t size, const char* sig, const char* mask, uintptr_t base_offset, uintptr_t pre_base_offset, uintptr_t rindex, bool addRip)
//{
//	// Use the modified PatternScanEx function that doesn't use hProcess
//	auto address = utility::PatternScanEx(start, size, sig, mask);
//	if (!address)
//		return 0;
//
//	address += rindex;
//
//	// Access the memory directly within the current process
//	auto ret = pre_base_offset + *reinterpret_cast<int32_t*>(address + base_offset);
//
//	if (addRip)
//	{
//		ret = ret + address;
//		if (ret)
//			return (ret - game_data->baseModule);
//	}
//
//	return ret;
//}

address_t utility::find_ida_sig(const char* mod, const char* sig)
{
    static auto pattern_to_byte = [](const char* pattern) {
        std::vector<int> bytes;
        const char* end = pattern + strlen(pattern);
        for (const char* current = pattern; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?') ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, const_cast<char**>(&current), 16));
            }
        }
        return bytes;
        };

    const auto module_handle = GetModuleHandleA(mod);
    if (!module_handle)
        return {};

    MODULEINFO module_info;
    GetModuleInformation(GetCurrentProcess(), module_handle, &module_info, sizeof(MODULEINFO));
    const auto image_size = module_info.SizeOfImage;
    if (!image_size)
        return {};

    auto pattern_bytes = pattern_to_byte(sig);
    auto* image_bytes = reinterpret_cast<byte*>(module_handle);
    auto signature_size = pattern_bytes.size();
    auto* signature_bytes = pattern_bytes.data();

    PSAPI_WORKING_SET_EX_INFORMATION ws_info;
    SIZE_T page_size = 4096;

    for (byte* current_address = image_bytes;
        current_address < image_bytes + image_size;
        current_address += page_size)
    {
        ws_info.VirtualAddress = reinterpret_cast<PVOID>(current_address);

        if (!QueryWorkingSetEx(GetCurrentProcess(), &ws_info, sizeof(ws_info)))
        {
            continue;
        }

        if (!ws_info.VirtualAttributes.Valid)
        {
            //LOG("found trap page\n");
            continue;
        }

        unsigned long protection = ws_info.VirtualAttributes.Win32Protection;
        if (protection != PAGE_EXECUTE_READ && protection != PAGE_EXECUTE_READWRITE)
        {
            continue;
        }

        for (const byte* current_byte = current_address;
            current_byte < current_address + page_size - signature_size;
            ++current_byte)
        {
            bool match = true;
            for (size_t i = 0; i < signature_size; ++i)
            {
                if (signature_bytes[i] != -1 && signature_bytes[i] != current_byte[i])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                return address_t(reinterpret_cast<uintptr_t>(current_byte));
            }
        }
    }

    //log_shit("Signature not found: %s", sig);

    return {};
}