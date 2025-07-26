#pragma once
#include "../gui/framework.h"
#include "vec.h"
#include "vec2.h"
#include "../util/utility.h"
#include "../cheat/blueprint/blueprints.h"
#include "../protect/hash.h"

#define PITCH 0
#define YAW 1
#define ROLL 2

float flRadarSize = 220.0f, flBlipSize = 7.0f;
ImVec2 vRadarPosition = ImVec2(134.f, 130.f);
ImVec2 vBlipPosition[152]{};

Vector2 compass_pos, arrow_pos[152];
float arrowAngle[152];
float scaling_factor;

const char* MapName;
int MapIndex;
int MapSource;

uintptr_t log_xuid;
unsigned __int64 R_AddDObjToSceneTrampoline;
unsigned __int64 ViewModelDetour;
unsigned __int64 LootModelDetour;
unsigned __int64 CG_NotifyServerOriginal;
unsigned __int64 test1_Original;
unsigned __int64 test2_Original;
unsigned __int64 CG_StartShellShock_original;
unsigned __int64 CG_DrawDamageDirectionIndicators_original;
unsigned __int64 CG_NotifyServerVisual;
unsigned __int64 CL_InputMP_ReadyToSendPacketOriginal;
unsigned __int64 PredictPlayerStateOriginal;
unsigned __int64 bulletHitEventOriginal;

uintptr_t  Plat_GetMacAddressAsUint64_original;
uintptr_t  bdEventLog_recordEventsMixed_ORIGINAL;
uintptr_t  Plat_GetMachineId_original;
uintptr_t macadress_original;

char Plat_GetMacAddressAsUint64Hook(BYTE* a1);
bool Plat_GetMachineIdHook(unsigned __int64* machineIDLow, unsigned __int64* machioneIDHigh);
char macadressHook(BYTE* bbmac);
int64_t __fastcall bdEventLog_recordEventsMixedHook(int64_t a1, int64_t* result, int64_t* eventInfo, uint32_t count, int64_t eventLogIDs);

enum DWLobbyService_LobbyStatus : __int32
{
	UNINITIALIZED = 0x0,
	NOT_CONNECTED = 0x1,
	CONNECTING = 0x2,
	WAITING_FOR_REPORT_PC_HARDWAREDETAILS = 0x3,
	CONNECTED = 0x4,
	DISCONNECTING = 0x5,
	DISCONNECTED = 0x6,
};

const struct _GUIDDD
{
	unsigned int Data1;
	unsigned __int16 Data2;
	unsigned __int16 Data3;
	unsigned __int8 Data4[8];
};

struct ChatInfo
{
	__int32 type;
	unsigned __int64 id;
};

namespace radar
{
	extern void draw();
}

namespace std
{
	template<typename ret, typename val>
	constexpr auto stcast(val v)
	{
		return static_cast<ret>(v);
	}

	template<typename ret, typename val>
	constexpr auto dycast(val v)
	{
		return dynamic_cast<ret>(v);
	}

	template<typename ret, typename val>
	constexpr auto cocast(val v)
	{
		return const_cast<ret>(v);
	}

	template<typename ret, typename val>
	constexpr auto recast(val v)
	{
		return reinterpret_cast<ret>(v);
	}
}

struct item_t
{
	item_t() = default;
	item_t(const hash_t name, const hash_t type, const std::any var) :m_name(name), m_type(type), m_var(var)
	{};

	~item_t() = default;

	/// <summary>
	/// Cast the inner variable to the desired type and returns it
	/// </summary>
	/// <returns>Casted variable</returns>
	template< typename t >
	t& get()
	{
		return *reinterpret_cast<t*>(std::any_cast<t>(&m_var));
	}

	/// <summary>
	/// Sets the inner variable to the desired value
	/// </summary>
	/// <param name="val">Value that the variable will be set to</param>
	template< typename t >
	void set(t val)
	{
		m_var.emplace<t>(val);
	}

	hash_t m_name;
	hash_t m_type;
	std::any m_var;
};

struct GfxSceneHudOutlineInfo
{
	union
	{
		DEFINE_MEMBER_N(uint32_t, color, 0x8);
		DEFINE_MEMBER_N(bool, drawOccludedPixels, 0x14);
		DEFINE_MEMBER_N(bool, drawNonOccludedPixels, 0x15);
		DEFINE_MEMBER_N(bool, fill, 0x17);
		DEFINE_MEMBER_N(uint8_t, renderMode, 0x1F);
		DEFINE_MEMBER_N(uint8_t, lineWidth, 0x20);
	};
};

struct GfxSceneEntityMutableShaderData
{
	char pd[0x48];
	GfxSceneHudOutlineInfo hudOutlineInfo;
};

struct cpose_t
{
	unsigned __int8 eType;
	unsigned __int8 cullIn;
	unsigned __int8 isPosePrecise;
	unsigned __int8 entOriginSet;
	volatile int mdaoCullIn;
	unsigned __int8 ragdollFlags;
	unsigned __int8 isEntityPose;
	unsigned __int8 hasStaleEntityPose;
	unsigned __int8 mayhemInstanceId;
	bool isMayhem;
	bool reusePreviousSkelPose;
	int ragdollHandle;
	int killcamRagdollHandle;
	int physicsId;
};

enum weapFireType_t : __int32
{
	WEAPON_FIRETYPE_FULLAUTO = 0x0,
	WEAPON_FIRETYPE_SINGLESHOT = 0x1,
	WEAPON_FIRETYPE_BURST = 0x2,
	WEAPON_FIRETYPE_DOUBLEBARREL = 0x3,
	WEAPON_FIRETYPE_DOUBLEBARREL_DUALTRIGGER = 0x4,
	WEAPON_FIRETYPE_BEAM = 0x5,
	WEAPON_FIRETYPECOUNT = 0x6,
};

enum WeapType {
	WEAPTYPE_NONE = 0,
	WEAPTYPE_MODEL_ONLY = 1,
	WEAPTYPE_BULLET = 2,
	WEAPTYPE_GRENADE = 3,
	WEAPTYPE_PROJECTILE = 4,
	WEAPTYPE_RIOTSHIELD = 5,
	WEAPTYPE_SCRIPT = 6,
	WEAPTYPE_SHIELD = 7,
	WEAPTYPE_CHARGE_SHIELD = 8,
	WEAPTYPE_LOCATION_SELECT = 9,
	WEAPTYPE_EQUIP_DEPLOY = 0x0A,
};

