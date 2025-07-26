#include "main.h"
#include "spoofer/spoof.h"
#include "hook_handler.cpp"
#include "spoofer/callstack.h"
#include "protect/vmp.h"

using hash_t = uint32_t;

namespace shared::hash
{
	constexpr uint64_t BASIS = 0x811c9dc5;
	constexpr uint64_t PRIME = 0x1000193;

	// Compile-time hash calculation
	inline constexpr hash_t get_const(const char* txt, const hash_t value = BASIS) noexcept
	{
		return (txt[0] == '\0') ? value :
			get_const(&txt[1], (value ^ hash_t(txt[0])) * PRIME);
	}

	// Run-time hash calculation
	inline hash_t get(const char* txt)
	{
		hash_t ret = BASIS;
		hash_t length = hash_t(strlen(txt));
		for (auto i = 0u; i < length; ++i)
		{
			ret ^= txt[i];
			ret *= PRIME;
		}
		return ret;
	}
}

#define CT_HASH(str) \
    ([&]() { \
        constexpr hash_t ret = shared::hash::get_const(str); \
        return ret; \
    }())

#define HASH(str) shared::hash::get(str)

//void Log_(const char* fmt, ...) {
//	char		text[4096];
//	va_list		ap;
//	va_start(ap, fmt);
//	vsprintf_s(text, fmt, ap);
//	va_end(ap);
//
//	std::ofstream logfile(XOR("C:\\debug\\_debuglog.logs"), std::ios::app);
//	if (logfile.is_open() && text)	logfile << text << std::endl;
//	logfile.close();
//}

namespace ctx
{

	mem_t mem = mem_t{};
}

static bool keyPressedLastFrame = false;

typedef long(__fastcall* Present)(IDXGISwapChain3*, UINT, UINT);
Present o_Present = nullptr;

HRESULT hkfunc(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
VMProtectBeginMutation("hkfunc");
	if (!pSwapChain)
	{
		VMProtectEnd();
		return o_Present(pSwapChain, SyncInterval, Flags);
	}

	g_game::hookcheck();

	DXGI_SWAP_CHAIN_DESC sdesc;
	if (FAILED(pSwapChain->GetDesc(&sdesc)))
	{
		VMProtectEnd();
		return o_Present(pSwapChain, SyncInterval, Flags);
	}

	if (sdesc.OutputWindow == GetForegroundWindow())
	{
		HWND hwnd = sdesc.OutputWindow;
		game_data->hWind = hwnd;

		if (!imgui::is_ready)
		{
			RECT rect;
			GetWindowRect(sdesc.OutputWindow, &rect);
			ImVec2 viewPort = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
			float scaling_factor = viewPort.y / 1080.0f;

			game_data->settings.compassSize = 300.0f * scaling_factor;
			game_data->settings.arrowSize = 27.0f * scaling_factor;
			compass_pos = { viewPort.x / 2.0f, viewPort.y / 2.0f };
		}

		ImFont* main_font = imgui::start(
			reinterpret_cast<IDXGISwapChain3*>(*(uintptr_t*)(ctx::mem.swap_chain)),
			reinterpret_cast<ID3D12CommandQueue*>(*(uintptr_t*)(ctx::mem.command_queue)),
			nullptr, 12.f
		);

		if (game_data->settings.slide_key != VK_LBUTTON && is_key_down(game_data->settings.slide_key))
		{
			if (!keyPressedLastFrame)
			{
				game_data->settings.b_menuEnable = !game_data->settings.b_menuEnable;
				keyPressedLastFrame = true;
			}
		}
		else
		{
			keyPressedLastFrame = false;
		}

		imgui::imgui_frame_header();

		menu->DrawMenu();

		imgui::imgui_no_border(g_game::init, main_font);

		imgui::imgui_frame_end();
	}

	VMProtectEnd();

	return o_Present(pSwapChain, SyncInterval, Flags);
}
\
void Initialize()
{
	game_data = new global_vars();
	game_data->baseModule = reinterpret_cast<QWORD>(GetModuleHandleA(nullptr));
}

DWORD64 resolveRelativeAddress(DWORD64 instr, DWORD offset, DWORD instrSize) {
	return instr == 0ui64 ? 0ui64 : (instr + instrSize + *(int*)(instr + offset));
}

void hook(const char* name, uintptr_t function, uintptr_t& original)
{
	if (game_data->game_offsets.HOOK_FUNC)
	{
		struct wtv {
			char* qbz;
			void* oei;
			wtv* somethingelse;
			uintptr_t ppp;
		};

		for (auto btza = reinterpret_cast<wtv*>(game_data->game_offsets.HOOK_FUNC);
			btza;
			btza = btza->somethingelse)
		{
			if (!strcmp(btza->qbz, name))
			{
				original = btza->ppp;
				btza->ppp = function;
				break;
			}
		}
	}
}

