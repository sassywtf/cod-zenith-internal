#include "sdk.h"
#include "math.h"
#include "../spoofer/spoof.h"
#include "../main.h"
#include "../spoofer/callstack.h"
#include "../protect/vmp.h"

#define pi2 3.14159265358979323846264338327950288419716939937510
#define pi_double ((float)pi2*2.0f)
#define DegreesToRadians(a) ((a)*((float)M_PI/180.0f))
#define RadiansToDegrees(a) ((a)*(180.0f/(float)M_PI))
#define M_PI       3.14159265358979323846   
constexpr auto radians_to_degrees(std::float_t a) { return a * (180.0f / std::stcast<std::float_t>(pi2)); }

void* fov_orig{};

#define M_PI       3.14159265358979323846   

constexpr auto radians_to_degrees2(std::float_t a) { return a * (180.0f / static_cast<std::float_t>(M_PI)); }
constexpr auto degrees_to_radians(std::float_t a) { return a * (static_cast<std::float_t>(M_PI) / 180.0f); }

template <typename T> inline T(*GameCall(unsigned __int64 address))(...) {
	__int64 func_address = address;
	T(*func)(...) = (T(*)(...))func_address;
	return func;
}

inline float deg2rad(const float deg)
{
	return static_cast<float>(deg * (M_PI / 180.f));
}

void VectorToAngles(Vector3 direction, Vector3& angles)
{
	float flTemp, flYaw, flPitch;

	if (direction.x == 0.f && direction.y == 0.f)
	{
		flYaw = 0.f;

		if (direction.z > 0.f)
			flPitch = 90.0f;

		else
			flPitch = 270.0f;
	}

	else
	{
		flYaw = RadiansToDegrees(atan2f(direction.y, direction.x));

		if (flYaw < 0.f)
			flYaw += 360.0f;

		flTemp = sqrtf(direction.x * direction.x + direction.y * direction.y);
		flPitch = RadiansToDegrees(atan2f(direction.z, flTemp));

		if (flPitch < 0.f)
			flPitch += 360.0f;
	}

	angles.x = -flPitch;
	angles.y = flYaw;
	angles.z = 0.f;
}

void sendchatmessage(const char* message) {
	DWORD64* ConstInstance = GameCall<DWORD64*>(game_data->game_offsets.GetConstInstance)();

	ChatInfo* m_activeChat = reinterpret_cast<ChatInfo*>(reinterpret_cast<char*>(ConstInstance) + 0x34FE8);
	m_activeChat->type = 0x4;

	size_t messageLength = strlen(message);
	if (messageLength >= 0x101) {
		//LOG("error: message too long");
		return;
	}

	DWORD64* ChatInstance = GameCall<DWORD64*>(game_data->game_offsets.GetInstance)();
	if (GameCall<bool>(game_data->game_offsets.SendMessageToChat)(ChatInstance, 0, m_activeChat, message, false)) {
		//LOG("SUCCESS");
	}
	else {
		//LOG("FAILED");
	}
}

void Spam_message(const char* message) {
	static auto lastSentTime = std::chrono::steady_clock::now();

	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSentTime).count();

	if (elapsed < 3000) {
		return; // Don't send if 3000ms hasn't passed
	}

	lastSentTime = now; // Update the last sent time

	DWORD64* ConstInstance = GameCall<DWORD64*>(game_data->game_offsets.GetConstInstance)();
	ChatInfo* m_activeChat = reinterpret_cast<ChatInfo*>(reinterpret_cast<char*>(ConstInstance) + 0x34FE8);
	m_activeChat->type = 0x4;

	size_t messageLength = strlen(message);
	if (messageLength >= 0x101) {
		// LOG("error: message too long");
		return;
	}

	DWORD64* ChatInstance = GameCall<DWORD64*>(game_data->game_offsets.GetInstance)();
	if (GameCall<bool>(game_data->game_offsets.SendMessageToChat)(ChatInstance, 0, m_activeChat, message, false)) {
		// LOG("SUCCESS");
	}
	else {
		// LOG("FAILED");
	}
}

OmnvarData getOmnvarData(const char* omnvar_name) {
	OmnvarData outData{};

	int index = GameCall<int>(game_data->game_offsets.A_BG_Omnvar_GetIndexByName)(omnvar_name);
	if (index > 0 && index < 2047)
	{
		GameCall<void>(game_data->game_offsets.A_CG_Omnvar_GetData)(0, index, &outData);
		return outData;
	}

	return outData;
}

OmnvarData* SetOmnvarData(const char* omnvar_name) {
	static OmnvarData outData{};

	int index = GameCall<int>(game_data->game_offsets.A_BG_Omnvar_GetIndexByName)(omnvar_name);
	if (index > 0 && index < 2047) {
		GameCall<void>(game_data->game_offsets.A_CG_Omnvar_GetData)(0, index, &outData);
		return &outData;
	}

	return nullptr;
}

//// shellcode to fix stack, and prepare to jump to legit end of function
//static unsigned char shell[] = {
//	0x48, 0x83, 0xC4, 0x08,                   // add rsp, 8
//	0x48, 0x8D, 0x05, 0x01, 0x00, 0x00, 0x00, // lea rax, [rip + 1]
//	0xE9, NULL, NULL, NULL, NULL,             // jmp xxxxxxxx
//
//	// memory that is used once the parent function dereferences the ptr we return
//	0x99, 0x00, 0x00, 0x00
//};
//
//UnlockerUtil* UnlockerUtil::get() 
//{
//	return (UnlockerUtil*)game_data->game_offsets.ptr_swap;
//}
//
//bool UnlockerUtil::unlock() 
//{
//	if (this->unlocker_active())
//		return false;
//
//	if (!this->vtable)
//		return false;
//
//	// calculate size and offset to write vtable and shellcode
//	unsigned data_size = sizeof InventoryVTable + sizeof shell;
//	uintptr_t info_address = uintptr_t(&this->vtable) - data_size;
//
//	// fill in info for the jmp
//	uintptr_t rip = uintptr_t(&this->vtable) - 4;
//	*reinterpret_cast<unsigned*>(shell + 12) = game_data->game_offsets.cleanup_instr - rip;
//
//	// copy game's legit vtable
//	InventoryVTable custom_vtable;
//	memcpy_s(&custom_vtable, sizeof InventoryVTable, this->vtable, sizeof InventoryVTable);
//
//	// edit 2nd ptr to point to our custom shellcode
//	*reinterpret_cast<uintptr_t*>(&custom_vtable.unk2) = info_address + sizeof InventoryVTable;
//
//	// grab cache to store memory for reverting the data
//	unsigned char* data_cache = this->get_data_cache();
//
//	// copy the original data to the cache (+8 to account for the ptr we're going to override)
//	memcpy_s(data_cache, 0x100, reinterpret_cast<void*>(info_address), data_size + sizeof(void*));
//
//	// copy new memory to safe region in game
//	memcpy_s(reinterpret_cast<void*>(info_address), sizeof InventoryVTable, &custom_vtable, sizeof InventoryVTable);
//	memcpy_s(reinterpret_cast<void*>(info_address + sizeof InventoryVTable), sizeof shell, shell, sizeof shell);
//	memcpy_s(reinterpret_cast<void*>(info_address + sizeof InventoryVTable + sizeof shell), sizeof(void*), &this->vtable, sizeof(void*));
//
//	// update game's vtable ptr to point to our new vtable
//	this->vtable = reinterpret_cast<InventoryVTable*>(info_address);
//
//	this->unlocker_active() = true;	
//
//	return true;
//}
//
//bool UnlockerUtil::revert() 
//{
//	if (!this->unlocker_active())
//		return false;
//
//	// get offsets and sizes
//	unsigned data_size = sizeof(InventoryVTable) + sizeof(shell);
//	uintptr_t info_address = uintptr_t(&this->vtable) - data_size;
//
//	// revert memory
//	unsigned char* cache = this->get_data_cache();
//	memcpy_s(reinterpret_cast<void*>(info_address), data_size + sizeof(void*), cache, data_size + sizeof(void*));
//
//	this->unlocker_active() = false;
//
//	return true;
//}
//
//// using static values for these because we are using the game's memory for variables, so have to resort to this
//unsigned char* UnlockerUtil::get_data_cache() {
//	static unsigned char data[0x100];
//	return data;
//};
//
//bool& UnlockerUtil::unlocker_active() {
//	static bool active = false;
//	return active;
//}

Vector3 RotateVector(const Vector3& vector, float yaw)
{
	float s, c;
	s = std::sin(yaw);
	c = std::cos(yaw);

	Vector3 rotatedvector;
	rotatedvector.x = vector.x * c - vector.y * s;
	rotatedvector.y = vector.x * s + vector.y * c;
	rotatedvector.z = vector.z;

	return rotatedvector;
}

bool stringContains(std::string cmp, std::string fnd) {
	return (cmp.find(fnd) != std::string::npos);
}

std::string getNameFromList(std::string model) {
	if (game_data->settings.lootequipment) {
		if (stringContains(model, XOR("keycard")))
			return XOR("Keycard");
		/*	if (stringContains(model, XOR("offhand_")) && game_data->settings.equipment)
				return XOR("Equipment");*/
		if (stringContains(model, XOR("locker_key")) && game_data->settings.lootequipment)
			return XOR("Locker Key");
		if (stringContains(model, XOR("vault_key")) && game_data->settings.lootequipment)
			return XOR("Vault Key");
		if (stringContains(model, XOR("token_")) && game_data->settings.lootequipment)
			return XOR("Token");
	}

	if (game_data->settings.lootlethals) {
		if (stringContains(model, XOR("grenade_frag")))
			return XOR("Frag");
		if (stringContains(model, XOR("grenade_semtex")) && game_data->settings.lootlethals)
			return XOR("Semtex");
		if (stringContains(model, XOR("grenade_thermite")) && game_data->settings.lootlethals)
			return XOR("Thermite");
		if (stringContains(model, XOR("t10_molotov")))
			return XOR("Molotov");
	}

	if (game_data->settings.tacticals) {
		if (stringContains(model, XOR("grenade_flash")))
			return XOR("Flash");
		if (stringContains(model, XOR("grenade_snapshot")) && game_data->settings.tacticals)
			return XOR("Snapshot");
		if (stringContains(model, XOR("smoke")) && game_data->settings.tacticals)
			return XOR("Smoke");
		if (stringContains(model, XOR("grenade_concussion")) && game_data->settings.tacticals)
			return XOR("Concussion");
		if (stringContains(model, XOR("grenade_decoy")) && game_data->settings.tacticals)
			return XOR("Decoy");
		if (stringContains(model, XOR("wm_stim")) && game_data->settings.tacticals)
			return XOR("Stim");
	}

	if (game_data->settings.lootextra) {
		if (stringContains(model, XOR("backpack")) && game_data->settings.lootextra)
			return XOR("Backpack");
		if (stringContains(model, XOR("duffle_bag")) && game_data->settings.lootextra)
			return XOR("Duffle Bag");
		if (stringContains(model, XOR("medicine_cabinet")) && game_data->settings.lootextra)
			return XOR("Medicine Cabinet");
		if (stringContains(model, XOR("tool_box")) && game_data->settings.lootextra)
			return XOR("Tool Box");
		if (stringContains(model, XOR("drug_brick")) && game_data->settings.lootextra)
			return XOR("Drugs");
		if (stringContains(model, XOR("fuel_")) && game_data->settings.lootextra)
			return XOR("Fuel Container");
		if (stringContains(model, XOR("store_cash")) && game_data->settings.lootextra)
			return XOR("Cash Register");
		if (stringContains(model, XOR("utility_metal_crate")) && game_data->settings.lootextra)
			return XOR("Utility Crate");
		if (stringContains(model, XOR("medical_")) && game_data->settings.lootextra)
			return XOR("Medical Supplies");
		if (stringContains(model, XOR("locker_")) && game_data->settings.lootextra)
			return XOR("Locker");
	}

	if (game_data->settings.lootammo)
		if (stringContains(model, XOR("ammo")) && game_data->settings.lootammo)
			return XOR("Ammo");

	if (game_data->settings.lootmoney)
		if (stringContains(model, XOR("money")) && game_data->settings.lootmoney)
			return XOR("Cash");

	if (game_data->settings.lootarmor)
		if (stringContains(model, XOR("loot_wm_armor")) && game_data->settings.lootarmor)
			return XOR("Armor Plate");

	if (game_data->settings.loot_FavoriteSupplyBox)
	{
		if (stringContains(model, XOR("personal_supply")) && game_data->settings.loot_FavoriteSupplyBox)
			return XOR("Favorite Supply Box");
	}

	if (game_data->settings.lootsupply)
		if (stringContains(model, XOR("crate")) && game_data->settings.lootsupply)
			return XOR("Loot Crate");

	if (game_data->settings.lootmisc) {
		if (stringContains(model, XOR("activity_phone")) && game_data->settings.lootmisc)
			return XOR("Activity Phone");
		if (stringContains(model, XOR("radio")) && game_data->settings.lootmisc)
			return XOR("Radio");
		if (stringContains(model, XOR("cassette_player")) && game_data->settings.lootmisc)
			return XOR("ZM Intel");
		if (stringContains(model, XOR("zm_junk_parts")) && game_data->settings.lootmisc)
			return XOR("ZM Salvage");
		if (stringContains(model, XOR("zm_crystal")) && game_data->settings.lootmisc)
			return XOR("Aether Crystal");
		if (stringContains(model, XOR("power_up")) && game_data->settings.lootmisc)
			return XOR("ZM Power Up");
	}

	if (game_data->settings.lootweapons)
	{
		if (stringContains(model, XOR("_ar_")) && game_data->settings.lootweapons)
			return XOR("Assault Rifle");

		if (stringContains(model, XOR("_lm_")) && game_data->settings.lootweapons)
			return XOR("Light Machine Gun");

		if (stringContains(model, XOR("_pi_")) && game_data->settings.lootweapons)
			return XOR("Pistol");

		if (stringContains(model, XOR("_sm_")) && game_data->settings.lootweapons)
			return XOR("Submachine Gun");

		if (stringContains(model, XOR("_sn_")) && game_data->settings.lootweapons)
			return XOR("Sniper Rifle");

		if (stringContains(model, XOR("_la_")) && game_data->settings.lootweapons)
			return XOR("Launcher");

		if (stringContains(model, XOR("_sh_")) && game_data->settings.lootweapons)
			return XOR("Shotgun");

		if (stringContains(model, XOR("_dm_")) && game_data->settings.lootweapons)
			return XOR("Marksman Rifle");

		if (stringContains(model, XOR("_me_")) && game_data->settings.lootweapons)
			return XOR("Melee");
	}

	//wall buys
	if (game_data->settings.Wallbuys)
		if (stringContains(model, XOR("zm_chalk_buy")) && game_data->settings.Wallbuys)
			return XOR("Wall Buy");

	//GG / Perks / Power
	if (game_data->settings.ZMMachines) {
		if (stringContains(model, XOR("gg_machine")) && game_data->settings.ZMMachines)
			return XOR("Gobble Gum Machine");
		if (stringContains(model, XOR("zm_machine_amp_crystal")) && game_data->settings.ZMMachines)
			return XOR("Power Machine");
		if (stringContains(model, XOR("zm_machine")) && !stringContains(model, XOR("zm_machine_amp_crystal")) && game_data->settings.ZMMachines)
			return XOR("Perk Machine");
	}

	if (game_data->settings.GasMask)
	{
		if (stringContains(model, XOR("gasmask")) && game_data->settings.GasMask)
			return XOR("Gas Mask");
	}

	if (game_data->settings.Stations) {
		if (stringContains(model, XOR("trading_station")) && game_data->settings.Stations)
			return XOR("Trading Station");
	}


	if (game_data->settings.Scanners) {
		if (stringContains(model, XOR("biometric_scanner")) && game_data->settings.Scanners)
			return XOR("Biometric Scanner");
	}

	if (game_data->settings.Contracts) {
		if (stringContains(model, XOR("_contract_")) && game_data->settings.Contracts)
			return XOR("Contract");
	}

	if (game_data->settings.Ziplines) {
		if (stringContains(model, XOR("military_ascendertop")) && game_data->settings.Ziplines)
			return XOR("Zipline");
	}

	//EE
	if (game_data->settings.lootEasterEgg) {
		if (stringContains(model, XOR("ob_mrg")) && game_data->settings.lootEasterEgg)
			return XOR("EE Bear");

		if (stringContains(model, XOR("ob_mrp")) && game_data->settings.lootEasterEgg)
			return XOR("EE Bear");

		if (stringContains(model, XOR("zm_aetherella")) && game_data->settings.lootEasterEgg)
			return XOR("EE Aetherella");

		if (stringContains(model, XOR("wonder_weapon_quest_water")) && game_data->settings.lootEasterEgg)
			return XOR("EE Water Valve");

		if (stringContains(model, XOR("toy_stuffed_animal")) && game_data->settings.lootEasterEgg)
			return XOR("EE Stuffed Animal");

		if (stringContains(model, XOR("ww_handbrake")) && game_data->settings.lootEasterEgg)
			return XOR("WW Part");
	}
	//Traps
	if (game_data->settings.ZMTrap) {
		if (stringContains(model, XOR("aether_field_emitter")) && game_data->settings.ZMTrap)
			return XOR("ZM Trap");

		if (stringContains(model, XOR("tentacle_trap")) && game_data->settings.ZMTrap)
			return XOR("ZM Trap");
	}

	return model; // Skip this item if it’s not enabled
}

