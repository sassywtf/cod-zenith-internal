#pragma once

#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <timeapi.h>
#include "../gui/imgui/imgui.h"
#include "../gui/imgui/imgui_internal.h"
#include <dxgi1_4.h>

#include "../gui/imgui/imgui_impl_win32.h"


#include <unordered_map>
#include <map>
#include <sysinfoapi.h>
#include <filesystem>

#include "../protect/xorstr.hpp"
#include "../util/lazyimporter.h"
#include "../util/defs.h"

#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")

#include "../hooks/minhook/include/MinHook.h"
#include "../shadow/sys.h"

#define QWORD unsigned __int64

#ifndef  NDEBUG
#define LOGS(fmt, ...) Log_(XOR(fmt), ##__VA_ARGS__)

#define LOGS_ADDR(var_name)										\
		LOGS(#var_name ": 0x%llX (0x%llX)", var_name, var_name > game_data->baseModule - 0x10000 ? var_name - game_data->baseModule : 0);

extern void Log_(const char* fmt, ...);
#endif NDEBUG

#define _PTR_MAX_VALUE ((PVOID)0x000F000000000000)
#define BYTEn(x, n)   (*((BYTE*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)

//auto padding
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

#define is_valid_ptr(p) ((uintptr_t)(p) <= 0x7FFFFFFEFFFF && (uintptr_t)(p) >= 0x1000) 
#define is_bad_ptr(p)	(!is_valid_ptr(p))

#define OFFSET(type, func, offset) \
    type& func() { return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); }