enum boneIndex : int32_t
{
	tag_origin,
	tag_sync,
	j_mainroot,
	j_spinelower,
	j_spineupper,
	j_spine4,
	j_neck,
	j_head,
	j_helmet,
	j_clavicle_le,
	j_shoulder_le,
	j_elbow_le,
	j_wrist_le,
	j_clavicle_ri,
	j_shoulder_ri,
	j_elbow_ri,
	j_wrist_ri,
	j_hip_le,
	j_knee_le,
	j_ankle_le,
	j_ball_le,
	j_hip_ri,
	j_knee_ri,
	j_ankle_ri,
	j_ball_ri,
	tag_stowed_hip_rear,
	j_hiptwist_le,
	j_hiptwist_ri,
	tag_inhand,
	tag_shield_back,
	tag_weapon_chest,
	j_gun_adjust,
	j_gun,
	j_shouldertwist_le,
	j_shouldertwist_ri,
	tag_eye,
	tag_weapon_right,
	j_sheild_ri,
	j_wristfronttwist1_le,
	tag_reflector_arm_le,
	tag_reflector_arm_ri,
	tag_helmetlight,
	tag_accessory_left,
	tag_accessory_right,
	tag_weapon_left,
	tag_ik_wrist_loc_le,
	tag_ik_wrist_loc_ri,
	j_hipholster_le,
	j_hipholster_ri,
	j_hip_proc_le,
	j_hip_proc_ri,
	j_dummy_knee_dist,
	j_spinelower_lift,
	j_proc_spinelower,
	j_proc_spinelower2,
	j_proc_spineupper,
	j_ik_foot_root,
	tag_ik_foot_loc_le,
	tag_ik_foot_loc_ri,
	j_antislide_ik,
	tag_wrist_attach_le,
	j_proc_hip_lift_le,
	j_proc_hip_lift_ri,
	j_proc_hip_lift_aim_le,
	j_proc_hip_lift_aim_ri,
	j_proc_dummy_spinelift,
	j_chest,
	j_dummy_slingcenteraim,
	j_dummy_sling_spine,
	j_groinprotector,
	j_proc_lift_clavicle_le,
	j_proc_lift_clavicle_ri,
	j_proc_stowed_chest,
	j_proc_spinelift,
	j_proc_spinelower_lift,
	j_proc_spineupper_lift,
	j_sling_spine
};

enum ZOMBIE_BONE_INDEX : int32_t
{
	BONE_POS_HEAD = 22,
	BONE_POS_NECK = 21,
	BONE_POS_LCLAVICAL = 19,
	BONE_POS_RCLAVICAL = 20,
	BONE_POS_UPPER_SPINE = 16,
	BONE_POS_LOWER_SPINE = 13,
	BONE_POS_MAINROOT = 2,
	BONE_POS_RSHOULDER = 24,
	BONE_POS_LSHOULDER = 23,
	BONE_POS_RELBOW = 26,
	BONE_POS_LELBOW = 25,
	BONE_POS_HELMET = 30,
	BONE_POS_HAIR = 27,
	BONE_POS_RHIP = 10,
	BONE_POS_LHIP = 9,
	BONE_POS_RKNUCKLE = 50,
	BONE_POS_LKNUCKLE = 49,
	BONE_POS_RKNEE = 12,
	BONE_POS_LKNEE = 11,
	BONE_POS_RANKLE = 8,
	BONE_POS_LANKLE = 7,
	BONE_POS_STOMACH = 6,
};

struct DObjAnimMat {
	Vector4 quat;
	Vector3 trans;
	float transWeight;
};

struct DObj {
	void* tree;
	char __pad_0x8[0xD0];
	DObjAnimMat* mat;

	static DObj* get(int entityNum);

};

struct bitflag_t
{
	/// <summary>
	/// Creates a default bitflag object
	/// </summary>
	bitflag_t() = default;

	/// <summary>
	/// Creates a bitflag object with the given flags
	/// </summary>
	bitflag_t(uintptr_t flags) : m_flags(flags) {}

	/// <summary>
	/// Destroys the bitflag object
	/// </summary>
	~bitflag_t() = default;

	/// <summary>
	/// Stored flags
	/// </summary>
	uintptr_t  m_flags{};

	/// <summary>
	/// Checks whether the given flag is present in the stored flags
	/// </summary>
	/// <param name="flag">Flag that will be checked</param>
	/// <returns>Returns true if the flag is present in the stored flags</returns>
	inline bool has_flag(uintptr_t flag)
	{
		return m_flags & flag;
	}

	/// <summary>
	/// Adds the given flag to the stored flags
	/// </summary>
	/// <param name="flag">Flag that will be added</param>
	inline void add_flag(uintptr_t flag)
	{
		m_flags |= flag;
	}

	/// <summary>
	/// Removes the given flag to the stored flags
	/// </summary>
	/// <param name="flag">Flag that will be removed</param>
	inline void remove_flag(uintptr_t flag)
	{
		m_flags &= ~flag;
	}

	/// <summary>
	/// Checks if there are no flags stored
	/// </summary>
	/// <returns>True if no flags stored</returns>
	inline bool is_empty()
	{
		return !m_flags;
	}
};

struct Weapon
{
	unsigned __int16 weaponIdx;
	char pad[0x44];
}; // Size: 0x46 | 70

typedef struct usercmd_s
{
	DWORD64 buttons;        // 0x0
	char pad_0[0x18];        // 0x8
	int serverTime;         // 0x20  
	char pad_1[0x8];        // 0x24
	int CommandTime;        // 0x2C
	char pad_3[0x4];        // 0x30 
	int angles[3];          // 0x34
	Weapon weapon;          // 0x40
	char pad_4[0x1E2];    // 0x86
} usercmd_t; // 0x268