ImColor stripRarityFromString(const std::string& name) {
	static const std::pair<const char*, ImColor> rarityColors[] = {
		{ "_rarity0", ImColor(185, 185, 185, 255) },
		{ "_rarity1", ImColor(126, 240, 129, 255) },
		{ "_rarity2", ImColor(126, 168, 240, 255) },
		{ "_rarity3", ImColor(179, 126, 240, 255) },
		{ "_rarity4", ImColor(247, 221, 113, 255) },
		{ "_rarity5", ImColor(255, 16, 240, 255) }
	};

	for (const auto& pair : rarityColors) {
		if (stringContains(name, pair.first)) {
			return pair.second;
		}
	}

	return ImColor(255, 255, 255, 255);
}

void getLoot()
{
	lootData.clear();
	for (int i = 0; i < 384; i++) {
		XModel* model = VariadicCall<XModel*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_ClientModel_GetModel, 0, i, 0);
		if (!model || !model->name_deprecated)
			continue;

		LootData data;
		VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_ClientModel_GetOrigin, 0, i, &data.pos, 0);

		int distance = (myCamPosition - data.pos).lengthDistance() * 0.0254f;
		if (distance > game_data->settings.loot_distanceFloat || distance < 0)
			continue;

		data.name = getNameFromList(model->name_deprecated);
		data.color = stripRarityFromString(model->name_deprecated);
		data.bounds = model->bounds;
		data.distance = distance;

		VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_ClientModel_GetAngles, 0, i, &data.ang, 0);

		// Ensure we can project to screen
		data.canDrawLoot = WorldToScreen(data.pos, &data.screenPos);
		if (!data.canDrawLoot)
			continue;

		if (game_data->settings.loot_bounds_3d)
		{

			float yaw = data.ang.y * M_PI / 180.f;
			Vector3 corners[8] = {
				{data.bounds.midPoint.x - data.bounds.halfSize.x, data.bounds.midPoint.y - data.bounds.halfSize.y, data.bounds.midPoint.z - data.bounds.halfSize.z},
				{data.bounds.midPoint.x + data.bounds.halfSize.x, data.bounds.midPoint.y - data.bounds.halfSize.y, data.bounds.midPoint.z - data.bounds.halfSize.z},
				{data.bounds.midPoint.x + data.bounds.halfSize.x, data.bounds.midPoint.y + data.bounds.halfSize.y, data.bounds.midPoint.z - data.bounds.halfSize.z},
				{data.bounds.midPoint.x - data.bounds.halfSize.x, data.bounds.midPoint.y + data.bounds.halfSize.y, data.bounds.midPoint.z - data.bounds.halfSize.z},
				{data.bounds.midPoint.x - data.bounds.halfSize.x, data.bounds.midPoint.y - data.bounds.halfSize.y, data.bounds.midPoint.z + data.bounds.halfSize.z},
				{data.bounds.midPoint.x + data.bounds.halfSize.x, data.bounds.midPoint.y - data.bounds.halfSize.y, data.bounds.midPoint.z + data.bounds.halfSize.z},
				{data.bounds.midPoint.x + data.bounds.halfSize.x, data.bounds.midPoint.y + data.bounds.halfSize.y, data.bounds.midPoint.z + data.bounds.halfSize.z},
				{data.bounds.midPoint.x - data.bounds.halfSize.x, data.bounds.midPoint.y + data.bounds.halfSize.y, data.bounds.midPoint.z + data.bounds.halfSize.z}
			};

			for (auto& corner : corners) {
				corner = RotateVector(corner, yaw);
				corner += data.pos;
			}

			const int edges[12][2] = {
				{0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4},
				{0,4}, {1,5}, {2,6}, {3,7}
			};

			for (int j = 0; j < 12; j++) {
				Vector2 screenpos1, screenpos2;
				if (WorldToScreen(corners[edges[j][0]], &screenpos1) && WorldToScreen(corners[edges[j][1]], &screenpos2)) {
					data.boxEdges.push_back(screenpos1);
					data.boxEdges.push_back(screenpos2);
				}
			}

		}

		lootData.push_back(std::move(data));
	}
}

bool get_bbox(Vector4& box, player* i)
{
	auto pos = i->playerPos;

	Vector2 screen_pos;
	if (!WorldToScreen(pos, &screen_pos))
		return false;

	pos.z += 70;

	Vector2 screen_pos_2;
	if (!WorldToScreen(pos, &screen_pos_2))
		return false;

	screen_pos_2.x -= (screen_pos.y - screen_pos_2.y) / 4;
	screen_pos.x += (screen_pos.y - screen_pos_2.y) / 4;

	box.x = screen_pos_2.x;
	box.y = screen_pos_2.y;
	box.w = screen_pos.x - screen_pos_2.x;
	box.z = screen_pos.y - screen_pos_2.y;

	return true;
}

__forceinline bool is_key_down(int vk_key)
{
	int controllerBind = 0;

	if (gamepad::Gpad_IsButtonActive(vk_key))
		return true;

	// Keyboard buttons only
	if (((GetAsyncKeyState(vk_key) & 0x8000) ? 1 : 0) || ImGui::IsKeyPressed((ImGuiKey)controllerBind)) // can i get rid of this? no
		return true;

	return false;
}

__forceinline bool is_overid_down(int vk_key)
{
	int controllerBind = 0;

	if (gamepad::Gpad_IsButtonActive(vk_key))
		return true;

	// Keyboard buttons only
	if (((GetAsyncKeyState(vk_key) & 0x8000) ? 1 : 0) || ImGui::IsKeyPressed((ImGuiKey)controllerBind)) // can i get rid of this? no
		return true;

	return false;
}

float dot_product(ImVec3 vec)
{
	return (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float dot_product(ImVec3 lhs, ImVec3 rhs)
{
	return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
}

float vector_length(ImVec3 vec)
{
	return std::sqrtf(dot_product(vec));
}

void make_vector(ImVec3 angles, ImVec3* out)
{
	auto up = degrees_to_radians(angles.x);
	auto right = degrees_to_radians(angles.y);

	out->x = -std::cosf(up) * -std::cosf(right);
	out->y = std::sinf(right) * std::cosf(up);
	out->z = -std::sinf(up);
}

float vector_normalize(ImVec3* direction)
{
	auto length = vector_length(*direction);
	auto result = length;

	if (-length >= 0.0f)
		length = 1.0f;

	direction->x *= 1.0f / length;
	direction->y *= 1.0f / length;
	direction->z *= 1.0f / length;

	return result;
}

void vec_to_angles(ImVec3 direction, ImVec3* angles)
{
	std::float_t temp, yaw, pitch;

	if (direction.x == 0.0f && direction.y == 0.0f)
	{
		yaw = 0.0f;

		if (direction.z > 0.0f)
			pitch = 90.0f;

		else
			pitch = 270.0f;
	}

	else
	{
		yaw = radians_to_degrees2(std::atan2f(direction.y, direction.x));

		if (yaw < 0.0f)
			yaw += 360.0f;

		temp = std::sqrtf(direction.x * direction.x + direction.y * direction.y);
		pitch = radians_to_degrees2(std::atan2f(direction.z, temp));

		if (pitch < 0.0f)
			pitch += 360.0f;
	}

	angles->x = -pitch;
	angles->y = yaw;
	angles->z = 0.0f;
}

float calculate_fov(Vector3 start, Vector3 end, ImVec3 angles)
{
	ImVec3 dir = ImVec3(start.x - end.x, start.y - end.y, start.z - end.z);
	ImVec3 aim_angles;

	vector_normalize(&dir);
	vec_to_angles(dir, &aim_angles);

	make_vector(angles, &angles);
	make_vector(aim_angles, &aim_angles);

	auto length = vector_length(angles);
	auto product = dot_product(angles, aim_angles);

	float result = radians_to_degrees(std::acosf(product / std::powf(length, 2.0f)));

	if (std::isnan(result))
		result = 0.0f;

	return result;
}

void normalize_angle(Vector3& ang)
{
	ang.x = std::isfinite(ang.x) ? std::remainderf(ang.x, 360.0f) : 0.0f;
	ang.y = std::isfinite(ang.y) ? std::remainderf(ang.y, 360.0f) : 0.0f;
	ang.z = 0.0f;
}
double Radians(float Degrees)
{
	return  0.01745329238474369 * Degrees;
}
signed char ClampChar(int i)
{
	if (i < -128)
		return -128;

	if (i > 127)
		return 127;

	return i;
}
//void CorrectMovement(usercmd_t* usercommand, Vector3 vec, Vector3 ViewAngles)
//{
//	Vector3 Fix = vec - ViewAngles;
//
//	int forward = usercommand->forwardmove;
//	int side = usercommand->rightmove;
//
//	if (forward >= 128)
//	{
//		forward = -(256 - forward);
//	}
//
//	if (side >= 128)
//	{
//		side = -(256 - side);
//	}
//
//	int fixedforward = (cos(Radians(Fix.y)) * forward) + (cos(Radians(Fix.y + 90)) * side);
//	//my_console.print(false, ImColor(255, 255, 255), "fixedforward: %i", fixedforward);
//	int fixedside = (sin(Radians(Fix.y)) * forward) + (sin(Radians(Fix.y + 90)) * side);
//	//my_console.print(false, ImColor(255, 255, 255), "fixedside: %i", fixedside);
//
//	usercommand->forwardmove = ClampChar(fixedforward);
//	usercommand->rightmove = ClampChar(fixedside);
//}
int ClActiveClient::cmdNumber()
{
	return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + game_data->game_offsets.cmdNumberPos);
}
void ClActiveClient::AddCmdNumber()
{
	++*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + game_data->game_offsets.cmdNumberPos);
}
unsigned int ClActiveClient::cmd_number_aab()
{
	return *reinterpret_cast<unsigned int*>(reinterpret_cast<uintptr_t>(this) + game_data->game_offsets.cmdNumberaabPos);
}
int ClActiveClient::GetCmdNumber()
{
	auto a1 = (uintptr_t)this;
	return cmdNumber() ^ (((a1 + game_data->game_offsets.cmdNumberPos) ^ cmd_number_aab()) * (((a1 + game_data->game_offsets.cmdNumberPos) ^ cmd_number_aab()) + 2));
}
usercmd_t* ClActiveClient::GetUserCmd(int sequence_number)
{

	auto a1 = (uintptr_t)this;

	if (game_data->game_offsets.usercmdPos != 1)
	{
		auto cmdList = reinterpret_cast<usercmd_s*>(game_data->game_offsets.userCmdSize * (sequence_number & 0x7F) + a1 + game_data->game_offsets.usercmdPos);
		return cmdList;
	}
	else
	{
		auto cmdList = reinterpret_cast<usercmd_s*>(a1 + game_data->game_offsets.userCmdSize * ((sequence_number & 0x7F) + 2LL));
		return cmdList;
	}
}
void ClActiveClient::SetCmdNumber(int newNumber)
{
	auto a1 = (uintptr_t)this;
	*(int*)(reinterpret_cast<uintptr_t>(this) + game_data->game_offsets.cmdNumberPos) = (newNumber ^ (((a1 + game_data->game_offsets.cmdNumberPos) ^ cmd_number_aab()) * (((a1 + game_data->game_offsets.cmdNumberPos) ^ cmd_number_aab()) + 2)));
}

uintptr_t decrypt_cg()
{
	uintptr_t gesture = VariadicCall<uintptr_t>(game_data->game_offsets.trampoline, game_data->game_offsets.GetGestureFunc, 0, 0, 0);
	return (gesture - game_data->game_offsets.cg_offset);
}

playerState_s* GetPlayerState(DWORD64 CG)
{
	return reinterpret_cast<playerState_s*>(CG + game_data->game_offsets.playerStatePadding);
}

ClActiveClient* ClActiveClient::GetActiveClient()
{
	return VariadicCall<ClActiveClient*>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CL_Input_ClearAutoForwardFlag, 0);
}

bool BG_Ladder_IsActive()
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_Ladder_IsActive_offset, client::localPlayer);
}

void CL_PlayerData_SetCustomClanTag(const char* clantag)
{
	VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.CL_PlayerData_SetCustomClanTag, 0, clantag);
}

void TriangleRotatedWithOutline(ImVec2 blip_pos, float blip_size, float rotationDegrees, ImColor color, ImColor outlineColor, float outlineThickness)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// Convert to radians, we don't add 90 degrees since we want exact alignment
	float rotationRadians = (rotationDegrees) * (M_PI / 180.0f);

	// Calculate triangle points
	// Tip point - this will exactly match player's view direction
	ImVec2 p1(
		blip_pos.x + blip_size * sin(rotationRadians),
		blip_pos.y - blip_size * cos(rotationRadians)
	);

	// Back points
	ImVec2 p2(
		blip_pos.x + blip_size * 0.7f * sin(rotationRadians + (2.0f * M_PI / 3.0f)),
		blip_pos.y - blip_size * 0.7f * cos(rotationRadians + (2.0f * M_PI / 3.0f))
	);

	ImVec2 p3(
		blip_pos.x + blip_size * 0.7f * sin(rotationRadians - (2.0f * M_PI / 3.0f)),
		blip_pos.y - blip_size * 0.7f * cos(rotationRadians - (2.0f * M_PI / 3.0f))
	);

	// Render triangle
	draw_list->AddTriangleFilled(p1, p2, p3, color);
	draw_list->AddLine(p1, p2, outlineColor, outlineThickness);
	draw_list->AddLine(p2, p3, outlineColor, outlineThickness);
	draw_list->AddLine(p3, p1, outlineColor, outlineThickness);
}

Vector3 clamp_angles(Vector3 angles)
{
	if (angles.x > 180.0f)
		angles.x -= 360.0f;

	if (angles.x < -180.0f)
		angles.x += 360.0f;

	if (angles.y > 180.0f)
		angles.y -= 360.0f;

	if (angles.y < -180.0f)
		angles.y += 360.0f;

	if (angles.z != 0.0f)
		angles.z = 0.0f;

	return angles;
}

namespace client {
	float radius;
	DWORD64 CgArray = 0;
	ClActiveClient* ClActive = NULL;
	usercmd_t* Cmd = NULL;
	playerState_s* localPlayer = NULL;

	int BG_UsrCmdPackAngle(float angle)
	{
		auto MSG_PackSignedFloat = reinterpret_cast<int(__fastcall*)(float value, float maxAbsValueSize, unsigned int bitCount)>(game_data->game_offsets.A_BG_UsrCmdPackAngle);

		__m128 v26 = { 0ui64, 0ui64 }, v27 = { 0ui64, 0ui64 }, nullm128 = { 0ui64, 0ui64 };
		v26.m128_f32[0] = angle * 0.0027777778;
		v27 = v26;
		v27.m128_f32[0] = v26.m128_f32[0] + 0.5;
		return MSG_PackSignedFloat(
			(float)(v26.m128_f32[0] - _mm_round_ss(nullm128, v27, 1).m128_f32[0]) * 360.0,
			180.0,
			20);
	}

	ImVec3 antiAimAngles = { 0,0,0 };

	void Jitteryboy(usercmd_s* usercmd) {
		static bool jitter = false;

		// Jitter
		antiAimAngles.x = (jitter ? 70 : -70);
		jitter = !jitter;

		// Set Angles
		usercmd->angles[0] = BG_UsrCmdPackAngle(antiAimAngles.x - client::localPlayer->deltaAngles.x);

		//antiAimAngles = { 0,0,0 };
	}
	float flAngle = 0.0f;

	void Spinnyboy(usercmd_s* usercmd) {
		static bool jitter = false;


		// Spin
		if (flAngle > 180.0f)
			flAngle -= 360.0f;
		antiAimAngles.y = flAngle;
		flAngle += 20.0f;

		// Set Angles
		usercmd->angles[0] = BG_UsrCmdPackAngle(-75.f - client::localPlayer->deltaAngles.x);
		usercmd->angles[1] = BG_UsrCmdPackAngle(antiAimAngles.y - client::localPlayer->deltaAngles.y);

		//antiAimAngles = { 0,0,0 };
		//Vector3 fixAngle = Vector3(-75.f, flAngle, 0.f);
		//CorrectMovement(usercmd, fixAngle, client::localPlayer->viewAngles);
	}

	// Call inside of PredictPlayerState
	void RapidFire(usercmd_t* cur_cmd, usercmd_t* old_cmd)
	{
		weapFireType_t weaponType = VariadicCall<weapFireType_t>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_GetWeaponFireType, &cur_cmd->weapon, false);
		//weapFireType_t weaponType = GameCall<weapFireType_t>(game_data->game_offsets.BG_GetWeaponFireType)(&cur_cmd->weapon, false);
		if (weaponType != WEAPON_FIRETYPE_SINGLESHOT && weaponType != WEAPON_FIRETYPE_BURST)
			return;

