#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.h"
#include "../util/nlohmann/json.hpp" // include nlohmann's json library
#include <fstream> // needed for file I/O
#include "../config/config.cpp"
#include "../protect/xorstr.hpp"
#include "../spoofer/callstack.h"
#include "algorithm"
#include "../protect/vmp.h"

//void catch_exception(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord, uintptr_t funcaddress = 0)
//{
//	MEMORY_BASIC_INFORMATION mbi;
//	VirtualQuery(ExceptionRecord->ExceptionAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
//
//	auto module_offset = reinterpret_cast<std::uintptr_t>(ExceptionRecord->ExceptionAddress) - reinterpret_cast<
//		std::uintptr_t>(mbi.AllocationBase);
//	if (module_offset == 0xFFE) return;
//
//	auto now = std::chrono::system_clock::now();
//	std::time_t t = std::chrono::system_clock::to_time_t(now);
//	std::tm time_info;
//	localtime_s(&time_info, &t);
//
//	std::stringstream ss;
//	ss << XOR("C:/exceptions/exceptions-") << std::put_time(&time_info, XOR("%y-%m-%d")) << XOR(".txt");
//	std::string filename = ss.str();
//
//	std::ofstream file(filename, std::ios_base::app);
//	if (!file.is_open())
//	{
//		return;
//	}
//
//	file << XOR("Image base: 0x") << std::uppercase << std::hex << game_data->baseModule << "\n";
//	file << XOR("Cheat base: 0x") << std::uppercase << std::hex << game_data->game_offsets.dllStartAdr << "\n";
//
//	file << XOR("Exception at: 0x") << std::uppercase << std::hex << ContextRecord->Rip << " ]\n";
//	file << XOR("Exception address: 0x") << std::uppercase << std::hex << ExceptionRecord->ExceptionAddress << " (0x" <<
//		std::uppercase << std::hex << module_offset << ")\n";
//	file << XOR("Exception code: 0x") << std::uppercase << std::hex << ExceptionRecord->ExceptionCode << "\n";
//
//	file.close();
//}
//
//
//NTSTATUS(*NtContinue)(PCONTEXT threadContext, BOOLEAN raiseAlert) = nullptr;
//
//LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
//{
//	if (pExceptionInfo && pExceptionInfo->ExceptionRecord->ExceptionCode != 0xE && pExceptionInfo->ExceptionRecord->ExceptionCode != 0x40010006)
//	{
//		if (pExceptionInfo && pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
//		{
//			catch_exception(pExceptionInfo->ExceptionRecord, pExceptionInfo->ContextRecord);
//			//    sigs::hookLUIBindingFunction(skCrypt("cg.alwaysLoaded.telemetry.volume_mute"), (uintptr_t)IsFiringHook, game_Offsets::isWeaponFiringFunction);
//			//NtContinue(pExceptionInfo->ContextRecord, 0);
//		}
//		//catch_exception(pExceptionInfo->ExceptionRecord, pExceptionInfo->ContextRecord);
//	}
//	return EXCEPTION_CONTINUE_SEARCH;
//}

static char messagetosend[257];

static bool Items_SingleStringGetter(void* data, int idx, const char** out_text)
{
    const char* items_separated_by_zeros = (const char*)data;
    int items_count = 0;
    const char* p = items_separated_by_zeros;

    // Find the nth item
    while (*p && idx > 0)
    {
        p += strlen(p) + 1;
        idx--;
    }

    if (!*p)
        return false;

    if (out_text)
        *out_text = p;

    return true;
}

static const int CamoIDs[] = {
    72158,  // Gold
    72159,  // Gold Tiger
    72160,  // Mystic Gold
    72161,  // Diamond
    72163,  // Opal
    72164,  // Dark Spine
    72166,  // AfterLife
    72167,  // Dark Matter
    72168,  // Abyss
    72169,  // Nebula
    33000,  // Pink Camo
    58114,  // Citadelle PAP 1
    58115,  // Citadelle PAP 2
    58116,  // Citadelle PAP 3
    58117,  // Liberty PAP 1
    58118,  // Liberty PAP 2
    58119,  // Liberty PAP 3
    58126,  // Terminus PAP 1
    58127,  // Terminus PAP 2
    58128,  // Terminus PAP 3
    58123,  // Tomb PAP 1
    58124,  // Tomb PAP 2
    58125,  // Tomb PAP 3
    61305,  // One Trick
    63955,  // Molten Obsidian
    68140,  // Mercury
    68141,  // Constellation's End
    12245,  // Green & Gold
    12244,  // MWII Gold
    12256,  // MWII Platinum
    12247,  // MWII Polyatomic
    12237,  // MWII Orion
    30196,  // S5 Gold
    30197,  // S5 Platinum
    30198,  // S5 Diamond
    30199,  // S5 Crimson
    30200,  // S5 Iridescent
    30201,  // S5 Top 250
    30208   // Flow Thru
};

static const int WeaponIDs[] = {
    91894,  // War Machine
    92028,  // Hand Cannon
    93664,  // Ice Staff
    50898,  // Terminus Wonder Weapon
    92354,  // BO6 Raygun
    19730,  // JetGun
    87545,  // Raven Sword
    85399,  // Fire Sword
    85400,  // Light Sword
    85534,  // Eletricity Sword
    20895,  // MW3 Plasma Gun
    17215,  // MW3 Raygun
    21637,  // MW3 War Machine
    23988,  // OxygenMask
    23560,  // Glass Shard
    24770,  // Cluster Luancher
    57385,  // MW3 Stormender
    53573,  // Deployable Cover
    26071   // Saw
};

enum PresetType {
    Legit,
    SemiLegit,
    Rage,
    PSilentRage,
    NumPresets  // This automatically gives the number of presets
};

PresetType currentPreset = Legit;  // Default preset

// Assuming 'presets' is an array or vector that contains settings for each preset.
// The function 'ApplySettings' now accepts a PresetType argument to switch settings based on the selected preset.
void ApplySettings(PresetType preset) {
    switch (preset) {
    case Legit:
        game_data->settings.AIMBOT = true;

        game_data->settings.Norecoil = true;
        game_data->settings.noRecoil = 0.0f;
        game_data->settings.prediction_value = 100.f;
        game_data->settings.Prediction = true;
        game_data->settings.aim_smooth = 50;
        game_data->settings.fov_int = 0;
        game_data->settings.targetting_closestby = 0;
        game_data->settings.aim_type = 0;
        game_data->settings.i_bone = 2;
        game_data->settings.aimbotFov = 10.f;
        game_data->settings.FREINDLYAIMBOTCHECK = true;
        game_data->settings.AIMBOTCHECK = true;
        game_data->settings.skipKnocked = true;
        game_data->settings.skipselfrevive = true;
        game_data->settings.isSticky = false;
        game_data->settings.distanceFloat = 100.f;
        break;
        // Add cases for other presets as needed
    case SemiLegit:
        // Update settings for SemiLegit
        game_data->settings.AIMBOT = true;
        game_data->settings.Norecoil = true;
        game_data->settings.noRecoil = 0.0f;
        game_data->settings.prediction_value = 100.f;
        game_data->settings.Prediction = true;
        game_data->settings.aim_smooth = 25;
        game_data->settings.fov_int = 0;
        game_data->settings.targetting_closestby = 0;
        game_data->settings.aim_type = 0;
        game_data->settings.i_bone = 5;
        game_data->settings.aimbotFov = 30.f;
        game_data->settings.FREINDLYAIMBOTCHECK = true;
        game_data->settings.AIMBOTCHECK = true;
        game_data->settings.skipKnocked = true;
        game_data->settings.skipselfrevive = true;
        game_data->settings.isSticky = false;
        game_data->settings.distanceFloat = 150.f;

        break;
    case Rage:
        // Update settings for Rage
        game_data->settings.AIMBOT = true;
        game_data->settings.Nospread = true;
        game_data->settings.no_recoil = true;
        game_data->settings.Norecoil = true;
        game_data->settings.noRecoil = 0.0f;
        game_data->settings.prediction_value = 100.f;
        game_data->settings.Prediction = true;
        game_data->settings.aim_smooth = 0;
        game_data->settings.fov_int = 0;
        game_data->settings.targetting_closestby = 0;
        game_data->settings.aim_type = 0;
        game_data->settings.i_bone = 5;
        game_data->settings.aimbotFov = 180.f;
        game_data->settings.FREINDLYAIMBOTCHECK = true;
        game_data->settings.AIMBOTCHECK = true;
        game_data->settings.skipKnocked = true;
        game_data->settings.skipselfrevive = true;
        game_data->settings.isSticky = true;
        game_data->settings.distanceFloat = 175.f;

        break;
    case PSilentRage:
        // Update settings for PSilent Rage
        game_data->settings.AIMBOT = true;
        game_data->settings.Nospread = true;
        game_data->settings.no_recoil = true;
        game_data->settings.Norecoil = true;
        game_data->settings.noRecoil = 0.0f;
        game_data->settings.prediction_value = 100.f;
        game_data->settings.Prediction = true;
        game_data->settings.aim_smooth = 0;
        game_data->settings.fov_int = 0;
        game_data->settings.targetting_closestby = 0;
        game_data->settings.aim_type = 1;
        game_data->settings.i_bone = 5;
        game_data->settings.aimbotFov = 180.f;
        game_data->settings.FREINDLYAIMBOTCHECK = true;
        game_data->settings.AIMBOTCHECK = true;
        game_data->settings.skipKnocked = true;
        game_data->settings.skipselfrevive = true;
        game_data->settings.isSticky = true;
        game_data->settings.distanceFloat = 175.f;

        break;
    default:
        // Handle unknown case
        break;
    }
}

int currentItem = static_cast<int>(currentPreset);

static char loadoutName[255];
static int loadoutIndex, loadoutStatGroup = 0;
static int CamoloadoutIndex, CamoloadoutStatGroup = 0;
static int WeaponloadoutIndex, WeaponloadoutStatGroup = 0;
static int inputValue;
static int weaponIndex;
static int weaponsIndex;
int myValue = 0; // Variable to store the input value

bool b_shadows = false;  // Checkbox state

void SetCamo(int lootID, int loadout, int Slot, int gamemode)
{
    static char context[255];
    static char state[255];
    static int navStringCount;
    static char* navStrings[32]{};
    static const char* mode = "";

    for (size_t i = 0; i < 14; i++)
    {
        if (CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, i))
        {
            __int64 wzrankedloadouts;
            if (gamemode == 0)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/mp/loadouts/cer_mp_loadouts.ddl"));
            if (gamemode == 1)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/zm/cer_zm_loadouts.ddl"));
            if (gamemode == 2)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/wz/loadouts/cer_wz_loadouts.ddl"));
            auto getstate = DDL_GetRootState((__int64)state, wzrankedloadouts);

            char buffer[127];
            sprintf_s(buffer, (XOR("loadouts.%d.weaponSetups.%d.camoLootID")), loadout, Slot);

            if (Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount))
            {
                if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
                {
                    auto type = DDL_GetType((__int64)state);

                    //ctx::config::logging::counter = type;
                    if (type == 4 || type == 2 || type == 8 || type == 6 || type == 10)
                    {
                        if (DDL_SetInt((__int64)state, (long long)context, lootID))
                        {

                        }
                    }
                }
            }
        }
    }
}

void SetName(int loadout, const char* name, int gamemode)
{
    static char context[255];
    static char state[255];
    static int navStringCount;
    static char* navStrings[32]{};
    static const char* mode = "";

    for (size_t i = 0; i < 14; i++)
    {
        if (CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, i))
        {
            __int64 wzrankedloadouts;
            if (gamemode == 0)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/mp/loadouts/cer_mp_loadouts.ddl"));
            if (gamemode == 1)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/zm/cer_zm_loadouts.ddl"));
            if (gamemode == 2)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/wz/loadouts/cer_wz_loadouts.ddl"));
            auto getstate = DDL_GetRootState((__int64)state, wzrankedloadouts);

            char buffer[127];
            sprintf_s(buffer, (XOR("loadouts.%d.name")), loadout);

            if (Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount))
            {
                if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
                {
                    auto type = DDL_GetType((__int64)state);

                    //ctx::config::logging::counter = type;
                    if (type == 4 || type == 2 || type == 8 || type == 6 || type == 10)
                    {
                        if (DDL_SetString((__int64)state, (long long)context, name))
                        {

                        }
                    }
                }
            }
        }
    }
}

void SetWeapon(int WeaponlootID, int loadout, int Slot, int gamemode)
{
    static char context[255];
    static char state[255];
    static int navStringCount;
    static char* navStrings[32]{};
    static const char* mode = "";

    for (size_t i = 0; i < 14; i++)
    {
        if (CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, i))
        {
            __int64 wzrankedloadouts;
            if (gamemode == 0)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/mp/loadouts/cer_mp_loadouts.ddl"));
            if (gamemode == 1)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/zm/cer_zm_loadouts.ddl"));
            if (gamemode == 2)
                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/wz/loadouts/cer_wz_loadouts.ddl"));
            auto getstate = DDL_GetRootState((__int64)state, wzrankedloadouts);

            char buffer[127];
            sprintf_s(buffer, (XOR("loadouts.%d.weaponSetups.%d.weaponLootID")), loadout, Slot);

            if (Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount))
            {
                if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
                {
                    auto type = DDL_GetType((__int64)state);

                    //ctx::config::logging::counter = type;
                    if (type == 4 || type == 2 || type == 8 || type == 6 || type == 10)
                    {
                        if (DDL_SetInt((__int64)state, (long long)context, WeaponlootID))
                        {

                        }
                    }
                }
            }
        }
    }
}

// Variables for ImGui inputs
static int WeaponlootID = 0;
static int loadout = 0;
static int Slot = 0;
static int gamemode = 0;

//void SetPerk(int PerklootID, int loadout, int Slot, int gamemode)
//{
//    static char context[255];
//    static char state[255];
//    static int navStringCount;
//    static char* navStrings[32]{};
//    static const char* mode = "";
//
//    for (size_t i = 0; i < 14; i++)
//    {
//        if (CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, i))
//        {
//            __int64 wzrankedloadouts;
//            if (gamemode == 0)
//                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/mp/loadouts/cer_mp_loadouts.ddl"));
//            if (gamemode == 1)
//                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/zm/cer_zm_loadouts.ddl"));
//            if (gamemode == 2)
//                wzrankedloadouts = Com_DDL_LoadAsset(XOR("ddl/cer/wz/loadouts/cer_wz_loadouts.ddl"));
//            auto getstate = DDL_GetRootState((__int64)state, wzrankedloadouts);
//
//            char buffer[127];
//            sprintf_s(buffer, (XOR("loadouts.%d.perkLootIDs.%d")), loadout, Slot);
//
//            if (Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount))
//            {
//                if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
//                {
//                    auto type = DDL_GetType((__int64)state);
//
//                    //ctx::config::logging::counter = type;
//                    if (type == 4 || type == 2 || type == 8 || type == 6 || type == 10)
//                    {
//                        if (DDL_SetInt((__int64)state, (long long)context, PerklootID))
//                        {
//
//                        }
//                    }
//                }
//            }
//        }
//    }
//}

std::string key_name = XOR("None");
std::string sz_text = XOR("Aim Key.");

void KeyBindButton(int& key, int width, int height)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    static auto b_get = false;

    if (ImGui::Button(sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    key_name = XOR("Left Mouse");
                    b_get = false;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    key_name = XOR("Right Mouse");
                    b_get = false;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    key_name = XOR("Middle Mouse");
                    b_get = false;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    key_name = XOR("X1 Mouse");
                    b_get = false;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    key_name = XOR("X2 Mouse");
                    b_get = false;
                    break;
                }
            }
        }

        for (auto i = 1; i < 511; i++)
        {
            if (ImGui::IsKeyDown(ImGuiKey(i), false))
            {
                key = i == VK_ESCAPE ? -1 : i;
                key_name = ImGui::GetKeyName(ImGuiKey(key));
                b_get = false;
            }
        }

        // Controller buttons
        int gpad_key = gamepad::GPad_ResolveButton();
        if (gpad_key >= 0) {
            key = gpad_key;
            key_name = gamepad::GPad_ResolveString(gpad_key).c_str();

            b_get = false;
        }
        sz_text = XOR("...");
    }
    else if (!b_get && key == -1)
        sz_text = XOR("None");
    else if (!b_get && key != -1)
        sz_text = key_name;
}