//struct Weapon
//{
//	//current size 0x4C | 76
//	unsigned __int16 weaponIndex;
//	char pad1[0x3A];
//	unsigned __int16 weaponCamo; // 0x3C | find in Scr_GetWeaponCamoName
//	__int16 pad2;
//	unsigned char scopeVariation; // 0x40 | find in Scr_GetWeaponReticleName
//	char pad3[0xA];
//};
//
//typedef struct usercmd_s
//{
//	bitflag_t buttons;
//	char pad[0x10];
//	int unk;//0x18
//	int serverTime;//0x1C
//	int prevInputTime;//0x20
//	int inputTime;//0x24
//	int commandTime;//0x28
//	int unk3;//0x2C
//	int angles[3];//30
//	Weapon weapon;//0x38
//	Weapon offHand;//0x82
//	char somemorePad[0x8];
//	char forwardmove;//0xDC
//	char rightmove;//0xDE
//	char pitchmove;//0xDF
//	char yawmove;//0xF0
//	char pad2[0x2];
//	unsigned __int16 meleeChargeEnt; // correct//0xD6
//} usercmd_t;

struct ClActiveClient
{
	unsigned int cmd_number_aab();
	int cmdNumber();
	static ClActiveClient* GetActiveClient();
	usercmd_t* GetUserCmd(int sequenceNumber);
	void SetCmdNumber(int newNumber);
	void AddCmdNumber();
	int GetCmdNumber();
};

struct pmove_t {
	void** __vftable;
	uintptr_t ps;
};

struct PlayerActiveWeaponState
{
	char pad_0[0x54]; // offset: 0x11A8
	int weaponState;  // offset: 0x11FC
	char pad_2[0x40]; // offset: 0x1200
}; 

struct playerState_s {
	int commandTime;  // offset: 0x0
	int serverTime; // offset: 0x4
	char inputTime[0x40]; // offset: 0x8
	Vector3 origin; // offset: 0x48
	Vector3 velocity; // offset: 0x54
	char impulseFieldSpeed[0x110]; // offset: 0x64
	Vector3 deltaAngles; // offset: 0x174
	char pad_0x174[0x19C]; // offset: 0x17C
	int clientNum; // offset: 0x318
	char pad_0x314[0x8]; // offset: 0x31C
	Vector3 viewAngles; // offset: 0x324
	char pad_0x328[0xAD]; // offset: 0x330

	unsigned __int8 damageEvent; // offset: 0x3DD
	unsigned __int8 damageYaw; // offset: 0x3DE
	unsigned __int8 damagePitch; // offset: 0x3DF
	unsigned __int8 damageCount; // offset: 0x3E0

	char pad_damageEvents_endPosition[0x18D];                  // offset: 0x3E4

	bool radarBlocked; // offset: 0x571

	char pad_radarShowEnemyDirection[0xC36]; // offset: 0x572

	PlayerActiveWeaponState weapState[2];   // offset: 0x11A8
};

namespace client {
	extern float radius;
	extern DWORD64 CgArray;
	extern ClActiveClient* ClActive;
	extern DWORD64 playerState;
	extern usercmd_t* Cmd;
	extern playerState_s* localPlayer;
	extern ImVec3 antiAimAngles;
	extern Vector3 FixedAngle;
	//extern playerState_s* getPlayerstateClient(DWORD64 CG);
	extern int BG_UsrCmdPackAngle(float angle);
	extern void Jitteryboy(usercmd_s* usercmd);
	extern void Spinnyboy(usercmd_s* usercmd);
	extern void AntiNig(usercmd_s* usercmd);
	extern void SilentAim(usercmd_s* usercmd);
	extern void RapidFire(usercmd_s* usercmd);
	extern void LongMelee(usercmd_t* userCmd);
	extern void AutoFire(usercmd_s* usercmd);
	//extern void LongMelee(usercmd_t* userCmd);
	extern void Airstuck(usercmd_s* usercmd);
	extern int GetHeldWeapon();
	extern void WeaponsEquipped(int weapon);
}

struct vec3_t
{
	vec3_t() = default;

	vec3_t(float xyz) : x(xyz), y(xyz), z(xyz) {};
	vec3_t(float x, float y) : x(x), y(y), z(0) {};
	vec3_t(float x, float y, float z) : x(x), y(y), z(z) {};

	inline float length_sqr() const
	{
		return x * x + y * y + z * z;
	}

	inline float length_2d_sqr() const
	{
		return x * x + y * y;
	}

	inline float length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline float length_2d() const
	{
		return sqrt(x * x + y * y);
	}

	inline vec3_t normalize() const
	{
		vec3_t out{};
		float l = length();

		if (l != 0)
		{
			out.x = x / l;
			out.y = y / l;
			out.z = z / l;
		}
		else
			out.x = out.y = 0.0f; out.z = 1.0f;

		return out;
	}

	inline vec3_t normalize_in_place() const
	{
		vec3_t out{};
		float l = length();
		float r = 1.f / (l + std::numeric_limits<float>::epsilon());

		if (l != 0)
		{
			out.x = x * r;
			out.y = y * r;
			out.z = z * r;
		}
		else
			out.x = out.y = 0.0f; out.z = 1.0f;

		return out;
	}

	inline float self_dot() const
	{
		return (x * x + y * y + z * z);
	}

	inline float dot(const vec3_t& in) const
	{
		return (x * in.x + y * in.y + z * in.z);
	}

#pragma region assignment
	inline vec3_t& operator=(const vec3_t& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;

		return *this;
	}
#pragma endregion assignment

#pragma region equality
	inline bool operator!=(const vec3_t& in)
	{
		return (x != in.x || y != in.y || z != in.z);
	}

	inline bool operator==(const vec3_t& in)
	{
		return (x == in.x && y == in.y && z == in.z);
	}
#pragma endregion equality

#pragma region addition
	inline vec3_t operator+(vec3_t in) const
	{
		return vec3_t(
			x + in.x,
			y + in.y,
			z + in.z
		);
	}

	inline vec3_t& operator+=(vec3_t in)
	{
		x += in.x;
		y += in.y;
		z += in.z;

		return *this;
	}
#pragma endregion addition

#pragma region substraction
	inline vec3_t operator-(vec3_t in) const
	{
		return vec3_t(
			x - in.x,
			y - in.y,
			z - in.z
		);
	}

	inline vec3_t& operator-=(vec3_t in)
	{
		x -= in.x;
		y -= in.y;
		z -= in.z;

		return *this;
	}
#pragma endregion substraction

#pragma region multiplication
	inline vec3_t operator*(vec3_t in) const
	{
		return vec3_t(
			x * in.x,
			y * in.y,
			z * in.z
		);
	}