		if (old_cmd->buttons & 0x80000000)
			cur_cmd->buttons &= ~0x80000000;
	}


	void AutoFire(usercmd_t* userCmd)
	{
		if (is_key_down(game_data->settings.aim_key))
		{
			if (bestTarget != nullptr)
			{
				if (players[client::localPlayer->clientNum].player_name.health != 0)
					userCmd->buttons |= 0x80000000;
			}
		}
	}
	void Airstuck(usercmd_s* usercmd)
	{
		if (is_key_down(game_data->settings.airstuck_key))
		{
			usercmd->serverTime = 0;
		}
	}

	//void LongMelee(usercmd_t* userCmd)
	//{
	//	if (bestTarget != nullptr)
	//	{
	//		userCmd->meleeChargeEnt = 0;
	//	}
	//}

	//void SilentAim(usercmd_s* usercmd)
	//{
	//	if (!bestTarget)
	//		return;

	//	if (!bestTarget->is_valid)
	//		return;

	//	if (!isTargetLocket)
	//		return;

	//	SilentAimAngle.x -= client::localPlayer->deltaAngles.x;
	//	SilentAimAngle.y -= client::localPlayer->deltaAngles.y;

	//	usercmd->angles[0] = BG_UsrCmdPackAngle(SilentAimAngle.x);
	//	usercmd->angles[1] = BG_UsrCmdPackAngle(SilentAimAngle.y);
	//}

	//void SilentAim(usercmd_s* usercmd)
	//{
	//	if (!bestTarget || !isTargetLocket)
	//		return;

	//	Vector3 targetPos;
	//	Vector3 silentangle{};
	//	Vector3 silentaimPos;

	//	bool useOverrideBones = is_overid_down(game_data->settings.aim_Override_key) && game_data->settings.A_bone >= 0 && game_data->settings.A_bone < 9;
	//	bool useNormalBones = is_key_down(game_data->settings.aim_key) && game_data->settings.i_bone >= 0 && game_data->settings.i_bone < 6;

	//	if (!useOverrideBones && !useNormalBones)
	//		return; // Early return if no valid aim key is pressed

	//	if (useOverrideBones) {
	//		targetPos = getTargetPositionOverride(game_data->settings.A_bone, bestTarget);
	//	}
	//	else {
	//		targetPos = getTargetPositionNormal(game_data->settings.i_bone, bestTarget);
	//	}

	//	Vector3 startPos;
	//	VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_Entity_GetPlayerViewOrigin, client::localPlayer, &startPos);

	//	// **Fix: Ensure correct prediction calculation**
	//	if (game_data->settings.Prediction) {
	//		targetPos = calculate_prediction(bestTarget->entnum, startPos, targetPos);
	//	}

	//	// Compute aim angles
	//	silentaimPos = targetPos - startPos;
	//	VectorToAngles(silentaimPos, silentangle);

	//	// **Fix: Adjust delta angles correctly**
	//	Vector3 deltaAngles = client::localPlayer->deltaAngles;
	//	silentangle.x -= deltaAngles.x;
	//	silentangle.y -= deltaAngles.y;

	//	// Apply to usercmd

	//	silentangle = clamp_angles(silentangle);

	//	usercmd->angles[0] = BG_UsrCmdPackAngle(silentangle.x);
	//	usercmd->angles[1] = BG_UsrCmdPackAngle(silentangle.y);

	//}

	void SilentAim(usercmd_s* usercmd)
	{
		if (isTargetLocket)
		{
			if (bestTarget != nullptr)
			{
				SilentAimAngle.x -= client::localPlayer->deltaAngles.x;
				SilentAimAngle.y -= client::localPlayer->deltaAngles.y;

				if (SilentAimAngle.x != 0.f && SilentAimAngle.y != 0.f)
				{
					usercmd->angles[0] = BG_UsrCmdPackAngle(SilentAimAngle.x);
					usercmd->angles[1] = BG_UsrCmdPackAngle(SilentAimAngle.y);
				}
			}
		}
	}

	void AntiNig(usercmd_s* usercmd)
	{
		auto viewAngles = client::localPlayer->viewAngles;

		float backAngle = viewAngles.y - 180.f;


		// Set Angles
		usercmd->angles[0] = BG_UsrCmdPackAngle(-85.f - client::localPlayer->deltaAngles.x);
		usercmd->angles[1] = BG_UsrCmdPackAngle(backAngle - client::localPlayer->deltaAngles.y);

		viewAngles = { 0,0,0 };
		antiAimAngles.y = 0.0f;
		//Vector3 FixedAngle = Vector3(-85.f, backAngle, 0.f);
		//CorrectMovement(usercmd, FixedAngle, client::localPlayer->viewAngles);
	}
}

namespace radar {
	float scale = 8.51f;

	float blip_size = 10.0f;

	float angle = 0;

	ImVec2 blip_pos[2048];

	ImVec2 pos;
	ImVec2 size;
	ImVec2 center;

	bool enabled = false;
	bool big_map = false;

	void draw()
	{
		ImGui::SetNextWindowPos(radar::pos);

		ImGui::SetNextWindowSize(radar::size);

		radar::center = {
			ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2.0f,
			ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f
		};

		for (size_t i = 0; i <= 2046; i++)
		{
			player* p = &players[i];

			if (!p->is_valid)
				continue;

			if (p->distance >= game_data->settings.distanceFloat)
				continue;

			if (game_data->settings.enemies_only && p->team == players[client::localPlayer->clientNum].team)
				continue;

			if (p->is_AI && !(game_data->settings.showaibool))
				continue;

			ImColor color = game_data->settings.UAV_color;

			float relativeAngle = client::localPlayer->viewAngles.y - getAngles(i).y;

			if (radar::big_map)
			{
				relativeAngle = radar::angle - getAngles(i).y;
			}

			if (game_data->settings.uav_type == 0)
			{
				ImGui::GetWindowDrawList()->AddCircleFilled(radar::blip_pos[i], 4.0f, color);

				ImGui::GetWindowDrawList()->AddCircle(radar::blip_pos[i], 4.0f, color);
			}
			else if (game_data->settings.uav_type == 1)
			{
				TriangleRotatedWithOutline(
					radar::blip_pos[i],
					radar::blip_size,
					relativeAngle,  // Directly pass the entity's angle
					color - ImColor(0, 0, 0, 100),
					color,
					2.0f
				);
			}
		}
	}
}

const char* DDL_GetString(__int64 state, __int64 ddlContext)
{
	return nullptr;
}

const int DDL_GetInt(__int64 state, __int64 ddlContext)
{
	return 0;
}

__int64 Com_DDL_LoadAsset(const char* file)
{
	return VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.Com_DDL_LoadAsset, file);
}

__int64 Com_DDL_LoadAsset2(__int64 file) {

	return VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.Com_DDL_LoadAsset, file);
}

bool Com_DDL_CreateContext(uintptr_t buffer, int len, uintptr_t def, uintptr_t ddlContext, uintptr_t accessdb, uintptr_t userData)
{
	return false;
}

bool NavigateBlueprintDDL(__int64 state, const char* path)
{
	return false;
}

int DDL_GetType(__int64 state)
{
	return VariadicCall<int>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_GetType, state);

}

bool DDL_SetFloat(__int64 state, __int64 ddlContext, float val)
{
	return false;
}

bool DDL_SetEnum(__int64 state, __int64 ddlContext, const char* val)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_SetEnum, state, ddlContext, val);

}

__int64 DDL_GetRootState(__int64 result, uintptr_t ddlDef)
{
	return VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_GetRootState, result, ddlDef);
}

bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.CL_PlayerData_GetDDLBuffer, context, controllerindex, stats_source, statsgroup);
}

__int64 DDL_MoveToName(__int64 fstate, __int64 tstate, __int64 path)
{
	return 0;
}

bool DDL_SetInt(__int64 fstate, __int64 tstate, int value)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_SetInt, fstate, tstate, value);

}

bool DDL_SetString(__int64 fstate, __int64 state, const char* val)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_SetString, fstate, state, val);

}

bool DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.DDL_MoveToPath, fromState, toState, depth, path);
}

bool Com_ParseNavStrings(const char* pStr, const char** navStrings, const int navStringMax, int* navStringCount)
{
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.Com_ParseNavStrings, pStr, navStrings, navStringMax, navStringCount);

}

void LUI_OpenMenu(__int64 a, const char* cmd, __int64 b, __int64 c, __int64 d) {
	uintptr_t address = game_data->game_offsets.a_LUI_OpenMenu;
	((void(*)(__int64, const char*, __int64, __int64, __int64))address)(a, cmd, b, c, d);
}

Vector3 CalcAngles(Vector3 src, Vector3 dest)
{
	Vector3 angles;

	Vector3 vec = dest - src;

	float length = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	vec.x /= length;
	vec.y /= length;
	vec.z /= length;
	float flYaw = atan2f(vec.y, vec.x) * (180.0f / M_PI);
	if (flYaw < 0.0f)
		flYaw += 360.0f;

	float flPitch = atan2f(vec.z, sqrtf(vec.x * vec.x + vec.y * vec.y)) * (180.0f / M_PI);
	if (flPitch < 0.0f)
		flPitch += 360.0f;

	angles.x = -flPitch;
	angles.y = flYaw;
	angles.z = 0.0f;

	while (angles.x > 180.0f)
		angles.x -= 360.0f;

	while (angles.x < -180.0f)
		angles.x += 360.0f;

	while (angles.y > 180.0f)
		angles.y -= 360.0f;

	while (angles.y < -180.0f)
		angles.y += 360.0f;

	if (angles.z != 0.0f)
		angles.z = 0.0f;

	return angles;
}

void R_AddDObjToScene(__int64 a1, cpose_t* a2, unsigned int a3, unsigned int entnum, int other, GfxSceneEntityMutableShaderData* a6, uint32_t* a7, int a8)
{
	if (game_data->settings.checkPassed && game_data->settings.b_ingame)
	{
		if (client::localPlayer)
		{
			if (entnum < 150)
			{
				player* p = &players[entnum];

				if (!p->is_valid)
				{
					VariadicCall<void>(game_data->game_offsets.trampoline, R_AddDObjToSceneTrampoline, a1, a2, a3, entnum, other, a6, a7, a8);
					return;
				}

				if (p->distance >= game_data->settings.distanceFloat)
				{
					VariadicCall<void>(game_data->game_offsets.trampoline, R_AddDObjToSceneTrampoline, a1, a2, a3, entnum, other, a6, a7, a8);
					return;
				}

				if (game_data->settings.enemies_only && p->team == players[client::localPlayer->clientNum].team)
				{
					VariadicCall<void>(game_data->game_offsets.trampoline, R_AddDObjToSceneTrampoline, a1, a2, a3, entnum, other, a6, a7, a8);
					return;
				}

				if (p->is_AI && !(game_data->settings.showaibool))
				{
					VariadicCall<void>(game_data->game_offsets.trampoline, R_AddDObjToSceneTrampoline, a1, a2, a3, entnum, other, a6, a7, a8);
					return;
				}

				bool isBestTarget = (game_data->settings.aim_target_esp && p == bestTarget);
				bool isVisible = (game_data->settings.b_visible && p->visible && !isBestTarget);

				// Default player color logic
				game_data->settings.playerColor = isVisible
					? ImGui::ColorConvertFloat4ToU32(game_data->settings.vis_player_colorVec4)
					: ImGui::ColorConvertFloat4ToU32(game_data->settings.player_colorVec4);

				// If this player is the best target, override with target color
				if (isBestTarget && bestTarget != nullptr && players[client::localPlayer->clientNum].player_name.health != 0)
				{
					game_data->settings.playerColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.Target_color);
				}

				if ((a2->eType == entityType_s::ET_PLAYER) ||
					(a2->eType == entityType_s::ET_AGENT && game_data->settings.showaibool))
				{
					a6->hudOutlineInfo.drawNonOccludedPixels = true;
					a6->hudOutlineInfo.drawOccludedPixels = true;

					if (game_data->settings.chamFill)
						a6->hudOutlineInfo.fill = true;

					a6->hudOutlineInfo.renderMode = 0;

					if (game_data->settings.rgbChams)
					{
						a6->hudOutlineInfo.color = game_data->settings.chamColorRGB;
					}
					else
					{
						a6->hudOutlineInfo.color = game_data->settings.playerColor;
					}

					if (!game_data->settings.real_chamFill)
					{
						a6->hudOutlineInfo.lineWidth = game_data->settings.lineWidthNum;
					}
					else
					{
						a6->hudOutlineInfo.lineWidth = -1;
					}
				}
			}
		}
	}

	VariadicCall<void>(game_data->game_offsets.trampoline, R_AddDObjToSceneTrampoline, a1, a2, a3, entnum, other, a6, a7, a8);
}

void R_AddViewmodelDObjToScene(__int64 a1, __int64 a2, int a3, int entnum, int renderFlags, GfxSceneEntityMutableShaderData* a6, __int64 a7, int a8, char a9)
{
	if (game_data->settings.checkPassed)
	{
		a6->hudOutlineInfo.drawNonOccludedPixels = true;

		a6->hudOutlineInfo.renderMode = 0;

		if (game_data->settings.HUDrgbChams)
		{
			a6->hudOutlineInfo.color = game_data->settings.chamColorRGB;
		}
		else
		{
			a6->hudOutlineInfo.color = game_data->settings.selfColor;
		}

		a6->hudOutlineInfo.lineWidth = game_data->settings.HUDlineWidthNum;
	}

	VariadicCall<void>(game_data->game_offsets.trampoline, ViewModelDetour, a1, a2, a3, entnum, renderFlags, a6, a7, a8, a9);
}

__int64 BG_FireWeaponRecoil(__int64 a1, __int64 a2, unsigned __int16* a3, float a4, float a5, int a6, float a7, float* a8, float* a9, float* a10, bool* a11, float* a12, bool* a13, char a14)
{
	return 1;
}

inline float BG_CalculateFinalSpreadForWeapon_hk(__int64 a1, uint32_t* a2, unsigned __int16* a3)
{
	return 0.0f;
}

float BG_CalculateFinalSpreadForWeapon(Weapon* weapon, float spreadscale)
{
	__int64 cgHnalder = VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.getCGHandler, 0);

	if (cgHnalder == 0ui64)
		return 0.0f;

	return VariadicCall<float>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_CalculateFinalSpreadForWeapon, cgHnalder, client::localPlayer, weapon, spreadscale);
}

bool CG_StartShellShock(int a1, __int64 a2, __int64 a3, int a4, int a5)
{
	return false;
}

void __fastcall CG_DrawDamageDirectionIndicators(int localClientNum, double cgameGlob)
{
	return;
}

//shellshock_t GetShellshock(uintptr_t cgOffset)
//{
//	return *reinterpret_cast<shellshock_t*>(uintptr_t(cgOffset) + game_data->game_offsets.shellshock);
//}

float scale_fov(float actualFov) {

	float viewAspect = 1.78f; // 16.0f / 9.0f (screenW / screenH)
	float scale = 0.75f; // 3.0f / 4.0f
	float ingameFOV = atan(tan((actualFov / 2) / 57.2957795) / (viewAspect * scale)) / 0.0087266462;
	return ingameFOV;
}

float GetLocalPlayerFOV()
{
	return *reinterpret_cast<float*>(client::CgArray + game_data->game_offsets.fov);
}

__int64 fov_hook(
	float* a1,
	int a2,
	int a3,
	char a4)
{

	auto ret = VariadicCall<__int64>(game_data->game_offsets.trampoline, (__int64)fov_orig, a1, a2, a3, a4);
	float fov_value = game_data->settings.f_fov; //range must be between 0-180;
	if (fov_value >= 0.1f)
	{
		a1[0] = scale_fov(fov_value);
	}
	return ret;
}

void UpdateToggleState(bool& toggleState, int key) {
	static bool wasGamepadButtonPressed = false;
	static bool wasKeyPressed = false;
	static auto lastToggleTime = std::chrono::steady_clock::now();
	const auto debounceDuration = std::chrono::milliseconds(250);

	bool isGamepadButtonPressed = gamepad::Gpad_IsButtonActive(key);
	bool isKeyPressed = GetAsyncKeyState(key) & 0x8000;

	auto currentTime = std::chrono::steady_clock::now();
	auto timeSinceLastToggle = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastToggleTime);

	if (isGamepadButtonPressed && !wasGamepadButtonPressed && timeSinceLastToggle >= debounceDuration) {
		toggleState = !toggleState;
		lastToggleTime = currentTime;
	}
	wasGamepadButtonPressed = isGamepadButtonPressed;

	if (isKeyPressed && !wasKeyPressed && timeSinceLastToggle >= debounceDuration) {
		toggleState = !toggleState;
		lastToggleTime = currentTime;
	}
	wasKeyPressed = isKeyPressed;
}

