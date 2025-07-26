#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#include "gui/framework.h"

typedef struct _settings
{
	//loot rarity
	bool loot_FavoriteSupplyBox = false;
	bool lootammo = false;
	bool lootweapons = false;
	bool lootmoney = false;
	bool lootarmor = false;
	bool lootmisc = false;
	bool lootsupply = false;
	bool Wallbuys = false;
	bool ZMMachines = false;
	bool Stations = false;
	bool Scanners = false;
	bool Contracts = false;
	bool Ziplines = false;
	bool lootEasterEgg = false;
	bool lootlethals = false;
	bool GasMask = false;
	bool tacticals = false;
	bool lootequipment = false;
	bool ZMTrap = false;
	bool lootextra = false;


	bool isbeenactivated = false;
	bool Norecoil = true;
	bool no_recoil = true;
	bool uavtest = false;
	bool Nospread = false;
	bool nomovement = false;
	bool reverseEnabled = false;
	bool firstTimeReverse = true;
	int i_menuTab = 0;
	int max_distance = 100;
	int player_max = 1;
	int box_options = 1;
	int bone_index = 1;
	bool fovscalewasset = false;
	bool b_fov = false;
	float f_fov = 100.0f;
	int menu_trans = 225;
	float gunoffsetx = 0.0;
	float gunoffsety = 0.0;
	float gunoffsetz = 0.0;
	float noRecoil = 0.0f;
	bool b_ingame;
	bool gamehook;
	bool b_menuEnable = true;
	bool visibleOutlines = true;
	bool HUDvisibleOutlines = true;
	bool b_visible = true;
	bool show_only_visible = false;
	bool show_aim_warning_nigs = false;
	bool show_only_hit_bullet_tracers = false;
	bool b_chamvisible = true;
	bool playerToggle = true;
	bool vehicleToggle = false;
	bool Compass_rgb = false;
	bool FOV_rgb = false;
	bool aiToggle = false;
	bool chams = false;
	bool chamHooked = false;
	bool HUDchams = false;
	bool HUDchamHooked = false;
	bool LootchamHooked = false;
	bool b_spinny_crosshair = false;
	bool b_crosshair = false;
	bool crosshairhooked = false;
	bool no_recoilhooked = false;
	bool uavtest_hooked = false;
	bool bullet_tracers_hooked = false;
	bool shellshockhooked = false;
	bool DamageDirectionIndicators_hooked = false;
	bool no_recoil_hook_exist = false;
	bool shellshock_hook_exist = false;
	bool chamHooked_hook_exist = false;
	bool HUDchamHooked_hook_exist = false;
	bool no_spreadhooked = false;
	bool nomovementhooked = false;
	bool ESPhooked = false;
	bool hooksAllowed = false;
	bool espUpdated = false;
	bool notVisibleOutlines = true;
	int rendertype = 0;
	bool real_chamFill = false;
	bool chamFill = false;
	bool veh_chamFill = false;

	bool screenshot_display = false;
	bool RGB_Menu = false;

	int lineWidthNum = 2;
	int targetting_closestby = 0;
	int fov_int = 0;
	int uav_type = 0;
	int Snapline_type = 0;
	int box_type = 0;
	int loot_box_type = 0;
	int HUDlineWidthNum = 2;
	int outline_style = 0;
	int veh_lineWidthNum = 2;
	int reportTypeTestInt = 2;
	int notifyData = 0;
	int chamColorRGB;
	int HUDchamColorRGB;
	int playerColor;
	int vis_playerColor;
	int selfColor;
	int lootColor;
	int aiColor;
	int vehicleColor;
	int DMZLOOTColor;
	bool initFont = false;
	bool lootChams = false;
	bool DMZlootChams = false;
	bool rgbChams = false;
	bool HUDrgbChams = false;
	bool disableChams = false;
	int ownEntNum = 0;
	bool esp = false;
	bool AIMBOTFOVCIRCLE = false;
	bool AIMBOT = false;
	bool zombie_esp = false;
	bool hwid_grabbed = false;
	bool FREINDLYAIMBOTCHECK = false;
	bool Prediction = true;
	bool isSticky = false;
	bool skip_Downed = false;
	bool AIMBOTCHECK = false;
	bool skipKnocked = false;
	bool skipselfrevive = false;
	float distanceFloat = 250.f;
	float loot_distanceFloat = 100.f;
	float AIMBOTFloat = 1.00;
	float RecoilFloat = 5.0F;
	bool distance_esp = true;
	bool weapon_esp = true;
	bool threat_warnings = false;
	bool bullet_tracers = false;
	bool custom_hit = false;
	bool kills_esp = false;
	bool deaths_esp = false;
	bool teamid_esp = false;
	bool dontrender_only = false;
	bool enemies_only = true;
	bool showaibool = false;
	bool targetaibool = false;
	bool cham_enemies_only = true;
	bool b_radar = false;
	bool unfilled_boxes = true;
	bool filled_boxes = false;
	bool corner_box = false;
	bool bone_esp = true;
	bool head_esp = false;
	bool compass_esp = true;
	bool realCompass = false;
	bool snaplines = false;
	bool name_esp = true;
	bool bubble_esp = false;
	bool Snapline_ESP = false;
	bool health_esp = true;
	bool shield_esp = false;
	bool isVisible = false;
	bool rgbcrosshair = false;
	bool rgb_bullettracer = false;
	bool aim_target_esp = false;

	//AB SHIT
	int silent_type = 1;
	int controllerAimkey = 0;
	bool target_bone;
	int aim_type = 0; //0 ~ 3
	int i_bone = 2; //0 ~ 3
	int A_bone = 1; //0 ~ 3
	int aim_key = 27;
	int Override_key = 27;
	int aim_Override_key = 27;
	int anti_aim_Override_key = 27;
	int slide_key = VK_INSERT;
	int thirdperson_key = 27;
	int airstuck_key = 27;
	int aim_smooth = 0; // 0 ~ 30
	int delay = 1; // 0 ~ 30
	int pred_destination = 0; // 0 ~ 30
	bool Ab_fov;
	float f_fov_size = 0.0f; // 0 ~ 1000


	bool g_enable_miss_chance = false;
	float g_miss_chance = 0.0f;
	float g_miss_max_angle_offset = 1.0f;

	ImVec4 UAV_color = ImColor(255, 0, 0);
	ImVec4 bullettracer_color = ImColor(255, 255, 255);
	ImVec4 crosshaircolor = ImColor(255, 255, 255);
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImVec4 player_colorVec4 = ImColor(255, 255, 255);
	ImVec4 vis_player_colorVec4 = ImColor(255, 0, 0);
	ImVec4 HUDplayer_colorVec4 = ImColor(255, 255, 255);
	ImVec4 item_colorVec4 = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImVec4 ai_colorVec4 = ImColor(255, 0, 0);
	ImVec4 vehicle_colorVec4 = ImColor(255, 255, 255);
	ImVec4 DMZLoot_colorVec4 = ImVec4(255, 255, 0, 1.00f);

	ImVec4 Target_color = ImColor(138, 43, 226);

	ImVec4 skeleton_colorVec4 = ImColor(255, 255, 255);
	ImVec4 V_skeleton_colorVec4 = ImColor(255, 0, 0);
	ImVec4 boxnotFilled_colorVec4 = ImColor(255, 255, 255, 255);
	ImVec4 vis_boxnotFilled_colorVec4 = ImColor(255, 0, 0, 255);
	ImVec4 team_boxnotFilled_colorVec4 = ImColor(0, 0, 0, 0);
	ImVec4 boxFilled_colorVec4 = ImColor(10, 10, 10, 100);
	ImVec4 boldcolorbox = ImColor(0, 0, 0, 100);
	ImVec4 distance_colorVec4 = ImColor(255, 255, 255);
	ImVec4 V_distance_colorVec4 = ImColor(255, 0, 0);
	ImVec4 V_aim_warnings_colorVec4 = ImColor(255, 255, 255);
	ImVec4 Snaplines_colorVec4 = ImColor(255, 255, 255);
	ImVec4 FOVCOLOR_colorVec4 = ImColor(255, 255, 255, 180);
	ImVec4 V_Snaplines_colorVec4 = ImColor(255, 0, 0);
	ImColor showfov{ 255, 255, 255, 255 };


	ImVec4 COMPASS_colorVec4 = ImColor(255, 255, 255);
	ImVec4 V_COMPASS_colorVec4 = ImColor(255, 0, 0);
	ImVec4 V2_COMPASS_colorVec4 = ImColor(255, 255, 0);

	int vis_cham = ImColor(255, 0, 0, 255);
	int cham = ImColor(255, 255, 255, 255);
	int teamcham = ImColor(255, 255, 255, 0);

	int screenshot_count = 0;
	int target_color = 0xff00ff;
	int V_compasscolor = 0xff00ff;
	int compasscolor = 0xff00ff;
	int boneColor = 0xff00ff;
	int V_boneColor = 0xff00ff;
	int boxColor = 0xff00ff;
	int vis_boxColor = 0xff00ff;
	int filledboxColor = 0xff00ff;
	int V_filledboxColor = 0xff00ff;
	int boldcolor = 0xff00ff;
	int distanceColor = 0xff00ff;
	int Aim_warnings_Color = 0xff00ff;
	int V_distanceColor = 0xff00ff;
	int FOVCOLOR = 0xff00ff;
	int crosshaircolorint = 0xff00ff;
	int bullettracer_colorint = 0xff00ff;
	float boldness = 1;
	float aimbotFov = 10.f;
	float target_velocity = 5.f;
	float radarScale = 500.f;
	float CompassScale = 300.f;
	bool UnlockAllItems = false;
	bool UnlockAllItems_hooked = false;
	bool checkPassed = false;
	bool istabbedin = false;
	bool enablecg_t = false;
	bool CL_hooked = false;
	bool CL_InputMP_ReadyToSendPacket_hooked = false;
	bool cham_hooked = false;
	bool CL_hook_exist = false;
	bool CL_InputMP_ReadyToSendPacket_hook_exist = false;
	bool PredictPlayerState_hooked = false;
	bool PredictPlayerState_hook_exist = false;
	bool cham_hook_exist = false;
	bool condition_met = false;
	bool cham_condition_met = false;
	bool recoil_condition_met = false;
	bool spread_condition_met = false;
	bool NoFlash = false;
	bool misc_nocounteruav = false;
	bool nodamage_bool = false;
	bool NoFlinch = false;
	bool Ulibreath = false;
	bool FastReload = false;
	bool rgbname = false;
	bool instant_swap = false;
	bool b_player_3dboxes = false;
	bool b_player_filled_3dboxes = false;
	bool antishake = false;
	bool Remove_Fog = false;
	bool NoStun = false;
	bool Spinbot = false;
	bool jitterbot = false;
	bool turned = false;

	bool superslide = false;

	bool silentAim = false;
	bool testcrasher = false;
	bool Safe_Mode = false;
	bool controllerToggle = false;
	bool autoFire = false;
	bool airstuck = false;
	bool Longmelee = false;
	bool Rapidfire = false;
	bool misc_noflash = false;
	bool misc_nostun = false;
	bool cg_hooked = false;
	bool CG_hook_exist = false;
	float bullet_trace_fade = 2.0f;
	float bullet_trace_thickness = 1.f;
	float bullet_x_trace_thickness = 1.f;
	float compassDistance = 75.f;
	float compassSize = 300.f;
	float arrowSize = 27.f;
	float arrowfill = 200.f;

	bool fovhooked = false;
	bool fovhook_exist = false;

	bool loot_cache = false;
	bool loot_name = false;
	bool loot_test = false;
	bool loot_bounds_3d = true;

	bool thirdpersonhook = false;
	bool oCG_EntityMP_CalcLerpPositions_hook = false;
	bool thirdpersonhook_exist = false;

	bool ffpAds = false;
	bool ffpInspect = false;

	bool ThirdPerson = false;
	bool ThirdPerson_state = false;
	bool Dynamic_Person = false;
	bool spamchat = false;

	bool is_bnet = false;
	bool is_steam = false;
	bool is_micro = false;

	// var shit
	bool ShowSpectator = false;
	bool removeweaponmovement = false;
	int activespec;

	//loot shit
	bool cache = false;

	float flScale = 8.0f;

	float aimwarning_fov = 150.f;
	float prediction_value = 100.f;
	float misschance_value = 0.f;


	bool useBezierInterpolation = true;
	bool useRandomizedAlpha = true;
	float bezierControlPoint = 0.5f; // Adjust as needed
	float randomAlphaMin = 0.0f;
	float randomAlphaMax = 0.3f;

	std::string buildStatus = "Release";

} settings;