	inline vec3_t operator*(float in) const
	{
		return vec3_t(
			x * in,
			y * in,
			z * in
		);
	}

	inline vec3_t& operator*=(vec3_t in)
	{
		x *= in.x;
		y *= in.y;
		z *= in.z;

		return *this;
	}

	inline vec3_t& operator*=(float in)
	{
		x *= in;
		y *= in;
		z *= in;

		return *this;
	}
#pragma endregion multiplication

#pragma region division
	inline vec3_t operator/(vec3_t in) const
	{
		return vec3_t(
			x / in.x,
			y / in.y,
			z / in.z
		);
	}

	inline vec3_t operator/(float in) const
	{
		return vec3_t(
			x / in,
			y / in,
			z / in
		);
	}

	inline vec3_t& operator/=(vec3_t in)
	{
		x /= in.x;
		y /= in.y;
		z /= in.z;

		return *this;
	}

	inline vec3_t& operator/=(float in)
	{
		x /= in;
		y /= in;
		z /= in;

		return *this;
	}
#pragma endregion division

	float x, y, z;
};

class secured_vec3_t
{
private:
	int keys[5];

	__forceinline uintptr_t base(int idx) const
	{
		return reinterpret_cast<uintptr_t>(&keys[idx]);
	}
public:
	vec3_t get() const
	{
		int v42 = 0, v54[3] = {};
		v42 = keys[3];
		v54[0] = static_cast<int>(keys[0] ^ ((base(0) ^ v42) * ((base(0) ^ v42) + 2)));
		v54[1] = static_cast<int>(keys[1] ^ ((base(1) ^ v42) * ((base(1) ^ v42) + 2)));
		v54[2] = static_cast<int>(keys[2] ^ ((v42 ^ base(2)) * ((v42 ^ base(2)) + 2)));
		return vec3_t(*reinterpret_cast<float*>(&v54[0]), *reinterpret_cast<float*>(&v54[1]), *reinterpret_cast<float*>(&v54[2]));
	}

	void set(float x, float y, float z)
	{
		int v42, v43, v54[3];
		v42 = keys[3];
		v54[0] = static_cast<int>(keys[0] ^ ((base(0) ^ v42) * ((base(0) ^ v42) + 2)));
		v54[1] = static_cast<int>(keys[1] ^ ((base(1) ^ v42) * ((base(1) ^ v42) + 2)));
		v54[2] = static_cast<int>(keys[2] ^ ((v42 ^ base(2)) * ((v42 ^ base(2)) + 2)));

		*reinterpret_cast<float*>(&v54[0]) = x;
		*reinterpret_cast<float*>(&v54[1]) = y;
		*reinterpret_cast<float*>(&v54[2]) = z;

		v43 = v42 + keys[4];
		keys[3] = v43;
		keys[0] = static_cast<int>(v54[0] ^ ((v43 ^ base(0)) * ((v43 ^ base(0)) + 2)));
		keys[1] = static_cast<int>(v54[1] ^ ((v43 ^ base(1)) * ((v43 ^ base(1)) + 2)));
		keys[2] = static_cast<int>(v54[2] ^ ((v43 ^ base(2)) * ((v43 ^ base(2)) + 2)));
	}

	void set(vec3_t value)
	{
		return set(value.x, value.y, value.z);
	}
};

struct angle_t
{
	angle_t() = default;

	angle_t(float xyz) : x(xyz), y(xyz), z(xyz) {};
	angle_t(float x, float y) : x(x), y(y), z(0) {};
	angle_t(float x, float y, float z) : x(x), y(y), z(z) {};
	angle_t(float* arr) : x(arr[PITCH]), y(arr[YAW]), z(arr[ROLL]) {};

	inline float length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline angle_t normal()
	{
		angle_t out{};
		float l = length();

		if (l != 0.f)
		{
			out.x = x / l;
			out.y = y / l;
			out.z = z / l;
		}
		else
			out.x = out.y = 0.0f; out.z = 1.0f;

		return out;
	}

	inline angle_t normalize()
	{
		angle_t out{};

		out.x = std::isfinite(x) ? std::remainderf(x, 360.f) : 0.f;
		out.y = std::isfinite(y) ? std::remainderf(y, 360.f) : 0.f;
		out.z = z;

		return out;
	}

	inline angle_t clamp()
	{
		angle_t out{};

		out.x = std::clamp(x, -85.f, 85.f);
		out.y = std::clamp(y, -180.f, 180.f);
		out.z = z;

		return out;
	}

#pragma region assignment
	inline angle_t& operator=(const angle_t& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;

		return *this;
	}
#pragma endregion assignment

#pragma region equality
	inline bool operator!=(const angle_t& in)
	{
		return (x != in.x || y != in.y || z != in.z);
	}

	inline bool operator==(const angle_t& in)
	{
		return (x == in.x && y == in.y && z == in.z);
	}
#pragma endregion equality

#pragma region addition
	inline angle_t operator+(angle_t in) const
	{
		return angle_t(
			x + in.x,
			y + in.y,
			z + in.z
		);
	}

	inline angle_t operator+(float in) const
	{
		return angle_t(
			x + in,
			y + in,
			z + in
		);
	}

	inline angle_t& operator+=(angle_t in)
	{
		x += in.x;
		y += in.y;
		z += in.z;

		return *this;
	}

	inline angle_t& operator+=(float in)
	{
		x += in;
		y += in;
		z += in;

		return *this;
	}
#pragma endregion addition

#pragma region substraction
	inline angle_t operator-(angle_t in) const
	{
		return angle_t(
			x - in.x,
			y - in.y,
			z - in.z
		);
	}

	inline angle_t operator-(float in) const
	{
		return angle_t(
			x - in,
			y - in,
			z - in
		);
	}

	inline angle_t& operator-=(angle_t in)
	{
		x -= in.x;
		y -= in.y;
		z -= in.z;

		return *this;
	}

	inline angle_t& operator-=(float in)
	{
		x -= in;
		y -= in;
		z -= in;

		return *this;
	}
#pragma endregion substraction

#pragma region multiplication
	inline angle_t operator*(angle_t in) const
	{
		return angle_t(
			x * in.x,
			y * in.y,
			z * in.z
		);
	}