std::string override_key_name = XOR("None");
std::string override_sz_text = XOR("Override Aim Key.");

void OverrideKeyBindButton(int& key, int width, int height)
{
    ImGuiContext& g2 = *GImGui;
    ImGuiIO& io2 = g2.IO;

    static auto b_get = false;

    if (ImGui::Button(override_sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io2.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    override_key_name = XOR("Left Mouse");
                    b_get = false;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    override_key_name = XOR("Right Mouse");
                    b_get = false;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    override_key_name = XOR("Middle Mouse");
                    b_get = false;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    override_key_name = XOR("X1 Mouse");
                    b_get = false;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    override_key_name = XOR("X2 Mouse");
                    b_get = false;
                    break;
                }
            }
        }

        for (auto i = 1; i < 511; i++)
        {
            if (ImGui::IsKeyDown(ImGuiKey(i), false))
            {
                key = i == VK_ESCAPE ? -1 : i;
                override_key_name = ImGui::GetKeyName(ImGuiKey(key));
                b_get = false;
            }
        }

        // Controller buttons
        int gpad_key = gamepad::GPad_ResolveButton();
        if (gpad_key >= 0) {
            key = gpad_key;
            override_key_name = gamepad::GPad_ResolveString(gpad_key).c_str();

            b_get = false;
        }
        override_sz_text = XOR("...");
    }
    else if (!b_get && key == -1)
        override_sz_text = XOR("None");
    else if (!b_get && key != -1)
        override_sz_text = override_key_name;
}

std::string anti_override_key_name = XOR("None");
std::string anti_override_sz_text = XOR("Override Anti-Aim Key.");

void anti_OverrideKeyBindButton(int& key, int width, int height)
{
    ImGuiContext& g2 = *GImGui;
    ImGuiIO& io2 = g2.IO;

    static auto b_get = false;

    if (ImGui::Button(anti_override_sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io2.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    anti_override_key_name = XOR("Left Mouse");
                    b_get = false;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    anti_override_key_name = XOR("Right Mouse");
                    b_get = false;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    anti_override_key_name = XOR("Middle Mouse");
                    b_get = false;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    anti_override_key_name = XOR("X1 Mouse");
                    b_get = false;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    anti_override_key_name = XOR("X2 Mouse");
                    b_get = false;
                    break;
                }
            }
        }

        for (auto i = 1; i < 511; i++)
        {
            if (ImGui::IsKeyDown(ImGuiKey(i), false))
            {
                key = i == VK_ESCAPE ? -1 : i;
                anti_override_key_name = ImGui::GetKeyName(ImGuiKey(key));
                b_get = false;
            }
        }

        // Controller buttons
        int gpad_key = gamepad::GPad_ResolveButton();
        if (gpad_key >= 0) {
            key = gpad_key;
            anti_override_key_name = gamepad::GPad_ResolveString(gpad_key).c_str();

            b_get = false;
        }
        anti_override_sz_text = XOR("...");
    }
    else if (!b_get && key == -1)
        anti_override_sz_text = XOR("None");
    else if (!b_get && key != -1)
        anti_override_sz_text = anti_override_key_name;
}

std::string AirStuck_key_name = XOR("Insert");
std::string AirStuck_sz_text = XOR("Activation Key.");

void AirStuck_OverrideKeyBindButton(int& key, int width, int height)
{
    ImGuiContext& g2 = *GImGui;
    ImGuiIO& io2 = g2.IO;

    static auto b_get = false;

    if (ImGui::Button(AirStuck_sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io2.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    AirStuck_key_name = XOR("Left Mouse");
                    b_get = false;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    AirStuck_key_name = XOR("Right Mouse");
                    b_get = false;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    AirStuck_key_name = XOR("Middle Mouse");
                    b_get = false;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    AirStuck_key_name = XOR("X1 Mouse");
                    b_get = false;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    anti_override_key_name = XOR("X2 Mouse");
                    b_get = false;
                    break;
                }
            }
        }

        for (auto i = 1; i < 511; i++)
        {
            if (ImGui::IsKeyDown(ImGuiKey(i), false))
            {
                key = i == VK_ESCAPE ? -1 : i;
                AirStuck_key_name = ImGui::GetKeyName(ImGuiKey(key));
                b_get = false;
            }
        }

        // Controller buttons
        int gpad_key = gamepad::GPad_ResolveButton();
        if (gpad_key >= 0) {
            key = gpad_key;
            AirStuck_key_name = gamepad::GPad_ResolveString(gpad_key).c_str();

            b_get = false;
        }
        AirStuck_sz_text = XOR("...");
    }
    else if (!b_get && key == -1)
        AirStuck_sz_text = XOR("None");
    else if (!b_get && key != -1)
        AirStuck_sz_text = AirStuck_key_name;
}

std::string ThirdPerson_key_name = XOR("None");
std::string ThirdPerson_sz_text = XOR("Activation Key.");

void ThirdPerson_KeyBindButton(int& key, int width, int height)
{
    ImGuiContext& g2 = *GImGui;
    ImGuiIO& io2 = g2.IO;

    static auto b_get = false;

    if (ImGui::Button(ThirdPerson_sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
        b_get = true;

    if (b_get)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io2.MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    ThirdPerson_key_name = XOR("Left Mouse");
                    b_get = false;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    ThirdPerson_key_name = XOR("Right Mouse");
                    b_get = false;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    ThirdPerson_key_name = XOR("Middle Mouse");
                    b_get = false;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    ThirdPerson_key_name = XOR("X1 Mouse");
                    b_get = false;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                    ThirdPerson_key_name = XOR("X2 Mouse");
                    b_get = false;
                    break;
                }
            }
        }

        for (auto i = 1; i < 511; i++)
        {
            if (ImGui::IsKeyDown(ImGuiKey(i), false))
            {
                key = i == VK_ESCAPE ? -1 : i;
                ThirdPerson_key_name = ImGui::GetKeyName(ImGuiKey(key));
                b_get = false;
            }
        }

        // Controller buttons
        int gpad_key = gamepad::GPad_ResolveButton();
        if (gpad_key >= 0) {
            key = gpad_key;
            ThirdPerson_key_name = gamepad::GPad_ResolveString(gpad_key).c_str();

            b_get = false;
        }
        ThirdPerson_sz_text = XOR("...");
    }
    else if (!b_get && key == -1)
        ThirdPerson_sz_text = XOR("None");
    else if (!b_get && key != -1)
        ThirdPerson_sz_text = ThirdPerson_key_name;
}

////// config shit

using json = nlohmann::json; // for convenience

std::string generate_unique_random_string(const int seed, const int len)
{
    VMProtectBeginMutation("generate_unique_random_string");

    char infoBuf[32767];
    DWORD bufCharCount = 32767;

    GetComputerNameA(infoBuf, &bufCharCount);

    auto new_seed = HASH(infoBuf);

    std::string tmp_s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    tmp_s.reserve(len);

    std::mt19937 gen(seed + new_seed);
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[dis(gen)];

    VMProtectEnd();

    return tmp_s;
}

void SaveSettings(int number) {
    // Create a json object
    VMProtectBeginMutation("SaveSettings");
    json settings;

    // Add all the settings to the JSON object
    // (your existing code for adding settings)

    // Generate random folder name
    std::string randomFolder = generate_unique_random_string(CT_HASH("Z"), 20);

    // Construct the full directory path
    std::string directoryPath = XOR("C:\\") + randomFolder + XOR("\\") + XOR("C-") + generate_unique_random_string(CT_HASH("cfg"), 20);

    // Create the directory if it doesn't exist
    std::filesystem::create_directories(directoryPath);

    // Generate random settings file name
    std::string randomSettingsName = XOR("profile");

    // Append the specified number to the settings file name
    randomSettingsName += XOR("_") + std::to_string(number) + XOR(".json");

    // Construct the full file path
    std::string fullFilePath = directoryPath + XOR("\\") + randomSettingsName;

    // Open the file stream
    std::ofstream o(fullFilePath);

    if (o.is_open()) {
        settings[XOR("139")] = game_data->settings.ShowSpectator;

        settings[XOR("140")] = game_data->settings.removeweaponmovement;
        settings[XOR("141")] = game_data->settings.ffpAds;
        settings[XOR("142")] = game_data->settings.ffpInspect;

        settings[XOR("143")] = game_data->settings.thirdperson_key;
        settings[XOR("144")] = ThirdPerson_key_name;

        settings[XOR("145")] = game_data->settings.misschance_value;

        settings[XOR("146")] = game_data->settings.fov_int;

        ///

        settings[XOR("1")] = game_data->settings.RGB_Menu;
        settings[XOR("2")] = game_data->settings.UnlockAllItems;
        settings[XOR("3")] = game_data->settings.misc_nocounteruav;
        settings[XOR("4")] = game_data->settings.targetaibool;
        settings[XOR("5")] = game_data->settings.showaibool;
        settings[XOR("6")] = game_data->settings.real_chamFill;
        settings[XOR("7")] = game_data->settings.chamFill;
        settings[XOR("8")] = { main_color.Value.x, main_color.Value.y, main_color.Value.z, main_color.Value.w };
        settings[XOR("9")] = game_data->settings.menu_trans;
        settings[XOR("10")] = game_data->settings.bubble_esp;
        settings[XOR("11")] = game_data->settings.head_esp;
        settings[XOR("12")] = game_data->settings.Nospread;
        settings[XOR("13")] = override_key_name;
        settings[XOR("14")] = key_name;
        settings[XOR("15")] = game_data->settings.aim_key;
        settings[XOR("16")] = game_data->settings.aim_Override_key;
        settings[XOR("17")] = game_data->settings.anti_aim_Override_key;
        settings[XOR("18")] = anti_override_key_name;
        settings[XOR("19")] = AirStuck_key_name;
        settings[XOR("20")] = game_data->settings.slide_key;
        settings[XOR("21")] = game_data->settings.aim_target_esp;
        settings[XOR("22")] = game_data->settings.Rapidfire;
        settings[XOR("23")] = game_data->settings.autoFire;
        settings[XOR("24")] = game_data->settings.isSticky;
        settings[XOR("25")] = game_data->settings.AIMBOTCHECK;
        settings[XOR("26")] = game_data->settings.AIMBOT;
        settings[XOR("27")] = game_data->settings.aim_type;
        settings[XOR("28")] = game_data->settings.i_bone;
        settings[XOR("29")] = game_data->settings.A_bone;
        settings[XOR("30")] = game_data->settings.aim_smooth;
        settings[XOR("31")] = game_data->settings.Prediction;
        settings[XOR("32")] = game_data->settings.prediction_value;
        settings[XOR("33")] = game_data->settings.FREINDLYAIMBOTCHECK;
        settings[XOR("34")] = game_data->settings.skipKnocked;
        settings[XOR("35")] = game_data->settings.skipselfrevive;
        settings[XOR("36")] = game_data->settings.targetting_closestby;
        settings[XOR("37")] = game_data->settings.aimbotFov;
        settings[XOR("38")] = game_data->settings.AIMBOTFOVCIRCLE;
        settings[XOR("39")] = game_data->settings.distanceFloat;
        settings[XOR("40")] = game_data->settings.b_crosshair;
        settings[XOR("41")] = game_data->settings.b_spinny_crosshair;
        settings[XOR("42")] = game_data->settings.Spinbot;
        settings[XOR("43")] = game_data->settings.jitterbot;
        settings[XOR("44")] = game_data->settings.turned;
        settings[XOR("45")] = game_data->settings.unfilled_boxes;
        settings[XOR("46")] = game_data->settings.corner_box;
        settings[XOR("47")] = game_data->settings.filled_boxes;
        settings[XOR("48")] = game_data->settings.b_player_3dboxes;
        settings[XOR("49")] = game_data->settings.b_player_filled_3dboxes;
        settings[XOR("50")] = game_data->settings.bone_esp;
        settings[XOR("51")] = game_data->settings.health_esp;
        settings[XOR("52")] = game_data->settings.snaplines;
        settings[XOR("53")] = game_data->settings.distance_esp;
        settings[XOR("54")] = game_data->settings.name_esp;
        settings[XOR("55")] = game_data->settings.teamid_esp;
        settings[XOR("56")] = game_data->settings.weapon_esp;
        settings[XOR("57")] = game_data->settings.kills_esp;
        settings[XOR("58")] = game_data->settings.deaths_esp;
        settings[XOR("59")] = game_data->settings.chams;
        settings[XOR("60")] = game_data->settings.HUDchams;
        settings[XOR("61")] = game_data->settings.realCompass;
        settings[XOR("62")] = game_data->settings.compass_esp;
        settings[XOR("63")] = game_data->settings.uavtest;
        settings[XOR("64")] = game_data->settings.uav_type;
        settings[XOR("65")] = game_data->settings.threat_warnings;
        settings[XOR("66")] = game_data->settings.bullet_tracers;
        settings[XOR("67")] = game_data->settings.show_only_hit_bullet_tracers;
        settings[XOR("68")] = game_data->settings.loot_test;
        settings[XOR("69")] = game_data->settings.lootammo;
        settings[XOR("70")] = game_data->settings.lootweapons;
        settings[XOR("71")] = game_data->settings.lootmoney;
        settings[XOR("72")] = game_data->settings.lootarmor;
        settings[XOR("73")] = game_data->settings.lootsupply;
        settings[XOR("74")] = game_data->settings.loot_cache;
        settings[XOR("75")] = game_data->settings.lootmisc;
        settings[XOR("76")] = game_data->settings.loot_bounds_3d;
        settings[XOR("77")] = game_data->settings.loot_box_type;
        settings[XOR("78")] = game_data->settings.loot_distanceFloat;
        settings[XOR("79")] = game_data->settings.enemies_only;
        settings[XOR("80")] = game_data->settings.b_visible;
        settings[XOR("81")] = game_data->settings.show_only_visible;
        settings[XOR("82")] = game_data->settings.show_aim_warning_nigs;
        settings[XOR("83")] = game_data->settings.chamFill;
        settings[XOR("84")] = game_data->settings.lineWidthNum;
        settings[XOR("85")] = game_data->settings.HUDlineWidthNum;
        settings[XOR("86")] = game_data->settings.flScale;
        settings[XOR("87")] = game_data->settings.aimwarning_fov;
        settings[XOR("88")] = game_data->settings.bullet_trace_fade;
        settings[XOR("89")] = game_data->settings.bullet_trace_thickness;
        settings[XOR("90")] = game_data->settings.bullet_x_trace_thickness;
        settings[XOR("91")] = game_data->settings.CompassScale;
        settings[XOR("92")] = game_data->settings.arrowSize;
        settings[XOR("93")] = game_data->settings.arrowfill;
        settings[XOR("94")] = game_data->settings.Compass_rgb;
        settings[XOR("95")] = game_data->settings.rgb_bullettracer;
        settings[XOR("96")] = game_data->settings.FOV_rgb;
        settings[XOR("97")] = game_data->settings.rgbChams;
        settings[XOR("98")] = game_data->settings.HUDrgbChams;
        settings[XOR("99")] = game_data->settings.rgbcrosshair;
        settings[XOR("100")] = game_data->settings.Norecoil;
        settings[XOR("101")] = game_data->settings.noRecoil;
        settings[XOR("102")] = game_data->settings.no_recoil;
        settings[XOR("103")] = game_data->settings.NoFlinch;
        settings[XOR("104")] = game_data->settings.NoStun;
        settings[XOR("105")] = game_data->settings.NoFlash;
        settings[XOR("106")] = game_data->settings.antishake;
        settings[XOR("107")] = game_data->settings.Remove_Fog;
        settings[XOR("108")] = game_data->settings.ThirdPerson;
        settings[XOR("109")] = game_data->settings.FastReload;
        settings[XOR("110")] = game_data->settings.instant_swap;
        settings[XOR("111")] = game_data->settings.b_fov;
        settings[XOR("112")] = game_data->settings.f_fov;

        // colors
        settings[XOR("113")] = {
            game_data->settings.FOVCOLOR_colorVec4.x,
            game_data->settings.FOVCOLOR_colorVec4.y,
            game_data->settings.FOVCOLOR_colorVec4.z,
            game_data->settings.FOVCOLOR_colorVec4.w
        };
        settings[XOR("114")] = {
            game_data->settings.crosshaircolor.x,
            game_data->settings.crosshaircolor.y,
            game_data->settings.crosshaircolor.z,
            game_data->settings.crosshaircolor.w
        };
        settings[XOR("115")] = {
            game_data->settings.player_colorVec4.x,
            game_data->settings.player_colorVec4.y,
            game_data->settings.player_colorVec4.z,
            game_data->settings.player_colorVec4.w
        };
        settings[XOR("116")] = {
            game_data->settings.vis_player_colorVec4.x,
            game_data->settings.vis_player_colorVec4.y,
            game_data->settings.vis_player_colorVec4.z,
            game_data->settings.vis_player_colorVec4.w
        };
        settings[XOR("117")] = {
            game_data->settings.HUDplayer_colorVec4.x,
            game_data->settings.HUDplayer_colorVec4.y,
            game_data->settings.HUDplayer_colorVec4.z,
            game_data->settings.HUDplayer_colorVec4.w
        };
        settings[XOR("118")] = {
            game_data->settings.V_skeleton_colorVec4.x,
            game_data->settings.V_skeleton_colorVec4.y,
            game_data->settings.V_skeleton_colorVec4.z,
            game_data->settings.V_skeleton_colorVec4.w
        };
        settings[XOR("119")] = {
            game_data->settings.skeleton_colorVec4.x,
            game_data->settings.skeleton_colorVec4.y,
            game_data->settings.skeleton_colorVec4.z,
            game_data->settings.skeleton_colorVec4.w
        };
        settings[XOR("120")] = {
            game_data->settings.vis_boxnotFilled_colorVec4.x,
            game_data->settings.vis_boxnotFilled_colorVec4.y,
            game_data->settings.vis_boxnotFilled_colorVec4.z,
            game_data->settings.vis_boxnotFilled_colorVec4.w
        };
        settings[XOR("121")] = {
            game_data->settings.boxnotFilled_colorVec4.x,
            game_data->settings.boxnotFilled_colorVec4.y,
            game_data->settings.boxnotFilled_colorVec4.z,
            game_data->settings.boxnotFilled_colorVec4.w
        };
        settings[XOR("122")] = {
            game_data->settings.V_distance_colorVec4.x,
            game_data->settings.V_distance_colorVec4.y,
            game_data->settings.V_distance_colorVec4.z,
            game_data->settings.V_distance_colorVec4.w
        };
        settings[XOR("123")] = {
            game_data->settings.distance_colorVec4.x,
            game_data->settings.distance_colorVec4.y,
            game_data->settings.distance_colorVec4.z,
            game_data->settings.distance_colorVec4.w
        };
        settings[XOR("124")] = {
            game_data->settings.V_Snaplines_colorVec4.x,
            game_data->settings.V_Snaplines_colorVec4.y,
            game_data->settings.V_Snaplines_colorVec4.z,
            game_data->settings.V_Snaplines_colorVec4.w
        };
        settings[XOR("125")] = {
            game_data->settings.Snaplines_colorVec4.x,
            game_data->settings.Snaplines_colorVec4.y,
            game_data->settings.Snaplines_colorVec4.z,
            game_data->settings.Snaplines_colorVec4.w
        };
        settings[XOR("126")] = {
            game_data->settings.boxFilled_colorVec4.x,
            game_data->settings.boxFilled_colorVec4.y,
            game_data->settings.boxFilled_colorVec4.z,
            game_data->settings.boxFilled_colorVec4.w
        };
        settings[XOR("127")] = {
            game_data->settings.vis_player_colorVec4.x,
            game_data->settings.vis_player_colorVec4.y,
            game_data->settings.vis_player_colorVec4.z,
            game_data->settings.vis_player_colorVec4.w
        };
        settings[XOR("128")] = {
            game_data->settings.player_colorVec4.x,
            game_data->settings.player_colorVec4.y,
            game_data->settings.player_colorVec4.z,
            game_data->settings.player_colorVec4.w
        };
        settings[XOR("129")] = {
            game_data->settings.crosshaircolor.x,
            game_data->settings.crosshaircolor.y,
            game_data->settings.crosshaircolor.z,
            game_data->settings.crosshaircolor.w
        };
        settings[XOR("130")] = {
            game_data->settings.HUDplayer_colorVec4.x,
            game_data->settings.HUDplayer_colorVec4.y,
            game_data->settings.HUDplayer_colorVec4.z,
            game_data->settings.HUDplayer_colorVec4.w
        };
        settings[XOR("131")] = {
            game_data->settings.V_COMPASS_colorVec4.x,
            game_data->settings.V_COMPASS_colorVec4.y,
            game_data->settings.V_COMPASS_colorVec4.z,
            game_data->settings.V_COMPASS_colorVec4.w
        };
        settings[XOR("132")] = {
            game_data->settings.COMPASS_colorVec4.x,
            game_data->settings.COMPASS_colorVec4.y,
            game_data->settings.COMPASS_colorVec4.z,
            game_data->settings.COMPASS_colorVec4.w
        };
        settings[XOR("133")] = {
            game_data->settings.V2_COMPASS_colorVec4.x,
            game_data->settings.V2_COMPASS_colorVec4.y,
            game_data->settings.V2_COMPASS_colorVec4.z,
            game_data->settings.V2_COMPASS_colorVec4.w
        };
        settings[XOR("134")] = {
            game_data->settings.V_aim_warnings_colorVec4.x,
            game_data->settings.V_aim_warnings_colorVec4.y,
            game_data->settings.V_aim_warnings_colorVec4.z,
            game_data->settings.V_aim_warnings_colorVec4.w
        };
        settings[XOR("135")] = {
            game_data->settings.bullettracer_color.x,
            game_data->settings.bullettracer_color.y,
            game_data->settings.bullettracer_color.z,
            game_data->settings.bullettracer_color.w
        };
        settings[XOR("136")] = {
            game_data->settings.UAV_color.x,
            game_data->settings.UAV_color.y,
            game_data->settings.UAV_color.z,
            game_data->settings.UAV_color.w
        };
        settings[XOR("137")] = {
            game_data->settings.Target_color.x,
            game_data->settings.Target_color.y,
            game_data->settings.Target_color.z,
            game_data->settings.Target_color.w
        };
        settings[XOR("138")] = {
            game_data->settings.UAV_color.x,
            game_data->settings.UAV_color.y,
            game_data->settings.UAV_color.z,
            game_data->settings.UAV_color.w
        };

        // Write the json object to the file
        o << std::setw(4) << settings << std::endl;

        // Close the file stream
        o.close();
    }

    VMProtectEnd();
}