typedef struct _offsets
{

	uintptr_t Live_GetXuid;
	uintptr_t  networkadapterMacptr;
	uintptr_t  DWServicesAccess_GetInstance;
	uintptr_t  DWServicesAccess_GetLogin;

	// General offsets
	uintptr_t cg_t;
	uintptr_t find_var;
	uintptr_t set_float;
	uintptr_t fov_max_value;
	uintptr_t cleanup_instr;
	uintptr_t ptr_swap;
	uintptr_t refdef_ptr;
	uintptr_t gamereturn;
	uintptr_t clientinfo_ptr;
	uintptr_t wepStructOffset;
	uintptr_t trampoline;
	uintptr_t fov;
	uintptr_t rbx_jmp;
	int Service;
	uintptr_t fov_hook;
	uintptr_t CL_InputMP_ReadyToSendPacket;
	uintptr_t DVARBOOL_cl_packetdup;
	uintptr_t command_queue;
	uintptr_t CL_RenderScene;
	uintptr_t A_CL_Input_ClearAutoForwardFlag;
	uintptr_t BG_Ladder_IsActive_offset;
	uintptr_t cmdNumberPos;
	uintptr_t cmdNumberaabPos;
	uintptr_t usercmdPos;
	uintptr_t userCmdSize;
	uintptr_t playerstate;
	uintptr_t shellshock;
	uintptr_t kickAngles;
	uintptr_t a_LUI_OpenMenu;
	uintptr_t CL_PlayerData_SetCustomClanTag;
	uintptr_t dllStartAdr;
	uintptr_t packet_shit;
	uintptr_t A_BG_UsrCmdPackAngle;
	uintptr_t gamecheck;
	uintptr_t clantag;
	uintptr_t oGetSTAsset;
	uintptr_t R_AddDObjToScene;
	uintptr_t R_AddViewmodelDObjToScene;
	uintptr_t LUI_CoD_SendOverIndulgenceNoficiation;
	uintptr_t LUI_beginevent;
	uintptr_t LUI_begintable;
	uintptr_t LUI_settablestring;
	uintptr_t LUI_settableint;
	uintptr_t LUI_endtable;
	uintptr_t LUI_endevent;
	//uintptr_t LUI_luaVM;

	// Lua offsets
	uintptr_t j_lua_remove;
	uintptr_t lua_pushboolean;
	uintptr_t lua_getfield;
	uintptr_t lua_pushstring;
	uintptr_t lua_registerFunction;
	uintptr_t lua_gettop;
	uintptr_t lua_settop;
	uintptr_t LUI_luaVM;

	// Additional offsets
	uintptr_t SetTableBool;
	uintptr_t updateByte;
	uintptr_t playerStatePadding;
	uintptr_t BG_GetWeaponFireType;
	uintptr_t oGetSTValue;
	uintptr_t CbufOffset;
	uintptr_t xpartyOffset;
	uintptr_t BG_WeaponFireRecoilOffset;
	uintptr_t BG_CalculateFinalSpreadForWeapon;
	uintptr_t BG_CalculateWeaponMovement_Sway;
	uintptr_t BG_CalculateWeaponMovement;
	uintptr_t CG_View_UpdateWeaponMovement;
	uintptr_t CG_StartShellShock;
	uintptr_t CG_DrawDamageDirectionIndicators;
	uintptr_t gamemode;
	uintptr_t Com_FrontEnd_IsInFrontEnd;
	uintptr_t end_bytes;
	uintptr_t cam;
	uintptr_t visible;
	uintptr_t bounds;
	uintptr_t G_Main_SightTrace;
	uintptr_t LegacyTrace;
	uintptr_t CG_ScoreboardMP_GetClientScore;
	uintptr_t visibleBit;
	uintptr_t distribute_ptr;
	uintptr_t visible_ptr;
	uintptr_t other;
	uintptr_t renderGame;
	uintptr_t getPos;
	uintptr_t cameraBaseOffset;
	uintptr_t nameArray;
	uintptr_t GetClientInfo;
	uintptr_t pmove;
	uintptr_t ps_offset;
	uintptr_t GetGestureFunc;
	uintptr_t cg_offset;
	uintptr_t A_CG_VehicleCam_SetClientViewAngles;
	uintptr_t CL_SetViewAngle;
	uintptr_t a_parse;
	uintptr_t BlueprintDataBuffer;
	uintptr_t bone_ptr;

	// Weapon-related offsets
	uintptr_t Weaponmap;
	uintptr_t BG_GetPlayerEyePosition;
	uintptr_t getCGHandler;
	uintptr_t weaponmap;
	uintptr_t bg_geteyeposition;
	//uintptr_t BG_Ballistics_TravelTimeForDistance;
	uintptr_t BG_CompleteWeaponDef;
	uintptr_t P_gamePads;
	uintptr_t A_GPad_GetButton;
	uintptr_t wepDefForVelocity;
	uintptr_t BG_WeaponIsDualWield;
	uintptr_t CG_GetEntity;
	uintptr_t CG_GetEntWeapon;
	uintptr_t CG_GetWeaponDisplayName;
	uintptr_t PredictPlayerState;
	uintptr_t PredictedPlayerEntity;
	uintptr_t a_BulletHitEvent_Internal;
	uintptr_t CG_EntityMP_CalcLerpPositions;
	uintptr_t GetItemQuanity;
	//uintptr_t BG_GetWeaponType;
	//uintptr_t CG_GetPoseOrigin;
	//uintptr_t CG_GetBoneIndex;
	uintptr_t LUI_GetEntityTagPositionAndAxis;
	uintptr_t CG_GetPoseOrigin;
	uintptr_t A_GetBRPlayerFlags;
	uintptr_t nextState;
	uintptr_t flags;
	uintptr_t Dvar_FindMalleableVar;
	uintptr_t CgWeaponSystem__ms_weaponSystemArray;
	uintptr_t G_Bullet_Endpos;
	uintptr_t BG_GetWeaponFlashTagname;
	uintptr_t CalcMuzzlePoint;
	uintptr_t BG_srand_timeangles;
	uintptr_t baseGunAngles;
	uintptr_t aimSpreadScale;
	uintptr_t A_DrawMapLayer;
	uintptr_t A_WorldPosToCompass;
	uintptr_t A_CG_CycleWeapon;
	uintptr_t A_BG_GetAmmoInClipForWeapon;
	uintptr_t A_BG_GetClipSize;
	uintptr_t AimTargetMP_GetTargetBounds;
	uintptr_t A_BG_GetWeaponDisplayName;
	uintptr_t A_UI_SafeTranslateString_Hash;
	//uintptr_t UI_SafeTranslateString;
	uintptr_t DDL_SetEnum;
	uintptr_t Com_ParseNavStrings;
	uintptr_t DDL_SetString;
	uintptr_t Com_DDL_CreateContext;
	uintptr_t Com_DDL_LoadAsset;
	uintptr_t CL_PlayerData_GetDDLBuffer;
	uintptr_t DDL_GetRootState;
	uintptr_t DDL_MoveToPath;
	uintptr_t DDL_GetType;
	uintptr_t DDL_SetInt;
	uintptr_t angle_offset;
	uintptr_t HOOK_FUNC;
	uintptr_t s_blackboxInitialized;
	uintptr_t vtp;
	uintptr_t HookOriginalFunction;

	// Ballistics and calculations
	uintptr_t BG_GetBallisticMuzzleVelocityScale;
	uintptr_t BG_Ballistics_HorizontalTravelTimeToTarget;
	uintptr_t CG_CalcTargetVelocity;
	uintptr_t BG_GetBallisticInfo;

	// Additional functions and data
	uintptr_t A_CG_Entity_GetPlayerViewOrigin;
	//uintptr_t A_GetCache;
	uintptr_t A_GetLootTable;
	//uintptr_t A_GetMaxLootScriptableIndex;
	//uintptr_t A_GetLootItemFromScriptableIndex;
	uintptr_t A_GetOriginAngles;
	//uintptr_t A_UI_SafeTranslateString_Hash;
	//uintptr_t A_CG_Weapons_GetWeaponForName;
	//uintptr_t A_CG_GetWeaponDisplayName;
	uintptr_t A_IsScriptableUsable;
	uintptr_t A_GetAbsBoundsCallback;
	uintptr_t equippedWeaponsPosition;
	uintptr_t Slide_EndCheck;

	uintptr_t Party_GetMapName;
	uintptr_t Live_GetMapIndex;
	uintptr_t Live_GetMapSource;
	uintptr_t Party_GetXuid;
	uintptr_t Party_GetActiveParty;
	uintptr_t Party_GetPlatformId;
	uintptr_t Party_FindMemberByXUID;

	//omnvar
	uintptr_t A_BG_Omnvar_GetIndexByName;
	uintptr_t A_CG_Omnvar_GetData;

	// Third person
	uintptr_t A_BG_ThirdPerson_IsEnabled;
	uintptr_t A_CG_WeaponInspect_IsUIHidden;

	//loot
	uintptr_t CG_ClientModel_GetModel;
	uintptr_t CG_ClientModel_GetOrigin;
	uintptr_t CG_ClientModel_GetAngles;

	//// chat shit
	uintptr_t GetInstance;
	uintptr_t GetConstInstance;
	uintptr_t SendMessageToChat;

	// Virtual table and function pointers
	void** GetClientInfo_vtbl;
	uintptr_t get_dobj;
} offsets;

typedef struct _colors
{
	ImColor Color{ 1.f, 1.f,1.f,1.f };

} colors;

typedef struct _global_vars
{
	DWORD procID;
	HWND hWind;
	DXGI_SWAP_CHAIN_DESC desc;
	HANDLE hProc;
	uintptr_t baseModule;
	DWORD64 Peb;
	offsets game_offsets;
	settings settings;
} global_vars;

extern global_vars* game_data;

#endif // !GLOBAL_H