	inline angle_t operator*(float in) const
	{
		return angle_t(
			x * in,
			y * in,
			z * in
		);
	}

	inline angle_t& operator*=(angle_t in)
	{
		x *= in.x;
		y *= in.y;
		z *= in.z;

		return *this;
	}

	inline angle_t& operator*=(float in)
	{
		x *= in;
		y *= in;
		z *= in;

		return *this;
	}
#pragma endregion multiplication

#pragma region division
	inline angle_t operator/(angle_t in) const
	{
		return angle_t(
			x / in.x,
			y / in.y,
			z / in.z
		);
	}

	inline angle_t operator/(float in) const
	{
		return angle_t(
			x / in,
			y / in,
			z / in
		);
	}

	inline angle_t& operator/=(angle_t in)
	{
		x /= in.x;
		y /= in.y;
		z /= in.z;

		return *this;
	}

	inline angle_t& operator/=(float in)
	{
		x /= in;
		y /= in;
		z /= in;

		return *this;
	}
#pragma endregion division

	float x, y, z;
};

//enum ShockViewTypes : __int32
//{
//	SHELLSHOCK_VIEWTYPE_BLURRED = 0x0,
//	SHELLSHOCK_VIEWTYPE_FLASHED = 0x1,
//	SHELLSHOCK_VIEWTYPE_PHOTO = 0x2,
//	SHELLSHOCK_VIEWTYPE_NONE = 0x3,
//};
//
//struct unnamed_type_screenBlend
//{
//	int blurredFadeTime;
//	int blurredEffectTime;
//	int flashWhiteFadeTime;
//	int flashShotFadeTime;
//	ShockViewTypes type;
//};
//
//struct unnamed_type_view
//{
//	int fadeTime;
//	float kickRate;
//	float kickRadius;
//};
//
//struct unnamed_type_sound
//{
//	bool affect;
//	char loop[64];
//	char loopSilent[64];
//	char end[64];
//	char endAbort[64];
//	int fadeInTime;
//	int fadeOutTime;
//	char audioZone[64];
//	int modEndDelay;
//	int loopFadeTime;
//	int loopEndDelay;
//};
//
//struct unnamed_type_lookControl
//{
//	bool affect;
//	int fadeTime;
//	float mouseSensitivity;
//	float maxPitchSpeed;
//	float maxYawSpeed;
//};
//
//struct unnamed_type_movement
//{
//	bool affect;
//	bool breakSprint;
//	bool breakJog;
//};
//
//struct unnamed_type_mount
//{
//	bool affect;
//};
//
//struct shellshock_parms_t
//{
//	unnamed_type_screenBlend screenBlend;
//	unnamed_type_view view;
//	unnamed_type_sound sound;
//	unnamed_type_lookControl lookControl;
//	unnamed_type_movement movement;
//	unnamed_type_mount mount;
//};
//
//
//struct shellshock_t
//{
//	shellshock_parms_t* parms;
//	int startTime;
//	int duration;
//	float sensitivity;
//	Vector2 viewDelta;
//	int wantSavedScreen;
//	int hasSavedScreen;
//	int hasScissor;
//	bool lookControlActive;
//	int loopEndTime;
//};

class RefdefView
{
public:
	Vector2 tanHalfFov; // 0x00
	unsigned __int8 unk1[0xC]; // 0x08
	Vector3 axis[3]; // 0x14
};

class refdef_t
{
public:
	int x; // 0x00
	int y; // 0x04
	int Width; // 0x08
	int Height; // 0x0C
	RefdefView view; // 0x10
};

enum Length_t
{
	LENGTH_NONE = 0x0,
	LENGTH_HALF = 0x1,
	LENGTH_HALF_HALF = 0x2,
	LENGTH_LONG = 0x3,
	LENGTH_LONG_LONG = 0x4,
	LENGTH_MAX_INT = 0x5,
	LENGTH_SIZE_T = 0x6,
	LENGTH_PTRDIFF = 0x7,
	LENGTH_LONG_DOUBLE = 0x8,
};
enum LocalClientNum_t {
	LOCAL_CLIENT_INVALID = 0xFFFFFFFF,
	LOCAL_CLIENT_0 = 0x0,
	LOCAL_CLIENT_1 = 0x1,
	LOCAL_CLIENT_LAST = 0x1,
	LOCAL_CLIENT_COUNT = 0x2,
};

//struct NameEntry {
//	int clientNum;
//	char name[0x24];//0x4
//	char pad2[0x48];
//	int extraGameInfo;//0x70
//	char pad3[0x10];
//	int health; //0x84
//	uint64_t unk;
//	int squadIndex;//0x90
//};

struct NameEntry {
	int clientNum;
	char name[36];
	char pad_0[0x48];
	int extraGameInfo;
	char pad_1[0x10];
	int health;
	char pad_2[0x8];
	int squadIndex;
	char pad_3[0x34];
};

enum BRDataFlags : __int32
{
	Downed = 0x1,
	BeingRevived = 0x2,
	SelfReviving = 0x4,
	SelfReviveToken = 0x8,
};

struct score_t
{
	int client;
	int status;
	int score;
	int ping;
	int deaths;
	int team;
	bool isBot;
	int kills;
	int rank_mp;
	int prestige_mp;
	const char* rankDisplayLevel;
	int assists;
	int extrascore0;
	int extrascore1;
	int extrascore2;
	int extrascore3;
	int extrascore4;
	int extrascore5;
};

struct bones
{
	Vector2  real_head_pos_screen, origin, head, j_helmetpos_screen, j_headpos_screen, j_eyeball_lepos_screen, j_eyeball_ripos_screen, j_neckpos_screen, j_clavicle_lepos_screen, j_clavicle_ripos_screen, j_shoulder_lepos_screen, j_shoulder_ripos_screen, j_spineupperpos_screen, j_spinelowerpos_screen,
		j_elbow_lepos_screen, j_elbow_ripos_screen, j_wrist_lepos_screen, j_wrist_ripos_screen, j_hip_lepos_screen, j_hip_ripos_screen, j_knee_lepos_screen, j_knee_ripos_screen, j_ankle_lepos_screen, j_ankle_ripos_screen,
		j_ball_lepos_screen, j_ball_ripos_screen, j_mainrootpos_screen, best_bone_pos_screen;