bool (*oBG_ThirdPerson_IsEnabled)(__int64 handler, __int64 ps);
bool hkBG_ThirdPerson_IsEnabled(__int64 handler, __int64 ps) {
	if (game_data->settings.ffpInspect && GameCall<bool>(game_data->game_offsets.A_CG_WeaponInspect_IsUIHidden)(0))
		return false;

	UpdateToggleState(game_data->settings.ThirdPerson_state, game_data->settings.thirdperson_key);

	return game_data->settings.ThirdPerson_state;
}

void (*oCG_EntityMP_CalcLerpPositions)(int localClientNum, DWORD64 centity);

void hkCG_EntityMP_CalcLerpPositions(int localClientNum, DWORD64 centity)
{
	auto tPredictedPlayerEntity = *(DWORD64*)(client::CgArray + game_data->game_offsets.PredictedPlayerEntity);
	if (centity != tPredictedPlayerEntity)
		return oCG_EntityMP_CalcLerpPositions(localClientNum, centity);

	if (game_data->settings.ffpInspect && GameCall<bool>(game_data->game_offsets.A_CG_WeaponInspect_IsUIHidden)(0))
		return oCG_EntityMP_CalcLerpPositions(localClientNum, centity);

	if (game_data->settings.Spinbot || game_data->settings.turned || game_data->settings.jitterbot)
	{
		ImVec3 antiAimAngles = client::antiAimAngles;

		// Set angles
		ImVec3 angles = *(ImVec3*)(centity + game_data->game_offsets.angle_offset);
		angles.x = antiAimAngles.x;
		angles.y = antiAimAngles.y;
		angles.z = antiAimAngles.z;

		// Assign angles back to entity
		*(ImVec3*)(centity + game_data->game_offsets.angle_offset) = angles;

		antiAimAngles = ImVec3(0, 0, 0);
	}
	else
	{
		oCG_EntityMP_CalcLerpPositions(localClientNum, centity);
	}
}

void NoFlinch()
{
	if (client::localPlayer)
	{
		client::localPlayer->damageEvent = 0;
		client::localPlayer->damageYaw = 255;
		client::localPlayer->damagePitch = 255;
		client::localPlayer->damageCount = 0;
	}
}

void No_CounterUAV()
{
	if (client::localPlayer)
	{
		client::localPlayer->radarBlocked = false;
	}
}

int previousCMDNumber = 0;

inline void sincos(const float rad, float& sine, float& cosine)
{
	sine = std::sin(rad);
	cosine = std::cos(rad);
}

void angle_vectors(const Vector3& ang, Vector3& out_forward, Vector3& out_right, Vector3& out_up)
{
	auto sine = Vector3(),
		cosine = Vector3();

	sincos(deg2rad(ang.x), sine.x, cosine.x);
	sincos(deg2rad(ang.y), sine.y, cosine.y);
	sincos(deg2rad(ang.z), sine.z, cosine.z);

	out_forward.x = cosine.x * cosine.y;
	out_forward.y = cosine.x * sine.y;
	out_forward.z = -sine.x;

	out_right.x = (-1 * sine.z * sine.x * cosine.y + -1 * cosine.z * -sine.y);
	out_right.y = (-1 * sine.z * sine.x * sine.y + -1 * cosine.z * cosine.y);
	out_right.z = (-1 * sine.z * cosine.x);

	out_up.x = (cosine.z * sine.x * cosine.y + -sine.z * -sine.y);
	out_up.y = (cosine.z * sine.x * sine.y + -sine.z * cosine.y);
	out_up.z = (cosine.z * cosine.x);
}

bool attemptHit(float hitChance) {

	float scaledHitChance = (100.0f - hitChance) / 100.0f;


	float randomChance = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);


	return randomChance <= scaledHitChance;
}

void PredictPlayerState_hk(int clientNum)
{
	//VMProtectBeginMutation("j");

	VariadicCall<void>(game_data->game_offsets.trampoline, PredictPlayerStateOriginal, clientNum);

	if (game_data->settings.checkPassed)
	{
		if (client::CgArray != 0)
		{
			if (client::localPlayer)
			{
				client::ClActive = ClActiveClient::GetActiveClient();
				if (client::ClActive)
				{
					if (players[client::localPlayer->clientNum].player_name.health != 0)
					{
						auto currentCmdNumber = client::ClActive->GetCmdNumber();
						auto oldCMD = client::ClActive->GetUserCmd(currentCmdNumber - 1);
						auto currentCMD = client::ClActive->GetUserCmd(currentCmdNumber);
						auto newCMD = client::ClActive->GetUserCmd(currentCmdNumber + 1);

						client::ClActive->SetCmdNumber(currentCmdNumber + 1);
						memcpy(newCMD, currentCMD, 0x268);

						if (currentCmdNumber > previousCMDNumber)
						{
							--currentCMD->CommandTime;
							++newCMD->serverTime;
						}

						if (game_data->settings.aim_type == 1)
						{
							if (attemptHit(game_data->settings.misschance_value))
							{
								client::SilentAim(currentCMD);
							}
						}

						if (!BG_Ladder_IsActive())
						{
							if (!is_key_down(game_data->settings.anti_aim_Override_key))
							{
								if (game_data->settings.Spinbot)
									client::Spinnyboy(newCMD);

								if (game_data->settings.turned)
									client::AntiNig(newCMD);

								if (game_data->settings.jitterbot)
									client::Jitteryboy(newCMD);
							}
						}

						if (game_data->settings.autoFire)
							client::AutoFire(currentCMD);

						if (game_data->settings.Rapidfire)
							client::RapidFire(currentCMD, oldCMD);

						if (game_data->settings.NoFlinch)
							NoFlinch();
					}

					previousCMDNumber = client::ClActive->GetCmdNumber();
				}
			}
		}
	}

	//VMProtectEnd();
}

//inline uintptr_t slide_endcheck_original;
//
//inline uintptr_t  slide_endcheck_hook(uintptr_t* pm, uintptr_t* pml, int* outTimeRemaining)
//{
//	if (client::ClActive)
//	{
//		auto cl = ClActiveClient::GetActiveClient();
//		if (cl)
//		{
//			auto oldcmd = cl->GetUserCmd(cl->GetCmdNumber() - 1);
//			auto cmd = cl->GetUserCmd(cl->GetCmdNumber());
//			if ((cmd && oldcmd && is_key_down(game_data->settings.anti_aim_Override_key)))
//			{
//				*(int*)((uintptr_t)cmd + 0x1C) = *(int*)((uintptr_t)oldcmd + 0x1C);
//				return 0;
//			}
//		}
//	}
//
//	return VariadicCall<__int64>(game_data->game_offsets.trampoline, slide_endcheck_original, pm, pml, outTimeRemaining);
//}

void (*oDrawPlayerMap)(CgCompassSystem* a1, DWORD64 mapOverlay, DWORD64 mapRotOverlay);

void hkDrawPlayerMap(CgCompassSystem* a1, DWORD64 mapOverlay, DWORD64 mapRotOverlay)
{

	if (a1 != nullptr)
	{
		radar::pos.x = a1->m_unscaledRect.x - 2;
		radar::pos.y = a1->m_unscaledRect.y - 2;
		radar::size.x = a1->m_unscaledRect.w + 4;
		radar::size.y = a1->m_unscaledRect.h + 4;
		radar::center.x = (radar::pos.x + (radar::size.x / 2));
		radar::center.y = (radar::pos.y + (radar::size.y / 2));
		radar::big_map = a1->m_currentCompassType == 2;
		radar::angle = a1->m_compassYawVector.y;
		for (size_t i = 0; i <= 2046; i++)
		{
			player* p = &players[i];

			if (!p->is_valid)
				continue;

			if (p->distance >= game_data->settings.distanceFloat)
				continue;

			if (game_data->settings.enemies_only && p->team == players[client::localPlayer->clientNum].team)
				continue;

			if (p->is_AI && !(game_data->settings.showaibool))
				continue;

			ImVec2 out, clipped;
			VariadicCall<void>(
				game_data->game_offsets.trampoline,
				game_data->game_offsets.A_WorldPosToCompass,
				a1,
				a1->m_currentCompassType,
				a1->m_minimapOptions->cropPartialMap,
				&a1->m_scaledRect,
				a1->m_minimapOptions->boundsRadius,
				&a1->m_compassYawVector,
				&client::localPlayer->origin,
				&p->playerPos,
				0.0,
				&out,
				&clipped
			);

			float base_height = 1080.0;
			float current_height = ImGui::GetIO().DisplaySize.y;

			// Calculate scaling factor based on height
			float scale_height = current_height / base_height;

			// Apply this scale factor to the positions
			radar::blip_pos[i].y = radar::center.y + (clipped.y * 2.1 * scale_height);
			radar::blip_pos[i].x = radar::center.x + (clipped.x * 2.1 * scale_height);
		}
	}

	return VariadicCall<void>(game_data->game_offsets.trampoline, (uintptr_t)oDrawPlayerMap, a1, mapOverlay, mapRotOverlay);
}

//void (*oCG_EntityMP_CalcLerpPositions)(int localClientNum, DWORD64 centity);
//
//void hkCG_EntityMP_CalcLerpPositions(int localClientNum, DWORD64 centity)
//{
//	auto tPredictedPlayerEntity = *(DWORD64*)(client::CgArray + game_data->game_offsets.PredictedPlayerEntity);
//	if (centity != tPredictedPlayerEntity)
//		return oCG_EntityMP_CalcLerpPositions(localClientNum, centity);
//
//	ImVec3 antiAimAngles = client::antiAimAngles;
//
//	// Set angles
//	ImVec3 angles = *(ImVec3*)(centity + game_data->game_offsets.angle_offset);
//	angles.x = antiAimAngles.x;
//	angles.y = antiAimAngles.y;
//	angles.z = antiAimAngles.z;
//
//	// Assign angles back to entity
//	*(ImVec3*)(centity + game_data->game_offsets.angle_offset) = angles;
//
//	antiAimAngles = ImVec3(0, 0, 0);
//}

__int64 BulletHitEvent_Internal(uintptr_t thisptr, int sourceEntityNum, int targetEntityNum,
	unsigned int targetScriptableIndex,
	unsigned __int8 boneIndex, const __int64* weapon, bool isAlternate,
	const Vector3* initialPos, const Vector3* startPos, const Vector3* position, const
	Vector3* normal, int surfType, int event, unsigned int impactEffects, int hitContents,
	const int hitImpactDelayOverride, __int64 hitArmorType, __int64 hitmarkerType,
	const __int64* optionalEventLodData, char unknown)
{
	if (game_data->settings.checkPassed && players[client::localPlayer->clientNum].player_name.health != 0 && client::CgArray != 0)
	{
		if (sourceEntityNum != client::localPlayer->clientNum && targetEntityNum > 150)
			return VariadicCall<__int64>(game_data->game_offsets.trampoline, bulletHitEventOriginal, thisptr, sourceEntityNum, targetEntityNum,
				targetScriptableIndex, boneIndex, weapon, isAlternate, initialPos, startPos,
				position, normal, surfType, event, impactEffects, hitContents,
				hitImpactDelayOverride, hitArmorType, hitmarkerType, optionalEventLodData,
				unknown);

		if (game_data->settings.bullet_tracers && sourceEntityNum == client::localPlayer->clientNum)
		{
			Vector3 startPos;
			LUI_GetEntityTagPositionAndAxis(client::localPlayer->clientNum, stringToHash64(XOR("j_gun")), &startPos);

			if (!game_data->settings.show_only_hit_bullet_tracers ||
				(game_data->settings.show_only_hit_bullet_tracers && targetEntityNum < 150))
			{
				auto tracer = cTracer();
				tracer.start3D = startPos;
				tracer.hit3D = *position;
				tracer.opacity = 255.0f;
				tracers.push_back(tracer);
			}
		}

		return VariadicCall<__int64>(game_data->game_offsets.trampoline, bulletHitEventOriginal, thisptr, sourceEntityNum, targetEntityNum,
			targetScriptableIndex, boneIndex, weapon, isAlternate, initialPos, startPos, position,
			normal, surfType, event, impactEffects, hitContents, hitImpactDelayOverride,
			hitArmorType, hitmarkerType, optionalEventLodData, unknown);
	}
	else
	{
		return VariadicCall<__int64>(game_data->game_offsets.trampoline, bulletHitEventOriginal, thisptr, sourceEntityNum, targetEntityNum,
			targetScriptableIndex, boneIndex, weapon, isAlternate, initialPos, startPos, position,
			normal, surfType, event, impactEffects, hitContents, hitImpactDelayOverride,
			hitArmorType, hitmarkerType, optionalEventLodData, unknown);
	}
}

//void fast_reload()
//{
//	if (!client::localPlayer)
//	{
//	//log_shit("localPlayer is NULL, exiting fast_reload");
//		return;
//	}
//
//	static std::mutex reload_mutex; // Protects reloaded_weapon
//	std::lock_guard<std::mutex> lock(reload_mutex);
//	static auto reloaded_weapon{ 0i64 };
//
//	Weapon* weapon = VariadicCall<Weapon*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, centity_t::GetEntity(client::localPlayer->clientNum));
//	if (weapon == 0ui64)
//	{
//	//log_shit("Weapon pointer is NULL, exiting fast_reload");
//		return;
//	}
//
//	auto is_reloading = [&]() {
//		return (client::localPlayer->weapState[0].weaponState == 30);
//		};
//
//	auto ammo_added = [&](Weapon* wp) { // Capture by value to avoid potential issues with external modifications
//		return (VariadicCall<int>(game_data->game_offsets.trampoline, game_data->game_offsets.A_BG_GetAmmoInClipForWeapon, client::localPlayer, wp, 0, 0) == VariadicCall<int>(game_data->game_offsets.trampoline, game_data->game_offsets.A_BG_GetClipSize, client::localPlayer, wp));
//		};
//
//	if (reloaded_weapon && !is_reloading())
//	{
//		VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_CycleWeapon, 0, 0, 0, 1, 0);
//	//log_shit("Weapon cycle performed due to reloaded_weapon flag being reset");
//		reloaded_weapon = 0;
//	}
//	else if (weapon && is_reloading() && ammo_added(weapon))
//	{
//		if (!reloaded_weapon) {
//		//log_shit("Weapon is reloading with ammo added, setting reloaded_weapon");
//		}
//		reloaded_weapon = 1;
//		VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_CycleWeapon, 0, 0, 0, 1, 0);
//	}
//}

__int64 CycleWeapon(int next)
{
	return VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_CycleWeapon, 0, 0, next, 0);
}

void fast_reload()
{
	if (client::localPlayer)
	{
		static bool reloaded_weapon = false;

		Weapon* weapon = VariadicCall<Weapon*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, centity_t::GetEntity(client::localPlayer->clientNum));
		if (!weapon)
		{
			return;
		}

		auto is_reloading = [&]() {
			return (client::localPlayer->weapState[0].weaponState == 30);
			};

		auto ammo_added = [&](Weapon* weapon) {
			return (VariadicCall<int>(game_data->game_offsets.trampoline, game_data->game_offsets.A_BG_GetAmmoInClipForWeapon, client::localPlayer, weapon, 0, 0) ==
				VariadicCall<int>(game_data->game_offsets.trampoline, game_data->game_offsets.A_BG_GetClipSize, client::localPlayer, weapon, 0));
			};

		if (reloaded_weapon)
		{
			CycleWeapon(1);
			reloaded_weapon = false;
		}

		if (weapon && is_reloading() && ammo_added(weapon))
		{
			reloaded_weapon = true;
			CycleWeapon(1);
		}
	}
}

BgWeaponHandle WeaponsEquipped(int weapon)
{
	return static_cast<BgWeaponHandle>((uintptr_t)(client::localPlayer) + game_data->game_offsets.equippedWeaponsPosition + (weapon * 0x4));
}

int GetHeldWeapon()
{
	return *(int*)((uintptr_t)(client::localPlayer) + game_data->game_offsets.equippedWeaponsPosition);
}

void InstantSwap()
{

	static int lastWeapon{ 0 };
	static int iReloadedWeapon{ 0 };
	if (lastWeapon != 0)
	{
		if (iReloadedWeapon)
		{
			for (int i = 0; i < 15; i++)
			{
				if (WeaponsEquipped(iReloadedWeapon).m_mapEntryId ==
					GetHeldWeapon())
				{
					CycleWeapon(0);
					break;
				}
			}
			CycleWeapon(0);
			lastWeapon = 0;
			iReloadedWeapon = 0;
		}
		if (iReloadedWeapon == 0)
		{
			if (int iCurrentWeapon{ GetHeldWeapon() }; iCurrentWeapon && (iCurrentWeapon != lastWeapon)
				&& lastWeapon != 0)
			{
				iReloadedWeapon = iCurrentWeapon;

				for (int i = 0; i < 15; i++)
				{
					if (int iWeapon{ WeaponsEquipped(i).m_mapEntryId }; iWeapon && iWeapon !=
						iReloadedWeapon)
					{
						CycleWeapon(1);
						break;
					}
				}
			}
		}
	}
	if (iReloadedWeapon == 0)
	{
		lastWeapon = GetHeldWeapon();
	}

}

