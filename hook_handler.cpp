//#include "gui/framework.h"
//#include "spoofer/callstack.h"
//
//struct hooked_function {
//	std::string name;
//	PVOID address;
//	DWORD rva;
//};
//
//PVOID g_ntdll_base = nullptr;
//PVOID g_ntdll_disk_image = nullptr;
//
//auto get_main_module_address_range(PVOID& start, PVOID& end) -> bool {
//	if (auto h_mod = GetModuleHandleA(XOR("cod.exe"))) {
//		MODULEINFO mod_info;
//		if (GetModuleInformation(GetCurrentProcess(), h_mod, &mod_info, sizeof(mod_info))) {
//			start = mod_info.lpBaseOfDll;
//			end = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(mod_info.lpBaseOfDll) + mod_info.SizeOfImage);
//			return true;
//		}
//	}
//	//std::cerr << "failed to get handle to cod.exe" << std::endl;
//	return false;
//}
//
//auto is_jump_to_main_module(PVOID address, PVOID module_start, PVOID module_end) -> bool {
//	std::array<BYTE, 14> instruction;
//	std::memcpy(instruction.data(), address, instruction.size());
//
//	if (instruction[0] == 0xE9) {
//		auto jump_target = reinterpret_cast<uintptr_t>(address) + 5 + *reinterpret_cast<const DWORD*>(&instruction[1]);
//		return (jump_target >= reinterpret_cast<uintptr_t>(module_start) && jump_target < reinterpret_cast<uintptr_t>(module_end));
//	}
//	else if (instruction[0] == 0xFF && instruction[1] == 0x25) {  // Indirect jump
//		auto jump_target = *reinterpret_cast<const uintptr_t*>(reinterpret_cast<uintptr_t>(address) + 6 + *reinterpret_cast<const DWORD*>(&instruction[2]));
//		return (jump_target >= reinterpret_cast<uintptr_t>(module_start) && jump_target < reinterpret_cast<uintptr_t>(module_end));
//	}
//
//	return false;
//}
//
//auto load_ntdll_from_disk() -> bool {
//	std::array<char, MAX_PATH> ntdll_path;
//	GetSystemDirectoryA(ntdll_path.data(), static_cast<UINT>(ntdll_path.size()));
//	strcat_s(ntdll_path.data(), ntdll_path.size(), XOR("\\ntdll.dll"));
//
//	auto file_handle = CreateFileA(ntdll_path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
//	if (file_handle == INVALID_HANDLE_VALUE) {
//		//std::cerr << "failed to open ntdll.dll file" << std::endl;
//		return false;
//	}
//
//	auto mapping_handle = CreateFileMappingA(file_handle, nullptr, PAGE_READONLY | SEC_IMAGE, 0, 0, nullptr);
//	if (!mapping_handle) {
//		CloseHandle(file_handle);
//		//std::cerr << "failed to create file mapping" << std::endl;
//		return false;
//	}
//
//	g_ntdll_disk_image = MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
//	if (!g_ntdll_disk_image) {
//		CloseHandle(mapping_handle);
//		CloseHandle(file_handle);
//		//std::cerr << "failed to map view of file" << std::endl;
//		return false;
//	}
//
//	CloseHandle(mapping_handle);
//	CloseHandle(file_handle);
//	return true;
//}
//
//auto scan_ntdll_for_hooks() -> std::vector<hooked_function> {
//	std::vector<hooked_function> hooked_functions;
//	PVOID module_start, module_end;
//	if (!get_main_module_address_range(module_start, module_end)) {
//		return hooked_functions;
//	}
//
//	g_ntdll_base = GetModuleHandleA(XOR("ntdll.dll"));
//	if (!g_ntdll_base) {
//		//std::cerr << "failed to get handle to ntdll.dll" << std::endl;
//		return hooked_functions;
//	}
//
//	if (!load_ntdll_from_disk()) {
//		return hooked_functions;
//	}
//
//	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(g_ntdll_base);
//	auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(g_ntdll_base) + dos_header->e_lfanew);
//	auto exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<uintptr_t>(g_ntdll_base) +
//		nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
//
//	auto functions = reinterpret_cast<PDWORD>(reinterpret_cast<uintptr_t>(g_ntdll_base) + exports->AddressOfFunctions);
//	auto names = reinterpret_cast<PDWORD>(reinterpret_cast<uintptr_t>(g_ntdll_base) + exports->AddressOfNames);
//	auto ordinals = reinterpret_cast<PWORD>(reinterpret_cast<uintptr_t>(g_ntdll_base) + exports->AddressOfNameOrdinals);
//
//	for (DWORD i = 0; i < exports->NumberOfNames; i++) {
//		auto function_addr = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(g_ntdll_base) + functions[ordinals[i]]);
//		auto function_name = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(g_ntdll_base) + names[i]);
//
//		if (is_jump_to_main_module(function_addr, module_start, module_end)) {
//			hooked_functions.push_back({ function_name, function_addr, functions[ordinals[i]] });
//		}
//	}
//
//	return hooked_functions;
//}
//
//auto remove_hook(const hooked_function& hf) -> bool {
//	constexpr size_t bytes_to_restore = 32;
//	std::array<BYTE, bytes_to_restore> original_bytes;
//
//	std::memcpy(original_bytes.data(), reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(g_ntdll_disk_image) + hf.rva), bytes_to_restore);
//
//	DWORD old_protect;
//	if (!VirtualProtect(hf.address, bytes_to_restore, PAGE_EXECUTE_READWRITE, &old_protect)) {
//		//std::cerr << "failed to change memory protection for " << hf.name << std::endl;
//		return false;
//	}
//
//	std::memcpy(hf.address, original_bytes.data(), bytes_to_restore);
//
//	DWORD temp;
//	VirtualProtect(hf.address, bytes_to_restore, old_protect, &temp);
//
//	//std::cout << "hook removed from " << hf.name << std::endl;
//	return true;
//}