	Vector3 originpos, j_helmetpos, j_headpos, j_eyeball_lepos, j_eyeball_ripos, j_neckpos, j_clavicle_lepos, j_clavicle_ripos, j_shoulder_lepos, j_shoulder_ripos, j_spineupperpos, j_spinelowerpos, j_elbow_lepos, j_elbow_ripos, j_wrist_lepos, j_wrist_ripos,
		j_hip_lepos, j_hip_ripos, j_knee_lepos, j_knee_ripos, j_ankle_ripos, j_ankle_lepos, j_ball_lepos, j_ball_ripos, best_bone_pos, j_mainrootpos;
};

struct AI_bones
{
	Vector2  real_head_pos_screen, origin, head, j_helmetpos_screen, j_headpos_screen, j_eyeball_lepos_screen, j_eyeball_ripos_screen, j_neckpos_screen, j_clavicle_lepos_screen, j_clavicle_ripos_screen, j_shoulder_lepos_screen, j_shoulder_ripos_screen, j_spineupperpos_screen, j_spinelowerpos_screen,
		j_elbow_lepos_screen, j_elbow_ripos_screen, j_wrist_lepos_screen, j_wrist_ripos_screen, j_hip_lepos_screen, j_hip_ripos_screen, j_knee_lepos_screen, j_knee_ripos_screen, j_ankle_lepos_screen, j_ankle_ripos_screen,
		j_ball_lepos_screen, j_ball_ripos_screen, j_mainrootpos_screen, best_bone_pos_screen;

	Vector3 originpos, j_helmetpos, j_headpos, j_eyeball_lepos, j_eyeball_ripos, j_neckpos, j_clavicle_lepos, j_clavicle_ripos, j_shoulder_lepos, j_shoulder_ripos, j_spineupperpos, j_spinelowerpos, j_elbow_lepos, j_elbow_ripos, j_wrist_lepos, j_wrist_ripos,
		j_hip_lepos, j_hip_ripos, j_knee_lepos, j_knee_ripos, j_ankle_ripos, j_ankle_lepos, j_ball_lepos, j_ball_ripos, best_bone_pos, j_mainrootpos;
};

struct Bounds
{
	Vector3 midPoint;
	Vector3 halfSize;
};

struct player
{
	// Identification
	NameEntry player_name;
	int AI_entnum;
	int entnum;
	int clientnum;
	int myentnum;
	uint64_t xuid = 0;
	__int64 PlatformId;

	// State & Validation
	bool is_AI = false;
	bool is_valid = false;
	bool bbox_is_valid = false;
	bool visible;
	bool aim_visible;
	bool isLookingAtMe;

	// Team & Statistics
	int team = 0;
	int kills = 0;
	int deaths = 0;
	int ping = 0;
	int rank_mp;
	int extrascore2 = 0;

	// Position & FOV
	Vector3 playerPos;
	Vector3 previousPostion;
	float distance;
	float fov;

	// Weapon Info
	std::string weaponDisplayName;

	// Bones & Bounding Boxes
	bones bone_array;
	AI_bones AI_bone_array;
	Bounds Player_Bounds{};
	Vector4 bbox{};
};

struct VelocityData {
	Vector3 value;
};
std::map<DWORD, VelocityData> clients;

struct BgWeaponParms
{
	Vector3 forward;
	Vector3 right;
	Vector3 up;
	Vector3 muzzleTrace;
	Vector3 gunForward;
};

struct WeaponForBullets
{
	unsigned __int16 weaponIdx;
	unsigned __int16 stickerIndices[4];
	unsigned __int16 weaponClientLoadout;
	unsigned __int16 weaponOthers;
	unsigned __int8 weaponAttachments[13];
	unsigned __int8 attachmentVariationIndices[29];
	unsigned __int8 weaponCamo;
	unsigned __int8 weaponLootId;
	unsigned __int8 scopeVariation;
	unsigned __int8 visualAttachmentHighAddr;
	unsigned __int16 unknown;
	char pad[0xE];
	char pad2[0xE];
};
struct WeaponDefBullets
{
	char pad1[0xf4];//0x0
	float distance;//0xF4
	float velocity; //0xF8
	char pad3[0x1CC];

};
struct WeaponDefComplete
{
	//char padding[0x8];
	//const char* internal_name;
	char padding2[0x40];
	const char* displayName;
	//0x40 from start is the name WEAPON/AR_MIKE4
};
struct WeaponDefCompleteOtherWepName
{
	char padding[0x8];
	const char* internal_name;
	//char padding2[0x40];
	//const char* displayName;
	//0x40 from start is the name WEAPON/AR_MIKE4
};

//struct BallisticInfo
//{
//	float unk_1;
//	float unk_2;
//	float muzzleVelocity;
//	float ballisticCoefficient;
//	float diameter;
//	float mass;
//	float gravityFactor;
//	float unk_3;
//	DWORD64 calculated;
//	bool enableBallisticTrajectory;
//	int lifeTimeMs;
//}; // Size : 0x30

struct BallisticInfo
{
	DWORD64 calculated;
	float unk_1;
	float unk_2;
	float muzzleVelocity;
	float ballisticCoefficient;
	float diameter;
	float mass;
	float gravityFactor;
	float unk_3;
	int lifeTimeMs;
	bool enableBallisticTrajectory;
}; // Size : 0x30

enum entityType_s : __int16
{
	ET_FIRST = 0x0,
	ET_GENERAL = 0x0,
	ET_PLAYER = 0x1,
	ET_PLAYER_CORPSE = 0x2,
	ET_ITEM = 0x3,
	ET_MISSILE = 0x4,
	ET_INVISIBLE = 0x5,
	ET_SCRIPTMOVER = 0x6,
	ET_SOUND = 0x7,
	ET_FX = 0x8,
	ET_LOOP_FX = 0x9,
	ET_PRIMARY_LIGHT = 0xA,
	ET_TURRET = 0xB,
	ET_HELICOPTER = 0xC,
	ET_PLANE = 0xD,
	ET_VEHICLE = 0xE,
	ET_VEHICLE_CORPSE = 0xF,
	ET_VEHICLE_SPAWNER = 0x10,
	ET_AGENT = 0x10,
	ET_AGENT_CORPSE = 0x12,
	ET_ACTOR = 0x13,
	ET_ACTOR_SPAWNER = 0x14,
	ET_ACTOR_CORPSE = 0x15,
	ET_PHYSICS_CHILD = 0x16,
	ET_BEAM = 0x17,
	ET_CLIENT_CHARACTER = 0x18,
	ET_RAGDOLL_CONSTRAINT = 0x19,
	ET_PHYSICS_VOLUME = 0x1A,
	ET_COVERWALL = 0x1B,
	ET_INFO_VOLUME_GRAPPLE = 0x1C,
	ET_EVENTS = 0x1D,
	ET_COUNT = 0x1E,
	ET_MAX = 0x100,
};