void draw_line(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness)
{
	auto window = ImGui::GetWindowDrawList();
	window->AddLine(from, to, color, thickness);
}

void draw_3d_box(const Bounds& bounds, const Vector3& origin, const  Vector3& angle, ImColor color)
{
	float yaw = angle.y * M_PI / 180.f;

	std::vector< Vector3>vertices(8);

	vertices[0] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[1] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[2] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[3] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[4] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[5] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[6] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[7] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };


	for (auto& vertex : vertices)
	{
		vertex = RotateVector(vertex, yaw);
		vertex += origin;
	}

	const std::array<std::array<int, 2>, 12>edgePairs = {
		{{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},
		{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}
	};

	for (const auto& [v0, v1] : edgePairs)
	{
		Vector2 screenpos1, screenpos2;

		if (WorldToScreen_2(vertices[v0], &screenpos1) && WorldToScreen_2(vertices[v1], &screenpos2))
			draw_line(ImVec2(screenpos1.x, screenpos1.y), ImVec2(screenpos2.x, screenpos2.y), color, 1.f);
	}
}

void player_draw_3d_box(const Bounds& bounds, const Vector3& origin, const  Vector3& angle, ImColor color)
{
	float yaw = angle.y * M_PI / 180.f;

	std::vector< Vector3>vertices(8);

	vertices[0] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[1] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[2] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[3] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z - bounds.halfSize.z };
	vertices[4] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[5] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y - bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[6] = { bounds.midPoint.x + bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };
	vertices[7] = { bounds.midPoint.x - bounds.halfSize.x,bounds.midPoint.y + bounds.halfSize.y,bounds.midPoint.z + bounds.halfSize.z };


	for (auto& vertex : vertices)
	{
		vertex = RotateVector(vertex, yaw);
		vertex += origin;
	}

	const std::array<std::array<int, 2>, 12>edgePairs = {
		{{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},
		{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}
	};

	for (const auto& [v0, v1] : edgePairs)
	{
		Vector2 screenpos1, screenpos2;

		if (WorldToScreen(vertices[v0], &screenpos1) && WorldToScreen(vertices[v1], &screenpos2))
			draw_line(ImVec2(screenpos1.x, screenpos1.y), ImVec2(screenpos2.x, screenpos2.y), color, 1.f);
	}
}

void draw_filled_rect_between_vertices(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3, ImColor color)
{
	Vector2 screenpos0, screenpos1, screenpos2, screenpos3;

	if (WorldToScreen(v0, &screenpos0) && WorldToScreen(v1, &screenpos1) &&
		WorldToScreen(v2, &screenpos2) && WorldToScreen(v3, &screenpos3))
	{
		// Adjust the alpha value for transparency
		color.Value.w *= 0.2f; // Example: set alpha to 50% transparency

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddQuadFilled(ImVec2(screenpos0.x, screenpos0.y),
			ImVec2(screenpos1.x, screenpos1.y),
			ImVec2(screenpos2.x, screenpos2.y),
			ImVec2(screenpos3.x, screenpos3.y),
			color);
	}
}

void draw_3d_box_filled(const Bounds& bounds, const Vector3& origin, const Vector3& angle, ImColor color)
{
	float yaw = angle.y * M_PI / 180.f;

	std::vector<Vector3> vertices(8);

	vertices[0] = { bounds.midPoint.x - bounds.halfSize.x, bounds.midPoint.y - bounds.halfSize.y, bounds.midPoint.z - bounds.halfSize.z };
	vertices[1] = { bounds.midPoint.x + bounds.halfSize.x, bounds.midPoint.y - bounds.halfSize.y, bounds.midPoint.z - bounds.halfSize.z };
	vertices[2] = { bounds.midPoint.x + bounds.halfSize.x, bounds.midPoint.y + bounds.halfSize.y, bounds.midPoint.z - bounds.halfSize.z };
	vertices[3] = { bounds.midPoint.x - bounds.halfSize.x, bounds.midPoint.y + bounds.halfSize.y, bounds.midPoint.z - bounds.halfSize.z };
	vertices[4] = { bounds.midPoint.x - bounds.halfSize.x, bounds.midPoint.y - bounds.halfSize.y, bounds.midPoint.z + bounds.halfSize.z };
	vertices[5] = { bounds.midPoint.x + bounds.halfSize.x, bounds.midPoint.y - bounds.halfSize.y, bounds.midPoint.z + bounds.halfSize.z };
	vertices[6] = { bounds.midPoint.x + bounds.halfSize.x, bounds.midPoint.y + bounds.halfSize.y, bounds.midPoint.z + bounds.halfSize.z };
	vertices[7] = { bounds.midPoint.x - bounds.halfSize.x, bounds.midPoint.y + bounds.halfSize.y, bounds.midPoint.z + bounds.halfSize.z };

	for (auto& vertex : vertices)
	{
		vertex = RotateVector(vertex, yaw);
		vertex += origin;
	}

	draw_filled_rect_between_vertices(vertices[0], vertices[1], vertices[2], vertices[3], color);
	draw_filled_rect_between_vertices(vertices[4], vertices[5], vertices[6], vertices[7], color);
	draw_filled_rect_between_vertices(vertices[0], vertices[1], vertices[5], vertices[4], color);
	draw_filled_rect_between_vertices(vertices[2], vertices[3], vertices[7], vertices[6], color);
	draw_filled_rect_between_vertices(vertices[0], vertices[3], vertices[7], vertices[4], color);
	draw_filled_rect_between_vertices(vertices[1], vertices[2], vertices[6], vertices[5], color);
}

std::string GetWeaponName(int index) {
	char namebuf[255];

	Weapon* weapon = GameCall<Weapon*>(game_data->game_offsets.CG_GetEntWeapon)(0, centity_t::GetEntity(index));
	if (weapon == NULL)
		return "None";

	DWORD64 loc_weapon = GameCall<DWORD64>(game_data->game_offsets.A_BG_GetWeaponDisplayName)(weapon);
	if (loc_weapon == NULL)
		return "None";

	return GameCall<const char*>(game_data->game_offsets.A_UI_SafeTranslateString_Hash)(loc_weapon);
}

//Vector3 CG_GetPoseOrigin(int index)
//{
//	Vector3 Position;
//	float matrix[64]{};
//	VariadicCall<uintptr_t>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetPoseOrigin, 0, index, matrix, &Position);
//
//	return Position;
//}
//
//Vector3 centity_t::get_pos()
//{
//	return CG_GetPoseOrigin(this->nextstate().number);
//}

entityState_t& nextstate(centity_t* entity) {
	return *reinterpret_cast<entityState_t*>(
		reinterpret_cast<uintptr_t>(entity) + game_data->game_offsets.nextState);
}

char& flag(centity_t* entity) {
	return *reinterpret_cast<char*>(
		reinterpret_cast<uintptr_t>(entity) + game_data->game_offsets.flags);
}

bool is_alive(centity_t* entity)
{
	return (flag(entity) & 1) != 0;
}

bool G_Main_SightTrace(const Vector3* start, const Vector3* end, int ownEntNum, int clientEntNum)
{
	int contextMasks = 41965571;
	//return GameCall<bool>(game_data->game_offsets.G_Main_SightTrace)(3, start, end, ownEntNum, clientEntNum, contextMasks) == 0;
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.G_Main_SightTrace, 3, start, end, ownEntNum, clientEntNum, contextMasks) == 0;
}

bool LegacyTrace(Vector3* start, Vector3* end, int index)
{
	// Assuming Vector3 is defined as a struct or class and initialized accordingly
	Vector3 bounds[2] = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f} };
	trace_t trace;

	auto ent = centity_t::GetEntity(client::localPlayer->clientNum);

	if (!ent)
		return false;

	// Corrected call to VariadicCall without taking the address of pointers
	VariadicCall<void>(
		game_data->game_offsets.trampoline,
		game_data->game_offsets.LegacyTrace,
		Physics_WorldId::PHYSICS_WORLD_ID_CLIENT0_FIRST,
		&trace,
		start,
		end,
		bounds,
		nextstate(ent).number,
		0,
		0x280EC93,
		0
	);

	// Ensure trace.hitId and trace.fraction are properly defined and initialized
	return (trace.hitId == index || trace.fraction >= 1.0f);
}

bool Legacy_is_visible(Vector3* start, Vector3* end, int index)
{
	return LegacyTrace(start, end, index);
}

bool blackbox_unk()
{
	*(BYTE*)(game_data->game_offsets.s_blackboxInitialized) = 0;

	Cbuf_AddText(XOR("seta #x3CFD6266AF0DF6273 1;")); //blackboxBandwidthLimited
	Cbuf_AddText(XOR("seta #x3318F6197BA31A549 0;")); //blackboxUseBlackbox
	Cbuf_AddText(XOR("seta #x37E5121256654538F 0;")); //blackboxHighVolumeProbability
	Cbuf_AddText(XOR("seta #x3315D4DEA39EF21F6 0;")); //blackbox_dumpSystemMemoryFootprint
	Cbuf_AddText(XOR("seta #x316E5B9A3B08C7122 0;")); //lui_root_dlog_enabled
	Cbuf_AddText(XOR("seta #x3F21EDBD36421E41A 0;")); //dlog_enabled
	Cbuf_AddText(XOR("seta #x37E5F6E4D4D6020CE 0;")); //online_blackbox_send_bootguid_in_connect
	Cbuf_AddText(XOR("seta #x33DD8187FB97C7571 0;")); //online_blackbox_send_snapshot_bw_data

	return false;
}

bool blackbox_off()
{
	*(BYTE*)(game_data->game_offsets.s_blackboxInitialized) = 1;

	Cbuf_AddText(XOR("seta #x3CFD6266AF0DF6273 0;")); //blackboxBandwidthLimited
	Cbuf_AddText(XOR("seta #x3318F6197BA31A549 1;")); //blackboxUseBlackbox
	Cbuf_AddText(XOR("seta #x37E5121256654538F 1;")); //blackboxHighVolumeProbability
	Cbuf_AddText(XOR("seta #x3315D4DEA39EF21F6 1;")); //blackbox_dumpSystemMemoryFootprint
	Cbuf_AddText(XOR("seta #x316E5B9A3B08C7122 1;")); //lui_root_dlog_enabled
	Cbuf_AddText(XOR("seta #x3F21EDBD36421E41A 1;")); //dlog_enabled
	Cbuf_AddText(XOR("seta #x37E5F6E4D4D6020CE 1;")); //online_blackbox_send_bootguid_in_connect
	Cbuf_AddText(XOR("seta #x33DD8187FB97C7571 1;")); //online_blackbox_send_snapshot_bw_data

	return false;
}

inline uintptr_t Online_Loot_GetItemQuantity_ori;

unsigned int Online_Loot_GetItemQuantity(__int64 Instance, __int64 controllerIndex, __int64 id)
{
	blackbox_unk();

	return 99;
}

void init_hooks()
{
	if (!game_data->settings.PredictPlayerState_hooked && game_data->settings.checkPassed && game_data->settings.b_ingame)
	{
		MH_CreateHook((LPVOID*)(game_data->game_offsets.PredictPlayerState), &PredictPlayerState_hk, (LPVOID*)&PredictPlayerStateOriginal);
		MH_EnableHook((LPVOID*)(game_data->game_offsets.PredictPlayerState));
		game_data->settings.PredictPlayerState_hooked = true;
	}

	if (game_data->settings.UnlockAllItems)
	{
		if (!game_data->settings.UnlockAllItems_hooked)
		{
			MH_CreateHook(reinterpret_cast<void**>(game_data->game_offsets.GetItemQuanity), Online_Loot_GetItemQuantity, reinterpret_cast<void**>(&Online_Loot_GetItemQuantity_ori));
			MH_EnableHook(reinterpret_cast<void**>(game_data->game_offsets.GetItemQuanity));
			game_data->settings.UnlockAllItems_hooked = true;
		}
	}
	else
	{
		if (game_data->settings.UnlockAllItems_hooked)
		{
			MH_DisableHook(reinterpret_cast<void**>(game_data->game_offsets.GetItemQuanity));
			MH_RemoveHook(reinterpret_cast<void**>(game_data->game_offsets.GetItemQuanity));
			blackbox_off();
			game_data->settings.UnlockAllItems_hooked = false;
		}
	}

	if (game_data->settings.uavtest)
	{
		if (!game_data->settings.uavtest_hooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.A_DrawMapLayer), &hkDrawPlayerMap, (LPVOID*)&oDrawPlayerMap);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.A_DrawMapLayer));
			game_data->settings.uavtest_hooked = true;
		}
	}
	else
	{
		if (game_data->settings.uavtest_hooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.A_DrawMapLayer));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.A_DrawMapLayer));
			game_data->settings.uavtest_hooked = false;
		}
	}


	if (game_data->settings.bullet_tracers)
	{
		if (!game_data->settings.bullet_tracers_hooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.a_BulletHitEvent_Internal), &BulletHitEvent_Internal, (LPVOID*)&bulletHitEventOriginal);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.a_BulletHitEvent_Internal));
			game_data->settings.bullet_tracers_hooked = true;
		}
	}
	else
	{
		if (game_data->settings.bullet_tracers_hooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.a_BulletHitEvent_Internal));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.a_BulletHitEvent_Internal));
			game_data->settings.bullet_tracers_hooked = false;
		}
	}

	if (game_data->settings.no_recoil)
	{
		if (!game_data->settings.no_recoilhooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.BG_WeaponFireRecoilOffset), &BG_FireWeaponRecoil, (LPVOID*)&CG_NotifyServerOriginal);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.BG_WeaponFireRecoilOffset));
			game_data->settings.no_recoilhooked = true;
		}
	}
	else
	{
		if (game_data->settings.no_recoilhooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.BG_WeaponFireRecoilOffset));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.BG_WeaponFireRecoilOffset));
			game_data->settings.no_recoilhooked = false;
		}
	}

	if (game_data->settings.Nospread)
	{
		if (!game_data->settings.no_spreadhooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.BG_CalculateFinalSpreadForWeapon), &BG_CalculateFinalSpreadForWeapon_hk, (LPVOID*)&test1_Original);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.BG_CalculateFinalSpreadForWeapon));
			game_data->settings.no_spreadhooked = true;
		}
	}
	else
	{
		if (game_data->settings.no_spreadhooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.BG_CalculateFinalSpreadForWeapon));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.BG_CalculateFinalSpreadForWeapon));
			game_data->settings.no_spreadhooked = false;
		}
	}

	if (game_data->settings.ThirdPerson)
	{
		if (!game_data->settings.thirdpersonhook && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.A_BG_ThirdPerson_IsEnabled), &hkBG_ThirdPerson_IsEnabled, (LPVOID*)&oBG_ThirdPerson_IsEnabled);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.A_BG_ThirdPerson_IsEnabled));

			MH_CreateHook((LPVOID*)(game_data->game_offsets.CG_EntityMP_CalcLerpPositions), &hkCG_EntityMP_CalcLerpPositions, (LPVOID*)&oCG_EntityMP_CalcLerpPositions);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.CG_EntityMP_CalcLerpPositions));

			game_data->settings.thirdpersonhook = true;
			game_data->settings.oCG_EntityMP_CalcLerpPositions_hook = true;
		}
	}
	else
	{
		if (game_data->settings.thirdpersonhook)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.A_BG_ThirdPerson_IsEnabled));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.A_BG_ThirdPerson_IsEnabled));
			MH_DisableHook((LPVOID*)(game_data->game_offsets.CG_EntityMP_CalcLerpPositions));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.CG_EntityMP_CalcLerpPositions));
			game_data->settings.thirdpersonhook = false;
			game_data->settings.oCG_EntityMP_CalcLerpPositions_hook = false;
		}
	}

	if (game_data->settings.b_fov)
	{
		if (!game_data->settings.fovhooked)
		{
			if (!game_data->settings.fovhook_exist)
			{
				MH_CreateHook((void*)(game_data->game_offsets.fov_hook), (void*)fov_hook, (LPVOID*)&fov_orig);
				game_data->settings.fovhook_exist = true;
			}
			MH_EnableHook((void*)(game_data->game_offsets.fov_hook));
			game_data->settings.fovhooked = true;
		}
	}
	else
	{
		if (game_data->settings.fovhooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.fov_hook));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.fov_hook));
			game_data->settings.fovhooked = false;
			game_data->settings.fovhook_exist = false;
		}
	}

	if (game_data->settings.NoStun || game_data->settings.NoFlash)
	{
		if (!game_data->settings.shellshockhooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.CG_StartShellShock), &CG_StartShellShock, (LPVOID*)&CG_StartShellShock_original);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.CG_StartShellShock));
			game_data->settings.shellshockhooked = true;
		}
	}
	else
	{
		if (game_data->settings.shellshockhooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.CG_StartShellShock));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.CG_StartShellShock));
			game_data->settings.shellshockhooked = false;
		}
	}

	//if (game_data->settings.nodamage_bool)
	//{
	//	if (!game_data->settings.DamageDirectionIndicators_hooked && game_data->settings.checkPassed)
	//	{
	//		MH_CreateHook)((LPVOID*)(game_data->game_offsets.CG_DrawDamageDirectionIndicators), &CG_DrawDamageDirectionIndicators, (LPVOID*)&CG_DrawDamageDirectionIndicators_original);
	//		MH_EnableHook)((LPVOID*)(game_data->game_offsets.CG_DrawDamageDirectionIndicators));
	//		game_data->settings.shellshockhooked = true;
	//	}
	//}
	//else
	//{
	//	if (game_data->settings.DamageDirectionIndicators_hooked)
	//	{
	//		MH_DisableHook)((LPVOID*)(game_data->game_offsets.CG_DrawDamageDirectionIndicators));
	//		MH_RemoveHook)((LPVOID*)(game_data->game_offsets.CG_DrawDamageDirectionIndicators));
	//		game_data->settings.DamageDirectionIndicators_hooked = false;
	//	}
	//}

	if (game_data->settings.chams)
	{
		if (!game_data->settings.chamHooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.R_AddDObjToScene), &R_AddDObjToScene, (LPVOID*)&R_AddDObjToSceneTrampoline);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.R_AddDObjToScene));
			game_data->settings.chamHooked = true;
		}
	}
	else
	{
		if (game_data->settings.chamHooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.R_AddDObjToScene));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.R_AddDObjToScene));
			game_data->settings.chamHooked = false;
		}
	}

	if (game_data->settings.HUDchams)
	{
		if (!game_data->settings.HUDchamHooked && game_data->settings.checkPassed)
		{
			MH_CreateHook((LPVOID*)(game_data->game_offsets.R_AddViewmodelDObjToScene), &R_AddViewmodelDObjToScene, (LPVOID*)&ViewModelDetour);
			MH_EnableHook((LPVOID*)(game_data->game_offsets.R_AddViewmodelDObjToScene));
			game_data->settings.HUDchamHooked = true;
		}
	}
	else
	{
		if (game_data->settings.HUDchamHooked)
		{
			MH_DisableHook((LPVOID*)(game_data->game_offsets.R_AddViewmodelDObjToScene));
			MH_RemoveHook((LPVOID*)(game_data->game_offsets.R_AddViewmodelDObjToScene));
			game_data->settings.HUDchamHooked = false;
		}
	}
}