bool InitHook(IDXGISwapChain3* pSwapChain)
{
	if (!pSwapChain) return false;

	void** vTable = *reinterpret_cast<void***>(pSwapChain);

	if (!vTable) return false; // Ensure vTable is valid

	if (MH_CreateHook(vTable[8], &hkfunc, reinterpret_cast<void**>(&o_Present)) != MH_OK) {
		return false; // Hook creation failed
	}

	if (MH_EnableHook(vTable[8]) != MH_OK) {
		MH_RemoveHook(vTable[8]); // Cleanup if enabling fails
		return false;
	}

	return true;
}

void entry()
{
	VMProtectBeginMutation("x");

	if (MH_Initialize() == MH_OK)
	{
		Initialize();

		game_data->game_offsets.trampoline = utility::find_ida_sig(NULL, XOR("FF 23"));

		game_data->game_offsets.HOOK_FUNC = utility::find_ida_sig(NULL, XOR("48 8B 05 ? ? ? ? 33 D2 48 85 C0 74 23 0F 1F 44 ? ?")).self_jmp(0x3);

		game_data->game_offsets.HOOK_FUNC = *(uintptr_t*)(game_data->game_offsets.HOOK_FUNC);

		hook(XOR("cg.alwaysLoaded.telemetry.volume_mute"), (uintptr_t)think, game_data->game_offsets.HookOriginalFunction);

		game_data->game_offsets.s_blackboxInitialized = utility::find_ida_sig(NULL, XOR("C6 05 ? ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?")).self_jmp(2).self_offset(0x1);

		game_data->game_offsets.CbufOffset = utility::find_ida_sig(NULL, XOR("3D 3F B0 8E CB F2 F5 EE ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ?")) + 8;
		game_data->game_offsets.CbufOffset = *(uintptr_t*)(game_data->game_offsets.CbufOffset);

		game_data->game_offsets.xpartyOffset = utility::find_ida_sig(NULL, XOR("78 70 61 72 74 79 64 69 73 62 61 6E 64 61 66 74 65 72 72 6F 75 6E 64 0A ? ? ? ?"));

		game_data->game_offsets.fov_hook = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? C5 FA 10 10 EB")).self_jmp(1);

		game_data->game_offsets.fov = utility::find_ida_sig(NULL, XOR("C5 FA 11 83 ? ? ? ? E8 ? ? ? ? C5 FA 58 54 24"));
		if (game_data->game_offsets.fov != 0x0)
		{
			game_data->game_offsets.fov = *(int*)(game_data->game_offsets.fov + 0x4);
		}

		// WEAPON SHIT
		game_data->game_offsets.BG_WeaponFireRecoilOffset = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 30 49 8B 6B 38 49 8B 73 40 C4 C1 78 28 73 ? C4 C1 78 28 7B ? C4 41 78 28 43 ? 49 8B E3")).self_jmp(1);
		game_data->game_offsets.BG_CalculateFinalSpreadForWeapon = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 8B 8B ? ? ? ? 8B C1 D1 E8 A8 ? 75 ? 8B 93 ? ? ? ? 8B C2 C1 E8 ? A8 ? 75 ? 8B C1 C1 E8 ? A8 ? 75 ? C1 E9 ? F6 C1 ? 75 ? F6 C2 ? 74 ? 40 B7 ? EB ? 40 32 FF 49 8B 07")).self_jmp(1);

		// CHAM SHIT
		game_data->game_offsets.R_AddDObjToScene = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EE ? 0F 85 ? ? ? ? 48 8B 74 24 ? 48 8B 5C 24")).self_jmp(1);
		game_data->game_offsets.R_AddViewmodelDObjToScene = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 49 8B D4 8B CE E8 ? ? ? ? 89 84 1F ? ? ? ?")).self_jmp(1);

		game_data->game_offsets.gamemode = utility::find_ida_sig(NULL, XOR("8B 15 ? ? ? ? E8 ? ? ? ? FF C3 89 07 48 8D 7F 04")).self_jmp(0x2);

		game_data->game_offsets.Com_FrontEnd_IsInFrontEnd = utility::find_ida_sig(NULL, XOR("80 3D ? ? ? ? ? 75 ? C6 05 ? ? ? ? ? E8 ? ? ? ? 48 8B 0D")).self_jmp(0x2).self_offset(1);

		game_data->game_offsets.cameraBaseOffset = utility::find_ida_sig(NULL, { XOR("48 8B 05 ? ? ? ? 48 8B 7C 24 ? 48 05 ? ? ? ? 48 69 CA ? ? ? ? 48 03 C1 C3") }).self_jmp(0x3);

		game_data->game_offsets.other = utility::find_ida_sig(NULL, { XOR("48 8D 0D ? ? ? ? C5 E8 57 D2 C5 F0 57 C9 C5 FA 11 44 24 ? E8 ? ? ? ? 8B 0D ? ? ? ?") }).self_jmp(0x3);
		game_data->game_offsets.cam = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 4C 8D 4D DF 49 8B D6 4C 8D 45 EF") }).self_jmp(1);


		game_data->game_offsets.G_Main_SightTrace = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 84 C0 74 ? FF C3 48 83 C7 ? 83 FB ? 0F 82")).self_jmp(1);

		//game_data->game_offsets.LegacyTrace = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? C5 FA 10 3E C5 F8 2E 3D")).self_jmp(1);

		//game_data->game_offsets.nameArray = utility::find_ida_sig(NULL, { XOR("48 8D 0D ? ? ? ? 48 8B 0C D1 8B D3 48 8B 01 FF 90") }).self_jmp(0x3);

		game_data->game_offsets.GetClientInfo = utility::find_ida_sig(NULL, XOR("4C 8D 0D ? ? ? ? 8B D7 4D")).self_jmp(0x3);

		game_data->game_offsets.GetClientInfo_vtbl = reinterpret_cast<void**>(game_data->game_offsets.GetClientInfo);

		game_data->game_offsets.CG_ScoreboardMP_GetClientScore = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 4C 8B C8 45 8B C5 48 8B D6")).self_jmp(1);

		uintptr_t refdef_ptr_temp = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8D 0D ? ? ? ? 4C 8B C6 33 0D"));
		game_data->game_offsets.refdef_ptr = resolveRelativeAddress(refdef_ptr_temp + 0x5, 3, 7);

		game_data->game_offsets.A_CG_VehicleCam_SetClientViewAngles = utility::find_ida_sig(NULL, XOR("40 55 53 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 63 81"));

		game_data->game_offsets.GetGestureFunc = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 8B D3 48 8B CE 48 8B F8 E8 ? ? ? ? 8B 4F 44")).self_jmp(1);

		game_data->game_offsets.cg_offset = utility::find_ida_sig(NULL, XOR("4C 8D 83 ? ? ? ? BA ? ? ? ? 48 8B CF FF 97 ? ? ? ? 40 84 ED"));
		if (game_data->game_offsets.cg_offset != 0x0)
		{
			game_data->game_offsets.cg_offset = *(uint32_t*)(game_data->game_offsets.cg_offset + 0x3);
		}
		else
		{
			game_data->game_offsets.cg_offset = utility::find_ida_sig(NULL, XOR("4C 8D ? ? BA ? ? ? ? 48 8B CF FF 97 ? ? ? ? 40 84 ED"));
			if (game_data->game_offsets.cg_offset != 0x0)
			{
				game_data->game_offsets.cg_offset = *(uint8_t*)(game_data->game_offsets.cg_offset + 0x3);
			}
		}

		game_data->game_offsets.playerStatePadding = utility::find_ida_sig(NULL, XOR("44 0F B6 C8 48 8D 96"));
		if (game_data->game_offsets.playerStatePadding != 0x0)
		{
			game_data->game_offsets.playerStatePadding = *(int*)(game_data->game_offsets.playerStatePadding + 0x7);
		}

		game_data->game_offsets.CG_StartShellShock = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 44 8B ? ? ? ? ? 41 8B CE 44 8B")).self_jmp(1);

		//game_data->game_offsets.CG_DrawDamageDirectionIndicators = utility::find_ida_sig(NULL, XOR("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 63 E1"));

		//game_data->game_offsets.CG_GetPoseOrigin = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? B0 ? E9 ? ? ? ? 48 69 CB")).self_jmp(1);

		//game_data->game_offsets.A_BG_ThirdPerson_IsEnabled = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 84 C0 74 ? 48 8B D6 8B CD")).self_jmp(1);

		game_data->game_offsets.BG_GetWeaponFireType = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? B2 ? 0F B6 D8 48 8D 4C 24") }).self_jmp(0x1);

		game_data->game_offsets.A_CL_Input_ClearAutoForwardFlag = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 8B CD E8 ? ? ? ? 8B CD E8 ? ? ? ? 8B CD E8 ? ? ? ? 4C 8B C3") }).self_jmp(1);

		game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 C1 E2 06 48 ? ? ? ? ? ? 48 03 D7 48 8D ? ? ? ? ? ? 33 C0") });
		if (game_data->game_offsets.usercmdPos == 0x0)
		{
			game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 C1 E2 06 48 ? ? ? 48 03 D7 48 8D ? ? ? ? ? ? 33 C0") });
			if (game_data->game_offsets.usercmdPos == 0x0)
			{
				game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 81 C7 ? ? ? ? 83 E6 7F 41 B8 ? ? ? ? 41 8B C8 48 8D 14 F6") });
				if (game_data->game_offsets.usercmdPos == 0x0)
				{
					game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 ? ? ? 83 E6 7F 48 69 C6 ? ? ? ? 41 B8 ? ? ? ? 48 8D BD ? ? ? ?") });
					if (game_data->game_offsets.usercmdPos == 0x0)
					{
						game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 ? ? ? 83 E6 7F 41 B8 ? ? ? ? 41 ? C8") });
						if (game_data->game_offsets.usercmdPos == 0x0)
						{
							game_data->game_offsets.usercmdPos = utility::find_ida_sig(NULL, { XOR("48 81 C7 ? ? ? ? 83 E6 ? 48 69 D6 ? ? ? ? 41 B8 ? ? ? ?") });
							if (game_data->game_offsets.usercmdPos == 0x0)
							{
								game_data->game_offsets.usercmdPos = 0x1;
							}
							else
							{
								game_data->game_offsets.usercmdPos = *(int*)(game_data->game_offsets.usercmdPos + 0x3);
							}
						}
						else
						{
							game_data->game_offsets.usercmdPos = *(uint8_t*)(game_data->game_offsets.usercmdPos + 0x3);
						}
					}
					else
					{
						game_data->game_offsets.usercmdPos = *(uint8_t*)(game_data->game_offsets.usercmdPos + 0x3);
					}
				}
				else
				{
					game_data->game_offsets.usercmdPos = *(int*)(game_data->game_offsets.usercmdPos + 0x3);
				}
			}
			else
			{
				game_data->game_offsets.usercmdPos = *(uint8_t*)(game_data->game_offsets.usercmdPos + 0x7);
			}
		}
		else
		{
			game_data->game_offsets.usercmdPos = *(int*)(game_data->game_offsets.usercmdPos + 0x7);
		}

		uintptr_t sig_result = utility::find_ida_sig(NULL, { XOR("8D 87 ? ? ? ? 33 87 ? ? ? ? 8D 58 02 0F AF D8 33 ? ? ? ? ?") });
		if (sig_result != 0x0)
		{
			game_data->game_offsets.cmdNumberPos = *(int*)(sig_result + 0x2);
			game_data->game_offsets.cmdNumberaabPos = *(int*)(sig_result + 0x8);
		}
		else
		{
			sig_result = utility::find_ida_sig(NULL, { XOR("8D ? ? 33 87 ? ? ? ? 8D 58 02 0F AF D8 33 ? ?") });
			if (sig_result != 0x0)
			{
				game_data->game_offsets.cmdNumberPos = *(__int8*)(sig_result + 0x2);
				game_data->game_offsets.cmdNumberaabPos = *(int*)(sig_result + 0x5);
			}
			else
			{
				sig_result = utility::find_ida_sig(NULL, { XOR("8D 87 ? ? ? ? 33 47 ? ? ? ? 0F AF D8 33 9F ? ? ? ?") });
				if (sig_result != 0x0)
				{
					game_data->game_offsets.cmdNumberPos = *(int*)(sig_result + 0x2);
					game_data->game_offsets.cmdNumberaabPos = *(__int8*)(sig_result + 0x8);
				}
			}
		}

		game_data->game_offsets.userCmdSize = 0x268;

		// CmdPackAngle
		game_data->game_offsets.A_BG_UsrCmdPackAngle = utility::find_ida_sig(NULL, { XOR("66 C1 E0 05 66 0B C7") }) + 0x13;

		game_data->game_offsets.getCGHandler = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 48 8B CF 48 8B D8 48 8D 3D") }).self_jmp(1);
		if (game_data->game_offsets.getCGHandler == 0x0)
		{
			game_data->game_offsets.getCGHandler = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 44 0F BF 43 ? 4C 8D 4C 24 ? 48 8D 54 24 ?") }).self_jmp(1);
		}

		game_data->game_offsets.BG_GetBallisticInfo = utility::find_ida_sig(NULL, { XOR("40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 41 0F B6 C0 48 8B DA") });


		game_data->game_offsets.CG_GetEntity = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 8B CD 48 8B F0 0F BF ?") }).self_jmp(1);

		game_data->game_offsets.CG_GetEntWeapon = utility::find_ida_sig(NULL, { XOR("E8 ? ? ? ? 8D 4B ? 83 F9") }).self_jmp(1);

		uintptr_t PredictPlayerState_temp = utility::find_ida_sig(NULL, XOR("40 53 48 83 EC ? 8B D9 E8 ? ? ? ? 8B CB E8 ? ? ? ? 8B CB E8 ? ? ? ? E8"));
		game_data->game_offsets.PredictPlayerState = resolveRelativeAddress(PredictPlayerState_temp + 0x22, 1, 5);

		game_data->game_offsets.a_BulletHitEvent_Internal = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? C5 78 28 84 24 ? ? ? ? C5 F8 28 BC 24 ? ? ? ? C5 F8 28 B4 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? C5 78 28 8C 24")).self_jmp(1);

		game_data->game_offsets.PredictedPlayerEntity = utility::find_ida_sig(NULL, XOR("48 8B 86 ? ? ? ? C5 F2 5C 80")); // old  // 8B 0D ? ? ? ? C5 78 28 B4 24 ? ? ? ? C5 78 28
		if (game_data->game_offsets.PredictedPlayerEntity != 0x0)
		{
			game_data->game_offsets.PredictedPlayerEntity = *(int*)(game_data->game_offsets.PredictedPlayerEntity + 0x3);
		}

		game_data->game_offsets.A_DrawMapLayer = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 80 7F 05 ? 74 0E 4D 8B C7 49 8B D7")).self_jmp(1);

		game_data->game_offsets.A_WorldPosToCompass = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 0F B6 85 ? ? ? ? 4C 8D 8D")).self_jmp(1);

		game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("4C 8D 86 ?? ?? ?? ?? 49 8B CD 48 8D 54 24 ?? E8 ?? ?? ?? ?? C5 FA 10 05"));
		if (game_data->game_offsets.angle_offset != 0x0)
		{
			game_data->game_offsets.angle_offset = *(uint32_t*)(game_data->game_offsets.angle_offset + 3);
		}
		else
		{
			game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("4C 8D 46 ?? 49 8B CD 48 8D 54 24 ?? E8 ?? ?? ?? ?? C5 FA 10 05"));
			if (game_data->game_offsets.angle_offset != 0x0)
			{
				game_data->game_offsets.angle_offset = *(uint8_t*)(game_data->game_offsets.angle_offset + 3);
			}
			else
			{
				game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("48 63 44 24 ?? 48 69 C8 ?? ?? ?? ?? 41 8B 47 7C"));
				if (game_data->game_offsets.angle_offset != 0x0)
				{
					game_data->game_offsets.angle_offset = *(int*)(game_data->game_offsets.angle_offset + 19);
				}
				else
				{
					game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("4C 8D 87 ?? ?? ?? ?? 48 8B CE 48 8D 55 A8"));
					if (game_data->game_offsets.angle_offset != 0x0)
					{
						game_data->game_offsets.angle_offset = *(uint32_t*)(game_data->game_offsets.angle_offset + 3);
					}
					else
					{
						game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("4C 8D 87 ?? ?? ?? ?? 48 8B CE 48 8D 55"));
						if (game_data->game_offsets.angle_offset != 0x0)
						{
							game_data->game_offsets.angle_offset = *(uint32_t*)(game_data->game_offsets.angle_offset + 3);
						}
						else
						{
							game_data->game_offsets.angle_offset = utility::find_ida_sig(NULL, XOR("4C 8D 47 ? 48 8B CE 48 8D 55"));
							if (game_data->game_offsets.angle_offset != 0x0)
							{
								game_data->game_offsets.angle_offset = *(uint32_t*)(game_data->game_offsets.angle_offset + 3);
							}
						}
					}
				}
			}
		}
		game_data->game_offsets.kickAngles = utility::find_ida_sig(NULL, XOR("4C 8D B7 ? ? ? ? 4C 8D BF ? ? ? ? 41 8B C6"));
		if (game_data->game_offsets.kickAngles != 0x0)
		{
			game_data->game_offsets.kickAngles = *(uint32_t*)(game_data->game_offsets.kickAngles + 0x3);
		}
		else
		{
			game_data->game_offsets.kickAngles = utility::find_ida_sig(NULL, XOR("49 8D ? ? ? ? ? 49 8D ? ? ? ? ? 8B 57"));
			if (game_data->game_offsets.kickAngles != 0x0)
			{
				game_data->game_offsets.kickAngles = *(uint32_t*)(game_data->game_offsets.kickAngles + 0x3);
			}
			else
			{
				game_data->game_offsets.kickAngles = utility::find_ida_sig(NULL, XOR("49 8D ? ? ? ? ? 8B 57 0C 49 8D"));
				if (game_data->game_offsets.kickAngles != 0x0)
				{
					game_data->game_offsets.kickAngles = *(uint32_t*)(game_data->game_offsets.kickAngles + 0xD);
				}
			}
		}

		game_data->game_offsets.A_CG_CycleWeapon = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? EB ? 66 44 89 7B")).self_jmp(1);

		game_data->game_offsets.A_BG_GetAmmoInClipForWeapon = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B 4C 24 ? 41 B4")).self_jmp(1);

		game_data->game_offsets.A_BG_GetClipSize = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 43 3B 84 3E")).self_jmp(1);

		game_data->game_offsets.BG_Ladder_IsActive_offset = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 45 84 F6 74 ? 84 C0 B9")).self_jmp(1);

		game_data->game_offsets.AimTargetMP_GetTargetBounds = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 4C 8D 44 24 ? 48 8B D5 8B CB")).self_jmp(1);

		//address[A_CG_GetEntWeapon] = SCAN(XOR("48 63 C1 4C 8D 05 ?? ?? ?? ?? 4D 8B 04 C0 0F B7 82 ?? ?? ?? ?? 48 8D 0C C0 49 8B 40 08 48 8D 04"));

		game_data->game_offsets.A_BG_GetWeaponDisplayName = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 48 BA ?? ?? ?? ?? ?? ?? ?? ?? 48 8D 4B 10")).self_jmp(1);
		game_data->game_offsets.A_UI_SafeTranslateString_Hash = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 4C 8B F0 48 89 05 ?? ?? ?? ?? 48 8D 4C 24")).self_jmp(1);

		//game_data->game_offsets.UI_SafeTranslateString = utility::find_ida_sig(NULL, XOR("40 53 48 83 EC ? 80 39 1F 48 8B D9"));

		//game_data->game_offsets.A_GetCache = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? 44 0F B7 85 ?? ?? ?? ?? 33 C0")).self_jmp(1);

		//game_data->game_offsets.A_GetMaxLootScriptableIndex = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 3B D8 77 2D 8B CB 48 8D 15")).self_jmp(1);

		//game_data->game_offsets.A_GetLootItemFromScriptableIndex = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 48 85 C0 74 20 48 8B 10 48 8B CE")).self_jmp(1);

		//game_data->game_offsets.A_UI_SafeTranslateString_Hash = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 4C 8B F0 48 89 05 ?? ?? ?? ?? 48 8D 4C 24")).self_jmp(1);

		//game_data->game_offsets.A_CG_Weapons_GetWeaponForName = utility::find_ida_sig(NULL, XOR("40 53 48 83 EC 20 48 8B C2 4C 8D 05 ?? ?? ?? ?? 48 8B D1 48 8B D9"));

		//game_data->game_offsets.A_CG_GetWeaponDisplayName = utility::find_ida_sig(NULL, XOR("40 55 53 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 66 83 39 00 44 0F B6 EA 4C 89 4C 24 ? 48 8B D9"));

		//game_data->game_offsets.BG_CompleteWeaponDef = utility::find_ida_sig(NULL, XOR("48 8D 15 ? ? ? ? 4C 8D 44 24 ? B9 ? ? ? ? 48 8B 14 C2")).self_jmp(0x3);

		//uintptr_t A_GetLootTable = utility::find_ida_sig(NULL, XOR("48 8D 0D ?? ?? ?? ?? 48 8B 0C D9 38 81 ?? ?? ?? ?? 0F 94 C0"));
		//game_data->game_offsets.A_GetLootTable = resolveRelativeAddress(A_GetLootTable, 3, 7);

		//uintptr_t A_GetOriginAngles = utility::find_ida_sig(NULL, XOR("48 8D 15 ? ? ? ? E8 ? ? ? ? C5 FA 10 15"));
		//if (A_GetOriginAngles != NULL)
		//{
		//	game_data->game_offsets.A_GetOriginAngles = resolveRelativeAddress(A_GetOriginAngles, 3, 7);
		//	game_data->game_offsets.A_GetOriginAngles = *(__int64*)(game_data->game_offsets.A_GetOriginAngles + 16);
		//}

		//game_data->game_offsets.A_IsScriptableUsable = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 44 0F B7 93")).self_jmp(1);

		//game_data->game_offsets.A_GetAbsBoundsCallback = utility::find_ida_sig(NULL, XOR("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 56 41 57 48 83 EC 30 48 63 D9 4C 8D 3D"));

		game_data->game_offsets.equippedWeaponsPosition = utility::find_ida_sig(NULL, XOR("49 8D BF ? ? ? ? 0F 1F 44 00 ? 0F B7 07"));
		if (game_data->game_offsets.equippedWeaponsPosition == 0)
		{
			game_data->game_offsets.equippedWeaponsPosition = 0x9A0;
		}
		else
		{
			game_data->game_offsets.equippedWeaponsPosition = *(uint32_t*)(game_data->game_offsets.equippedWeaponsPosition + 0x3);
		}

		game_data->game_offsets.A_CG_Entity_GetPlayerViewOrigin = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B 4D ? 48 8D 85 ? ? ? ? 48 89 44 24")).self_jmp(1);

		uintptr_t BG_GetBallisticMuzzleVelocityScale_temp = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? C5 FA 59 43 ? 33 D2"));
		game_data->game_offsets.BG_GetBallisticMuzzleVelocityScale = resolveRelativeAddress(BG_GetBallisticMuzzleVelocityScale_temp, 1, 5);

		uintptr_t BG_Ballistics_HorizontalTravelTimeToTarget_temp = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 44 0F B6 86 ? ? ? ? 48 8B D6"));
		game_data->game_offsets.BG_Ballistics_HorizontalTravelTimeToTarget = resolveRelativeAddress(BG_Ballistics_HorizontalTravelTimeToTarget_temp, 1, 5);

		uintptr_t CG_CalcTargetVelocity_temp = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? C5 FC 10 44 24 ?? C5 F8 10 4C 24 ?? C5 78 28 84 24"));
		game_data->game_offsets.CG_CalcTargetVelocity = resolveRelativeAddress(CG_CalcTargetVelocity_temp, 1, 5);


		game_data->game_offsets.P_gamePads = utility::find_ida_sig(NULL, XOR("48 8D 05 ? ? ? ? 48 03 C8 0F BA E2 1C")).self_jmp(0x3);
		game_data->game_offsets.A_GPad_GetButton = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? C5 FA 59 CE BA ? ? ? ? 33 C9 C5 FA 2C F9 E8")).self_jmp(1);

		game_data->game_offsets.a_LUI_OpenMenu = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? E9 ? ? ? ? 41 B9 ? ? ? ? 4C 8D 84 24")).self_jmp(1);

		game_data->game_offsets.CL_PlayerData_SetCustomClanTag = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 4C 8B 7C 24 ? 4C 8B 74 24 ? 4C 8B 6C 24 ? 4C 8B 64 24 ? 48 8B 74 24 ? 48 8B 6C 24 ? 48 8B 4C 24")).self_jmp(1);

		game_data->game_offsets.CG_EntityMP_CalcLerpPositions = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B D3 41 8B CD E8 ? ? ? ? 84 C0")).self_jmp(1);

		game_data->game_offsets.GetItemQuanity = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 33 C9 48 8D 7F")).self_jmp(1);

		game_data->game_offsets.LUI_GetEntityTagPositionAndAxis = utility::find_ida_sig(NULL, XOR("40 53 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B 84 24 ? ? ? ? 4C 8D 2D"));

		//game_data->game_offsets.Live_GetMapIndex = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 85 C0 79 ? 83 CB")).self_jmp(1);
		//game_data->game_offsets.Live_GetMapSource = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 0B D8 0F B7 86")).self_jmp(1);
		//game_data->game_offsets.Party_GetMapName = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 44 8B 8B")).self_jmp(1);
		////game_data->game_offsets.Party_GetXuid = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 33 DB 4C 69 F6")).self_jmp(1);
		//game_data->game_offsets.Party_GetActiveParty = utility::find_ida_sig(NULL, XOR("80 3D ? ? ? ? ? 74 ? 80 3D ? ? ? ? ? 74 ? 48 8D 05 ? ? ? ? C3"));
		//game_data->game_offsets.Party_GetPlatformId = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B 44 24 ? EB ? 33 C0 8B D3")).self_jmp(1);
		//game_data->game_offsets.Party_FindMemberByXUID = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 8B D8 3B F0")).self_jmp(1);

		game_data->game_offsets.A_GetBRPlayerFlags = utility::find_ida_sig(NULL, XOR("8B 41 70 C1 E8 08 C3"));

		game_data->game_offsets.nextState = utility::find_ida_sig(NULL, XOR("0F BF 93 ?? ?? ?? ?? B8"));
		if (game_data->game_offsets.nextState != 0x0)
		{
			game_data->game_offsets.nextState = *(int*)(game_data->game_offsets.nextState + 0x3);
		}
		else
		{
			game_data->game_offsets.nextState = utility::find_ida_sig(NULL, XOR("0F BF 53 ? B8"));
			if (game_data->game_offsets.nextState != 0x0)
			{
				game_data->game_offsets.nextState = *(uint8_t*)(game_data->game_offsets.nextState + 0x3);
			}
		}

		// 4C 8B 74 24 ? 45 33 C0 4C 8B 6C 24 ? 4C 8B 64 24 ? 48 8B 7C 24 ? 4C 8B 7C 24 ? 48 8B 9C 24 ? ? ? ? 48 63 CE 48 69 D1 ? ? ? ? 48 03 D0 48 8B C2

		game_data->game_offsets.flags = utility::find_ida_sig(NULL, XOR("F6 82 ? ? ? ? ? 74 ? B1"));

		if (game_data->game_offsets.flags != 0x0)
		{
			game_data->game_offsets.flags = *(int*)(game_data->game_offsets.flags + 0x2);
		}
		else
		{
			game_data->game_offsets.flags = utility::find_ida_sig(NULL, XOR("F6 42 ? ? 74 ? B1"));
			if (game_data->game_offsets.flags != 0x0)
			{
				game_data->game_offsets.flags = *(uint8_t*)(game_data->game_offsets.flags + 0x2);
			}
			else
			{
				game_data->game_offsets.flags = utility::find_ida_sig(NULL, XOR("F6 82 ? ? ? ? ? 74 ? 48 8B C2 B1"));
				if (game_data->game_offsets.flags != 0x0)
				{
					game_data->game_offsets.flags = *(int*)(game_data->game_offsets.flags + 0x2);
				}
			}
		}

		game_data->game_offsets.DDL_SetEnum = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 49 B8 79 A6 3B 34 ED ED DA 38")).self_jmp(1);

		game_data->game_offsets.Com_ParseNavStrings = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B 54 24 ? 48 8D 4C 24 ? 40 32 F6")).self_jmp(1);

		game_data->game_offsets.DDL_SetString = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8D 55 ? 48 8D 4D ? E8 ? ? ? ? 84 C0 74 ? 49 B8")).self_jmp(1);

		game_data->game_offsets.Com_DDL_CreateContext = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B D3 48 8D 4D 90 E8 ? ? ? ? C5 F9 EF C0")).self_jmp(1);

		game_data->game_offsets.Com_DDL_LoadAsset = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 48 8B D8 E8 ? ? ? ? 4C 8D 4C 24 ? 4C 8B C3")).self_jmp(1);

		game_data->game_offsets.CL_PlayerData_GetDDLBuffer = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 4C 8B C7 48 8D 8C 24 ? ? ? ?")).self_jmp(1);

		game_data->game_offsets.DDL_GetRootState = utility::find_ida_sig(NULL, XOR("33 C0 C7 41 ? ? ? ? ? 48 89 41 10 48 85 D2 89 41 04 0F 95 C0 48 89 51 18 88 01 48 8B"));

		game_data->game_offsets.DDL_MoveToPath = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 84 C0 74 ? 48 8D 55 ? 48 8D 4C 24 ? E8 ? ? ? ? 8B F0")).self_jmp(1);

		game_data->game_offsets.DDL_GetType = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 83 F8 02 75 26 48 8D 54 24 ?")).self_jmp(1);

		game_data->game_offsets.DDL_SetInt = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? C6 43")).self_jmp(1);

		game_data->game_offsets.A_BG_Omnvar_GetIndexByName = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 8B C8 8B D8 E8 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 4C")).self_jmp(1);
		game_data->game_offsets.A_CG_Omnvar_GetData = utility::find_ida_sig(NULL, XOR("E8 ? ? ? ? 8B 4F ? 8B 45")).self_jmp(1);

		game_data->game_offsets.A_BG_ThirdPerson_IsEnabled = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 84 C0 75 09 F6 83")).self_jmp(1);
		game_data->game_offsets.A_CG_WeaponInspect_IsUIHidden = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 84 C0 0F 85 ?? ?? ?? ?? 4C 8D 44 24 ?? 8B CF C5 F8 28 CE")).self_jmp(1);

		game_data->game_offsets.CG_ClientModel_GetModel = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? C6 44 24 ?? ?? 48 8D 15 ?? ?? ?? ?? 41 B8 ?? ?? ?? ?? 48 8B 48 70")).self_jmp(1);
		game_data->game_offsets.CG_ClientModel_GetOrigin = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? C5 FB 10 74 24 ?? 8B 6C 24 58 49 8B CE 41 FF D4")).self_jmp(1);
		game_data->game_offsets.CG_ClientModel_GetAngles = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? C5 FA 10 44 24 ?? C5 FA 5C CE C4 C1 72 58 16 C5 FA 10 44 24 ?? C5 FA 5C CF")).self_jmp(1);

		game_data->game_offsets.GetInstance = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 4C 8D 4C 24 ?? C6 44 24 ?? ?? 4C 8D 44 24 ?? 8B D7")).self_jmp(1);

		game_data->game_offsets.GetConstInstance = utility::find_ida_sig(NULL, XOR("E8 ?? ?? ?? ?? 45 33 C0 48 8B CB C5 F8 10 B0")).self_jmp(1);

		uintptr_t SendMessageToChat = utility::find_ida_sig(NULL, XOR("C5 F9 7F 74 24 ?? E8 ?? ?? ?? ?? 33 D2 84 C0 0F 95 C2"));

		game_data->game_offsets.SendMessageToChat = resolveRelativeAddress(SendMessageToChat + 6, 1, 5);

		//log_shit("scan done...");

		while (hook_in_game())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (!InitHook(reinterpret_cast<IDXGISwapChain3*>(*(uintptr_t*)(ctx::mem.swap_chain)))) 
		{
			MessageBoxA(NULL, XOR("Failed To Init Menu"), XOR("Error"), MB_OK);
		}

		//log_shit("present done...");
	}
	else
	{
		MessageBoxA(NULL, XOR("Failed To Init Hooks"), XOR("Error"), MB_OK);
	}

	VMProtectEnd();
}