struct trajectory_t_secure
{
	uint32_t trType;        // 0x00
	int trTime;             // 0x04
	int trDuration;         // 0x08
	Vector3 trBase;    // {0x0C, 0x10, 0x14}
	Vector3 trDelta;   // {0x18, 0x1C, 0x20}
};

struct LerpEntityState
{
	uint64_t nFlags;         // lerp_eFlags (0x10)
	trajectory_t_secure pos; // lerp_pos (0x18)
	trajectory_t_secure apos;// lerp_apos (0x3C)
};

#pragma pack(push, 1)
struct entityState_t
{
	int16_t number;           // 0x00
	int16_t otherEntityNum;   // 0x02
	int16_t attackerEntityNum;// 0x04
	int16_t groundEntityNum;  // 0x06
	entityType_s eType;           // 0x08
	uint8_t surfType;         // 0x0A
	uint8_t inAltWeaponMode;  // 0x0B
	int time2;                // 0x0C
	LerpEntityState lerp;     // starts at 0x10
};
#pragma pack(pop)

struct centity_t
{
	static centity_t* GetEntity(int idx);
	WeaponForBullets* CG_GetEntWeapon2();
};

struct __declspec(align(4)) rectDef_s
{
	float x;
	float y;
	float w;
	float h;
	unsigned __int8 horzAlign;
	unsigned __int8 vertAlign;
};

struct __declspec(align(8)) MinimapOptions
{
	char pad_0[0x14];        // 0x0
	bool cropPartialMap;    // 0x14
	char pad_1[0x43];        // 0x15
	float boundsRadius;        // 0x58
	char pad_2[0x44];        // 0x5C
}; // Size : 0xA0

struct CgCompassSystem
{
	DWORD64 vftable;
	char pad_0[0x10];
	__int32 m_currentCompassType;
	char pad_1[0x604];
	rectDef_s unk_rect;
	rectDef_s m_rect;
	rectDef_s m_scaledRect;
	rectDef_s m_unscaledRect;
	ImVec2 unk_size;
	ImVec4 unk_color_1;
	ImVec4 unk_color_2;
	MinimapOptions* m_minimapOptions;
	char pad_2[0x24];
	float m_compassYaw;
	ImVec2 m_compassYawVector;
	char pad_3[0x58];
}; // 0x728

centity_t* cEntityGlobal;
WeaponForBullets* wepForBulletsGlobal;

namespace sdk
{
	extern refdef_t* refdef;
	void clear_cached();
}

enum GamePadButton : __int32
{
	GPAD_NONE = 0x0,
	GPAD_UP = 0x10000001,
	GPAD_DOWN = 0x10000002,
	GPAD_LEFT = 0x10000004,
	GPAD_RIGHT = 0x10000008,
	GPAD_START = 0x10000010,
	GPAD_BACK = 0x10000020,
	GPAD_L3 = 0x10000040,
	GPAD_R3 = 0x10000080,
	GPAD_A = 0x10001000,
	GPAD_B = 0x10002000,
	GPAD_X = 0x10004000,
	GPAD_Y = 0x10008000,
	GPAD_L_SHLDR = 0x10000100,
	GPAD_R_SHLDR = 0x10000200,
	GPAD_L_TRIG = 0x20000000,
	GPAD_R_TRIG = 0x20000001,
};

struct GamePad {
	char pad_0[0x88];	  // 0x0
	bool keyboardEnabled; // 0x88
	int digitals;		  // 0x8C
	int lastDigitals;	  // 0x90
	float analogs[2];	  // 0x94
	float lastAnalogs[2]; // 0x9C
	char pad_1[0x19C];    // 0xA4
}; // Size : 0x240

namespace gamepad {
	extern float GPad_GetButton(GamePadButton button);

	extern int GPad_ResolveButton();

	extern std::string GPad_ResolveString(int button);

	extern bool Gpad_IsButtonActive(int button);
}

struct BgWeaponHandle
{
	int m_mapEntryId;
};

class cTracer
{
public:
	Vector3 hit3D;
	Vector3 start3D;
	Vector2 hit2D;   // Screen-space coordinates
	Vector2 start2D; // Screen-space coordinates
	int startTime;
	float opacity;
	Vector4 color;
};


std::vector<cTracer> tracers;

struct __declspec(align(8)) XModel
{
	DWORD64 name;                     // 0x0
	const char* name_deprecated;     // 0x8
	char pad_0[0x20];                // 0x10
	Bounds bounds;                     // 0x30
	char pad_1[0xB0];                // 0x48
}; // Size : 0xF8


struct LootData {
	std::string name;
	Vector3 pos;
	Vector3 ang;
	ImColor color;
	Bounds bounds;
	int distance;
	bool canDrawLoot;
	Vector2 screenPos;

	std::vector<Vector2> boxEdges;
};

std::vector<LootData> lootData;

// Global Variables
Vector3 GetCameraPosition();
Vector3 SilentAimAngle{};
Vector3 aimPos;
Vector3 myCamPosition;

//AI_player ai_players[150];

player players[2047];  // 0-2046

player* bestTarget{};
std::vector<player*> Targets{};

// Unified target structure
struct UnifiedTarget {
	Vector3 position;
	Vector3 screenPosition;
	float distance;
	bool visible;
	int entityNum;
	bool isAI;  // Indicates if the target is an AI
};