void updateClientVelocity(int i)
{
	VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_CalcTargetVelocity, 0, centity_t::GetEntity(i), &clients[i].value);

	float scale = game_data->settings.prediction_value / 100.0f; // Convert 0-100 to 0.0-1.0
	clients[i].value.x *= scale;
	clients[i].value.y *= scale;
	clients[i].value.z *= scale;
}

float toRadians(float deg)
{
	return 0.01745329238474369f * deg;
}

void angleVectors(Vector3 angles, Vector3* forward, Vector3* right, Vector3* up)
{
	float angle;
	static float sr, sp, sy, cr, cp, cy;

	angle = toRadians(angles.y);
	sy = sinf(angle);
	cy = cosf(angle);

	angle = toRadians(angles.x);
	sp = sinf(angle);
	cp = cosf(angle);

	angle = toRadians(angles.z);
	sr = sinf(angle);
	cr = cosf(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1.0f * sr * sp * cy + -1.0f * cr * -sy);
		right->y = (-1.0f * sr * sp * sy + -1.0f * cr * cy);
		right->z = -1.0f * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

Vector3 ImVec3Normalize(const Vector3& vec) {
	float magnitude = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	Vector3 normalized_vec;
	if (magnitude > 0.0f) {
		normalized_vec.x = vec.x / magnitude;
		normalized_vec.y = vec.y / magnitude;
		normalized_vec.z = vec.z / magnitude;
	}
	else {
		normalized_vec.x = normalized_vec.y = normalized_vec.z = 0.0f;
	}
	return normalized_vec;
}

Vector3 predict_Position(const Vector3& targetPos, const Vector3& targetVelocity, float travelTime, float gravity) {
	Vector3 predictedPosition = targetPos + (targetVelocity * travelTime);
	predictedPosition.y -= 0.5 * gravity * travelTime * travelTime;

	return predictedPosition;
}

centity_t* centity_t::GetEntity(int idx)
{
	return VariadicCall<centity_t*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntity, 0, &idx);
}

void AimTargetMP_GetTargetBounds(centity_t* targetEnt, Bounds* box)
{
	VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.AimTargetMP_GetTargetBounds, 0, targetEnt, box);
}

ImVec3 getAngles(int i)
{
	return *(ImVec3*)(centity_t::GetEntity(i) + game_data->game_offsets.angle_offset);
}

BallisticInfo* BG_GetBallisticInfo(WeaponForBullets* weapon)
{
	__int64 cgHnalder = VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.getCGHandler, 0);
	return VariadicCall<BallisticInfo*>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_GetBallisticInfo, cgHnalder, weapon, false);
}

WeaponForBullets* centity_t::CG_GetEntWeapon2()
{
	//return GameCall<WeaponForBullets*>(game_data->game_offsets.CG_GetEntWeapon)(0, reinterpret_cast<uintptr_t>(this));
	return VariadicCall<WeaponForBullets*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, reinterpret_cast<uintptr_t>(this));
}

//const char* UI_SafeTranslateString(const char* reference)
//{
//	//return GameCall<const char*>(game_data->game_offsets.UI_SafeTranslateString)(reference);
//	return VariadicCall<const char*>(game_data->game_offsets.trampoline, game_data->game_offsets.UI_SafeTranslateString, reference);
//}

//std::string_view CG_GetWeaponDisplayName(Weapon* weapon)
//{
//	char outputBuffer[512];
//	VariadicCall<char*>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_GetWeaponDisplayName, weapon, false, outputBuffer, sizeof(outputBuffer), false);
//	return std::string_view(outputBuffer);
//}

//std::string_view GetWeaponName(int Entity_IDX)
//{
//	//Weapon* weapon = GameCall<Weapon*>(game_data->game_offsets.CG_GetEntWeapon)(0, centity_t::GetEntity(Entity_IDX));
//	Weapon* weapon = VariadicCall<Weapon*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, centity_t::GetEntity(Entity_IDX));
//	if (weapon == NULL)
//		return std::string_view();
//
//	return UI_SafeTranslateString(CG_GetWeaponDisplayName(weapon).data());
//}

Vector3 calculate_prediction(int target, Vector3 p_shootingPos, Vector3 p_targetPos) {

	__int64 cgHnalder = VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.getCGHandler, 0);

	if (cgHnalder == 0ui64)
		return p_targetPos;

	Weapon* weapon = VariadicCall<Weapon*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, centity_t::GetEntity(client::localPlayer->clientNum));
	if (weapon == 0ui64)
		return p_targetPos;

	bool isAlternate = false;

	BallisticInfo* ballistic = VariadicCall<BallisticInfo*>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_GetBallisticInfo, cgHnalder, weapon, isAlternate);
	if (ballistic == 0ui64 || ballistic->calculated == 0ui64 || ballistic->enableBallisticTrajectory == 0ui64)
		return p_targetPos;

	float BallisticMuzzleVelocityScale = VariadicCall<float>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_GetBallisticMuzzleVelocityScale, weapon, isAlternate, 0);
	if (BallisticMuzzleVelocityScale <= 0.0f)
		return p_targetPos;

	updateClientVelocity(target);

	Vector3 dir = Vector3(0, 0, 0);
	angleVectors(client::localPlayer->viewAngles, &dir, 0i64, 0i64);
	dir = ImVec3Normalize(dir);

	float travelTime = VariadicCall<float>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_Ballistics_HorizontalTravelTimeToTarget, cgHnalder, weapon, isAlternate, 0, &p_shootingPos, &dir, &p_targetPos);
	if ((travelTime > 0.0) && (travelTime <= (float)((float)ballistic->lifeTimeMs * 0.001)))
	{
		p_targetPos.z += ((ballistic->gravityFactor * 193.f) * travelTime) * travelTime;

		Vector3 outPos = predict_Position(p_targetPos, clients[target].value, travelTime, ballistic->gravityFactor);
		return outPos;
	}
	return p_targetPos;
}

Vector3 getTargetPositionOverride(int boneIndex, const player* bestTarget) {
	switch (boneIndex) {
	case 0: return bestTarget->bone_array.j_ankle_lepos;
	case 1: return bestTarget->bone_array.j_ankle_ripos;
	case 2: return bestTarget->bone_array.j_knee_lepos;
	case 3: return bestTarget->bone_array.j_knee_ripos;
	case 4: return bestTarget->bone_array.j_helmetpos;
	case 5: return bestTarget->bone_array.j_headpos;
	case 6: return bestTarget->bone_array.j_neckpos;
	case 7: return bestTarget->bone_array.j_spineupperpos;
	case 8: return bestTarget->bone_array.j_spinelowerpos;
	default: return Vector3(); // Return a default value if boneIndex is invalid
	}
}

Vector3 getTargetPositionNormal(int boneIndex, const player* bestTarget) {
	switch (boneIndex) {
	case 0: return bestTarget->bone_array.j_helmetpos;
	case 1: return bestTarget->bone_array.j_headpos;
	case 2: return bestTarget->bone_array.j_neckpos;
	case 3: return bestTarget->bone_array.j_spineupperpos;
	case 4: return bestTarget->bone_array.j_spinelowerpos;
	case 5: return bestTarget->bone_array.best_bone_pos;
	default: return Vector3(); // Return a default value if boneIndex is invalid
	}
}

//void BG_GetPlayerEyePosition(Vector3* outOrigin)
//{
//	__int64 cgHnalder = VariadicCall<__int64>(game_data->game_offsets.trampoline, game_data->game_offsets.getCGHandler, 0);
//
//	if (cgHnalder == 0ui64)
//		return;
//
//	if (!client::localPlayer)
//		return;
//
//	return VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.bg_geteyeposition, game_data->game_offsets.weaponmap, client::localPlayer, outOrigin, cgHnalder);
//}

//float AutoWall(centity_t* ent, Vector3 start, Vector3 end)
//{
//	BulletFireParams BP_Enter;
//	BulletTraceResults TR_Enter;
//	auto cg = client::CgArray;
//	auto localent = client::localPlayer;
//
//	Weapon* weapon = VariadicCall<Weapon*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_GetEntWeapon, 0, centity_t::GetEntity(client::localPlayer->clientNum));
//	if (weapon == 0ui64)
//		return 0.0f;
//
//	// E8 ? ? ? ? 48 8B 0B 48 8B 91 ? ? ? ? 48 8B CB
//	WeapType weaponType = VariadicCall<WeapType>(game_data->game_offsets.trampoline, game_data->game_offsets.BG_GetWeaponType, weapon, false);
//
//	if (weaponType != WeapType::WEAPTYPE_BULLET)
//		return 0.0f;
//
//	ZeroMemory(&BP_Enter, sizeof(BulletFireParams));
//	ZeroMemory(&TR_Enter, sizeof(BulletTraceResults));
//
//	BP_Enter.dir = (end - start).Normalize();
//
//
//	BP_Enter.ignoreHitEntCount = 1;
//	BP_Enter.weaponEntIndex = 2046;
//	BP_Enter.damageMultiplier = 1.0f;
//	BP_Enter.methodOfDeath = 1;
//	BP_Enter.ignoreHitEntityQueue[0] = client::localPlayer->clientNum;
//	//BP_Enter.ignoreHitEntityQueue[1] = ent->nextstate().number;
//
//	BP_Enter.initialPos = start;
//	BP_Enter.start = start;
//	BP_Enter.end = end;
//
//	bool bEnterHit = CG_Weapons_BulletTrace(0, &BP_Enter, BP_Enter.ignoreHitEntityQueue, BP_Enter.ignoreHitEntCount, localent->GetWeapon(), localent->nextstate().inAltWeaponMode, localent, &TR_Enter, 0, nullptr, false);
//	if (bEnterHit)
//	{
//		int iSurfaceCount = 0;
//		float flEnterDepth = 0.0f, flExitDepth = 0.0f, flSurfaceDepth = 0.0f;
//
//		game_struct::BulletFireParams BP_Exit;
//		game_struct::BulletTraceResults TR_Exit;
//
//		Vector3 vHitPos = { 0.0f };
//
//		while (true)
//		{
//			flEnterDepth = GameFunction::BG::BG_GetSurfacePenetrationDepth(localent->GetWeapon(), localent->nextstate().inAltWeaponMode, TR_Enter.depthSurfaceType);
//			if (g_menu::renderlogs)
//				//////IMGUI_Log.AddLog("flEnterDepth: %f\n", flEnterDepth);
//			/*	if (HasPerk(0x1cu))
//					flEnterDepth *= FindVariable("perk_bulletPenetrationMultiplier")->Current.flValue;*/
//
//				if (flEnterDepth <= 0.0f)
//					return 0.0f;
//
//			vHitPos = TR_Enter.hitPos;
//
//			if (!GameFunction::BG::BG_AdvanceTrace(&BP_Enter, &TR_Enter.trace, &TR_Enter.hitPos, 0.13500001f))
//				return 0.0f;
//
//			bEnterHit = GameFunction::CG::CG_Weapons_BulletTrace(0, &BP_Enter, BP_Enter.ignoreHitEntityQueue, BP_Enter.ignoreHitEntCount, localent->GetWeapon(), localent->nextstate().inAltWeaponMode, localent, &TR_Enter, TR_Enter.depthSurfaceType, nullptr, false);
//
//			CopyMemory(&BP_Exit, &BP_Enter, sizeof(game_struct::BulletFireParams));
//			VectorScale(BP_Enter.dir, -1.0f, BP_Exit.dir);
//
//			BP_Exit.start = BP_Exit.end;
//			VectorMA(vHitPos, 0.0099999998f, BP_Exit.dir, BP_Exit.end);
//
//			CopyMemory(&TR_Exit, &TR_Enter, sizeof(game_struct::BulletTraceResults));
//			VectorScale(TR_Exit.trace.normal, -1.0f, TR_Exit.trace.normal);
//
//			if (bEnterHit)
//				GameFunction::BG::BG_AdvanceTrace(&BP_Exit, &TR_Exit.trace, &TR_Exit.hitPos, 0.0099999998f);
//
//
//			bool bExitHit = GameFunction::CG::CG_Weapons_BulletTrace(0, &BP_Exit, BP_Exit.ignoreHitEntityQueue, BP_Exit.ignoreHitEntCount, localent->GetWeapon(), localent->nextstate().inAltWeaponMode, localent, &TR_Exit, TR_Exit.depthSurfaceType, nullptr, false);
//
//			bool bSolid = (bExitHit && TR_Exit.trace.allsolid) || (TR_Enter.trace.startsolid && TR_Exit.trace.startsolid);
//
//			if (bExitHit || bSolid)
//			{
//				if (bSolid)
//					flSurfaceDepth = BP_Exit.end.distance_to(BP_Exit.start);
//
//				else
//					flSurfaceDepth = vHitPos.distance_to(TR_Exit.hitPos);
//
//
//				flSurfaceDepth = std::max(flSurfaceDepth, 1.0f);
//
//				if (g_menu::renderlogs)
//					//////IMGUI_Log.AddLog("flSurfaceDepth: %f\n", flSurfaceDepth);
//
//					if (bExitHit)
//					{
//						flExitDepth = GameFunction::BG::BG_GetSurfacePenetrationDepth(localent->GetWeapon(), localent->nextstate().inAltWeaponMode, TR_Exit.depthSurfaceType);
//						if (g_menu::renderlogs)
//							////IMGUI_Log.AddLog("flExitDepth: %f\n", flExitDepth);
//
//						/*	if (HasPerk(0x1cu))
//					flEnterDepth *= FindVariable("perk_bulletPenetrationMultiplier")->Current.flValue;*/
//
//							flEnterDepth = std::min(flEnterDepth, flExitDepth);
//
//						if (g_menu::renderlogs)
//							////IMGUI_Log.AddLog("flEnterDepth2: %f\n", flEnterDepth);
//
//							if (flEnterDepth <= 0.0f)
//								return 0.0f;
//					}
//
//				BP_Enter.damageMultiplier -= flSurfaceDepth / flEnterDepth;
//
//				if (BP_Enter.damageMultiplier <= 0.0f)
//					return 0.0f;
//
//
//			}
//			else if (!bEnterHit)
//				return BP_Enter.damageMultiplier;
//
//			// only can penetrate max 5 walls
//			if (bEnterHit)
//			{
//				if (++iSurfaceCount < 5)
//					continue;
//			}
//
//			return 0.0f;
//
//		}
//	}
//
//	if (g_menu::renderlogs)
//		////IMGUI_Log.AddLog("P_Enter.damageMultiplier: %f\n", BP_Enter.damageMultiplier);
//
//		return BP_Enter.damageMultiplier;
//}

//uintptr_t Dvar_FindMalleableVar(__int64 hash)
//{
//	return VariadicCall<uintptr_t>(game_data->game_offsets.trampoline, game_data->game_offsets.Dvar_FindMalleableVar, hash);
//}