void LoadSettings(int number)
{
    VMProtectBeginMutation("LoadSettings");
    // Construct the full directory path
    std::string randomFolder = generate_unique_random_string(CT_HASH("Z"), 20);
    std::string directoryPath = XOR("C:\\") + randomFolder + XOR("\\C-") + generate_unique_random_string(CT_HASH("cfg"), 20);

    // Construct the full file path
    std::string randomSettingsName = XOR("profile_") + std::to_string(number) + XOR(".json");

    std::string fullFilePath = directoryPath + XOR("\\") + randomSettingsName;

    // Open the file stream for reading
    std::ifstream i(fullFilePath);

    if (i.is_open()) {
        // Parse the JSON content
        json settings;
        i >> settings;

        float r = settings[XOR("8")][0].get<float>();
        float g = settings[XOR("8")][1].get<float>();
        float b = settings[XOR("8")][2].get<float>();
        float a = settings[XOR("8")][3].get<float>();

        // Assign the values to 'main_color'
        main_color = ImColor(r, g, b, a);

        if (settings.contains(XOR("139"))) {
            game_data->settings.ShowSpectator = settings[XOR("139")];
        }

        if (settings.contains(XOR("140"))) {
            game_data->settings.removeweaponmovement = settings[XOR("140")];
        }

        if (settings.contains(XOR("141"))) {
            game_data->settings.ffpAds = settings[XOR("141")];
        }

        if (settings.contains(XOR("142"))) {
            game_data->settings.ffpInspect = settings[XOR("142")];
        }

        if (settings.contains(XOR("143"))) {
            game_data->settings.thirdperson_key = settings[XOR("143")];
        }

        if (settings.contains(XOR("144"))) {
            ThirdPerson_key_name = settings[XOR("144")];
        }

        if (settings.contains(XOR("145"))) {
            game_data->settings.misschance_value = settings[XOR("145")];
        }

        if (settings.contains(XOR("146"))) {
            game_data->settings.fov_int = settings[XOR("146")];
        }

        /////

        if (settings.contains(XOR("2"))) {
            game_data->settings.UnlockAllItems = settings[XOR("2")];
        }

        if (settings.contains(XOR("3"))) {
            game_data->settings.misc_nocounteruav = settings[XOR("3")];
        }

        if (settings.contains(XOR("4"))) {
            game_data->settings.targetaibool = settings[XOR("4")];
        }

        if (settings.contains(XOR("5"))) {
            game_data->settings.showaibool = settings[XOR("5")];
        }

        if (settings.contains(XOR("6"))) {
            game_data->settings.real_chamFill = settings[XOR("6")];
        }

        if (settings.contains(XOR("7"))) {
            game_data->settings.chamFill = settings[XOR("7")];
        }

        game_data->settings.menu_trans = settings[XOR("9")];
        game_data->settings.RGB_Menu = settings[XOR("1")];

        game_data->settings.bubble_esp = settings[XOR("10")];
        game_data->settings.head_esp = settings[XOR("11")];

        game_data->settings.Nospread = settings[XOR("12")];
        key_name = settings[XOR("14")];
        override_key_name = settings[XOR("13")];

        game_data->settings.aim_key = settings[XOR("15")];
        game_data->settings.aim_Override_key = settings[XOR("16")];

        game_data->settings.slide_key = settings[XOR("20")];
        AirStuck_key_name = settings[XOR("19")];

        game_data->settings.anti_aim_Override_key = settings[XOR("17")];
        anti_override_key_name = settings[XOR("18")];

        game_data->settings.aim_target_esp = settings[XOR("21")];

        game_data->settings.Rapidfire = settings[XOR("22")];
        game_data->settings.autoFire = settings[XOR("23")];

        game_data->settings.isSticky = settings[XOR("24")];

        game_data->settings.AIMBOTCHECK = settings[XOR("25")];
        game_data->settings.AIMBOT = settings[XOR("26")];
        game_data->settings.aim_type = settings[XOR("27")];
        game_data->settings.i_bone = settings[XOR("28")];
        game_data->settings.A_bone = settings[XOR("29")];
        game_data->settings.aim_smooth = settings[XOR("30")];
        game_data->settings.Prediction = settings[XOR("31")];
        game_data->settings.prediction_value = settings[XOR("32")];
        game_data->settings.FREINDLYAIMBOTCHECK = settings[XOR("33")];
        game_data->settings.skipKnocked = settings[XOR("34")];
        game_data->settings.skipselfrevive = settings[XOR("35")];
        game_data->settings.targetting_closestby = settings[XOR("36")];
        game_data->settings.aimbotFov = settings[XOR("37")];
        game_data->settings.AIMBOTFOVCIRCLE = settings[XOR("38")];
        game_data->settings.distanceFloat = settings[XOR("39")];
        game_data->settings.b_crosshair = settings[XOR("40")];
        game_data->settings.b_spinny_crosshair = settings[XOR("41")];
        game_data->settings.Spinbot = settings[XOR("42")];
        game_data->settings.jitterbot = settings[XOR("43")];
        game_data->settings.turned = settings[XOR("44")];
        game_data->settings.unfilled_boxes = settings[XOR("45")];
        game_data->settings.corner_box = settings[XOR("46")];
        game_data->settings.filled_boxes = settings[XOR("47")];
        game_data->settings.b_player_3dboxes = settings[XOR("48")];
        game_data->settings.b_player_filled_3dboxes = settings[XOR("49")];
        game_data->settings.bone_esp = settings[XOR("50")];
        game_data->settings.health_esp = settings[XOR("51")];
        game_data->settings.snaplines = settings[XOR("52")];
        game_data->settings.distance_esp = settings[XOR("53")];
        game_data->settings.name_esp = settings[XOR("54")];
        game_data->settings.teamid_esp = settings[XOR("55")];
        game_data->settings.weapon_esp = settings[XOR("56")];
        game_data->settings.kills_esp = settings[XOR("57")];
        game_data->settings.deaths_esp = settings[XOR("58")];
        game_data->settings.chams = settings[XOR("59")];
        game_data->settings.HUDchams = settings[XOR("60")];
        game_data->settings.realCompass = settings[XOR("61")];
        game_data->settings.compass_esp = settings[XOR("62")];
        game_data->settings.uavtest = settings[XOR("63")];
        game_data->settings.uav_type = settings[XOR("64")];
        game_data->settings.threat_warnings = settings[XOR("65")];
        game_data->settings.bullet_tracers = settings[XOR("66")];
        game_data->settings.show_only_hit_bullet_tracers = settings[XOR("67")];
        game_data->settings.loot_test = settings[XOR("68")];
        game_data->settings.lootammo = settings[XOR("69")];
        game_data->settings.lootweapons = settings[XOR("70")];
        game_data->settings.lootmoney = settings[XOR("71")];
        game_data->settings.lootarmor = settings[XOR("72")];
        game_data->settings.lootsupply = settings[XOR("73")];
        game_data->settings.loot_cache = settings[XOR("74")];
        game_data->settings.lootmisc = settings[XOR("75")];
        game_data->settings.loot_bounds_3d = settings[XOR("76")];
        game_data->settings.loot_box_type = settings[XOR("77")];
        game_data->settings.loot_distanceFloat = settings[XOR("78")];
        game_data->settings.enemies_only = settings[XOR("79")];
        game_data->settings.b_visible = settings[XOR("80")];
        game_data->settings.show_only_visible = settings[XOR("81")];
        game_data->settings.show_aim_warning_nigs = settings[XOR("82")];
        game_data->settings.chamFill = settings[XOR("83")];
        game_data->settings.lineWidthNum = settings[XOR("84")];
        game_data->settings.HUDlineWidthNum = settings[XOR("85")];
        game_data->settings.flScale = settings[XOR("86")];
        game_data->settings.aimwarning_fov = settings[XOR("87")];
        game_data->settings.bullet_trace_fade = settings[XOR("88")];
        game_data->settings.bullet_trace_thickness = settings[XOR("89")];
        game_data->settings.bullet_x_trace_thickness = settings[XOR("90")];
        game_data->settings.CompassScale = settings[XOR("91")];
        game_data->settings.arrowSize = settings[XOR("92")];
        game_data->settings.arrowfill = settings[XOR("93")];
        game_data->settings.Compass_rgb = settings[XOR("94")];
        game_data->settings.rgb_bullettracer = settings[XOR("95")];
        game_data->settings.FOV_rgb = settings[XOR("96")];
        game_data->settings.rgbChams = settings[XOR("97")];
        game_data->settings.HUDrgbChams = settings[XOR("98")];
        game_data->settings.rgbcrosshair = settings[XOR("99")];
        game_data->settings.Norecoil = settings[XOR("100")];
        game_data->settings.noRecoil = settings[XOR("101")];
        game_data->settings.no_recoil = settings[XOR("102")];
        game_data->settings.NoFlinch = settings[XOR("103")];
        game_data->settings.NoStun = settings[XOR("104")];
        game_data->settings.NoFlash = settings[XOR("105")];
        game_data->settings.antishake = settings[XOR("106")];
        game_data->settings.Remove_Fog = settings[XOR("107")];
        game_data->settings.ThirdPerson = settings[XOR("108")];
        game_data->settings.FastReload = settings[XOR("109")];
        game_data->settings.instant_swap = settings[XOR("110")];
        game_data->settings.b_fov = settings[XOR("111")];
        game_data->settings.f_fov = settings[XOR("112")];

        // Update colors
        game_data->settings.FOVCOLOR_colorVec4 = {
            settings[XOR("113")][0],
            settings[XOR("113")][1],
            settings[XOR("113")][2],
            settings[XOR("113")][3]
        };
        game_data->settings.crosshaircolor = {
            settings[XOR("114")][0],
            settings[XOR("114")][1],
            settings[XOR("114")][2],
            settings[XOR("114")][3]
        };
        game_data->settings.player_colorVec4 = {
            settings[XOR("115")][0],
            settings[XOR("115")][1],
            settings[XOR("115")][2],
            settings[XOR("115")][3]
        };
        game_data->settings.vis_player_colorVec4 = {
            settings[XOR("116")][0],
            settings[XOR("116")][1],
            settings[XOR("116")][2],
            settings[XOR("116")][3]
        };
        game_data->settings.HUDplayer_colorVec4 = {
            settings[XOR("117")][0],
            settings[XOR("117")][1],
            settings[XOR("117")][2],
            settings[XOR("117")][3]
        };
        game_data->settings.V_skeleton_colorVec4 = {
            settings[XOR("118")][0],
            settings[XOR("118")][1],
            settings[XOR("118")][2],
            settings[XOR("118")][3]
        };
        game_data->settings.skeleton_colorVec4 = {
            settings[XOR("119")][0],
            settings[XOR("119")][1],
            settings[XOR("119")][2],
            settings[XOR("119")][3]
        };
        game_data->settings.vis_boxnotFilled_colorVec4 = {
            settings[XOR("120")][0],
            settings[XOR("120")][1],
            settings[XOR("120")][2],
            settings[XOR("120")][3]
        };
        game_data->settings.boxnotFilled_colorVec4 = {
            settings[XOR("121")][0],
            settings[XOR("121")][1],
            settings[XOR("121")][2],
            settings[XOR("121")][3]
        };
        game_data->settings.V_distance_colorVec4 = {
            settings[XOR("122")][0],
            settings[XOR("122")][1],
            settings[XOR("122")][2],
            settings[XOR("122")][3]
        };
        game_data->settings.distance_colorVec4 = {
            settings[XOR("123")][0],
            settings[XOR("123")][1],
            settings[XOR("123")][2],
            settings[XOR("123")][3]
        };
        game_data->settings.V_Snaplines_colorVec4 = {
            settings[XOR("124")][0],
            settings[XOR("124")][1],
            settings[XOR("124")][2],
            settings[XOR("124")][3]
        };
        game_data->settings.Snaplines_colorVec4 = {
            settings[XOR("125")][0],
            settings[XOR("125")][1],
            settings[XOR("125")][2],
            settings[XOR("125")][3]
        };
        game_data->settings.boxFilled_colorVec4 = {
            settings[XOR("126")][0],
            settings[XOR("126")][1],
            settings[XOR("126")][2],
            settings[XOR("126")][3]
        };
        // Note: Removed duplicates for vis_player_colorVec4, player_colorVec4, crosshaircolor, and HUDplayer_colorVec4 as they were already set above

        game_data->settings.V_COMPASS_colorVec4 = {
            settings[XOR("131")][0],
            settings[XOR("131")][1],
            settings[XOR("131")][2],
            settings[XOR("131")][3]
        };
        game_data->settings.COMPASS_colorVec4 = {
            settings[XOR("132")][0],
            settings[XOR("132")][1],
            settings[XOR("132")][2],
            settings[XOR("132")][3]
        };
        game_data->settings.V2_COMPASS_colorVec4 = {
            settings[XOR("133")][0],
            settings[XOR("133")][1],
            settings[XOR("133")][2],
            settings[XOR("133")][3]
        };
        game_data->settings.V_aim_warnings_colorVec4 = {
            settings[XOR("134")][0],
            settings[XOR("134")][1],
            settings[XOR("134")][2],
            settings[XOR("134")][3]
        };
        game_data->settings.bullettracer_color = {
            settings[XOR("135")][0],
            settings[XOR("135")][1],
            settings[XOR("135")][2],
            settings[XOR("135")][3]
        };
        game_data->settings.UAV_color = {
            settings[XOR("136")][0],
            settings[XOR("136")][1],
            settings[XOR("136")][2],
            settings[XOR("136")][3]
        };
        game_data->settings.Target_color = {
            settings[XOR("137")][0],
            settings[XOR("137")][1],
            settings[XOR("137")][2],
            settings[XOR("137")][3]
        };

        // Close the file stream
        i.close();
    }

    VMProtectEnd();
}