uintptr_t __cdecl I_beginthreadex(void* _Security, unsigned _StackSize, _beginthreadex_proc_type _StartAddress, void* _ArgList, unsigned _InitFlag, unsigned* _ThrdAddr) {
	return LI_FN(_beginthreadex).get()(_Security, _StackSize, _StartAddress, _ArgList, _InitFlag, _ThrdAddr);
}

#define PAGE_SIZE 4096

void CleanPE(HMODULE Module)
{
	auto memset = [](void* Address, int Value, size_t Size) -> void* __cdecl
	{
		__stosb(static_cast<unsigned char*>(Address), static_cast<unsigned char>(Value), Size);
		return Address;
	};

	memset(Module, 0, PAGE_SIZE);

	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)Module;
	PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)((PBYTE)DosHeader + (ULONG)DosHeader->e_lfanew);

	ULONG DosImageSize = sizeof(IMAGE_DOS_HEADER);

	if (DosHeader)
		for (ULONG i = 0; i < DosImageSize; i++)
			*(BYTE*)((BYTE*)DosHeader + i) = 0;

	ULONG NtImageSize = sizeof(IMAGE_NT_HEADERS);

	if (NTHeader)
		for (ULONG i = 0; i < NtImageSize; i++)
			*(BYTE*)((BYTE*)NTHeader + i) = 0;
}

//BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
//{
//	if (reason == DLL_PROCESS_ATTACH)
//	{
//		DisableThreadLibraryCalls(module);
//
//		CleanPE(module);
//
//		LI_FN(CloseHandle)(reinterpret_cast<HANDLE>(I_beginthreadex(0, 0, [](void* hmod) -> unsigned { entry(); return 0; }, module, 0, 0)));
//
//		return TRUE;
//	}
//	return TRUE;
//}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		log_shit("DLL_PROCESS_ATTACH");

		DisableThreadLibraryCalls(hModule);

		CleanPE(hModule);

		LI_FN(CloseHandle)(reinterpret_cast<HANDLE>(I_beginthreadex(0, 0, [](void* hmod) -> unsigned { entry(); return 0; }, hModule, 0, 0)));
	}
	return TRUE;
}