void setAimAngles() {
	//if (game_data->settings.aim_type != 1)
	//{
		Vector3 targetPos;
		bool useOverrideBones = is_overid_down(game_data->settings.aim_Override_key) && game_data->settings.A_bone >= 0 && game_data->settings.A_bone < 9;
		bool useNormalBones = is_key_down(game_data->settings.aim_key) && game_data->settings.i_bone >= 0 && game_data->settings.i_bone < 6;

		if (!useOverrideBones && !useNormalBones) {
			return; // Early return if no valid aim key is pressed
		}

		if (useOverrideBones) {
			targetPos = getTargetPositionOverride(game_data->settings.A_bone, bestTarget);
		}
		else {
			targetPos = getTargetPositionNormal(game_data->settings.i_bone, bestTarget);
		}

		Vector3 startPos;

		VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_Entity_GetPlayerViewOrigin, client::localPlayer, &startPos);

		if (game_data->settings.Prediction) {
			targetPos = calculate_prediction(bestTarget->entnum, startPos, targetPos);
		}

		aimPos = targetPos - startPos;
		VectorToAngles(aimPos, SilentAimAngle);

		if (game_data->settings.aim_type != 1)
		{
			if (client::CgArray != 0 && client::localPlayer != nullptr) {
				if (game_data->settings.aim_smooth > 1.0f) {
					auto viewAngles = client::localPlayer->viewAngles;
					normalize_angle(viewAngles);

					auto angleDelta = SilentAimAngle - viewAngles;
					normalize_angle(angleDelta);

					SilentAimAngle = viewAngles + angleDelta / game_data->settings.aim_smooth;
					normalize_angle(SilentAimAngle);
				}

				VariadicCall<void>(game_data->game_offsets.trampoline, game_data->game_offsets.A_CG_VehicleCam_SetClientViewAngles, client::CgArray, &SilentAimAngle);
			}
		}
	//}
}

namespace sdk
{
	refdef_t* refdef = 0;

	refdef_t* get_refdef()
	{
		uint32_t crypt_0 = *(uint32_t*)(game_data->game_offsets.refdef_ptr);
		uint32_t crypt_1 = *(uint32_t*)(game_data->game_offsets.refdef_ptr + 0x4);
		uint32_t crypt_2 = *(uint32_t*)(game_data->game_offsets.refdef_ptr + 0x8);
		uint32_t entry_1 = (uint32_t)(game_data->game_offsets.refdef_ptr);
		uint32_t entry_2 = (uint32_t)(game_data->game_offsets.refdef_ptr + 0x4);
		uint32_t _low = entry_1 ^ crypt_2;
		uint32_t _high = entry_2 ^ crypt_2;
		uint32_t low_bit = crypt_0 ^ _low * (_low + 2);
		uint32_t high_bit = crypt_1 ^ _high * (_high + 2);
		auto unencrypted_refdef = ((QWORD)high_bit << 32) + low_bit;
		return (refdef_t*)(unencrypted_refdef);
	}
}

void Cbuf_AddText(const char* fmt, ...)
{
	char cmd[512]{};
	char post[512]{};

	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(cmd, fmt, ap);
	va_end(ap);

	memcpy(post, (BYTE*)game_data->game_offsets.xpartyOffset, 512);
	strcpy((char*)game_data->game_offsets.xpartyOffset, cmd);
	reinterpret_cast<void(*)(void*)>(game_data->game_offsets.CbufOffset)(nullptr);
	memcpy((BYTE*)game_data->game_offsets.xpartyOffset, post, 512);
}

bool BoneScanVisCheck(int entNum)
{
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_helmetpos, client::localPlayer->clientNum, entNum))
		return true;
	//if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_neckpos, client::localPlayer->clientNum, entNum))
	//    return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_ankle_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_ankle_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_ball_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_ball_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_clavicle_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_clavicle_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_elbow_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_elbow_ripos, client::localPlayer->clientNum, entNum))
		return true;
	//if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_eyeball_lepos, client::localPlayer->clientNum, entNum))
	//    return true;
	//if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_eyeball_ripos, client::localPlayer->clientNum, entNum))
	//    return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_headpos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_hip_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_hip_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_knee_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_knee_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_mainrootpos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_shoulder_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_shoulder_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_wrist_lepos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_wrist_ripos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_spinelowerpos, client::localPlayer->clientNum, entNum))
		return true;
	if (G_Main_SightTrace(&myCamPosition, &players[entNum].bone_array.j_spineupperpos, client::localPlayer->clientNum, entNum))
		return true;

	return false;
}

void AimBoneScan(player* pl)
{
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_headpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_headpos; //return;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_helmetpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_helmetpos; //return;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_neckpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_neckpos; //return;
		return;
	}
	/*if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_eyeball_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_eyeball_lepos; return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_eyeball_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_eyeball_ripos; return;
	}*/
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_spineupperpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_spineupperpos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_spinelowerpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_spinelowerpos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_mainrootpos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_spinelowerpos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_shoulder_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_shoulder_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_shoulder_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_shoulder_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_hip_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_hip_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_hip_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_hip_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_clavicle_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_clavicle_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_clavicle_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_clavicle_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_elbow_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_elbow_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_elbow_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_elbow_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_wrist_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_wrist_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_wrist_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_wrist_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_knee_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_knee_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_knee_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_knee_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_ball_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_ball_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_ball_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_ball_ripos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_ankle_lepos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_ankle_lepos;
		return;
	}
	if (G_Main_SightTrace(&myCamPosition, &pl->bone_array.j_ankle_ripos, client::localPlayer->clientNum, pl->entnum))
	{
		pl->bone_array.best_bone_pos = pl->bone_array.j_ankle_ripos;
		return;
	}
	pl->bone_array.best_bone_pos = pl->bone_array.j_neckpos;
}

Vector3 GetCameraPosition()
{
	auto camera = *(uintptr_t*)(game_data->game_offsets.cameraBaseOffset);
	if (!camera)
		return Vector3(0, 0, 0);
	return *(Vector3*)(camera + 0x204);
}

bool WorldToScreen(const Vector3& WorldPos, Vector2* ScreenPos)
{
	DWORD64 scrPlace = game_data->game_offsets.other;
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.cam, 0, scrPlace, WorldPos, ScreenPos);
}

bool WorldToScreen_2(const Vector3& WorldPos, Vector2* ScreenPos)
{
	auto refdefs = sdk::refdef;
	Vector3 myLocation = myCamPosition;
	Vector3 vLocal = WorldPos - myLocation;
	Vector3 vTrans = Vector3(vLocal.Dot(refdefs->view.axis[1]), vLocal.Dot(refdefs->view.axis[2]), vLocal.Dot(refdefs->view.axis[0]));

	if (vTrans.z < 0.01f)
		return false;

	ScreenPos->x = ((refdefs->Width / 2) * (1 - (vTrans.x / refdefs->view.tanHalfFov.x / vTrans.z)));
	ScreenPos->y = ((refdefs->Height / 2) * (1 - (vTrans.y / refdefs->view.tanHalfFov.y / vTrans.z)));

	return true;
}

float dot_product(Vector3 vec)
{
	return (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float vector_length(Vector3 vec)
{
	return std::sqrtf(dot_product(vec));
}

float vector_normalize(Vector3* direction)
{
	auto length = vector_length(*direction);
	auto result = length;

	if (-length >= 0.0f)
		length = 1.0f;

	direction->x *= 1.0f / length;
	direction->y *= 1.0f / length;
	direction->z *= 1.0f / length;

	return result;
}

void vec_to_angles(Vector3 direction, Vector3* angles)
{
	std::float_t temp, yaw, pitch;

	if (direction.x == 0.0f && direction.y == 0.0f)
	{
		yaw = 0.0f;

		if (direction.z > 0.0f)
			pitch = 90.0f;

		else
			pitch = 270.0f;
	}

	else
	{
		yaw = radians_to_degrees(std::atan2f(direction.y, direction.x));

		if (yaw < 0.0f)
			yaw += 360.0f;

		temp = std::sqrtf(direction.x * direction.x + direction.y * direction.y);
		pitch = radians_to_degrees(std::atan2f(direction.z, temp));

		if (pitch < 0.0f)
			pitch += 360.0f;
	}

	angles->x = -pitch;
	angles->y = yaw;
	angles->z = 0.0f;
}

void world_to_compass(Vector3 startPos, Vector3 world, Vector2 compasspos, float compasssize, Vector2* screen, int index)
{
	auto dir = startPos - world;

	Vector3 angles;

	vector_normalize(&dir);
	vec_to_angles(dir, &angles);

	angles = client::localPlayer->viewAngles - angles;
	angles = clamp_angles(angles);

	float angle = ((angles.y + 180.0f) / 360.0f - 0.25f) * pi_double;
	float angle2 = ((angles.y) / 360.0f - 0.25f) * pi_double;
	arrowAngle[index] = angle2;
	screen->x = compasspos.x + (std::cosf(angle) * (compasssize / 2.0f));
	screen->y = compasspos.y + (std::sinf(angle) * (compasssize / 2.0f));
}

void WorldToRadar(Vector3 world, ImVec2 radarpos, float scale, float radarsize, float blipsize, ImVec2& screen)
{
	float flCosYaw = cosf(DegreesToRadians(client::localPlayer->viewAngles.y)),
		flSinYaw = sinf(DegreesToRadians(client::localPlayer->viewAngles.y)),
		flDeltaX = world.x - myCamPosition.x,
		flDeltaY = world.y - myCamPosition.y,
		flLocationX = (flDeltaY * flCosYaw - flDeltaX * flSinYaw) / scale,
		flLocationY = (flDeltaX * flCosYaw + flDeltaY * flSinYaw) / scale;

	if (flLocationX < -(radarsize / 2.0f - blipsize / 2.0f - 1.0f))
		flLocationX = -(radarsize / 2.0f - blipsize / 2.0f - 1.0f);

	else if (flLocationX > (radarsize / 2.0f - blipsize / 2.0f - 1.0f))
		flLocationX = (radarsize / 2.0f - blipsize / 2.0f - 1.0f);

	if (flLocationY < -(radarsize / 2.0f - blipsize / 2.0f - 1.0f))
		flLocationY = -(radarsize / 2.0f - blipsize / 2.0f - 1.0f);

	else if (flLocationY > (radarsize / 2.0f - blipsize / 2.0f))
		flLocationY = (radarsize / 2.0f - blipsize / 2.0f);

	screen[0] = -flLocationX + radarpos[0];
	screen[1] = -flLocationY + radarpos[1];
}

score_t* CG_ScoreboardMP_GetClientScore(int clientNum)
{
	return VariadicCall<score_t*>(game_data->game_offsets.trampoline, game_data->game_offsets.CG_ScoreboardMP_GetClientScore, 0, clientNum);
}

float measure(int x1, int y1, int x2, int y2)
{
	// Helping Measure neck to helm
	return (sqrt(pow(x2 - x1, 2) +
		pow(y2 - y1, 2) * 1.0));
}

bool LUI_GetEntityTagPositionAndAxis(int entityNum, __int64 taghash, Vector3* outWorldPosition)
{
	Vector3 m[4];
	return VariadicCall<bool>(game_data->game_offsets.trampoline, game_data->game_offsets.LUI_GetEntityTagPositionAndAxis, 0, entityNum, taghash, m, outWorldPosition);
}

//Vector3 CG_GetPoseOrigin(int index)
//{
//	Vector3 position{ 0.0f, 0.0f, 0.0f };
//	float matrix[64]{};
//
//	VariadicCall<uintptr_t>(
//		game_data->game_offsets.trampoline,
//		game_data->game_offsets.CG_GetPoseOrigin,
//		0,
//		index,
//		matrix,
//		&position
//	);
//
//	return position;
//}

inline unsigned __int64 stringToHash64(const char* str)
{
	if (!str || *str == '\0')
		return 0;

	if (str[0] == '#' && str[1] == 'x')
	{
		if (str[2] == '0')
		{
			return _strtoui64(str + 3, nullptr, 16);
		}
		else
		{
			return 0;
		}
	}

	unsigned __int64 hash = 0xCBF29CE484222325uLL;
	while (*str)
	{
		int character = *str++;
		if (character >= 'A' && character <= 'Z')
			character += 32;
		hash = (hash ^ character) * 0x100000001B3LL;
	}
	return hash;
}

bones update_bones(int i)
{
	bones bone_arr;

	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("tag_origin")), &bone_arr.originpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_ball_le")), &bone_arr.j_ball_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_ball_ri")), &bone_arr.j_ball_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_ankle_le")), &bone_arr.j_ankle_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_ankle_ri")), &bone_arr.j_ankle_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_knee_le")), &bone_arr.j_knee_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_knee_ri")), &bone_arr.j_knee_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_hip_le")), &bone_arr.j_hip_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_hip_ri")), &bone_arr.j_hip_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_mainroot")), &bone_arr.j_mainrootpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_wrist_le")), &bone_arr.j_wrist_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_wrist_ri")), &bone_arr.j_wrist_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_elbow_le")), &bone_arr.j_elbow_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_elbow_ri")), &bone_arr.j_elbow_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_spinelower")), &bone_arr.j_spinelowerpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_spineupper")), &bone_arr.j_spineupperpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_shoulder_le")), &bone_arr.j_shoulder_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_shoulder_ri")), &bone_arr.j_shoulder_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_clavicle_le")), &bone_arr.j_clavicle_lepos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_clavicle_ri")), &bone_arr.j_clavicle_ripos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_neck")), &bone_arr.j_neckpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_head")), &bone_arr.j_headpos);
	LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("j_helmet")), &bone_arr.j_helmetpos);

	WorldToScreen(bone_arr.originpos, &bone_arr.origin);
	WorldToScreen(bone_arr.j_ball_lepos, &bone_arr.j_ball_lepos_screen);
	WorldToScreen(bone_arr.j_ball_ripos, &bone_arr.j_ball_ripos_screen);
	WorldToScreen(bone_arr.j_ankle_lepos, &bone_arr.j_ankle_lepos_screen);
	WorldToScreen(bone_arr.j_ankle_ripos, &bone_arr.j_ankle_ripos_screen);
	WorldToScreen(bone_arr.j_knee_lepos, &bone_arr.j_knee_lepos_screen);
	WorldToScreen(bone_arr.j_knee_ripos, &bone_arr.j_knee_ripos_screen);
	WorldToScreen(bone_arr.j_hip_lepos, &bone_arr.j_hip_lepos_screen);
	WorldToScreen(bone_arr.j_hip_ripos, &bone_arr.j_hip_ripos_screen);
	WorldToScreen(bone_arr.j_wrist_lepos, &bone_arr.j_wrist_lepos_screen);
	WorldToScreen(bone_arr.j_wrist_ripos, &bone_arr.j_wrist_ripos_screen);
	WorldToScreen(bone_arr.j_elbow_lepos, &bone_arr.j_elbow_lepos_screen);
	WorldToScreen(bone_arr.j_elbow_ripos, &bone_arr.j_elbow_ripos_screen);
	WorldToScreen(bone_arr.j_spinelowerpos, &bone_arr.j_spinelowerpos_screen);
	WorldToScreen(bone_arr.j_spineupperpos, &bone_arr.j_spineupperpos_screen);
	WorldToScreen(bone_arr.j_shoulder_lepos, &bone_arr.j_shoulder_lepos_screen);
	WorldToScreen(bone_arr.j_shoulder_ripos, &bone_arr.j_shoulder_ripos_screen);
	WorldToScreen(bone_arr.j_clavicle_lepos, &bone_arr.j_clavicle_lepos_screen);
	WorldToScreen(bone_arr.j_clavicle_ripos, &bone_arr.j_clavicle_ripos_screen);
	WorldToScreen(bone_arr.j_neckpos, &bone_arr.j_neckpos_screen);
	WorldToScreen(bone_arr.j_headpos, &bone_arr.j_headpos_screen);
	WorldToScreen(bone_arr.j_helmetpos, &bone_arr.j_helmetpos_screen);
	WorldToScreen(bone_arr.j_mainrootpos, &bone_arr.j_mainrootpos_screen);

	return bone_arr;
}

float getDistance(const Vector3& mine, const Vector3& other)
{
	Vector3 delta;
	delta.x = mine.x - other.x;
	delta.y = mine.y - other.y;
	delta.z = mine.z - other.z;
	return delta.length() * 0.0254;
}

//uintptr_t GetNameList()
//{
//	auto namePtr = *(uintptr_t*)(game_data->game_offsets.nameArray);
//	return namePtr + 0x1838;
//}
//
//NameEntry GetNameEntry(uint32_t index)
//{
//	return *(NameEntry*)(GetNameList() + (index * 0xC8)); //0xD8
//}

NameEntry* Get_ClientInfo(int index)
{
	auto vtable = (uintptr_t*)game_data->game_offsets.GetClientInfo_vtbl[0];

	NameEntry* pClientInfo = (NameEntry*)(*(__int64(__fastcall**)(uintptr_t*, __int64))(*(__int64*)(__int64)vtable + 0x128))(vtable, index);

	return pClientInfo;
}