///////
//Helpers
ImDrawList* drawlist;

ImDrawList* drawredef;

ImVec2 pos;

int tabs = 0;

inline ImVec2 center_text(ImVec2 min, ImVec2 max, const char* text)
{
    return min + (max - min) / 2 - ImGui::CalcTextSize(text) / 2;
}

//Fonts
ImFont* mainfont;
ImFont* arrow;

float m_fade;

bool language_bool = false;
bool placeholder = false;

inline const char* u8cstr(const char8_t* str) {
    return reinterpret_cast<const char*>(str);
}

void cMenu::DrawMenu()
{
    float fade_speed = (1.f * ImGui::GetIO().DeltaTime) / 0.25f;
    m_fade = std::clamp(m_fade + (game_data->settings.b_menuEnable ? fade_speed : -fade_speed), 0.f, 1.f);

    if (m_fade <= 0.01f) return;

    VMProtectBeginMutation("DrawMenu");

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    float original_alpha = style.Alpha;
    style.Alpha = m_fade;

    style.TabRounding = 0.f;
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.WindowTitleAlign = ImVec2(0.50, 0.50);
    style.ItemSpacing = ImVec2(20, 20);
    style.FrameRounding = 4.f;
    style.GrabRounding = 4.f;
    style.ScrollbarSize = 6.f;
    style.WindowRounding = 4.f;
    style.ChildRounding = 4.f;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui::Begin(XOR("B"), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowSize(ImVec2(990, 720));

    drawlist = ImGui::GetWindowDrawList();
    pos = ImGui::GetWindowPos() + ImVec2(5, 5);

    // Main rect
    drawlist->AddRectFilled(pos, ImVec2(pos.x + 980, pos.y + 710), ImColor(15, 15, 15, game_data->settings.menu_trans), 0.f);

    ImGui::PushFont(mainfont);

    ImGui::Checkbox(u8cstr(XOR(u8"中文")), &language_bool);

    if (!language_bool)
    {
        // Title
        drawlist->AddText(center_text(pos, ImVec2(pos.x + 980, pos.y + 30), XOR("Menu")), main_color, XOR("Menu"));

        // Build number using draw list instead of immediate mode
        const char* build_text = XOR("Build: 7.2");
        ImVec2 text_size = ImGui::CalcTextSize(build_text);
        drawlist->AddText(ImVec2(pos.x + 970 - text_size.x, pos.y + 10),
            ImColor(255, 255, 255, game_data->settings.menu_trans),
            build_text);

    }
    else
    {
        // Title
        drawlist->AddText(center_text(pos, ImVec2(pos.x + 980, pos.y + 30), u8cstr(XOR(u8"Menu"))), main_color, u8cstr(XOR(u8"Menu")));

        // Build number using draw list instead of immediate mode
        const char* build_text = u8cstr(XOR(u8"Build: 7.2"));
        ImVec2 text_size = ImGui::CalcTextSize(build_text);
        drawlist->AddText(ImVec2(pos.x + 970 - text_size.x, pos.y + 10),
            ImColor(255, 255, 255, game_data->settings.menu_trans),
            build_text);

    }

    //tabs
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::SetCursorPos(ImVec2(5, 35));
    ImGui::BeginGroup();
    {
        if (language_bool)
        {
            if (ImGui::Tabs(u8cstr(XOR(u8"瞄准")), ImVec2(140, 30), tabs == 0)) tabs = 0;
            if (ImGui::Tabs(u8cstr(XOR(u8"反瞄准")), ImVec2(140, 30), tabs == 1)) tabs = 1;
            if (ImGui::Tabs(u8cstr(XOR(u8"视觉")), ImVec2(140, 30), tabs == 2)) tabs = 2;
            if (ImGui::Tabs(u8cstr(XOR(u8"杂项")), ImVec2(140, 30), tabs == 3)) tabs = 3;
            if (ImGui::Tabs(u8cstr(XOR(u8"设置")), ImVec2(140, 30), tabs == 4)) tabs = 4;
            if (ImGui::Tabs(u8cstr(XOR(u8"账户")), ImVec2(140, 30), tabs == 5)) tabs = 5;
            if (ImGui::Tabs(u8cstr(XOR(u8"房间")), ImVec2(140, 30), tabs == 6)) tabs = 6;
        }
        else
        {
            if (ImGui::Tabs(XOR("Aim"), ImVec2(140, 30), tabs == 0)) tabs = 0;
            if (ImGui::Tabs(XOR("Anti Aim"), ImVec2(140, 30), tabs == 1)) tabs = 1;
            if (ImGui::Tabs(XOR("Visuals"), ImVec2(140, 30), tabs == 2)) tabs = 2;
            if (ImGui::Tabs(XOR("Misc"), ImVec2(140, 30), tabs == 3)) tabs = 3;
            if (ImGui::Tabs(XOR("Settings"), ImVec2(140, 30), tabs == 4)) tabs = 4;
            if (ImGui::Tabs(XOR("Account"), ImVec2(140, 30), tabs == 5)) tabs = 5;
            if (ImGui::Tabs(XOR("Lobby"), ImVec2(140, 30), tabs == 6)) tabs = 6;
        }
    }
    ImGui::EndGroup();
    ImGui::PopStyleVar();

    game_data->settings.target_color = ImGui::ColorConvertFloat4ToU32(game_data->settings.Target_color);

    game_data->settings.boxColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.boxnotFilled_colorVec4);
    game_data->settings.vis_boxColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.vis_boxnotFilled_colorVec4);
    game_data->settings.boneColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.skeleton_colorVec4);
    game_data->settings.V_boneColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V_skeleton_colorVec4);
    //game_data->settings.snaplinescolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.Snaplines_colorVec4);
    //game_data->settings.V_snaplinescolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V_Snaplines_colorVec4);
    game_data->settings.filledboxColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.boxFilled_colorVec4);
    game_data->settings.boldcolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.boldcolorbox);
    game_data->settings.distanceColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.distance_colorVec4);
    game_data->settings.V_distanceColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V_distance_colorVec4);
    game_data->settings.vis_playerColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.vis_player_colorVec4);
    game_data->settings.playerColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.player_colorVec4);
    game_data->settings.aiColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.ai_colorVec4);
    game_data->settings.vehicleColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.vehicle_colorVec4);
    game_data->settings.DMZLOOTColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.DMZLoot_colorVec4);
    game_data->settings.FOVCOLOR = ImGui::ColorConvertFloat4ToU32(game_data->settings.FOVCOLOR_colorVec4);
    game_data->settings.crosshaircolorint = ImGui::ColorConvertFloat4ToU32(game_data->settings.crosshaircolor);
    game_data->settings.selfColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.HUDplayer_colorVec4);
    game_data->settings.V_compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V_COMPASS_colorVec4);
    game_data->settings.compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.COMPASS_colorVec4);

    //separator
    drawlist->AddLine(ImVec2(pos.x, pos.y + 30), ImVec2(pos.x + 980, pos.y + 30), ImColor(61, 61, 61, 255), 1.f);

    //separator
    drawlist->AddLine(ImVec2(pos.x, pos.y + 60), ImVec2(pos.x + 980, pos.y + 60), ImColor(61, 61, 61, 255), 1.f);

    if (!language_bool)
    {

        if (tabs == 0)
        {

            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("A"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Aim")), ImColor(255, 255, 255, 255), XOR("Aim"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Enabled"), &game_data->settings.AIMBOT);
                        ImGui::Combo(XOR("Aim Type"), &game_data->settings.aim_type, XOR("Normal\0\Server Silent\0\0"), 2);
                        ImGui::Combo(XOR("Aim Bone"), &game_data->settings.i_bone, XOR("Helmet\0\Head\0\Neck\0\Chest\0\Pelvis\0\Best Bone\0\0"), 6);
                        ImGui::Text(XOR("Aim Button"));
                        KeyBindButton(game_data->settings.aim_key, 260, 25);
                        if (ImGui::Combo(XOR("Aim Presets"), &currentItem, XOR("Legit\0\Semi\0\Rage\0\0"), NumPresets)) {
                            currentPreset = static_cast<PresetType>(currentItem);
                            ApplySettings(currentPreset);
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("B"), ImVec2(460, 170));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Override")), ImColor(255, 255, 255, 255), XOR("Override"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 170), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Combo(XOR("Override Bone"), &game_data->settings.A_bone, XOR("Left Foot\0Right Foot\0Left Knee\0Right Knee\0Helmet\0\Head\0\Neck\0\Chest\0\Pelvis\0\0"), 9);
                        OverrideKeyBindButton(game_data->settings.aim_Override_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("C"), ImVec2(460, 120));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Visuals")), ImColor(255, 255, 255, 255), XOR("Visuals"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 120), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Show FOV Circle"), &game_data->settings.AIMBOTFOVCIRCLE);
                        //ImGui::Checkbox(XOR("Show Snapline"), &game_data->settings.snaplines);
                        ImGui::Checkbox(XOR("Show Aim Target Color"), &game_data->settings.aim_target_esp);
                        ImGui::Checkbox(XOR("Show Crosshair"), &game_data->settings.b_crosshair);
                        ImGui::Checkbox(XOR("Show Spin Crosshair"), &game_data->settings.b_spinny_crosshair);
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("D"), ImVec2(460, 290));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Targeting")), ImColor(255, 255, 255, 255), XOR("Targeting"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 290), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Visibility Check"), &game_data->settings.AIMBOTCHECK);
                        ImGui::Checkbox(XOR("Target Agents"), &game_data->settings.targetaibool);
                        ImGui::Checkbox(XOR("Sticky Aim"), &game_data->settings.isSticky);
                        ImGui::Checkbox(XOR("Skip Friendly"), &game_data->settings.FREINDLYAIMBOTCHECK);
                        ImGui::Checkbox(XOR("Skip Downed"), &game_data->settings.skipKnocked);
                        ImGui::Checkbox(XOR("Skip Self Revive"), &game_data->settings.skipselfrevive);
                        ImGui::Combo(XOR("Targeting Type:"), &game_data->settings.targetting_closestby, XOR("FOV+Distance\0FOV\0Distance\0\0"));
                        ImGui::Combo(XOR("FOV Type:"), &game_data->settings.fov_int, XOR("Dynamic\0Static\0\0"));
                        ImGui::SliderFloat(XOR("Aim FOV"), &game_data->settings.aimbotFov, 1, 180, XOR("%.0f"));
                        ImGui::SliderFloat(XOR("Max Distance"), &game_data->settings.distanceFloat, 5, 5000.f, XOR("%.1f"));
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("E"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Accuracy")), ImColor(255, 255, 255, 255), XOR("Accuracy"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderInt(XOR("Aim Smoothing"), &game_data->settings.aim_smooth, 0, 100);
                        ImGui::Checkbox(XOR("Prediction"), &game_data->settings.Prediction);
                        if (game_data->settings.Prediction)
                        {
                            ImGui::SliderFloat(XOR("Prediction Strength"), &game_data->settings.prediction_value, 0, 100, XOR("%.1f"));
                        }
                        ImGui::SliderFloat(XOR("Miss Chance %"), &game_data->settings.misschance_value, 0, 100, XOR("%.1f"));
                        ImGui::Checkbox(XOR("Reduce Recoil"), &game_data->settings.Norecoil);
                        ImGui::SliderFloat(XOR("Recoil Value"), &game_data->settings.noRecoil, 0.0f, 100.0f, XOR("%.1f"));
                        ImGui::Checkbox(XOR("Reduce Sway"), &game_data->settings.no_recoil);
                        ImGui::SameLine();
                        ImGui::Checkbox(XOR("Reduce Spread"), &game_data->settings.Nospread);
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 1)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("F"), ImVec2(460, 200));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Anti-Aim")), ImColor(255, 255, 255, 255), XOR("Anti-Aim"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 200), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(XOR("Use at your own risk."));
                        ImGui::Checkbox(XOR("Spin"), &game_data->settings.Spinbot);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                        ImGui::Checkbox(XOR("Jitter"), &game_data->settings.jitterbot);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                        ImGui::Checkbox(u8cstr(XOR(u8"180°")), &game_data->settings.turned);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("G"), ImVec2(460, 100));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Settings")), ImColor(255, 255, 255, 255), XOR("Settings"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 100), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        anti_OverrideKeyBindButton(game_data->settings.anti_aim_Override_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 2)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("H"), ImVec2(460, 620));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Visuals")), ImColor(255, 255, 255, 255), XOR("Visuals"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 620), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Box"), &game_data->settings.unfilled_boxes);
                        ImGui::Checkbox(XOR("Corner Box"), &game_data->settings.corner_box);
                        ImGui::Checkbox(XOR("Filled Box"), &game_data->settings.filled_boxes);
                        //ImGui::Combo(XOR("Box Type:"), &game_data->settings.box_type, XOR("Adaptive\0Fixed\0\0"));
                        ImGui::Checkbox(XOR("3D Box"), &game_data->settings.b_player_3dboxes);
                        ImGui::Checkbox(XOR("3D Filled Box"), &game_data->settings.b_player_filled_3dboxes);
                        ImGui::Checkbox(XOR("Skeleton"), &game_data->settings.bone_esp);
                        ImGui::Checkbox(XOR("Head Skeleton"), &game_data->settings.head_esp);
                        ImGui::Checkbox(XOR("Health Bar"), &game_data->settings.health_esp);
                        ImGui::Checkbox(XOR("Distance"), &game_data->settings.distance_esp);
                        ImGui::Checkbox(XOR("Player Name"), &game_data->settings.name_esp);
                        ImGui::Checkbox(XOR("Snapline"), &game_data->settings.Snapline_ESP);
                        if (game_data->settings.Snapline_ESP)
                        {
                            ImGui::Combo(XOR("Snapline Position:"), &game_data->settings.Snapline_type, XOR("Top\0Center\0Bottom\0\0"));
                        }
                        ImGui::Checkbox(XOR("Bubble"), &game_data->settings.bubble_esp);
                        ImGui::Checkbox(XOR("Team ID"), &game_data->settings.teamid_esp);
                        ImGui::Checkbox(XOR("Weapon"), &game_data->settings.weapon_esp);
                        ImGui::Checkbox(XOR("Kills"), &game_data->settings.kills_esp);
                        ImGui::Checkbox(XOR("Deaths"), &game_data->settings.deaths_esp);
                        ImGui::Checkbox(XOR("Player Outlines"), &game_data->settings.chams);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Not Streamproof."));
                        }
                        ImGui::Checkbox(XOR("Weapon Outline"), &game_data->settings.HUDchams);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Not Streamproof."));
                        }
                        ImGui::Checkbox(XOR("Compass"), &game_data->settings.realCompass);
                        //ImGui::Checkbox(XOR("2D Radar"), &game_data->settings.compass_esp);
                        ImGui::Checkbox(XOR("Spectator Count"), &game_data->settings.ShowSpectator);
                        ImGui::Checkbox(XOR("Radar"), &game_data->settings.uavtest);
                        ImGui::Combo(XOR("Radar Type:"), &game_data->settings.uav_type, XOR("Dot\0Directional\0\0"));
                        ImGui::Checkbox(XOR("Warnings"), &game_data->settings.threat_warnings);
                        ImGui::Checkbox(XOR("Bullet Tracers"), &game_data->settings.bullet_tracers);
                        if (game_data->settings.bullet_tracers)
                        {
                            ImGui::Checkbox(XOR("Bullet Tracers Hit Check"), &game_data->settings.show_only_hit_bullet_tracers);
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("I"), ImVec2(460, 280));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Loot")), ImColor(255, 255, 255, 255), XOR("Loot"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 280), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(0, 35));
                    ImGui::BeginChild(XOR("J"), ImVec2(460, 243));
                    {
                        ImGui::SetCursorPos(ImVec2(10, 15));
                        ImGui::BeginGroup();
                        {
                            ImGui::Checkbox(XOR("Enabled"), &game_data->settings.loot_test);

                            ImGui::Checkbox(XOR("Favorite Supply Box"), &game_data->settings.loot_FavoriteSupplyBox);
                            ImGui::Checkbox(XOR("Ammo"), &game_data->settings.lootammo);
                            ImGui::Checkbox(XOR("Weapons"), &game_data->settings.lootweapons);
                            ImGui::Checkbox(XOR("Money"), &game_data->settings.lootmoney);
                            ImGui::Checkbox(XOR("Armor Plates"), &game_data->settings.lootarmor);
                            ImGui::Checkbox(XOR("Supply Box"), &game_data->settings.lootsupply);
                            ImGui::Checkbox(XOR("Gas Mask"), &game_data->settings.GasMask);
                            ImGui::Checkbox(XOR("Lethals"), &game_data->settings.lootlethals);
                            ImGui::Checkbox(XOR("Tacticals"), &game_data->settings.tacticals);
                            ImGui::Checkbox(XOR("Equipment"), &game_data->settings.lootequipment);
                            ImGui::Checkbox(XOR("Misc"), &game_data->settings.lootmisc);
                            ImGui::Checkbox(XOR("Misc Extra"), &game_data->settings.lootextra);
                            ImGui::Checkbox(XOR("Wall Buy"), &game_data->settings.Wallbuys);
                            ImGui::Checkbox(XOR("Stations"), &game_data->settings.Stations);
                            ImGui::Checkbox(XOR("Scanners"), &game_data->settings.Scanners);
                            ImGui::Checkbox(XOR("Contracts"), &game_data->settings.Contracts);
                            ImGui::Checkbox(XOR("Ziplines"), &game_data->settings.Ziplines);
                            ImGui::Checkbox(XOR("Machines"), &game_data->settings.ZMMachines);
                            ImGui::Checkbox(XOR("Easter Egg"), &game_data->settings.lootEasterEgg);
                            ImGui::Checkbox(XOR("Traps"), &game_data->settings.ZMTrap);

                            ImGui::Checkbox(XOR("Loot Bounds"), &game_data->settings.loot_bounds_3d);
                            ImGui::SliderFloat(XOR("Loot Distance"), &game_data->settings.loot_distanceFloat, 0.f, 500.f, XOR("%.1f"));

                            ImGui::Spacing();
                            ImGui::Spacing();
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("K"), ImVec2(460, 320));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Settings")), ImColor(255, 255, 255, 255), XOR("Settings"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 320), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Enemies Only"), &game_data->settings.enemies_only);
                        ImGui::Checkbox(XOR("Visibility Check"), &game_data->settings.b_visible);
                        ImGui::Checkbox(XOR("Show Agents"), &game_data->settings.showaibool);

                        ImGui::Checkbox(XOR("Compass Visibility Check"), &game_data->settings.show_only_visible);
                        ImGui::Checkbox(XOR("Compass Warnings Check"), &game_data->settings.show_aim_warning_nigs);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Warnings Have To Be Enabled."));
                        }

                        ImGui::Checkbox(XOR("Player Outlines Opaque Fill"), &game_data->settings.real_chamFill);
                        ImGui::Checkbox(XOR("Player Outlines Transparent Fill"), &game_data->settings.chamFill);
                        ImGui::SliderInt(XOR("Player Outlines Width"), &game_data->settings.lineWidthNum, 0, 100);

                        ImGui::SliderInt(XOR("Gun Outline Width"), &game_data->settings.HUDlineWidthNum, 0, 100);

                        ImGui::SliderFloat(XOR("Radar Zoom"), &game_data->settings.flScale, 8.0f, 5000.f, XOR("%.1f"));

                        ImGui::SliderFloat(XOR("Warnings FOV"), &game_data->settings.aimwarning_fov, 0.0f, 200.f, XOR("%.1f"));
                        ImGui::SliderFloat(XOR("Tracer Fade Out Duration"), &game_data->settings.bullet_trace_fade, 0.5f, 10.f, XOR("%.1f"));
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("The higher the value, the faster tracers disappear."));
                        }
                        ImGui::SliderFloat(XOR("Tracer Line Thickness"), &game_data->settings.bullet_trace_thickness, 1.f, 10.f, XOR("%.1f"));
                        ImGui::SliderFloat(XOR("Tracer Line X Thickness"), &game_data->settings.bullet_x_trace_thickness, 1.f, 10.f, XOR("%.1f"));

                        ImGui::SliderFloat(XOR("Compass Zoom"), &game_data->settings.CompassScale, 1.f, 5000.f, XOR("%.1f"));
                        ImGui::SliderFloat(XOR("Compass Arrow Size"), &game_data->settings.arrowSize, 1.f, 100.f, XOR("%.1f"));
                        ImGui::SliderFloat(XOR("Compass Arrow Fill"), &game_data->settings.arrowfill, 0.f, 255.f, XOR("%.1f"));
                        ImGui::Checkbox(XOR("RGB Compass"), &game_data->settings.Compass_rgb);

                        ImGui::Checkbox(XOR("RGB Bullet Tracers"), &game_data->settings.rgb_bullettracer);

                        ImGui::Checkbox(XOR("RGB Fov Circle"), &game_data->settings.FOV_rgb);

                        ImGui::Checkbox(XOR("RGB Player Outlines"), &game_data->settings.rgbChams);

                        ImGui::Checkbox(XOR("RGB Weapon Outline"), &game_data->settings.HUDrgbChams);

                        ImGui::Checkbox(XOR("RGB Crosshair"), &game_data->settings.rgbcrosshair);

                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 3)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("L"), ImVec2(460, 320));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Removals")), ImColor(255, 255, 255, 255), XOR("Removals"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 320), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Disable Weapon Motion"), &game_data->settings.removeweaponmovement);
                        ImGui::Checkbox(XOR("Disable Counter UAV"), &game_data->settings.misc_nocounteruav);
                        ImGui::Checkbox(XOR("Disable Flinch Effects"), &game_data->settings.NoFlinch);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                        ImGui::Checkbox(XOR("Disable Stun Effects"), &game_data->settings.NoStun);
                        ImGui::Checkbox(XOR("Disable Flashbang Effects"), &game_data->settings.NoFlash);
                        //ImGui::Checkbox(XOR("Disable Flashbang Movement Effects"), &game_data->settings.nomovement);
                        ImGui::Checkbox(XOR("Disable Shield Shake"), &game_data->settings.antishake);
                        if (ImGui::Checkbox(XOR("Disable All Shadows"), &b_shadows))
                        {
                            if (b_shadows)
                            {
                                Cbuf_AddText(XOR("set #x353D347C4D236E028 0; set #x3704554F429DAB488 0; set #x3AD42CA33A427DE58 0; set #x38667C0BB90C5BFC3 0; set #x3DF200A089A3B3FEB 0; set #x35D474D39E096F8F1 0;"));
                            }
                            else
                            {
                                Cbuf_AddText(XOR("set #x353D347C4D236E028 1; set #x3704554F429DAB488 1; set #x3AD42CA33A427DE58 1; set #x38667C0BB90C5BFC3 1; set #x3DF200A089A3B3FEB 1; set #x35D474D39E096F8F1 1;"));
                            }
                        }
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("If it's not functioning, try turning it on and off."));
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("M"), ImVec2(460, 265));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("View")), ImColor(255, 255, 255, 255), XOR("View"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 265), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Field Of View"), &game_data->settings.b_fov);
                        ImGui::SliderFloat(XOR("FOV Value"), &game_data->settings.f_fov, 0.0f, 180.0f, XOR("%.1f"));
                        ImGui::Checkbox(XOR("Third Person"), &game_data->settings.ThirdPerson);
                        ImGui::SameLine();
                        ThirdPerson_KeyBindButton(game_data->settings.thirdperson_key, 150, 25);

                        ImGui::Checkbox(XOR("Force First Person ADS"), &game_data->settings.ffpAds);
                        ImGui::Checkbox(XOR("Force First Person Inspect"), &game_data->settings.ffpInspect);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("N"), ImVec2(460, 190));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Weapon")), ImColor(255, 255, 255, 255), XOR("Weapon"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 190), ImColor(61, 61, 61, 255), 4.f);
                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Rapid Fire"), &game_data->settings.Rapidfire);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                        ImGui::Checkbox(XOR("Auto Fire"), &game_data->settings.autoFire);
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        //}
                        ImGui::Checkbox(XOR("Fast Reload"), &game_data->settings.FastReload);
                        ImGui::Checkbox(XOR("Instant Swap"), &game_data->settings.instant_swap);
                        //ImGui::Checkbox(XOR("Server Crasher"), &game_data->settings.testcrasher);
                        //ImGui::Spacing();
                        //ImGui::Checkbox(XOR("Super Slide"), &game_data->settings.superslide);
                        //AirStuck_OverrideKeyBindButton(game_data->settings.slide_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("O"), ImVec2(460, 400));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Loadout")), ImColor(255, 255, 255, 255), XOR("Loadout"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 400), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        //if (ImGui::Button(XOR("Crash Server"), ImVec2(260, 25)))
                        //{
                        //    Cbuf_AddText(XOR("seta #x373350030F99412CB 1;seta #x36192E114DE58E5C5 aaaa;cmd requestcodcasterspectateclient 9999;seta #x342D15B663FEF84E4 0.5;"));
                        //}
                        ImGui::Text(XOR("Set Name"));
                        ImGui::InputTextWithHint(XOR("##a"), XOR("Name"), loadoutName, IM_ARRAYSIZE(loadoutName));
                        ImGui::Combo(XOR("Name Index"), &loadoutIndex, XOR("All Loadouts\0Loadout 1\0Loadout 2\0Loadout 3\0Loadout 4\0Loadout 5\0Loadout 6\0Loadout 7\0Loadout 8\0Loadout 9\0Loadout 10\0\0"));
                        ImGui::Combo(XOR("Name Gamemode"), &loadoutStatGroup, XOR("Multiplayer\0Zombies\0Warzone\0\0"));
                        if (ImGui::Button(XOR("Set Name"), ImVec2(260, 25)))
                        {
                            if (loadoutIndex == 0)
                            {
                                for (int i = 0; i < 15; i++)
                                {
                                    SetName(i, loadoutName, loadoutStatGroup);
                                }
                            }
                            else
                            {
                                SetName(loadoutIndex - 1, loadoutName, loadoutStatGroup);
                            }
                        }

                        ///

                        ImGui::Spacing();
                        // In your UI code:
                        ImGui::Text(XOR("Set Camo"));
                        static int selected_camo_index = 0;  // Keeps track of which camo is selected in the dropdown

                        const char* preview_value = nullptr;
                        if (selected_camo_index >= 0) {
                            Items_SingleStringGetter((void*)XOR("Gold\0Gold Tiger\0Mystic Gold\0Diamond\0Opal\0Dark Spine\0AfterLife\0Dark Matter\0Abyss\0Nebula\0Pink Camo\0Citadelle PAP 1\0Citadelle PAP 2\0Citadelle PAP 3\0Liberty PAP 1\0Liberty PAP 2\0Liberty PAP 3\0Terminus PAP 1\0Terminus PAP 2\0Terminus PAP 3\0Tomb PAP 1\0Tomb PAP 2\0Tomb PAP 3\0One Trick\0Molten Obsidian\0Mercury\0Constellation's End\0Green & Gold\0MWII Gold\0MWII Platinum\0MWII Polyatomic\0MWII Orion\0S5 Gold\0S5 Platinum\0S5 Diamond\0S5 Crimson\0S5 Iridescent\0S5 Top 250\0Flow Thru\0\0"), selected_camo_index, &preview_value);
                        }
                        if (!preview_value) {
                            preview_value = XOR("Select Camo");
                        }

                        if (ImGui::BeginCombo(XOR("Select Camo"), preview_value, ImGuiComboFlags_HeightLarge))
                        {
                            const char* items = XOR("Gold\0Gold Tiger\0Mystic Gold\0Diamond\0Opal\0Dark Spine\0AfterLife\0Dark Matter\0Abyss\0Nebula\0Pink Camo\0Citadelle PAP 1\0Citadelle PAP 2\0Citadelle PAP 3\0Liberty PAP 1\0Liberty PAP 2\0Liberty PAP 3\0Terminus PAP 1\0Terminus PAP 2\0Terminus PAP 3\0Tomb PAP 1\0Tomb PAP 2\0Tomb PAP 3\0One Trick\0Molten Obsidian\0Mercury\0Constellation's End\0Green & Gold\0MWII Gold\0MWII Platinum\0MWII Polyatomic\0MWII Orion\0S5 Gold\0S5 Platinum\0S5 Diamond\0S5 Crimson\0S5 Iridescent\0S5 Top 250\0Flow Thru\0\0");
                            const char* item = items;
                            for (int i = 0; item[0]; item += strlen(item) + 1, i++)
                            {
                                bool is_selected = (selected_camo_index == i);
                                if (ImGui::Selectable(item, is_selected))
                                    selected_camo_index = i;
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::Combo(XOR("Camo Index"), &CamoloadoutIndex, XOR("All Loadouts\0Loadout 1\0Loadout 2\0Loadout 3\0Loadout 4\0Loadout 5\0Loadout 6\0Loadout 7\0Loadout 8\0Loadout 9\0Loadout 10\0\0"));
                        ImGui::Combo(XOR("Weapon Index"), &weaponIndex, XOR("Primary\0Secondary\0Tertiary\0\0"));
                        ImGui::Combo(XOR("Camo Gamemode"), &CamoloadoutStatGroup, XOR("Multiplayer\0Zombies\0Warzone\0\0"));
                        if (ImGui::Button(XOR("Set Camo"), ImVec2(260, 25)))
                        {
                            int camo_value = CamoIDs[selected_camo_index];

                            if (CamoloadoutIndex == 0)
                            {
                                for (int i = 0; i < 15; i++)
                                {
                                    SetCamo(camo_value, i, weaponIndex, CamoloadoutStatGroup);
                                }
                            }
                            else
                            {
                                SetCamo(camo_value, CamoloadoutIndex - 1, weaponIndex, CamoloadoutStatGroup);
                            }
                        }

                        ImGui::Text(XOR("Set Weapon"));
                        // ImGui::InputInt(XOR("Weapon ID"), &inputValue);
                        static int selected_weapon_index = 0;  // Keeps track of which camo is selected in the dropdown

                        const char* preview_value_weapon = nullptr;
                        if (selected_weapon_index >= 0) {
                            Items_SingleStringGetter((void*)XOR(
                                "War Machine\0"
                                "Hand Cannon\0"
                                "Ice Staff\0"
                                "Terminus Wonder Weapon\0"
                                "BO6 Raygun\0"
                                "JetGun\0"
                                "Raven Sword\0"
                                "Fire Sword\0"
                                "Light Sword\0"
                                "Electricity Sword\0"
                                "MW3 Plasma Gun\0"
                                "MW3 Raygun\0"
                                "MW3 War Machine\0"
                                "Oxygen Mask\0"
                                "Glass Shard\0"
                                "Cluster Launcher\0"
                                "MW3 Stormender\0"
                                "Deployable Cover\0"
                                "Saw\0"
                            ), selected_weapon_index, &preview_value_weapon);

                        }
                        if (!preview_value_weapon) {
                            preview_value_weapon = XOR("Select Weapon");
                        }

                        if (ImGui::BeginCombo(XOR("Select Weapon"), preview_value_weapon, ImGuiComboFlags_HeightLarge))
                        {
                            const char* items = XOR(
                                "War Machine\0"
                                "Hand Cannon\0"
                                "Ice Staff\0"
                                "Terminus Wonder Weapon\0"
                                "BO6 Raygun\0"
                                "JetGun\0"
                                "Raven Sword\0"
                                "Fire Sword\0"
                                "Light Sword\0"
                                "Electricity Sword\0"
                                "MW3 Plasma Gun\0"
                                "MW3 Raygun\0"
                                "MW3 War Machine\0"
                                "Oxygen Mask\0"
                                "Glass Shard\0"
                                "Cluster Launcher\0"
                                "MW3 Stormender\0"
                                "Deployable Cover\0"
                                "Saw\0"
                            );

                            const char* item = items;
                            for (int i = 0; item[0]; item += strlen(item) + 1, i++)
                            {
                                bool is_selected = (selected_weapon_index == i);
                                if (ImGui::Selectable(item, is_selected))
                                    selected_weapon_index = i;
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::Combo(XOR("Weapon Loadout Index"), &WeaponloadoutIndex, XOR("All Loadouts\0Loadout 1\0Loadout 2\0Loadout 3\0Loadout 4\0Loadout 5\0Loadout 6\0Loadout 7\0Loadout 8\0Loadout 9\0Loadout 10\0\0"));
                        ImGui::Combo(XOR("Weapons Index"), &weaponsIndex, XOR("Primary\0Secondary\0Tertiary\0\0"));
                        ImGui::Combo(XOR("Weapon Gamemode"), &WeaponloadoutStatGroup, XOR("Multiplayer\0Zombies\0Warzone\0\0"));
                        if (ImGui::Button(XOR("Set Weapon"), ImVec2(260, 25)))
                        {
                            // SetWeapon(inputValue, WeaponloadoutIndex - 1, weaponsIndex, WeaponloadoutStatGroup);

                            int weapon_value = WeaponIDs[selected_weapon_index];

                            if (WeaponloadoutIndex == 0)
                            {
                                for (int i = 0; i < 15; i++)
                                {
                                    SetWeapon(weapon_value, i, weaponsIndex, WeaponloadoutStatGroup);
                                }
                            }
                            else
                            {
                                SetWeapon(weapon_value, WeaponloadoutIndex - 1, weaponsIndex, WeaponloadoutStatGroup);
                            }
                        }

                        //ImGui::Text(XOR("Set Perk"));

                        //ImGui::InputInt(XOR("Perk Loot ID"), &WeaponlootID);
                        //ImGui::InputInt(XOR("Loadout"), &loadout);
                        //ImGui::InputInt(XOR("Slot"), &Slot);
                        //ImGui::InputInt(XOR("Game Mode (0-MP, 1-ZM, 2-WZ)"), &gamemode);

                        //if (ImGui::Button(XOR("Set Perk")))
                        //{
                        //    SetPerk(WeaponlootID, loadout, Slot, gamemode);
                        //}

                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 4)
        {
            static int currentSettingIndex = 0;

            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("P"), ImVec2(460, 435));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Settings List")), ImColor(255, 255, 255, 255), XOR("Settings List"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //demo functions
                    const char* items[] = { XOR(" [F1] Setting 1"), XOR(" [F2] Setting 2"), XOR(" [F3] Setting 3"), XOR(" [F4] Setting 4"), XOR(" [F5] Setting 5"), XOR(" Setting 6"), XOR(" Setting 7"), XOR(" Setting 8"), XOR(" Setting 9"), XOR(" Setting 10") };

                    //render functions
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
                    ImGui::SetCursorPos(ImVec2(0, 50));
                    ImGui::SetNextItemWidth(1000);
                    ImGui::ListBox(XOR("##b"), &currentSettingIndex, items, IM_ARRAYSIZE(items), 10);
                    ImGui::PopStyleColor();

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 435), ImColor(61, 61, 61, 255), 4.f);
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("Q"), ImVec2(460, 150));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Settings Buttons")), ImColor(255, 255, 255, 255), XOR("Settings Buttons"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 150), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(70, 50));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Button(XOR("Load Setting"), ImVec2(320, 30)))
                        {
                             LoadSettings(currentSettingIndex + 1);
                             //SendToastNotification(XOR("MWIII"), XOR("^2Config Loaded"), XOR("cac_equip_fill"));
                        }
                        if (ImGui::Button(XOR("Save Setting"), ImVec2(320, 30)))
                        {
                            SaveSettings(currentSettingIndex + 1);
                            // SendToastNotification(XOR("MWIII"), XOR("^2Config Saved"), XOR("cac_equip_fill"));
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("R"), ImVec2(460, 610));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Color Customize")), ImColor(255, 255, 255, 255), XOR("Color Customize"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 610), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(XOR("Menu Keybind."));
                        AirStuck_OverrideKeyBindButton(game_data->settings.slide_key, 150, 25);

                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 16));

                        ImGui::ColorEdit4(XOR("Aim Target Color"), (float*)&game_data->settings.Target_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::Checkbox(XOR("RGB Menu Color"), &game_data->settings.RGB_Menu);


                        ImGui::ColorEdit4(XOR("Visible Skeleton Color"), (float*)&game_data->settings.V_skeleton_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(XOR("Main Menu Color"), (float*)&main_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(XOR("Visible Box Color"), (float*)&game_data->settings.vis_boxnotFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::SliderInt(XOR("##c"), &game_data->settings.menu_trans, 0, 255);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Menu Transparency"));
                        }

                        ImGui::ColorEdit4(XOR("Visible Text Color"), (float*)&game_data->settings.V_distance_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(XOR("Warnings Color"), (float*)&game_data->settings.V_aim_warnings_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);


                        ImGui::ColorEdit4(XOR("Visible Snaplines Color"), (float*)&game_data->settings.V_Snaplines_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(XOR("Bullet Tracers Color"), (float*)&game_data->settings.bullettracer_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(XOR("Visible Outline Color"), (float*)&game_data->settings.vis_player_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(XOR("Advanced Radar Color"), (float*)&game_data->settings.UAV_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(XOR("Invisible Skeleton Color"), (float*)&game_data->settings.skeleton_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Invisible Box Color"), (float*)&game_data->settings.boxnotFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Invisible Text Color"), (float*)&game_data->settings.distance_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Invisible Snaplines Color"), (float*)&game_data->settings.Snaplines_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Invisible Outline Color"), (float*)&game_data->settings.player_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(XOR("Box Fill Color"), (float*)&game_data->settings.boxFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("FOV Circle Color"), (float*)&game_data->settings.FOVCOLOR_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Crosshair Color"), (float*)&game_data->settings.crosshaircolor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Weapon Outline Color"), (float*)&game_data->settings.HUDplayer_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Visible Compass Color"), (float*)&game_data->settings.V_COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Invisible Compass Color"), (float*)&game_data->settings.COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(XOR("Warnings Compass Color"), (float*)&game_data->settings.V2_COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::PopStyleVar();
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 5)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("X"), ImVec2(460, 180));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Account Clantag")), ImColor(255, 255, 255, 255), XOR("Account Clantag"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 180), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        static char name1[6] = "";
                        ImGui::InputTextWithHint(XOR("##d"), XOR("Custom Clantag"), name1, 6);
                        if (ImGui::Button(XOR("Apply Clantag"), ImVec2(320, 30)))
                        {
                            CL_PlayerData_SetCustomClanTag(name1);
                            //SendColoredToastNotification(XOR("^2Clantag Applied!"), XOR("cac_equip_fill"));
                        }
                        ImGui::Checkbox(XOR("RGB Name"), &game_data->settings.rgbname);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("Xbn"), ImVec2(460, 180));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Chat")), ImColor(255, 255, 255, 255), XOR("Chat"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 180), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::InputTextWithHint(XOR("##e"), XOR("Enter Message"), messagetosend, IM_ARRAYSIZE(messagetosend));
                        ImGui::SameLine();
                        ImGui::Checkbox(XOR("Spam"), &game_data->settings.spamchat);
                        if (ImGui::Button(XOR("Send"), ImVec2(320, 30)))
                        {
                            sendchatmessage(messagetosend);
                        }
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Be careful! Clicking this too quickly might trigger rate limiting. For continuous spam, try using the Spam checkbox instead."));
                        }

                        // Copy the current message to use in your spam loop
                        strcpy(currentMessage, messagetosend);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("Y"), ImVec2(460, 100));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Account Info")), ImColor(255, 255, 255, 255), XOR("Account Info"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 100), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(XOR("Cleaned Account Screenshots: %i"), game_data->settings.screenshot_count);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("Z"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Account")), ImColor(255, 255, 255, 255), XOR("Account"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(XOR("Unlock All Items (RISKY)"), &game_data->settings.UnlockAllItems);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("Turn off to avoid temporary ban errors before entering a match."));
                        }
                        //if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        //{
                        //    ImGui::SetTooltip("May result in a temporary ban. Disable before entering a game to avoid this.");
                        //}
                        if (ImGui::Button(XOR("Bypass Account Name Profanity"), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("#x3BB203010F77299D4 0;"));
                        }
                        if (ImGui::Button(XOR("Skip Account Cutscenes"), ImVec2(320, 30)))
                        {
                            LUI_OpenMenu(0, XOR(""), 0, 0, 0);
                        }
                        if (ImGui::Button(XOR("Force Start"), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("xstartprivateparty; xstartprivatematchlobby; xpartygo; party_startPrivateDSGame;"));
                        }
                        if (ImGui::Button(XOR("Force Disconnect"), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("disconnect;"));
                        }
                        //if (ImGui::Button(XOR("Set Up Exception Hook"), ImVec2(200, 25)))
                        //{
                        //    HMODULE ntdll = GetModuleHandleA(XOR("ntdll"));
                        //    NtContinue = (decltype(NtContinue))GetProcAddress(ntdll, XOR("NtContinue"));

                        //    void(*RtlAddVectoredExceptionHandler)(LONG First, PVECTORED_EXCEPTION_HANDLER Handler) = (decltype(RtlAddVectoredExceptionHandler))GetProcAddress(ntdll, XOR("RtlAddVectoredExceptionHandler"));
                        //    RtlAddVectoredExceptionHandler(0, TopLevelExceptionHandler);
                        //    log_shit("hook attached nigger!");
                        //}

                        //ImGui::InputInt(XOR("Weapon Camo"), &nigger);
                        //ImGui::InputInt(XOR("statsgroup"), &nigger22);

                        //static char name1[256] = "";
                        //ImGui::InputTextWithHint(XOR("##Custommode"), XOR("Custom Mode"), name1, IM_ARRAYSIZE(name1));
                        //
                        //if (ImGui::Button(XOR("set ddl file!"), ImVec2(320, 30)))
                        //{
                        //    char context[255];
                        //    char state[255];
                        //    int navStringCount;
                        //    char* navStrings[16]{};

                        //    CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, nigger22);
                        //    log_shit("Called CL_PlayerData_GetDDLBuffer with context: %s", context);

                        //    log_shit("nigger11");
                        //    __int64 ddl_file = Com_DDL_LoadAsset((__int64)name1);
                        //    log_shit("Loaded DDL file: %lld", ddl_file);

                        //    DDL_GetRootState((__int64)state, ddl_file);
                        //    log_shit("Retrieved root state for ddl_file: %lld", ddl_file);

                        //    char buffer[200];
                        //    memset(buffer, 0, 200);
                        //    sprintf_s(buffer, XOR("squadMembers.loadouts.%i.weaponSetups.%i.camo"), 1, 1);
                        //    log_shit("Formatted buffer: %s", buffer);

                        //    ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
                        //    log_shit("Parsed navigation strings. Count: %d", navStringCount);

                        //    if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
                        //    {
                        //        log_shit("Successfully moved to path.");
                        //        DDL_SetInt((__int64)state, (__int64)context, nigger);
                        //        log_shit("Set integer value at state: %s", state);
                        //    }
                        //    else
                        //    {
                        //        log_shit("Failed to move to path.");
                        //    }
                        //}
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("CC"), ImVec2(460, 160));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), XOR("Commands")), ImColor(255, 255, 255, 255), XOR("Commands"));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 160), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        static char name1[256] = "";
                        ImGui::InputTextWithHint(XOR("##f"), XOR("Custom Command"), name1, IM_ARRAYSIZE(name1));
                        if (ImGui::Button(XOR("Send Command"), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(name1);
                        }

                        //static char name231[256] = "";
                        //ImGui::InputTextWithHint(XOR("##g"), XOR("Custom LUI"), name231, IM_ARRAYSIZE(name231));
                        //if (ImGui::Button(XOR("Send LUI"), ImVec2(320, 30)))
                        //{
                        //    LUI_OpenMenu(0, name231, 0, 0, 0);
                        //}
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 6)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("nnn"), ImVec2(950, 620));
                {
                    // Helper definitions
                    ImDrawList* drawlist = ImGui::GetWindowDrawList();
                    ImVec2 pos = ImGui::GetWindowPos();

                    // Title background
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 950, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    // Title text
                    ImVec2 textPos = ImVec2(pos.x + (950 / 2), pos.y + (35 / 2));
                    drawlist->AddText(ImVec2(textPos.x - ImGui::CalcTextSize(XOR("Lobby")).x / 2, textPos.y - ImGui::GetTextLineHeight() / 2), ImColor(255, 255, 255, 255), XOR("Lobby"));

                    // Separator line
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 950, pos.y + 35), ImColor(61, 61, 61, 255));

                    // Border
                    drawlist->AddRect(pos, ImVec2(pos.x + 950, pos.y + 620), ImColor(61, 61, 61, 255), 4.f);

                    // Render functions
                    ImGui::SetCursorPos(ImVec2(0, 50));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::BeginTable(XOR("1"), 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame))
                        {
                            ImGui::TableSetupColumn(XOR("Team ID"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(XOR("Player Name"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(XOR("Ping"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(XOR("Kills"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(XOR("XUID"), ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(XOR("Rank"), ImGuiTableColumnFlags_WidthStretch);

                            ImGui::TableHeadersRow();

                            if (game_data->settings.checkPassed && game_data->settings.b_ingame)
                            {
                                int numPlayers = *(int*)(game_data->game_offsets.gamemode);
                                for (int i = 0; i < numPlayers; ++i)
                                {
                                    player* p = &players[i];
                                    ImGui::TableNextRow();
                                    // First Column (Team ID)
                                    ImGui::TableNextColumn();
                                    char teamIdLabel[32];
                                    sprintf_s(teamIdLabel, XOR("Team %d"), p->team);
                                    ImGui::Text(teamIdLabel);
                                    // Second Column (Player Name)
                                    ImGui::TableNextColumn();
                                    ImGui::Text(p->player_name.name);
                                    // Third Column (Ping)
                                    ImGui::TableNextColumn();
                                    char pingLabel[32];
                                    sprintf_s(pingLabel, XOR("%d ms"), p->ping);
                                    ImGui::Text(pingLabel);
                                    // Fourth Column (Kills)
                                    ImGui::TableNextColumn();
                                    char killsLabel[32];
                                    sprintf_s(killsLabel, XOR("%d"), p->kills);
                                    ImGui::Text(killsLabel);
                                    // Fifth Column (XUID)
                                    ImGui::TableNextColumn();
                                    char xuidLabel[32];
                                    sprintf_s(xuidLabel, XOR("%llu"), p->xuid);
                                    ImGui::Text(xuidLabel);
                                    // Sixth Column (Rank)
                                    ImGui::TableNextColumn();
                                    char rankLabel[32];
                                    sprintf_s(rankLabel, XOR("%d"), p->rank_mp);
                                    ImGui::Text(rankLabel);
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }
    }
    else
    {

        if (tabs == 0)
        {

            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("A"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"瞄准"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"瞄准")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"啟用瞄準機器人")), &game_data->settings.AIMBOT);
                        ImGui::Combo(u8cstr(XOR(u8"瞄准类型")), &game_data->settings.aim_type, u8cstr(XOR(u8"视角\0无声自瞄 (风险)\0\0")), 2);
                        ImGui::Combo(u8cstr(XOR(u8"瞄准部位")), &game_data->settings.i_bone, u8cstr(XOR(u8"头盔\0头部\0颈部\0胸部\0骨盆\0骨骼漏打\0\0")), 6);
                        ImGui::Text(u8cstr(XOR(u8"瞄准快捷键")));
                        KeyBindButton(game_data->settings.aim_key, 260, 25);
                        if (ImGui::Combo(u8cstr(XOR(u8"瞄准预设")), &currentItem, XOR("Legit\0\Semi Legit\0\Rage\0\P-Silent Rage\0\0"), NumPresets))
                        {
                            currentPreset = static_cast<PresetType>(currentItem);
                            ApplySettings(currentPreset);
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("B"), ImVec2(460, 170));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"覆盖"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"覆盖")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 170), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Combo(u8cstr(XOR(u8"覆盖骨骼")), &game_data->settings.A_bone, u8cstr(XOR(u8"左脚\0右脚\0左膝\0右膝\0头盔\0头部\0颈部\0胸部\0骨盆\0\0")), 9);
                        OverrideKeyBindButton(game_data->settings.aim_Override_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("C"), ImVec2(460, 120));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"视觉"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"视觉")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 120), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"视野圈")), &game_data->settings.AIMBOTFOVCIRCLE);
                        //ImGui::Checkbox(u8cstr(XOR(u8"追踪线")), &game_data->settings.snaplines);
                        ImGui::Checkbox(u8cstr(XOR(u8"瞄准目标")), &game_data->settings.aim_target_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"准心")), &game_data->settings.b_crosshair);
                        ImGui::Checkbox(u8cstr(XOR(u8"旋转准心")), &game_data->settings.b_spinny_crosshair);
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("D"), ImVec2(460, 290));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"目标"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"目标")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 290), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"仅敌人")), &game_data->settings.FREINDLYAIMBOTCHECK);
                        ImGui::Checkbox(u8cstr(XOR(u8"可见性检查")), &game_data->settings.AIMBOTCHECK);
                        ImGui::Checkbox(u8cstr(XOR(u8"目标角色")), &game_data->settings.targetaibool);
                        ImGui::Checkbox(u8cstr(XOR(u8"粘性瞄准")), &game_data->settings.isSticky);
                        ImGui::Checkbox(u8cstr(XOR(u8"跳过倒地")), &game_data->settings.skipKnocked);
                        ImGui::Checkbox(u8cstr(XOR(u8"跳过自救")), &game_data->settings.skipselfrevive);
                        ImGui::Combo(u8cstr(XOR(u8"最近目标方式:")), &game_data->settings.targetting_closestby, u8cstr(XOR(u8"视野+距离\0视野\0距离\0\0")));
                        ImGui::Combo(u8cstr(XOR(u8"视野类型:")), &game_data->settings.fov_int, u8cstr(XOR(u8"动态\0静态\0\0")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"瞄准视野")), &game_data->settings.aimbotFov, 1, 180, u8cstr(XOR(u8"%.0f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"最大距离")), &game_data->settings.distanceFloat, 5, 5000.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("E"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"精准度"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"精准度")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderInt(u8cstr(XOR(u8"瞄准平滑度")), &game_data->settings.aim_smooth, 0, 100);
                        ImGui::Checkbox(u8cstr(XOR(u8"预判")), &game_data->settings.Prediction);
                        if (game_data->settings.Prediction)
                        {
                            ImGui::SliderFloat(u8cstr(XOR(u8"预判强度")), &game_data->settings.prediction_value, 0, 100, u8cstr(XOR(u8"%.1f")));
                        }
                        ImGui::Checkbox(u8cstr(XOR(u8"无后座")), &game_data->settings.Norecoil);
                        ImGui::SliderFloat(u8cstr(XOR(u8"后座力")), &game_data->settings.noRecoil, 0.0f, 100.0f, u8cstr(XOR(u8"%.1f")));
                        ImGui::Checkbox(u8cstr(XOR(u8"无摇晃")), &game_data->settings.no_recoil);
                        ImGui::SameLine();
                        ImGui::Checkbox(u8cstr(XOR(u8"无扩散")), &game_data->settings.Nospread);
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 1)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("F"), ImVec2(460, 160));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"反瞄准"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"反瞄准")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 160), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"陀螺仪")), &game_data->settings.Spinbot);
                        ImGui::Checkbox(u8cstr(XOR(u8"抖动机器")), &game_data->settings.jitterbot);
                        ImGui::Checkbox(u8cstr(XOR(u8"180度转身")), &game_data->settings.turned);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("G"), ImVec2(460, 100));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"设置"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"设置")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 100), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        anti_OverrideKeyBindButton(game_data->settings.anti_aim_Override_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 2)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("H"), ImVec2(460, 620));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"视觉"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"视觉")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 620), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"方框透视")), &game_data->settings.unfilled_boxes);
                        ImGui::Checkbox(u8cstr(XOR(u8"角框透视")), &game_data->settings.corner_box);
                        ImGui::Checkbox(u8cstr(XOR(u8"填充方框透视")), &game_data->settings.filled_boxes);
                        ImGui::Checkbox(u8cstr(XOR(u8"3D方框透视")), &game_data->settings.b_player_3dboxes);
                        ImGui::Checkbox(u8cstr(XOR(u8"3D填充方框透视")), &game_data->settings.b_player_filled_3dboxes);
                        ImGui::Checkbox(u8cstr(XOR(u8"骨骼透视")), &game_data->settings.bone_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"头部骨骼透视")), &game_data->settings.head_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"血量条透视")), &game_data->settings.health_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"距离透视")), &game_data->settings.distance_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"玩家名称透视")), &game_data->settings.name_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"气泡透视")), &game_data->settings.bubble_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"队伍ID透视")), &game_data->settings.teamid_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"武器透视")), &game_data->settings.weapon_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"击杀透视")), &game_data->settings.kills_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"死亡透视")), &game_data->settings.deaths_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"玩家轮廓透视")), &game_data->settings.chams);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"非直播防护")));
                        }
                        ImGui::Checkbox(u8cstr(XOR(u8"武器轮廓透视")), &game_data->settings.HUDchams);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"非直播防护")));
                        }
                        ImGui::Checkbox(u8cstr(XOR(u8"指南针透视")), &game_data->settings.realCompass);
                        ImGui::Checkbox(u8cstr(XOR(u8"2D雷达")), &game_data->settings.compass_esp);
                        ImGui::Checkbox(u8cstr(XOR(u8"高级雷达")), &game_data->settings.uavtest);
                        ImGui::Combo(u8cstr(XOR(u8"雷达类型:")), &game_data->settings.uav_type, u8cstr(XOR(u8"点状\0方向性\0\0")));
                        ImGui::Checkbox(u8cstr(XOR(u8"瞄准警告")), &game_data->settings.threat_warnings);
                        ImGui::Checkbox(u8cstr(XOR(u8"子弹轨迹")), &game_data->settings.bullet_tracers);
                        if (game_data->settings.bullet_tracers)
                        {
                            ImGui::Checkbox(u8cstr(XOR(u8"子弹命中检测")), &game_data->settings.show_only_hit_bullet_tracers);
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("I"), ImVec2(460, 280));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"物品透视"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"物品透视")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 280), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(0, 35));
                    ImGui::BeginChild(XOR("J"), ImVec2(460, 243));
                    {
                        ImGui::SetCursorPos(ImVec2(10, 15));
                        ImGui::BeginGroup();
                        {
                            ImGui::Checkbox(u8cstr(XOR(u8"启用物品透视")), &game_data->settings.loot_test);
                            ImGui::Checkbox(u8cstr(XOR(u8"弹药透视")), &game_data->settings.lootammo);
                            ImGui::Checkbox(u8cstr(XOR(u8"武器透视")), &game_data->settings.lootweapons);
                            ImGui::Checkbox(u8cstr(XOR(u8"金钱透视")), &game_data->settings.lootmoney);
                            ImGui::Checkbox(u8cstr(XOR(u8"护甲板透视")), &game_data->settings.lootarmor);
                            ImGui::Checkbox(u8cstr(XOR(u8"补给箱透视")), &game_data->settings.lootsupply);
                            ImGui::Checkbox(u8cstr(XOR(u8"补给缓存透视")), &game_data->settings.loot_cache);
                            ImGui::Checkbox(u8cstr(XOR(u8"杂项透视")), &game_data->settings.lootmisc);
                            ImGui::Checkbox(u8cstr(XOR(u8"物品边界透视")), &game_data->settings.loot_bounds_3d);
                            if (game_data->settings.loot_bounds_3d)
                            {
                                ImGui::Combo(u8cstr(XOR(u8"边界类型:")), &game_data->settings.loot_box_type, u8cstr(XOR(u8"方框\0填充\0\0")));
                            }
                            ImGui::SliderFloat(u8cstr(XOR(u8"物品距离")), &game_data->settings.loot_distanceFloat, 0.f, 500.f, u8cstr(XOR(u8"%.1f")));

                            ImGui::Spacing();
                            ImGui::Spacing();
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("K"), ImVec2(460, 320));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"视觉设置"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"视觉设置")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 320), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"仅敌人")), &game_data->settings.enemies_only);
                        ImGui::Checkbox(u8cstr(XOR(u8"可见性检查")), &game_data->settings.b_visible);
                        ImGui::Checkbox(u8cstr(XOR(u8"显示角色")), &game_data->settings.showaibool);
                        ImGui::Checkbox(u8cstr(XOR(u8"指南针可见性检查")), &game_data->settings.show_only_visible);
                        ImGui::Checkbox(u8cstr(XOR(u8"指南针瞄准警告检查")), &game_data->settings.show_aim_warning_nigs);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"需要启用瞄准警告")));
                        }
                        ImGui::Checkbox(u8cstr(XOR(u8"玩家轮廓不透明填充")), &game_data->settings.real_chamFill);
                        ImGui::Checkbox(u8cstr(XOR(u8"玩家轮廓透明填充")), &game_data->settings.chamFill);
                        ImGui::SliderInt(u8cstr(XOR(u8"玩家轮廓宽度")), &game_data->settings.lineWidthNum, 0, 100);
                        ImGui::SliderInt(u8cstr(XOR(u8"武器轮廓宽度")), &game_data->settings.HUDlineWidthNum, 0, 100);
                        ImGui::SliderFloat(u8cstr(XOR(u8"雷达缩放")), &game_data->settings.flScale, 8.0f, 5000.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"瞄准警告视野")), &game_data->settings.aimwarning_fov, 0.0f, 200.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"轨迹淡出时间")), &game_data->settings.bullet_trace_fade, 0.5f, 10.f, u8cstr(XOR(u8"%.1f")));
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"数值越高，轨迹消失越快")));
                        }
                        ImGui::SliderFloat(u8cstr(XOR(u8"轨迹线粗细")), &game_data->settings.bullet_trace_thickness, 1.f, 10.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"轨迹线X轴粗细")), &game_data->settings.bullet_x_trace_thickness, 1.f, 10.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"指南针缩放")), &game_data->settings.CompassScale, 1.f, 5000.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"指南针箭头大小")), &game_data->settings.arrowSize, 1.f, 100.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::SliderFloat(u8cstr(XOR(u8"指南针箭头填充")), &game_data->settings.arrowfill, 0.f, 255.f, u8cstr(XOR(u8"%.1f")));
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹指南针")), &game_data->settings.Compass_rgb);
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹子弹轨迹")), &game_data->settings.rgb_bullettracer);
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹视野圈")), &game_data->settings.FOV_rgb);
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹玩家轮廓")), &game_data->settings.rgbChams);
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹武器轮廓")), &game_data->settings.HUDrgbChams);
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹准心")), &game_data->settings.rgbcrosshair);

                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 3)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("L"), ImVec2(460, 390));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"清除"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"清除")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 390), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"禁用反无人机")), &game_data->settings.misc_nocounteruav);
                        ImGui::Checkbox(u8cstr(XOR(u8"禁用畏缩效果")), &game_data->settings.NoFlinch);
                        ImGui::Checkbox(u8cstr(XOR(u8"禁用眩晕效果")), &game_data->settings.NoStun);
                        ImGui::Checkbox(u8cstr(XOR(u8"禁用闪光弹效果")), &game_data->settings.NoFlash);
                        ImGui::Checkbox(u8cstr(XOR(u8"禁用盾牌震动")), &game_data->settings.antishake);
                        if (ImGui::Checkbox(u8cstr(XOR(u8"禁用所有阴影")), &b_shadows))
                        {
                            if (b_shadows)
                            {
                                Cbuf_AddText(XOR("set #x353D347C4D236E028 0; set #x3704554F429DAB488 0; set #x3AD42CA33A427DE58 0; set #x38667C0BB90C5BFC3 0; set #x3DF200A089A3B3FEB 0; set #x35D474D39E096F8F1 0;"));
                            }
                            else
                            {
                                Cbuf_AddText(XOR("set #x353D347C4D236E028 1; set #x3704554F429DAB488 1; set #x3AD42CA33A427DE58 1; set #x38667C0BB90C5BFC3 1; set #x3DF200A089A3B3FEB 1; set #x35D474D39E096F8F1 1;"));
                            }
                        }
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"如果功能失效，请尝试重新开关。")));
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("M"), ImVec2(460, 200));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"视角"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"视角")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 200), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"视场角")), &game_data->settings.b_fov);
                        ImGui::SliderFloat(u8cstr(XOR(u8"视场角数值")), &game_data->settings.f_fov, 0.0f, 180.0f, u8cstr(XOR(u8"%.1f")));
                        ImGui::Checkbox(u8cstr(XOR(u8"第三人称")), &game_data->settings.ThirdPerson);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("N"), ImVec2(460, 190));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"武器"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"武器")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 190), ImColor(61, 61, 61, 255), 4.f);
                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"快速射击")), &game_data->settings.Rapidfire);
                        ImGui::Checkbox(u8cstr(XOR(u8"自动射击")), &game_data->settings.autoFire);
                        ImGui::Checkbox(u8cstr(XOR(u8"快速换弹")), &game_data->settings.FastReload);
                        ImGui::Checkbox(u8cstr(XOR(u8"快速切枪")), &game_data->settings.instant_swap);
                        //ImGui::Checkbox(XOR("Server Crasher"), &game_data->settings.testcrasher);
                        //ImGui::Spacing();
                        //ImGui::Checkbox(XOR("Super Slide"), &game_data->settings.superslide);
                        //AirStuck_OverrideKeyBindButton(game_data->settings.slide_key, 260, 25);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("O"), ImVec2(460, 340));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"功能"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"功能")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 340), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 4)
        {
            static int currentSettingIndex = 0;

            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("P"), ImVec2(460, 435));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"设置列表"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"设置列表")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //demo functions
                    const char* items[] = { u8cstr(XOR(u8" 配置 1")), u8cstr(XOR(u8" 配置 2")), u8cstr(XOR(u8" 配置 3")), u8cstr(XOR(u8" 配置 4")), u8cstr(XOR(u8" 配置 5")), u8cstr(XOR(u8" 配置 6")), u8cstr(XOR(u8" 配置 7")), u8cstr(XOR(u8" 配置 8")), u8cstr(XOR(u8" 配置 9")), u8cstr(XOR(u8" 配置 10")) };

                    //render functions
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
                    ImGui::SetCursorPos(ImVec2(0, 50));
                    ImGui::SetNextItemWidth(1000);
                    ImGui::ListBox(XOR("##i"), &currentSettingIndex, items, IM_ARRAYSIZE(items), 10);
                    ImGui::PopStyleColor();

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 435), ImColor(61, 61, 61, 255), 4.f);
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("Q"), ImVec2(460, 150));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"设置按钮"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"设置按钮")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 150), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(70, 50));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Button(u8cstr(XOR(u8"保存设置")), ImVec2(320, 30)))
                        {
                            SaveSettings(currentSettingIndex + 1);
                        }
                        if (ImGui::Button(u8cstr(XOR(u8"加载设置")), ImVec2(320, 30)))
                        {
                            LoadSettings(currentSettingIndex + 1);
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("R"), ImVec2(460, 610));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"自定义颜色"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"自定义颜色")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 610), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(u8cstr(XOR(u8"菜单快捷键")));
                        AirStuck_OverrideKeyBindButton(game_data->settings.slide_key, 150, 25);

                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 16));

                        ImGui::ColorEdit4(u8cstr(XOR(u8"瞄准目标颜色")), (float*)&game_data->settings.Target_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹菜单颜色")), &game_data->settings.RGB_Menu);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见骨骼颜色")), (float*)&game_data->settings.V_skeleton_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(u8cstr(XOR(u8"主菜单颜色")), (float*)&main_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见方框颜色")), (float*)&game_data->settings.vis_boxnotFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::SliderInt(XOR("##j"), &game_data->settings.menu_trans, 0, 255);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(u8cstr(XOR(u8"菜单透明度")));
                        }

                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见文字颜色")), (float*)&game_data->settings.V_distance_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(u8cstr(XOR(u8"瞄准警告颜色")), (float*)&game_data->settings.V_aim_warnings_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见射线颜色")), (float*)&game_data->settings.V_Snaplines_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(u8cstr(XOR(u8"子弹轨迹颜色")), (float*)&game_data->settings.bullettracer_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见轮廓颜色")), (float*)&game_data->settings.vis_player_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::SameLine();
                        ImGui::ColorEdit4(u8cstr(XOR(u8"高级雷达颜色")), (float*)&game_data->settings.UAV_color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身骨骼颜色")), (float*)&game_data->settings.skeleton_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身方框颜色")), (float*)&game_data->settings.boxnotFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身文字颜色")), (float*)&game_data->settings.distance_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身射线颜色")), (float*)&game_data->settings.Snaplines_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身轮廓颜色")), (float*)&game_data->settings.player_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);

                        ImGui::ColorEdit4(u8cstr(XOR(u8"方框填充颜色")), (float*)&game_data->settings.boxFilled_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"视场角圈颜色")), (float*)&game_data->settings.FOVCOLOR_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"准心颜色")), (float*)&game_data->settings.crosshaircolor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"武器轮廓颜色")), (float*)&game_data->settings.HUDplayer_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"可见指南针颜色")), (float*)&game_data->settings.V_COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"隐身指南针颜色")), (float*)&game_data->settings.COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::ColorEdit4(u8cstr(XOR(u8"瞄准警告指南针颜色")), (float*)&game_data->settings.V2_COMPASS_colorVec4, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip);
                        ImGui::PopStyleVar();
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 5)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("X"), ImVec2(460, 180));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"战队标签"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"战队标签")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 180), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        static char name1[6] = "";
                        ImGui::InputTextWithHint(XOR("##k"), u8cstr(XOR(u8"自定义战队标签")), name1, 6);
                        if (ImGui::Button(u8cstr(XOR(u8"应用战队标签")), ImVec2(320, 30)))
                        {
                            CL_PlayerData_SetCustomClanTag(name1);
                            //SendColoredToastNotification(XOR("^2Clantag Applied!"), XOR("cac_equip_fill"));
                        }
                        ImGui::Checkbox(u8cstr(XOR(u8"彩虹名字")), &game_data->settings.rgbname);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("x1"), ImVec2(460, 180));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"地图信息"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"地图信息")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 180), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(u8cstr(XOR(u8"地图名称: %s")), MapName);        // Display map name
                        ImGui::Text(u8cstr(XOR(u8"地图序号: %d")), MapIndex);      // Display map index as integer 
                        ImGui::Text(u8cstr(XOR(u8"地图来源: %d")), MapSource);    // Display map source as integer
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("Y"), ImVec2(460, 100));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"账号信息"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"账号信息")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 100), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Text(u8cstr(XOR(u8"已清理截图数量: %i")), game_data->settings.screenshot_count);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("Z"), ImVec2(460, 300));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"账号"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"账号")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 300), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox(u8cstr(XOR(u8"解锁所有物品")), &game_data->settings.UnlockAllItems);
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(XOR("This Feature is Disabled."));
                        }
                        if (ImGui::Button(u8cstr(XOR(u8"绕过账号名称过滤")), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("#x3BB203010F77299D4 0;"));
                        }
                        if (ImGui::Button(u8cstr(XOR(u8"跳过账号过场动画")), ImVec2(320, 30)))
                        {
                            LUI_OpenMenu(0, XOR(""), 0, 0, 0);
                        }
                        if (ImGui::Button(u8cstr(XOR(u8"强制启动专用服务器")), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("xstartprivateparty; xstartprivatematchlobby; xpartygo; party_startPrivateDSGame;"));
                        }
                        if (ImGui::Button(u8cstr(XOR(u8"强制断开连接")), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(XOR("disconnect;"));
                        }
                        //if (ImGui::Button(XOR("Set Up Exception Hook"), ImVec2(200, 25)))
                        //{
                        //    HMODULE ntdll = GetModuleHandleA(XOR("ntdll"));
                        //    NtContinue = (decltype(NtContinue))GetProcAddress(ntdll, XOR("NtContinue"));

                        //    void(*RtlAddVectoredExceptionHandler)(LONG First, PVECTORED_EXCEPTION_HANDLER Handler) = (decltype(RtlAddVectoredExceptionHandler))GetProcAddress(ntdll, XOR("RtlAddVectoredExceptionHandler"));
                        //    RtlAddVectoredExceptionHandler(0, TopLevelExceptionHandler);
                        //    log_shit("hook attached nigger!");
                        //}

                        //ImGui::InputInt(XOR("Weapon Camo"), &nigger);
                        //ImGui::InputInt(XOR("statsgroup"), &nigger22);

                        //static char name1[256] = "";
                        //ImGui::InputTextWithHint(XOR("##Custommode"), XOR("Custom Mode"), name1, IM_ARRAYSIZE(name1));
                        //
                        //if (ImGui::Button(XOR("set ddl file!"), ImVec2(320, 30)))
                        //{
                        //    char context[255];
                        //    char state[255];
                        //    int navStringCount;
                        //    char* navStrings[16]{};

                        //    CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, nigger22);
                        //    log_shit("Called CL_PlayerData_GetDDLBuffer with context: %s", context);

                        //    log_shit("nigger11");
                        //    __int64 ddl_file = Com_DDL_LoadAsset((__int64)name1);
                        //    log_shit("Loaded DDL file: %lld", ddl_file);

                        //    DDL_GetRootState((__int64)state, ddl_file);
                        //    log_shit("Retrieved root state for ddl_file: %lld", ddl_file);

                        //    char buffer[200];
                        //    memset(buffer, 0, 200);
                        //    sprintf_s(buffer, XOR("squadMembers.loadouts.%i.weaponSetups.%i.camo"), 1, 1);
                        //    log_shit("Formatted buffer: %s", buffer);

                        //    ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
                        //    log_shit("Parsed navigation strings. Count: %d", navStringCount);

                        //    if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
                        //    {
                        //        log_shit("Successfully moved to path.");
                        //        DDL_SetInt((__int64)state, (__int64)context, nigger);
                        //        log_shit("Set integer value at state: %s", state);
                        //    }
                        //    else
                        //    {
                        //        log_shit("Failed to move to path.");
                        //    }
                        //}
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::BeginChild(XOR("CC"), ImVec2(460, 160));
                {
                    //helpers def
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //tite bg
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 460, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    //title
                    drawlist->AddText(center_text(pos, ImVec2(pos.x + 460, pos.y + 35), u8cstr(XOR(u8"命令"))), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"命令")));

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 460, pos.y + 35), ImColor(61, 61, 61, 255));

                    //border
                    drawlist->AddRect(pos, ImVec2(pos.x + 460, pos.y + 160), ImColor(61, 61, 61, 255), 4.f);

                    //render funcs
                    ImGui::SetCursorPos(ImVec2(10, 50));
                    ImGui::BeginGroup();
                    {
                        static char name1[256] = "";
                        ImGui::InputTextWithHint(XOR("##l"), u8cstr(XOR(u8"自定义命令")), name1, IM_ARRAYSIZE(name1));
                        if (ImGui::Button(u8cstr(XOR(u8"发送命令")), ImVec2(320, 30)))
                        {
                            Cbuf_AddText(name1);
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

        if (tabs == 6)
        {
            ImGui::SetCursorPos(ImVec2(25, 80));
            ImGui::BeginGroup();
            {
                ImGui::BeginChild(XOR("m1"), ImVec2(950, 620));
                {
                    // Helper definitions
                    ImDrawList* drawlist = ImGui::GetWindowDrawList();
                    ImVec2 pos = ImGui::GetWindowPos();

                    // Title background
                    drawlist->AddRectFilled(pos, ImVec2(pos.x + 950, pos.y + 35), ImColor(30, 30, 30, 255), 4.f, ImDrawFlags_RoundCornersTop);

                    // Title text
                    ImVec2 textPos = ImVec2(pos.x + (950 / 2), pos.y + (35 / 2));
                    drawlist->AddText(ImVec2(textPos.x - ImGui::CalcTextSize(u8cstr(XOR(u8"房间"))).x / 2, textPos.y - ImGui::GetTextLineHeight() / 2), ImColor(255, 255, 255, 255), u8cstr(XOR(u8"房间")));

                    // Separator line
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 950, pos.y + 35), ImColor(61, 61, 61, 255));

                    // Border
                    drawlist->AddRect(pos, ImVec2(pos.x + 950, pos.y + 620), ImColor(61, 61, 61, 255), 4.f);

                    // Render functions
                    ImGui::SetCursorPos(ImVec2(0, 50));
                    ImGui::BeginGroup();
                    {
                        ImGui::SetCursorPosX(10);
                        if (ImGui::BeginTable(u8cstr(XOR(u8"玩家列表")), 6, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
                        {
                            // Set up table headers with specific widths
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"队伍编号")), ImGuiTableColumnFlags_None, 60.0f);
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"玩家名称")), ImGuiTableColumnFlags_None, 120.0f);
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"延迟")), ImGuiTableColumnFlags_None, 50.0f);
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"击杀")), ImGuiTableColumnFlags_None, 40.0f);
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"XUID")), ImGuiTableColumnFlags_None, 100.0f);
                            ImGui::TableSetupColumn(u8cstr(XOR(u8"等级")), ImGuiTableColumnFlags_None, 10.0f);
                            ImGui::TableHeadersRow();
                            if (game_data->settings.checkPassed && game_data->settings.b_ingame)
                            {
                                int numPlayers = *(int*)(game_data->game_offsets.gamemode);
                                for (int i = 0; i < numPlayers; ++i)
                                {
                                    player* p = &players[i];
                                    ImGui::TableNextRow();
                                    // First Column (Team ID)
                                    ImGui::TableNextColumn();
                                    char teamIdLabel[32];
                                    sprintf_s(teamIdLabel, u8cstr(XOR(u8"队伍 %d")), p->team);
                                    ImGui::Text(teamIdLabel);
                                    // Second Column (Player Name)
                                    ImGui::TableNextColumn();
                                    ImGui::Text(p->player_name.name);
                                    // Third Column (Ping)
                                    ImGui::TableNextColumn();
                                    char pingLabel[32];
                                    sprintf_s(pingLabel, u8cstr(XOR(u8"%d 毫秒")), p->ping);
                                    ImGui::Text(pingLabel);
                                    // Fourth Column (Kills)
                                    ImGui::TableNextColumn();
                                    char killsLabel[32];
                                    sprintf_s(killsLabel, XOR("%d"), p->kills);
                                    ImGui::Text(killsLabel);
                                    // Fifth Column (XUID)
                                    ImGui::TableNextColumn();
                                    char xuidLabel[32];
                                    sprintf_s(xuidLabel, XOR("%llu"), p->xuid);
                                    ImGui::Text(xuidLabel);
                                    // Sixth Column (Rank)
                                    ImGui::TableNextColumn();
                                    char rankLabel[32];
                                    sprintf_s(rankLabel, XOR("%d"), p->rank_mp);
                                    ImGui::Text(rankLabel);
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }
            ImGui::EndGroup();
        }

    }

    ImGui::PopFont();
    ImGui::End();

    VMProtectEnd();
}