enum Physics_WorldId
{
	PHYSICS_WORLD_ID_INVALID = 0xFFFFFFFF,
	PHYSICS_WORLD_ID_FIRST = 0x0,
	PHYSICS_WORLD_ID_SERVER_FIRST = 0x0,
	PHYSICS_WORLD_ID_SERVER_MAIN = 0x0,
	PHYSICS_WORLD_ID_SERVER_DETAIL = 0x1,
	PHYSICS_WORLD_ID_SERVER_LAST = 0x1,
	PHYSICS_WORLD_ID_CLIENT_FIRST = 0x2,
	PHYSICS_WORLD_ID_CLIENT0_FIRST = 0x2,
	PHYSICS_WORLD_ID_CLIENT0_PREDICTIVE = 0x2,
	PHYSICS_WORLD_ID_CLIENT0_AUTHORITATIVE = 0x3,
	PHYSICS_WORLD_ID_CLIENT0_DETAIL = 0x4,
	PHYSICS_WORLD_ID_CLIENT0_LAST = 0x4,
	PHYSICS_WORLD_ID_CLIENT1_FIRST = 0x5,
	PHYSICS_WORLD_ID_CLIENT1_PREDICTIVE = 0x5,
	PHYSICS_WORLD_ID_CLIENT1_AUTHORITATIVE = 0x6,
	PHYSICS_WORLD_ID_CLIENT1_DETAIL = 0x7,
	PHYSICS_WORLD_ID_CLIENT1_LAST = 0x7,
	PHYSICS_WORLD_ID_CLIENT_LAST = 0x7,
	PHYSICS_WORLD_ID_LAST = 0x7,
	PHYSICS_WORLD_ID_COUNT = 0x8,
};

enum TraceHitType : __int32
{
	TRACE_HITTYPE_BEGIN = 0x0,
	TRACE_HITTYPE_NONE = 0x0,
	TRACE_HITTYPE_ENTITY = 0x1,
	TRACE_HITTYPE_DYNENT_MODEL = 0x2,
	TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
	TRACE_HITTYPE_GLASS = 0x4,
	TRACE_HITTYPE_SCRIPTABLE = 0x5,
	TRACE_HITTYPE_CLIENT_MODEL = 0x6,
	TRACE_HITTYPE_END = 0x6,
};



/* 1037 */
enum TraceHitSubType : __int32
{
	TRACE_HITSUBTYPE_NONE = 0x0,
	TRACE_HITSUBTYPE_COVERWALL = 0x1,
};

/* 962 */

/* 21856 */
struct TraceSubTypeData_CoverWall
{
	unsigned __int16 id;
};

/* 21857 */
union TraceHitSubTypeData
{
	TraceSubTypeData_CoverWall coverWall;
};

struct trace_t
{
	float fraction;
	Vector3 position;
	Vector3 normal;
	int surfaceFlags;
	int contents;
	TraceHitType hitType;
	TraceHitSubType hitSubType;
	unsigned int hitId;
	float fractionForHitType;
	unsigned __int16 modelIndex;
	uint32_t partName;
	unsigned __int16 partGroup;
	TraceHitSubTypeData subTypeData;
	bool allsolid;
	bool startsolid;
	bool walkable;
	bool getPenetration;
	bool removePitchAndRollRotations;
	float closestPointsPenetration;
	char surfaceIndex;
	const char* debugHitName;
};

union OmnvarValue
{
	bool m_enabled;
	int m_integer;
	unsigned int m_unsignedInteger;
	float m_value;
	unsigned int m_ncsString;
	unsigned int m_xhash32;
	unsigned int m_anonymousCompare;
};

struct OmnvarData
{
	OmnvarValue current;
};

std::vector<UnifiedTarget> unifiedTargets;

// Flags and State
bool isTargetLocket;

// Function Declarations
// Screen and World Space Conversion
bool WorldToScreen(const Vector3& WorldPos, Vector2* ScreenPos);
bool hook_in_game();
bool WorldToScreen_2(const Vector3& WorldPos, Vector2* ScreenPos);
void AimTargetMP_GetTargetBounds(centity_t* targetEnt, Bounds* box);

// Angle Calculations
ImVec3 getAngles(int i);
Vector3 getTargetPositionNormal(int boneIndex, const player* bestTarget);
Vector3 getTargetPositionOverride(int boneIndex, const player* bestTarget);

// Miscellaneous Functions
void CL_PlayerData_SetCustomClanTag(const char* clantag);

// Menu and Hooks
void Cbuf_AddText(const char* fmt, ...);
void init_hooks();
void player_draw_3d_box(const Bounds& bounds, const Vector3& origin, const Vector3& angle, ImColor color);
void draw_3d_box(const Bounds& bounds, const Vector3& origin, const Vector3& angles, ImColor color);
void draw_3d_box_filled(const Bounds& bounds, const Vector3& origin, const Vector3& angle, ImColor color);

// Entity Interaction
void LUI_OpenMenu(__int64 a, const char* cmd, __int64 b, __int64 c, __int64 d);
//bool LUI_GetEntityTagPositionAndAxis(int entityNum, int tagName, Vector3* outWorldPosition);
bool LUI_GetEntityTagPositionAndAxis(int entityNum, __int64 taghash, Vector3* outWorldPosition);
inline unsigned __int64 stringToHash64(const char* str);

// Debug and Utility
bool think();
void ESPThread();
void updateRainbowColor();
//void DevMode();

// Hooks and Patches
//char packet_test_hook(int a1, unsigned int a2);
//uintptr_t slide_endcheck_hook(uintptr_t* pm, uintptr_t* pml, int* outTimeRemaining);

// Bounding Box and Target Functions
bool get_bbox(Vector4& box, player* i);

std::string getNameFromList(std::string model);

// Inline Functions
extern __forceinline bool is_key_down(int vk_key);
extern __forceinline bool is_overid_down(int vk_key);

//void SendToastNotification(const char* header, const char* description, const char* icon);

Vector3 calculate_prediction(int target, Vector3 p_shootingPos, Vector3 p_targetPos);
bool blackbox_unk();
bool blackbox_off();

playerState_s* GetPlayerState(DWORD64 CG);

bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup);
__int64 Com_DDL_LoadAsset(const char* file);
__int64 DDL_GetRootState(__int64 result, uintptr_t ddlDef);
bool Com_ParseNavStrings(const char* pStr, const char** navStrings, const int navStringMax, int* navStringCount);
bool DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path);
int DDL_GetType(__int64 state);
bool DDL_SetInt(__int64 fstate, __int64 tstate, int value);
bool DDL_SetString(__int64 fstate, __int64 state, const char* val);

void sendchatmessage(const char* message);
void Spam_message(const char* message);