__int64 GetBRPlayerFlags(int i) {
	return GameCall<__int64>(game_data->game_offsets.A_GetBRPlayerFlags)((DWORD64)(Get_ClientInfo(i)));
}

float DistanceCursor(Vector2 vec)
{
	if (vec.x == 0.f || vec.y == 0.f)
	{
		return 9999.f;
	}
	float ydist = (vec.y - ImGui::GetIO().DisplaySize.y / 2.f);
	float xdist = (vec.x - ImGui::GetIO().DisplaySize.x / 2.f);
	float ret = sqrt(pow(ydist, 2) + pow(xdist, 2));
	return ret;
}

float calculate_real_fov(const Vector3& local_angles, const Vector3& local_pos, const Vector3& target_pos)
{
	const auto distance = (target_pos - local_pos).length();
	const auto angle_difference = (local_angles - CalcAngles(local_pos, target_pos));

	auto pitch_difference = sin(deg2rad(angle_difference.x)) * distance;
	auto yaw_difference = sin(deg2rad(angle_difference.y)) * distance;

	return sqrt(pitch_difference * pitch_difference + yaw_difference * yaw_difference);
}

bool hook_in_game()
{
	return *reinterpret_cast<const char*>(game_data->game_offsets.Com_FrontEnd_IsInFrontEnd) == 0;
}

void UpdateTracers()
{
	for (auto& tracer : tracers)
	{
		WorldToScreen(tracer.start3D, &tracer.start2D);
		WorldToScreen(tracer.hit3D, &tracer.hit2D);
	}
}

bool think()
{
	if (!hook_in_game())
	{
		game_data->settings.checkPassed = false;
		return 0;
	}

	//log_shit("1");

	if (is_bad_ptr(sdk::get_refdef()))
	{
		game_data->settings.checkPassed = false;
		return 0;
	}

	//log_shit("2");

	if (sdk::refdef == 0)
	{
		sdk::refdef = sdk::get_refdef();
		return 0;
	}

	//log_shit("3");

	client::CgArray = decrypt_cg();

	//log_shit("4");

	if ((uintptr_t)client::CgArray == 0)
	{
		game_data->settings.checkPassed = false;
		return 0;
	}

	//log_shit("5");

	client::localPlayer = GetPlayerState(client::CgArray);

	//log_shit("6");

	if (!client::localPlayer)
	{
		game_data->settings.checkPassed = false;
		return 0;
	}

	//log_shit("7");

	myCamPosition = GetCameraPosition();

	//log_shit("8");

	if (myCamPosition.x == 0.0f && myCamPosition.y == 0.0f && myCamPosition.z == 0.0f)
	{
		game_data->settings.checkPassed = false;
		return 0;
	}

	//log_shit("9");

	if (game_data->settings.fov_int == 0)
	{
		client::radius = (tanf(deg2rad(game_data->settings.aimbotFov - 1) / 2.f) / tanf(deg2rad(scale_fov(GetLocalPlayerFOV())) / 2.f) * (ImGui::GetIO().DisplaySize.x));
	}
	else
	{
		client::radius = game_data->settings.aimbotFov * 10;
	}

	//log_shit("10");

	if (!is_key_down(game_data->settings.aim_key))
	{
		isTargetLocket = false;
	}
	if (!isTargetLocket)
	{
		Targets.clear();
		bestTarget = nullptr;
	}

	//log_shit("11");

	for (size_t i = 0; i <= 2046; i++)
	{
		player* p = &players[i];

		auto ent = centity_t::GetEntity(i);

		//log_shit("12");

		if (!ent)
		{
			p->is_valid = false;
			continue;
		}

		//log_shit("13");

		if (!is_alive(ent))
		{
			p->is_valid = false;
			continue;
		}

		//log_shit("14");

		auto state = nextstate(ent);

		//log_shit("15");

		if (state.eType != entityType_s::ET_PLAYER &&
			!(state.eType == entityType_s::ET_AGENT &&
				(game_data->settings.showaibool || game_data->settings.targetaibool))) {
			p->is_valid = false;
			continue;
		}

		//log_shit("16");

		if (!LUI_GetEntityTagPositionAndAxis(i, stringToHash64(XOR("tag_origin")), &p->playerPos))
		{
			p->is_valid = false;
			continue;
		}

		//log_shit("17");

		auto distance = (p->playerPos - myCamPosition).lengthDistance() * 0.0254f;
		if (distance > game_data->settings.distanceFloat)
		{
			p->is_valid = false;
			continue;
		}

		//log_shit("18");

		p->is_AI = (state.eType == entityType_s::ET_AGENT);

		p->distance = distance;

		p->entnum = i;

		//log_shit("19");

		p->bone_array = update_bones(i);

		//log_shit("20");

		p->bbox_is_valid = get_bbox(p->bbox, p);

		//log_shit("21");

		world_to_compass(myCamPosition, p->playerPos, compass_pos, game_data->settings.CompassScale, &arrow_pos[i], i);

		WorldToRadar(p->playerPos, vRadarPosition, game_data->settings.flScale, flRadarSize, flBlipSize, vBlipPosition[i]);

		if (game_data->settings.b_player_3dboxes || game_data->settings.b_player_filled_3dboxes)
		{
			AimTargetMP_GetTargetBounds(ent, &p->Player_Bounds);
		}

		//log_shit("22");

		//head shit
		Vector3 real_head_pos = p->bone_array.j_headpos;
		real_head_pos.z += 2.f;
		WorldToScreen(real_head_pos, &p->bone_array.real_head_pos_screen);

		//log_shit("23");

		if (p->bone_array.origin.x != 0.0f && p->bone_array.origin.y != 0.0f && p->bone_array.j_headpos.x != 0.0f && p->bone_array.j_headpos.y != 0.0f && p->bone_array.j_spineupperpos.x != 0.0f && p->bone_array.j_spineupperpos.y != 0.0f)
		{
			p->is_valid = true;
		}
		else
		{
			p->is_valid = false;
		}

		if (state.eType == entityType_s::ET_PLAYER)
		{
			//log_shit("24");

			auto clientinfo = Get_ClientInfo(i);

			//log_shit("25");

			if (!clientinfo)
				continue;


			//log_shit("26");
			p->player_name = *clientinfo;

			//log_shit("27");

			if (CG_ScoreboardMP_GetClientScore(i))
			{
				//log_shit("28");
				p->team = CG_ScoreboardMP_GetClientScore(i)->team;

				//log_shit("29");

				p->kills = CG_ScoreboardMP_GetClientScore(i)->kills;
				p->deaths = CG_ScoreboardMP_GetClientScore(i)->deaths;
				p->ping = CG_ScoreboardMP_GetClientScore(i)->ping;
				p->rank_mp = CG_ScoreboardMP_GetClientScore(i)->rank_mp;
				//Party_GetXuid(i, &p->xuid);
				//Party_GetPlatformId(&p->PlatformId, i);
			}

			if (game_data->settings.weapon_esp)
			{
				p->weaponDisplayName = GetWeaponName(i);
			}
		}

		if (client::localPlayer->clientNum == i)
		{
			p->is_valid = false;
			continue;
		}

		Targets.push_back(p);

		if (game_data->settings.threat_warnings)
		{
			p->aim_visible = G_Main_SightTrace(&myCamPosition, &players[i].bone_array.j_neckpos, client::localPlayer->clientNum, i);
			p->fov = calculate_fov(p->playerPos, myCamPosition, getAngles(i));
		}

		Vector3* bonePositions[] = {
&players[i].bone_array.j_helmetpos,
&players[i].bone_array.j_headpos,
&players[i].bone_array.j_neckpos,
&players[i].bone_array.j_spineupperpos,
&players[i].bone_array.j_spinelowerpos
		};

		if (game_data->settings.i_bone >= 0 && game_data->settings.i_bone < 5)
		{
			//log_shit("30");
			p->visible = G_Main_SightTrace(&myCamPosition, bonePositions[game_data->settings.i_bone], client::localPlayer->clientNum, i);

			//log_shit("31");
		}
		else if (game_data->settings.i_bone == 5)
		{
			p->visible = BoneScanVisCheck(i);
			AimBoneScan(p);
		}
	}

	game_data->settings.checkPassed = true;

	UpdateTracers();

	if (game_data->settings.FastReload)
	{
		fast_reload();
	}

	//if (game_data->settings.NoFlinch)
	//	NoFlinch();

	if (game_data->settings.checkPassed)
	{
		if (client::CgArray)
		{
			if (client::localPlayer != nullptr)
			{
				if (game_data->settings.Norecoil)
				{
					auto& kickAngles = *(secured_vec3_t*)(client::CgArray + game_data->game_offsets.kickAngles);

					auto currentValue = kickAngles.get();

					auto recoilMultiplier = (game_data->settings.noRecoil / 100.0f);

					if (currentValue != currentValue * recoilMultiplier)
					{
						kickAngles.set(currentValue * recoilMultiplier);
					}
				}
			}
		}
	}

	if (game_data->settings.AIMBOT && game_data->settings.checkPassed)
	{
		if (!Targets.empty())
		{
			bestTarget = nullptr;

			// Cache local player position and view angles
			Vector3 localPos = client::localPlayer->origin;
			Vector3 localAngles = client::localPlayer->viewAngles;

			std::vector<std::pair<float, player*>> filteredTargets;

			for (auto target : Targets)
			{
				if (!target->is_valid)
					continue;

				// Apply filtering conditions early to reduce workload
				if (target->player_name.name[0] == '\n')
					continue;
				if (target->is_AI && !game_data->settings.targetaibool)
					continue;
				if (target->team == players[client::localPlayer->clientNum].team && game_data->settings.FREINDLYAIMBOTCHECK)
					continue;
				if (game_data->settings.AIMBOTCHECK && !target->visible)
					continue;
				if (game_data->settings.skipKnocked && (GetBRPlayerFlags(target->player_name.clientNum) & Downed))
					continue;
				if (game_data->settings.skipselfrevive &&
					(GetBRPlayerFlags(target->player_name.clientNum) & Downed) &&
					(GetBRPlayerFlags(target->player_name.clientNum) & SelfReviveToken))
					continue;

				float deltaValue = 0.f;

				if (game_data->settings.targetting_closestby == 0)
				{
					// Cache angles only once per target
					deltaValue = calculate_real_fov(localAngles, localPos, players[target->entnum].bone_array.originpos);
				}
				else if (game_data->settings.targetting_closestby == 1)
				{
					// Cache cursor distance instead of recalculating
					deltaValue = DistanceCursor(players[target->entnum].bone_array.j_headpos_screen);
				}
				else if (game_data->settings.targetting_closestby == 2)
				{
					deltaValue = players[target->entnum].distance;
				}

				// Ignore targets way out of FOV or cursor range
				if (deltaValue < 9999.f)
				{
					filteredTargets.emplace_back(deltaValue, target);
				}
			}

			// Sort only the filtered targets
			std::sort(filteredTargets.begin(), filteredTargets.end(), [](const auto& a, const auto& b) {
				return a.first < b.first;
				});

			for (const auto& [_, target] : filteredTargets)
			{
				if (bestTarget)
					break;

				if (DistanceCursor(players[target->entnum].bone_array.j_headpos_screen) < client::radius &&
					target->distance < game_data->settings.distanceFloat)
				{
					bestTarget = target;
				}
			}
		}

		// Validate the best target
		if (bestTarget && bestTarget->is_valid)
		{
			if (game_data->settings.AIMBOTCHECK && !bestTarget->visible)
				bestTarget = nullptr;
			else if (game_data->settings.skipKnocked && (GetBRPlayerFlags(bestTarget->player_name.clientNum) & Downed))
				bestTarget = nullptr;
			else if (game_data->settings.skipselfrevive &&
				(GetBRPlayerFlags(bestTarget->player_name.clientNum) & Downed) &&
				(GetBRPlayerFlags(bestTarget->player_name.clientNum) & SelfReviveToken))
				bestTarget = nullptr;
			else if (!game_data->settings.isSticky &&
				DistanceCursor(players[bestTarget->entnum].bone_array.j_headpos_screen) >= client::radius)
				bestTarget = nullptr;
		}

		// Apply aim angles if locked onto a valid target
		if (bestTarget && bestTarget->is_valid && is_key_down(game_data->settings.aim_key))
		{
			if (players[client::localPlayer->clientNum].player_name.health != 0)
			{
				setAimAngles();
				isTargetLocket = true;
			}
			else
			{
				isTargetLocket = false;
			}
		}
		else
		{
			isTargetLocket = false;
		}
	}

	if (game_data->settings.ShowSpectator)
	{
		OmnvarData isactive_spectate_count = getOmnvarData(XOR("ui_br_active_spectators"));
		game_data->settings.activespec = isactive_spectate_count.current.m_integer;
	}

	if (game_data->settings.removeweaponmovement)
	{
		Cbuf_AddText(XOR("#x3262B7887C73C5A1E 0;"));
	}
	else
	{
		Cbuf_AddText(XOR("#x3262B7887C73C5A1E 1;"));
	}

	if (game_data->settings.ffpAds)
	{
		Cbuf_AddText(XOR("#x3FEAA1FC363C63B90 1;"));
	}
	else
	{
		Cbuf_AddText(XOR("#x3FEAA1FC363C63B90 0;"));
	}

	if (game_data->settings.loot_test && game_data->settings.checkPassed)
	{
		getLoot();
	}

	return 0;
}

namespace gamepad
{
	float GPad_GetButton(GamePadButton button)
	{
		return VariadicCall<float>(game_data->game_offsets.trampoline, game_data->game_offsets.A_GPad_GetButton, 0, button);
	}

	int GPad_ResolveButton() {
		int key = -1;

		if (GPad_GetButton(GPAD_UP) > 0.0f)
			key = GPAD_UP;
		if (GPad_GetButton(GPAD_DOWN) > 0.0f)
			key = GPAD_DOWN;
		if (GPad_GetButton(GPAD_LEFT) > 0.0f)
			key = GPAD_LEFT;
		if (GPad_GetButton(GPAD_RIGHT) > 0.0f)
			key = GPAD_RIGHT;
		if (GPad_GetButton(GPAD_START) > 0.0f)
			key = GPAD_START;
		if (GPad_GetButton(GPAD_BACK) > 0.0f)
			key = GPAD_BACK;
		if (GPad_GetButton(GPAD_L3) > 0.0f)
			key = GPAD_L3;
		if (GPad_GetButton(GPAD_R3) > 0.0f)
			key = GPAD_R3;
		if (GPad_GetButton(GPAD_A) > 0.0f)
			key = GPAD_A;
		if (GPad_GetButton(GPAD_B) > 0.0f)
			key = GPAD_B;
		if (GPad_GetButton(GPAD_X) > 0.0f)
			key = GPAD_X;
		if (GPad_GetButton(GPAD_Y) > 0.0f)
			key = GPAD_Y;
		if (GPad_GetButton(GPAD_L_SHLDR) > 0.0f)
			key = GPAD_L_SHLDR;
		if (GPad_GetButton(GPAD_R_SHLDR) > 0.0f)
			key = GPAD_R_SHLDR;
		if (GPad_GetButton(GPAD_L_TRIG) > 0.0f)
			key = GPAD_L_TRIG;
		if (GPad_GetButton(GPAD_R_TRIG) > 0.0f)
			key = GPAD_R_TRIG;

		return key;
	}

	std::string GPad_ResolveString(int button) {
		switch (button)
		{
		case GPAD_UP:
			return XOR("D-Pad Up");
		case GPAD_DOWN:
			return XOR("D-Pad Down");
		case GPAD_LEFT:
			return XOR("D-Pad Left");
		case GPAD_RIGHT:
			return XOR("D-Pad Right");
		case GPAD_START:
			return XOR("Start");
		case GPAD_BACK:
			return XOR("Back");
		case GPAD_L3:
			return XOR("L3");
		case GPAD_R3:
			return XOR("R3");
		case GPAD_A:
			return XOR("Gamepad A");
		case GPAD_B:
			return XOR("Gamepad B");
		case GPAD_X:
			return XOR("Gamepad X");
		case GPAD_Y:
			return XOR("Gamepad Y");
		case GPAD_L_SHLDR:
			return XOR("Left Bumper");
		case GPAD_R_SHLDR:
			return XOR("Right Bumper");
		case GPAD_L_TRIG:
			return XOR("Left Trigger");
		case GPAD_R_TRIG:
			return XOR("Right Trigger");
		default:
			return XOR("Unknown");
		}
	}
	bool Gpad_IsButtonActive(int button) {
		bool pressed = false, held = false;

		GamePad* data = (GamePad*)(game_data->game_offsets.P_gamePads);
		if ((button & 0x10000000) != 0)
		{
			pressed = (button & data->digitals & 0xEFFFFFFF) != 0;
			held = (button & data->lastDigitals & 0xEFFFFFFF) != 0;
		}
		else
		{
			if ((button & 0x20000000) == 0)
				return 0;
			unsigned __int64 v5 = (int)button & 0xFFFFFFFFDFFFFFFFui64;
			pressed = data->analogs[v5] > 0.0;
			held = data->lastAnalogs[v5] > 0.0;
		}
		return pressed || held;
	}
}