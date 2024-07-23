#define WIN32_LEAN_AND_MEAN
#include "MinHook/MinHook.h"
#if _WIN64 
#pragma comment(lib, "MinHook/libMinHook.x64.lib")
#else
#pragma comment(lib, "MinHook/libMinHook.x86.lib")
#endif

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <vector>
#include <cstdint>
#include <Windows.h>
#include <wow64apiset.h>
#include "globals.h"


// Globals
HINSTANCE dll_handle;

bool show = true; // used to toggle menu
bool Detach_ImGui = false;

// Gets "ViceCity.exe" base module address
HMODULE hModule = GetModuleHandle(NULL);
uintptr_t moduleBase = reinterpret_cast<uintptr_t>(hModule);

// Godmode
bool Godmode_flag = false;
uintptr_t JumpBack0{};
//uintptr_t JumpBack6{};
//uintptr_t JumpBack7{};
//uintptr_t conditionaljumpD{ moduleBase + 0x101555A };

// One Hit Kill
bool ohk_flag{};
uintptr_t JumpBack1{};
float dmgVal{ 2000 };

// Infinite Stamina
bool stamina_flag{ false };
static float staminaValue{};

// Wanted Level
bool wanted_flag{};
uintptr_t JumpBack2{};
static DWORD wantedLevel{};
static DWORD chaosLevel{};
uintptr_t conditionaljumpG{ moduleBase + 0x1071693 };

// Free Mouse Cursor
uintptr_t JumpBack3{};

// Infinite Ammo
bool ammo_flag{};
uintptr_t JumpBack4{};
uintptr_t conditionaljumpA{ moduleBase + 0x11368A5 };
uintptr_t conditionaljumpB{ moduleBase + 0x1136E71 };
// calling Give Weapon from Main Thread
uintptr_t JumpBack9{};

// Infinite Vehicle Health
bool vehicleHP_flag{};
static float vehicleHP{ 1000 };
// Repair Vehicle
bool vehicle_flag{};

// Player Fly/No Clip
bool toggle1 = false;
// Vehicle Fly/No Clip
bool toggle2 = false;
//uintptr_t JumpBack1{}; 

// Teleport to Waypoint
//bool waypoints_flag{};
uintptr_t waypointBase{};
uintptr_t JumpBack8{};

// Freeze Mission Timer
bool timer1_flag{};
uintptr_t JumpBack5{};
uintptr_t conditionaljumpC{ moduleBase + 0xFF837F };

// Can't fall off bike
bool toggle10{ false };
uintptr_t JumpBack10{};
uintptr_t conditionaljumpF{ moduleBase + 0x10FCCEB };

// Function for access rights to a specific memory with specific length ... setting bytes / hook/inject
bool Far_Hook(void* hookAddress, void* functionA, size_t len)
{
	if (len < 14)
	{
		return false;
	}

	DWORD protection;
	VirtualProtect(hookAddress, len, PAGE_EXECUTE_READWRITE, &protection);

	memset(hookAddress, 0x00, len);

	*(WORD*)hookAddress = 0x25FF; //Far jump
	*(uintptr_t*)((uintptr_t)hookAddress + 6) = (uintptr_t)(functionA);

	if (len > 14)
	{
		uintptr_t nop_offset = (uintptr_t)hookAddress + 14;

		memset((void*)nop_offset, 0x90, len - 14);
	}

	DWORD temp;
	VirtualProtect(hookAddress, len, protection, &temp);

	return true;
}

// Function for restoring bytes back 
void setmemory_back(void* AddressHere, BYTE* value, unsigned int Length)
{
	DWORD Protector;
	VirtualProtect(AddressHere, Length, PAGE_EXECUTE_READWRITE, &Protector);

	memcpy(AddressHere, value, Length); // No need to cast value to BYTE*

	DWORD Temporary;
	VirtualProtect(AddressHere, Length, Protector, &Temporary);
}

namespace mem
{
	uintptr_t FindAddress(uintptr_t ptr, std::vector<unsigned int> offsets)
	{
		uintptr_t address = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			address = *(uintptr_t*)address;

			if (address == NULL)
			{
				break;
			}

			address += offsets[i];
		}
		return address;
	}
}

void PlayerFly_Function()
{
	float speed{ 0.5f };   // for coords
	//float speed2{ 0.02f };  // for z vel
	uintptr_t playerObj = *(uintptr_t*)(moduleBase + 0x4D6CC70);
	uintptr_t ViewAngles_base = *(uintptr_t*)(moduleBase + 0x0513C658);
	uintptr_t AddrPlayerState = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x38C });
		
	if (GetAsyncKeyState('T'))  // Speed up
	{
		speed = 2.0f;
		//speed2 = 0.25f;
	}
	else if (GetAsyncKeyState('G'))  // Slow down
	{
		speed = 0.09f;
	}

	if (GetAsyncKeyState(VK_LSHIFT))  // Forwards
	{
		if (playerObj && ViewAngles_base && AddrPlayerState)
		{
			//DWORD playerState = 1;
			//*(DWORD*)AddrPlayerState = playerState;  // freeze playerState to this animation to stop momentum when jumping

			uintptr_t physObj = *(uintptr_t*)(ViewAngles_base + 0x108);
			float vectorx = *(float*)((uintptr_t)physObj + 0x700);
			float vectory = *(float*)((uintptr_t)physObj + 0x730);

			float posx = *(float*)((uintptr_t)playerObj + 0x38);
			float posy = *(float*)((uintptr_t)playerObj + 0x3C);
			
			*(float*)((uintptr_t)playerObj + 0x38) = posx + speed * vectorx;
			*(float*)((uintptr_t)playerObj + 0x3C) = posy + speed * vectory;
		}
	}
	//else  // freeze momentum when not moving
	//{
	//	if (playerObj)
	//	{
	//		float velx = *(float*)((uintptr_t)playerObj + 0xFC);
	//		float vely = *(float*)((uintptr_t)playerObj + 0x100);

	//		*(float*)((uintptr_t)playerObj + 0xFC) = velx * 0;
	//		*(float*)((uintptr_t)playerObj + 0x100) = vely * 0;
	//	}
	//}
	
	if (playerObj)
	{
		float velz = *(float*)((uintptr_t)playerObj + 0x104);
		if (velz < 0)
		{
			*(float*)((uintptr_t)playerObj + 0x104) = velz * 0;  // makes player float
		}
	}

	if (GetAsyncKeyState(VK_SPACE))  // Upwards
	{
		if (playerObj && AddrPlayerState)
		{
			if (*(DWORD*)AddrPlayerState != 58 && *(DWORD*)AddrPlayerState != 50) 
			{
				DWORD playerState = 1;
				*(DWORD*)AddrPlayerState = playerState; // Freeze playerState to this animation to stop momentum when jumping
			}

			float posz = *(float*)((uintptr_t)playerObj + 0x40);
			*(float*)((uintptr_t)playerObj + 0x40) = posz + speed;
		}
	}
	
	if (GetAsyncKeyState(VK_CAPITAL))  // Downwards
	{
		if (playerObj && AddrPlayerState)
		{
			if (*(DWORD*)AddrPlayerState != 58 && *(DWORD*)AddrPlayerState != 50)
			{
				DWORD playerState = 1;
				*(DWORD*)AddrPlayerState = playerState; // Freeze playerState to this animation to stop momentum when jumping
			}

			float velz = *(float*)((uintptr_t)playerObj + 0x104);
			*(float*)((uintptr_t)playerObj + 0x104) = velz - speed;
		}
	}
}

void VehicleFly_Function()
{
	float speed{ 0.5f };   // for coords
	//float speed2{ 0.02f };  // for z vel
	uintptr_t vehicleObj = *(uintptr_t*)(moduleBase + 0x5189090);
	uintptr_t ViewAngles_base = *(uintptr_t*)(moduleBase + 0x0513C658);
	
	if (GetAsyncKeyState('T'))  // Speed up
	{
		speed = 2.0f;
		//speed2 = 0.25f;
	}
	else if (GetAsyncKeyState('G'))  // Slow down
	{
		speed = 0.09f;
	}

	if (GetAsyncKeyState(VK_LSHIFT))  // Forwards
	{
		if (vehicleObj && ViewAngles_base)
		{
			uintptr_t physObj = *(uintptr_t*)(ViewAngles_base + 0x108);
			float vectorx = *(float*)((uintptr_t)physObj + 0x700);
			float vectory = *(float*)((uintptr_t)physObj + 0x730);

			float posx = *(float*)((uintptr_t)vehicleObj + 0x38);
			float posy = *(float*)((uintptr_t)vehicleObj + 0x3C);

			*(float*)((uintptr_t)vehicleObj + 0x38) = posx + speed * vectorx;
			*(float*)((uintptr_t)vehicleObj + 0x3C) = posy + speed * vectory;
		}
	}
	
	if (vehicleObj)
	{
		float velz = *(float*)((uintptr_t)vehicleObj + 0x104);
		if (velz < 0)
		{
			*(float*)((uintptr_t)vehicleObj + 0x104) = velz * 0;  // makes vehicle float
		}
	}

	if (GetAsyncKeyState(VK_SPACE))  // Upwards
	{
		if (vehicleObj)
		{
			float posz = *(float*)((uintptr_t)vehicleObj + 0x40);
			*(float*)((uintptr_t)vehicleObj + 0x40) = posz + speed;
		}
	}

	if (GetAsyncKeyState(VK_CAPITAL))  // Downwards
	{
		if (vehicleObj)
		{
			float velz = *(float*)((uintptr_t)vehicleObj + 0x104);
			*(float*)((uintptr_t)vehicleObj + 0x104) = velz - speed;
		}
	}
}

uintptr_t Get_Player_Object()  // player+vehicle (shared)
{
	return *(uintptr_t*)(moduleBase + 0x4D6ABC0);
}

float XPos = 0.0f;
float YPos = 0.0f;
float ZPos = 0.0f;
float XPos2 = 0.0f;
float YPos2 = 0.0f;
float ZPos2 = 0.0f;

void SaveLocation()  // Save Location for Teleport
{
	uintptr_t playerObj = Get_Player_Object();
	if (playerObj)
	{
		// Save Current Location Coordinates
		XPos = *(float*)((uintptr_t)playerObj + 0x38);
		YPos = *(float*)((uintptr_t)playerObj + 0x3C);
		ZPos = *(float*)((uintptr_t)playerObj + 0x40);
	}
}

void LoadLocation()  // Teleport to Saved Location
{
	uintptr_t playerObj = Get_Player_Object();
	if (playerObj)
	{
		// Save Coordinates for Undo Teleport
		XPos2 = *(float*)((uintptr_t)playerObj + 0x38);
		YPos2 = *(float*)((uintptr_t)playerObj + 0x3C);
		ZPos2 = *(float*)((uintptr_t)playerObj + 0x40);

		if (XPos != 0 && YPos != 0 && ZPos != 0)
		{
			// Load Coordinates to Teleport to Saved Location
			*(float*)((uintptr_t)playerObj + 0x38) = XPos;
			*(float*)((uintptr_t)playerObj + 0x3C) = YPos;
			*(float*)((uintptr_t)playerObj + 0x40) = ZPos;
		}
	}
}

void UndoLocation()
{
	uintptr_t playerObj = Get_Player_Object();
	if (playerObj && XPos2 != 0 && YPos2 != 0 && ZPos2 != 0)
	{
		*(float*)((uintptr_t)playerObj + 0x38) = XPos2;
		*(float*)((uintptr_t)playerObj + 0x3C) = YPos2;
		*(float*)((uintptr_t)playerObj + 0x40) = ZPos2;
	}
}

void WaypointTeleport()
{
	uintptr_t playerObj = Get_Player_Object();
	float waypointX = 0.0f;
	float waypointY = 0.0f;
	float waypointZ = 0.0f;
	
	if (waypointBase && playerObj)
	{
		// store waypoint coordinates in variables
		waypointX = *(float*)((uintptr_t)waypointBase + 0x518629C);
		waypointY = *(float*)((uintptr_t)waypointBase + 0x51862A0);
		waypointZ = *(float*)((uintptr_t)waypointBase + 0x51862A4);

		// teleport to those waypoint coordinates if waypoints aren't pointing to default centre of map (coordinate 0)
		if (waypointX != 0 && waypointY != 0 && waypointZ != 0)
		{
			*(float*)((uintptr_t)playerObj + 0x38) = waypointX;
			*(float*)((uintptr_t)playerObj + 0x3C) = waypointY;
			*(float*)((uintptr_t)playerObj + 0x40) = waypointZ + 10.0f;  // Z Increase (teleport higher so player doesn't fall through ground)
		}
	}
}

// Fix Vehicles (mostly even fixes car appearance, for bikes restores tires since their appearance can't be damaged and same for helicopter n boats
typedef void(__thiscall* _Fix_Vehicle)(uintptr_t vehicle);
_Fix_Vehicle Fix_Vehicle;

void Repair_Vehicle()
{
	uintptr_t vehicleBase = *reinterpret_cast<uintptr_t*>(moduleBase + 0x5189090);
	if (vehicleBase)
	{
		uintptr_t vehClass = *reinterpret_cast<uintptr_t*>(vehicleBase + 0x0);

		if (vehClass == moduleBase + 0x404B310) // cars / helicopters
		{
			*(float*)(vehicleBase + 0x320) = 1000.0f;
			Fix_Vehicle(vehicleBase);
		}
		else if (vehClass == moduleBase + 0x404CC08) // bikes
		{
			*(float*)(vehicleBase + 0x320) = 1000.0f;
			*(bool*)(vehicleBase + 0x47C) = 0;
			*(bool*)(vehicleBase + 0x47D) = 0;
		}
		else if (vehClass == moduleBase + 0x404CA98) // boats
		{
			*(float*)(vehicleBase + 0x320) = 1000.0f;
		}
	}
}

// Vehicle Color Changer
typedef void(__fastcall* _Color_Changer)(uintptr_t cObj);
_Color_Changer Color_Changer;

void Change_VehicleColor(int selectedItem)
{
	uintptr_t vehicleBase = *reinterpret_cast<uintptr_t*>(moduleBase + 0x5189090);
	if (vehicleBase)
	{
		//uintptr_t vehicleObject = *(uintptr_t*)(vehicleBase + 0x58);
		
		switch (selectedItem)
		{
		case 0:	// Purple
		{
			*(BYTE*)(vehicleBase + 0x290) = 5;
			*(BYTE*)(vehicleBase + 0x291) = 5;
			Color_Changer(vehicleBase);
			break;
		}
		case 1:	// Green
		{
			*(BYTE*)(vehicleBase + 0x290) = 46;
			*(BYTE*)(vehicleBase + 0x291) = 46;
			Color_Changer(vehicleBase);
			break;
		}
		case 2: // Yellow
		{
			*(BYTE*)(vehicleBase + 0x290) = 6;
			*(BYTE*)(vehicleBase + 0x291) = 6;
			Color_Changer(vehicleBase);
			break;
		}
		case 3:	// Gold
		{
			*(BYTE*)(vehicleBase + 0x290) = 35;
			*(BYTE*)(vehicleBase + 0x291) = 35;
			Color_Changer(vehicleBase);
			break;
		}
		case 4:	// Orange
		{
			*(BYTE*)(vehicleBase + 0x290) = 28;
			*(BYTE*)(vehicleBase + 0x291) = 28;
			Color_Changer(vehicleBase);
			break;
		}
		case 5:	// Salmon
		{
			*(BYTE*)(vehicleBase + 0x290) = 24;
			*(BYTE*)(vehicleBase + 0x291) = 24;
			Color_Changer(vehicleBase);
			break;
		}
		case 6:	// Cherry Red
		{
			*(BYTE*)(vehicleBase + 0x290) = 3;
			*(BYTE*)(vehicleBase + 0x291) = 3;
			Color_Changer(vehicleBase);
			break;
		}
		case 7:	// Red
		{
			*(BYTE*)(vehicleBase + 0x290) = 12;
			*(BYTE*)(vehicleBase + 0x291) = 12;
			Color_Changer(vehicleBase);
			break;
		}
		case 8:	// Blue
		{
			*(BYTE*)(vehicleBase + 0x290) = 2;
			*(BYTE*)(vehicleBase + 0x291) = 2;
			Color_Changer(vehicleBase);
			break;
		}
		case 9: // Bright Blue
		{
			*(BYTE*)(vehicleBase + 0x290) = 7;
			*(BYTE*)(vehicleBase + 0x291) = 7;
			Color_Changer(vehicleBase);
			break;
		}
		case 10: // Light Blue
		{
			*(BYTE*)(vehicleBase + 0x290) = 51;
			*(BYTE*)(vehicleBase + 0x291) = 51;
			Color_Changer(vehicleBase);
			break;
		}
		case 11: // Navy
		{
			*(BYTE*)(vehicleBase + 0x290) = 50;
			*(BYTE*)(vehicleBase + 0x291) = 50;
			Color_Changer(vehicleBase);
			break;
		}
		case 12: // Teal
		{
			*(BYTE*)(vehicleBase + 0x290) = 56;
			*(BYTE*)(vehicleBase + 0x291) = 56;
			Color_Changer(vehicleBase);
			break;
		}
		case 13: // Grey
		{
			*(BYTE*)(vehicleBase + 0x290) = 74;
			*(BYTE*)(vehicleBase + 0x291) = 74;
			Color_Changer(vehicleBase);
			break;
		}
		case 14: // Black
		{
			*(BYTE*)(vehicleBase + 0x290) = 70;
			*(BYTE*)(vehicleBase + 0x291) = 70;
			Color_Changer(vehicleBase);
			break;
		}
		case 15: // White
		{
			*(BYTE*)(vehicleBase + 0x290) = 1;
			*(BYTE*)(vehicleBase + 0x291) = 1;
			Color_Changer(vehicleBase);
			break;
		}
		}
	}
}

struct unknownStruct
{
	unsigned long field1_ = 0x34EE26;
	unsigned long field2_ = 0;
	unsigned char field3_ = 2;
	unsigned char field4_ = 0;
	unsigned char field5_ = 0;
	unsigned char field6_ = 0;
	unsigned long field7_ = 0xFFFFFFFF;
};

class Vector3
{
public:
	float X,Y,Z;

	Vector3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
};

uintptr_t Get_Player_Object2()  // player only, so not vehicle+player
{
	return *(uintptr_t*)(moduleBase + 0x4D6CC70);
}

enum VEHICLE_TYPE
{
	MOBILE_VEHICLE,
	BOAT,
	HELICOPTER = 3,
	BIKE = 5
};

enum VEHICLES
{
	LANDSTALKER = 130,
	IDAHO,
	STINGER,
	LINERUNNER,
	PERENNIAL,
	SENTINEL,
	RIO,
	FIRETRUCK,
	TRASHMASTER,
	STRETCH,
	MANANA,
	INFERNUS,
	VOODOO,
	PONY,
	MULE,
	CHEETAH,
	AMBULANCE,
	FBI_WASHINGTON,
	MOONBEAM,
	ESPERANTO,
	TAXI,
	WASHINGTON,
	BOBCAT,
	MR_WHOOPEE,
	BF_INJECTION,
	HUNTER,
	POLICE,
	ENFORCER,
	SECURICAR,
	BANSHEE,
	PREDATOR,
	BUS,
	RHINO,
	BARRACKS_OL,
	CUBAN_HERMES,
	POLICE_HELICOPTER,
	ANGEL,
	COACH,
	CABBIE,
	STALLION,
	RUMPO,
	RC_BANDIT,
	ROMEROS_HEARSE,
	PACKER,
	SENTINEL_XS,
	ADMIRAL,
	SQUALO,
	SEA_SPARROW,
	PIZZA_BOY,
	GANG_BURRITO,
	AIRTRAIN,
	DODO,
	SPEEDER,
	REEFER,
	TROPIC,
	FLATBED,
	YANKEE,
	CADDY,
	ZEBRA_CAB,
	TOP_FUN,
	SKIMMER,
	PCJ_600,
	FAGGIO,
	FREEWAY,
	RC_BARON,
	RC_RAIDER,
	GLENDALE,
	OCEANIC,
	SANCHEZ,
	SPARROW,
	PATRIOT,
	LOVE_FIST,
	COAST_GUARD,
	DINGHY,
	HERMES,
	SABRE,
	SABRE_TURBO,
	PHOENIX,
	WALTON,
	REGINA,
	COMET,
	DELUXO,
	BURRITO,
	SPAND_EXPRESS,
	MARQUIS,
	BAGGAGE_HANDLER,
	KAUFMAN_CAB,
	MAVERICK,
	VCN_MAVERICK,
	RANCHER,
	FBI_RANCHER,
	VIRGO,
	GREENWOOD,
	CUBAN_JETMAX,
	HOTRING_RACER,
	SANDKING,
	BLISTA_COMPACT,
	POLICE_MAVERICK,
	BOXVILLE,
	BENSON,
	MESA_GRANDE,
	RC_GOBLIN,
	HOTRING_RACER2,
	HOTRING_RACER3,
	BLOODRING_BANGER,
	BLOODRING_BANGER2,
	POLICE_CHEETAH
};

// Wanted Level
typedef void(__fastcall* _Wanted_Level)(uintptr_t wantedobj, int stars);
_Wanted_Level Wanted_Level;

// Vehicle Material Color Changer
typedef void(__fastcall* _Material_Color_Changer)(uintptr_t material, const unknownStruct& a2, const ImVec4& rgba);
_Material_Color_Changer Material_Color_Changer;

// Request Model
typedef void(__fastcall* _Request_Model)(int modelID, int a2, int a3);
_Request_Model Request_Model;
// Get Resource Pointer
typedef uintptr_t(__thiscall* _Resource_Ptr)(uintptr_t a1);
_Resource_Ptr Resource_Ptr;
// Load Requested Model
typedef void(__fastcall* _Load_Requested_Model)(uintptr_t a1, bool a2);
_Load_Requested_Model Load_Requested_Model;
// Load All Models Now
typedef void(__fastcall* _Load_Model_Now)(uintptr_t a1);
_Load_Model_Now Load_Model_Now;
// Mark Model as no longer needed
typedef void(__fastcall* _Destroy_Model)(int modelID);
_Destroy_Model Destroy_Model;

		// call vehicle spawn functions //
// Get vehicle pointer
typedef uintptr_t(__fastcall* _Get_Vehicle_Pointer)();
_Get_Vehicle_Pointer Get_Vehicle_Pointer;
// Init_Vehicle_Model
typedef void(__thiscall* _Init_Vehicle_Model)(uintptr_t veh);
_Init_Vehicle_Model Init_Vehicle_Model;
// Vehicle Model Stuff
typedef void(__thiscall* _Vehicle_Model_Stuff)(uintptr_t veh_model);
_Vehicle_Model_Stuff Vehicle_Model_Stuff;
// Vehicle Model Stuff 2
typedef void(__fastcall* _Vehicle_Model_Stuff2)(uintptr_t veh_model, float transparency);
_Vehicle_Model_Stuff2 Vehicle_Model_Stuff2;
// Init Vehicle Physics
typedef void(__fastcall* _Init_Vehicle_Physics)(uintptr_t veh);
_Init_Vehicle_Physics Init_Vehicle_Physics;
		// call car spawn functions //
// Init CVehicle
typedef uintptr_t(__fastcall* _Init_CVehicle)(uintptr_t vehpointer, VEHICLES modelID, BYTE a3);  // a3 by default is 1
_Init_CVehicle Init_CVehicle;
// Update_CVehicle_Transform
typedef void(__fastcall* _Update_CVehicle_Transform)(uintptr_t veh_transform, float x_angle, float y_angle, float z_angle);
_Update_CVehicle_Transform Update_CVehicle_Transform;
		// call bike spawn functions //
// Init CBike
typedef uintptr_t(__fastcall* _Init_CBike)(uintptr_t vehpointer, VEHICLES modelID, BYTE a3);  // a3 by default is 1
_Init_CBike Init_CBike;
// Update CBike Transform
typedef void(__fastcall* _Update_CBike_Transform)(uintptr_t veh_transform, float x_angle, float y_angle, float z_angle);
_Update_CBike_Transform Update_CBike_Transform;
		// call boat spawn functions //
// Init CBoat
typedef uintptr_t(__fastcall* _Init_CBoat)(uintptr_t vehpointer, VEHICLES modelID, BYTE a3);  //a3 for boats it set to 2
_Init_CBoat Init_CBoat;
// Update CBoat Transform
typedef void(__fastcall* _Update_CBoat_Transform)(uintptr_t veh_pos, uintptr_t veh);
_Update_CBoat_Transform Update_CBoat_Transform;
// Init CHelicopter
typedef uintptr_t(__fastcall* _Init_CHelicopter)(uintptr_t vehpointer, VEHICLES modelID, BYTE a3);  //a3 for boats it set to 1
_Init_CHelicopter Init_CHelicopter;


void Set_Wanted(int stars_var)
{
	uintptr_t wanted_Object = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0 });  // Wanted Object
	Wanted_Level(*(uintptr_t*)wanted_Object, stars_var);
}

void Create_Vehicle(VEHICLES vehicle)
{
	float Cam_XVec = *(float*)((uintptr_t)moduleBase + 0x4D6B154);
	float Cam_YVec = (*(float*)((uintptr_t)moduleBase + 0x4D6B150)) * -1.0f;

	if (uintptr_t player = Get_Player_Object2())
	{
		Vector3 playerPos = *(Vector3*)((uintptr_t)player + 0x38);
		uintptr_t re_ptr = *reinterpret_cast<uintptr_t*>(moduleBase + 0x539EED8);

		Request_Model(vehicle, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Destroy_Model(vehicle);

		uintptr_t vehicle_info = *(uintptr_t*)(moduleBase + static_cast<long long>(vehicle) * 8 + 0x50AD8F0);
		if (vehicle_info)
		{
			VEHICLE_TYPE vehtype = *(VEHICLE_TYPE*)((uintptr_t)vehicle_info + 0x84);
			BYTE vehtype2 = *(BYTE*)((uintptr_t)vehicle_info + 0x3C);

			if (vehtype2 != 6)
			{
				if (uintptr_t veh = Get_Vehicle_Pointer())
				{
					if (Init_CVehicle(veh, vehicle, 1))
					{
						*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
						*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
						*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

						Update_CVehicle_Transform(veh + 0x8, 0, 0, 3.49f);

						*(int*)((uintptr_t)veh + 0x6C) &= -217;
						*(int*)((uintptr_t)veh + 0x6C) |= 32;

						uintptr_t vehclass = *(uintptr_t*)(veh);

						Init_Vehicle_Model = (_Init_Vehicle_Model)(*(uintptr_t*)(vehclass + 0x168));

						Init_Vehicle_Model(veh);

						uintptr_t vehmodel = *(uintptr_t*)(veh + 0x58);
						uintptr_t vehmodel_class = *(uintptr_t*)(vehmodel);

						Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(*(uintptr_t*)(vehmodel_class + 0x670));

						Vehicle_Model_Stuff(vehmodel);

						Vehicle_Model_Stuff2(vehmodel, 3.0f);

						Init_Vehicle_Physics(veh);

					}
				}
			}
			else
			{
				switch (vehtype)
				{
				case MOBILE_VEHICLE:
				{
					if (uintptr_t veh = Get_Vehicle_Pointer())
					{
						if (Init_CVehicle(veh, vehicle, 1))
						{
							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							Update_CVehicle_Transform(veh + 0x8, 0, 0, 3.49f);

							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							*(int*)((uintptr_t)veh + 0x6C) &= -217;
							*(int*)((uintptr_t)veh + 0x6C) |= 32;

							*(int*)((uintptr_t)veh + 0x358) = 1;

							uintptr_t vehclass = *(uintptr_t*)(veh);

							Init_Vehicle_Model = (_Init_Vehicle_Model)(*(uintptr_t*)(vehclass + 0x168));
							Init_Vehicle_Model(veh);

							uintptr_t vehmodel = *(uintptr_t*)(veh + 0x58);
							uintptr_t vehmodel_class = *(uintptr_t*)(vehmodel);

							Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(*(uintptr_t*)(vehmodel_class + 0x670));
							Vehicle_Model_Stuff(vehmodel);
							Vehicle_Model_Stuff2(vehmodel, 3.0f);
							Init_Vehicle_Physics(veh);
						}
					}
				}
				break;
				case BOAT:
				{
					if (uintptr_t veh = Get_Vehicle_Pointer())
					{
						if (Init_CBoat(veh, vehicle, 2))
						{
							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							Update_CBoat_Transform(veh + 0x38, veh);

							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							*(int*)((uintptr_t)veh + 0x6C) &= -217;
							*(int*)((uintptr_t)veh + 0x6C) |= 32;
							*(BYTE*)((uintptr_t)veh + 0x315) |= 8;
							*(WORD*)((uintptr_t)veh + 0x21E) = 0;
							*(float*)((uintptr_t)veh + 0x224) = 20.0f;
							*(BYTE*)((uintptr_t)veh + 0x228) = 20;

							uintptr_t vehclass = *(uintptr_t*)(veh);

							Init_Vehicle_Model = (_Init_Vehicle_Model)(*(uintptr_t*)(vehclass + 0x168));
							Init_Vehicle_Model(veh);

							uintptr_t vehmodel = *(uintptr_t*)(veh + 0x58);
							uintptr_t vehmodel_class = *(uintptr_t*)(vehmodel);

							Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(*(uintptr_t*)(vehmodel_class + 0x670));
							Vehicle_Model_Stuff(vehmodel);
							Vehicle_Model_Stuff2(vehmodel, 3.0f);
							Init_Vehicle_Physics(veh);
						}
					}
				}
				break;
				case HELICOPTER:
				{
					if (uintptr_t veh = Get_Vehicle_Pointer())
					{
						if (Init_CHelicopter(veh, vehicle, 1))
						{
							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							Update_CVehicle_Transform(veh + 0x8, 0, 0, 3.49f);

							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							*(int*)((uintptr_t)veh + 0x6C) &= -217;
							*(int*)((uintptr_t)veh + 0x6C) |= 32;

							BYTE value_ = *(BYTE*)((uintptr_t)veh + 0x315);
							value_ &= 239;
							value_ |= 40;
							*(BYTE*)((uintptr_t)veh + 0x315) = value_;

							*(BYTE*)((uintptr_t)veh + 0x400) = 0;
							*(BYTE*)((uintptr_t)veh + 0x423) = 0;

							uintptr_t vehclass = *(uintptr_t*)(veh);

							Init_Vehicle_Model = (_Init_Vehicle_Model)(*(uintptr_t*)(vehclass + 0x168));
							Init_Vehicle_Model(veh);

							uintptr_t vehmodel = *(uintptr_t*)(veh + 0x58);
							uintptr_t vehmodel_class = *(uintptr_t*)(vehmodel);

							Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(*(uintptr_t*)(vehmodel_class + 0x670));
							Vehicle_Model_Stuff(vehmodel);
							Vehicle_Model_Stuff2(vehmodel, 3.0f);
							Init_Vehicle_Physics(veh);
						}
					}
				}
				break;
				case BIKE:
				{
					if (uintptr_t veh = Get_Vehicle_Pointer())
					{
						if (Init_CBike(veh, vehicle, 1))
						{
							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							Update_CBike_Transform(veh + 0x8, 0, 0, 3.49f);

							*(float*)((uintptr_t)veh + 0x38) = playerPos.X + Cam_XVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x3C) = playerPos.Y + Cam_YVec * 5.0f;
							*(float*)((uintptr_t)veh + 0x40) = playerPos.Z + 4.0f;

							*(int*)((uintptr_t)veh + 0x6C) &= -217;
							*(int*)((uintptr_t)veh + 0x6C) |= 32;

							uintptr_t vehclass = *(uintptr_t*)(veh);

							Init_Vehicle_Model = (_Init_Vehicle_Model)(*(uintptr_t*)(vehclass + 0x168));
							Init_Vehicle_Model(veh);

							uintptr_t vehmodel = *(uintptr_t*)(veh + 0x58);
							uintptr_t vehmodel_class = *(uintptr_t*)(vehmodel);

							Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(*(uintptr_t*)(vehmodel_class + 0x670));
							Vehicle_Model_Stuff(vehmodel);
							Vehicle_Model_Stuff2(vehmodel, 3.0f);
							Init_Vehicle_Physics(veh);
						}
					}
				}
				break;
				}
			}
		}
	}
}

enum WEAPONS
{
	FIST,
	BRASS_KNUCKLES,
	SCREWDRIVER,
	GOLF_CLUB,
	NITESTICK,
	KNIFE,
	BASEBALL_BAT,
	HAMMER,
	MEAT_CLEAVER,
	MACHETE,
	KATANA,
	CHAINSAW,
	GRENADE,
	REMOTE_GRENADE,
	TEARGAS,
	MOLOTOV_COCKTAIL,
	ROCKET,
	COLT_45,
	PYTHON,
	SHOTGUN,
	SPAZ_SHOTGUN,
	STUBBY_SHOTGUN,
	TEC9,
	UZI,
	INGRAM,
	MP5,
	M4,
	KRUGER,
	SNIPER_RIFLE,
	LASER_SNIPER,
	RPG,
	FLAME_THROWER,
	M60,
	MINIGUN,
	CAMERA = 36,
};

// Give Weapon
typedef void(__fastcall* _Give_Weapon)(uintptr_t ped, WEAPONS ID, int ammo);
_Give_Weapon Give_Weapon;

void Grant_Player_Weapon(WEAPONS weapon)  // grants each individual weapon
{
	uintptr_t playerObj = Get_Player_Object2();  // player only gameObject
	uintptr_t re_ptr = *reinterpret_cast<uintptr_t*>(moduleBase + 0x539EED8);

	if (playerObj && re_ptr)
	{
		switch (weapon)
		{
		case FIST:
			Request_Model(-1, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, FIST, 1);
			break;
		case BRASS_KNUCKLES:
			Request_Model(259, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, BRASS_KNUCKLES, 1);
			Destroy_Model(259);
			break;
		case SCREWDRIVER:
			Request_Model(260, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, SCREWDRIVER, 1);
			Destroy_Model(260);
			break;
		case GOLF_CLUB:
			Request_Model(261, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, GOLF_CLUB, 1);
			Destroy_Model(261);
			break;
		case NITESTICK:
			Request_Model(262, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, NITESTICK, 1);
			Destroy_Model(262);
			break;
		case KNIFE:
			Request_Model(263, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, KNIFE, 1);
			Destroy_Model(263);
			break;
		case BASEBALL_BAT:
			Request_Model(264, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, BASEBALL_BAT, 1);
			Destroy_Model(264);
			break;
		case HAMMER:
			Request_Model(265, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, HAMMER, 1);
			Destroy_Model(265);
			break;
		case MEAT_CLEAVER:
			Request_Model(266, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, MEAT_CLEAVER, 1);
			Destroy_Model(266);
			break;
		case MACHETE:
			Request_Model(267, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, MACHETE, 1);
			Destroy_Model(267);
			break;
		case KATANA:
			Request_Model(268, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, KATANA, 1);
			Destroy_Model(268);
			break;
		case CHAINSAW:
			Request_Model(269, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, CHAINSAW, 1);
			Destroy_Model(269);
			break;
		case GRENADE:
			Request_Model(270, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 10);
			Destroy_Model(270);
			break;
		case REMOTE_GRENADE:
			Request_Model(270, 1, 0);
			Request_Model(291, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 10);
			Destroy_Model(270);
			Destroy_Model(291);
			break;
		case TEARGAS:
			Request_Model(271, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 10);
			Destroy_Model(271);
			break;
		case MOLOTOV_COCKTAIL:
			Request_Model(272, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 10);
			Destroy_Model(272);
			break;
		case ROCKET:
			Request_Model(273, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 5);
			Destroy_Model(273);
			break;
		case COLT_45:
			Request_Model(274, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 100);
			Destroy_Model(274);
			break;
		case PYTHON:
			Request_Model(275, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 40);
			Destroy_Model(275);
			break;
		case SHOTGUN:
			Request_Model(277, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 50);
			Destroy_Model(277);
			break;
		case SPAZ_SHOTGUN:
			Request_Model(278, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 30);
			Destroy_Model(278);
			break;
		case STUBBY_SHOTGUN:
			Request_Model(279, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 25);
			Destroy_Model(279);
			break;
		case TEC9:
			Request_Model(281, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 150);
			Destroy_Model(281);
			break;
		case UZI:
			Request_Model(282, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 100);
			Destroy_Model(282);
			break;
		case INGRAM:
			Request_Model(283, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 100);
			Destroy_Model(283);
			break;
		case MP5:
			Request_Model(284, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 100);
			Destroy_Model(284);
			break;
		case M4:
			Request_Model(280, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 150);
			Destroy_Model(280);
			break;
		case KRUGER:
			Request_Model(276, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 120);
			Destroy_Model(276);
			break;
		case SNIPER_RIFLE:
			Request_Model(285, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 25);
			Destroy_Model(285);
			break;
		case LASER_SNIPER:
			Request_Model(286, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 21);
			Destroy_Model(286);
			break;
		case RPG:
			Request_Model(287, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 5);
			Destroy_Model(287);
			break;
		case FLAME_THROWER:
			Request_Model(288, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 200);
			Destroy_Model(288);
			break;
		case M60:
			Request_Model(289, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 500);
			Destroy_Model(289);
			break;
		case MINIGUN:
			Request_Model(290, 1, 0);
			Request_Model(294, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 500);
			Destroy_Model(290);
			Destroy_Model(294);
			break;
		case CAMERA:
			Request_Model(292, 1, 0);
			Load_Requested_Model(Resource_Ptr(re_ptr), true);
			Load_Model_Now(re_ptr);
			Give_Weapon(playerObj, weapon, 100);
			Destroy_Model(292);
			break;
		default:
			break;
		}
	}
}

void PROFESSIONALTOOLS()
{

	uintptr_t playerObj = Get_Player_Object2();  // player only gameObject
	uintptr_t re_ptr = *reinterpret_cast<uintptr_t*>(moduleBase + 0x539EED8);

	if (playerObj && re_ptr)
	{
		Request_Model(-1, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::FIST, 1);
		
		Request_Model(268, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::KATANA, 1);
		Destroy_Model(268);

		Request_Model(270, 1, 0);
		Request_Model(291, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::REMOTE_GRENADE, 10);
		Destroy_Model(270);
		Destroy_Model(291);

		Request_Model(275, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::PYTHON, 40);
		Destroy_Model(275);

		Request_Model(283, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::INGRAM, 100);
		Destroy_Model(283);

		Request_Model(280, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::M4, 150);
		Destroy_Model(280);

		Request_Model(279, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::STUBBY_SHOTGUN, 25);
		Destroy_Model(279);

		Request_Model(286, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::LASER_SNIPER, 21);
		Destroy_Model(286);

		Request_Model(287, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::RPG, 5);
		Destroy_Model(287);
	}
}

void NUTTERTOOLS()
{
	uintptr_t playerObj = Get_Player_Object2();  // player only gameObject
	uintptr_t re_ptr = *reinterpret_cast<uintptr_t*>(moduleBase + 0x539EED8);

	if (playerObj && re_ptr)
	{
		Request_Model(-1, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::FIST, 1);
		
		Request_Model(269, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::CHAINSAW, 1);
		Destroy_Model(269);

		Request_Model(270, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::GRENADE, 10);
		Destroy_Model(270);

		Request_Model(275, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::PYTHON, 40);
		Destroy_Model(275);

		Request_Model(284, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::MP5, 100);
		Destroy_Model(284);

		Request_Model(280, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::M4, 150);
		Destroy_Model(280);

		Request_Model(278, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::SPAZ_SHOTGUN, 30);
		Destroy_Model(278);

		Request_Model(286, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::LASER_SNIPER, 21);
		Destroy_Model(286);

		Request_Model(290, 1, 0);
		Request_Model(294, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::MINIGUN, 500);
		Destroy_Model(290);
		Destroy_Model(294);
	}
}

void THUGSTOOLS()
{
	uintptr_t playerObj = Get_Player_Object2();  // player only gameObject
	uintptr_t re_ptr = *reinterpret_cast<uintptr_t*>(moduleBase + 0x539EED8);

	if (playerObj && re_ptr)
	{
		Request_Model(259, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::BRASS_KNUCKLES, 1);
		Destroy_Model(259);

		Request_Model(264, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::BASEBALL_BAT, 1);
		Destroy_Model(264);

		Request_Model(272, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::MOLOTOV_COCKTAIL, 10);
		Destroy_Model(272);

		Request_Model(274, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::COLT_45, 100);
		Destroy_Model(274);

		Request_Model(281, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::TEC9, 150);
		Destroy_Model(281);

		Request_Model(276, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::KRUGER, 120);
		Destroy_Model(276);

		Request_Model(277, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::SHOTGUN, 50);
		Destroy_Model(277);

		Request_Model(285, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::SNIPER_RIFLE, 25);
		Destroy_Model(285);

		Request_Model(288, 1, 0);
		Load_Requested_Model(Resource_Ptr(re_ptr), true);
		Load_Model_Now(re_ptr);
		Give_Weapon(playerObj, WEAPONS::FLAME_THROWER, 200);
		Destroy_Model(288);
	}
}

void Change_Weather(int selectedItem3)
{
	int* weather_ptr1 = reinterpret_cast<int*>(moduleBase + 0x50CA35C);
	int* weather_ptr2 = reinterpret_cast<int*>(moduleBase + 0x50CD400);
	int* weather_ptr3 = reinterpret_cast<int*>(moduleBase + 0x50CCFC4);
		
	if (weather_ptr1 && weather_ptr2 && weather_ptr3)
	{
		switch (selectedItem3)
		{
		case 0:  // Clear
			*weather_ptr1 = 4;
			*weather_ptr2 = 4;
			*weather_ptr3 = 4;
			break;
		case 1:  // Overcast
			*weather_ptr1 = 1;
			*weather_ptr2 = 1;
			*weather_ptr3 = 1;
			break;
		case 2:  // Rain/Lightning
			*weather_ptr1 = 2;
			*weather_ptr2 = 2;
			*weather_ptr3 = 2;
			break;
		case 3:  // Fog
			*weather_ptr1 = 3;
			*weather_ptr2 = 3;
			*weather_ptr3 = 3;
			break;
		case 4:  // Hurricane
			*weather_ptr1 = 5;
			*weather_ptr2 = 5;
			*weather_ptr3 = 5;
			break;
		}
	}
}

HWND Get_Game_HWND()
{
	HWND hwnd_ret = *(HWND*)((uintptr_t)moduleBase + 0x53711D0);
	if (hwnd_ret)
	{
		hwnd_ret = *(HWND*)((uintptr_t)hwnd_ret + 0x48);
		if (hwnd_ret)
		{
			hwnd_ret = *(HWND*)((uintptr_t)hwnd_ret + 0x928);
			hwnd_ret = *(HWND*)((uintptr_t)hwnd_ret + 0x28);
			return hwnd_ret;
		}
	}
	return NULL;
}

// Setup | Hook into rendering process of DirectX | Retrieve function pointer to 'present' function which is used for DirectX apps
typedef long(__stdcall* present)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT); //New
present p_present;
present p_present_target;
ResizeBuffers p_resizebuffers; //New
ResizeBuffers p_resizebuffers_target; //New
HWND window = NULL;
bool get_present_pointer()
{
	// Godmode cheat hook
	uintptr_t module_address0 = moduleBase + 0x108B2D0;
	JumpBack0 = module_address0 + 15; // Address + length
	Far_Hook((void*)module_address0, (void*)Godmode, 15);
	// Prevent dying when car explodes cheat hook
	//uintptr_t module_address6 = moduleBase + 0x1060FD0;
	//JumpBack6 = module_address6 + 15; // Address + length
	//Far_Hook((void*)module_address6, (void*)Godmode_vehicle, 15);
	// Enable ability to get out of car when you die in exploding car hook
	//uintptr_t module_address7 = moduleBase + 0x1015543;
	//JumpBack7 = module_address7 + 17; // Address + length
	//Far_Hook((void*)module_address7, (void*)Godmode_vehicle2, 17);

	// Super Damage / One Hit Kill cheat hook
	uintptr_t module_address1 = moduleBase + 0x108B2DF;
	JumpBack1 = module_address1 + 18; // Address + length
	Far_Hook((void*)module_address1, (void*)OneHitKill, 18);
	
	// wanted 1 (prevents Wanted stars from pointing gun at cops and trying to jack their car) cheat hook
	uintptr_t module_address2 = moduleBase + 0x102D3A0;
	JumpBack2 = module_address2 + 15; // Address + length
	Far_Hook((void*)module_address2, (void*)wanted1, 15);
	// wanted level call
	Wanted_Level = (_Wanted_Level)(moduleBase + 0x102D530);
	
	// Free Mouse Cursor hook
	uintptr_t module_address3 = moduleBase + 0x1B2AC40;
	JumpBack3 = module_address3 + 15; // Address + length
	Far_Hook((void*)module_address3, (void*)FreeCursor, 15);

	// Infinite Ammo cheat hook
	uintptr_t module_address4 = moduleBase + 0x1136876;
	JumpBack4 = module_address4 + 19; // Address + length
	Far_Hook((void*)module_address4, (void*)InfiniteAmmo, 19);

	// Freeze Mission Timer cheat hook
	uintptr_t module_address5 = moduleBase + 0xFF8356;
	JumpBack5 = module_address5 + 18; // Address + length
	Far_Hook((void*)module_address5, (void*)MissionTimer, 18);
	
	// get Waypoints cheat hook
	uintptr_t module_address8 = moduleBase + 0x112EC89;
	JumpBack8 = module_address8 + 20; // Address + length
	Far_Hook((void*)module_address8, (void*)Waypoints, 20);

	// for calling Give Weapon from Main Thread hook
	uintptr_t module_address9 = moduleBase + 0x10059E0;
	JumpBack9 = module_address9 + 15; // Address + length
	Far_Hook((void*)module_address9, (void*)MainThread, 15);

	// Can't fall off bike hook
	uintptr_t module_address10 = moduleBase + 0x10FCCD1;
	JumpBack10 = module_address10 + 18; // Address + length
	Far_Hook((void*)module_address10, (void*)AlwaysOnBike, 18);

	// Fix Car call
	Fix_Vehicle = (_Fix_Vehicle)(moduleBase + 0x10F7950);
	// Vehicle Color Changer call
	Color_Changer = (_Color_Changer)(moduleBase + 0x11223F0);
	// Vehicle Material Color Changer call
	Material_Color_Changer = (_Material_Color_Changer)(moduleBase + 0x311CFB0);

	// Request Model call
	Request_Model = (_Request_Model)(moduleBase + 0x10C2BD0);
	// Get Resource Pointer
	uintptr_t func1 = *(uintptr_t*)(moduleBase + 0x0404DD08 + 0x158);
	Resource_Ptr = (_Resource_Ptr)(func1);
	// Load Requested Model call
	Load_Requested_Model = (_Load_Requested_Model)(moduleBase + 0x3488100);
	// Load All Models Now call
	Load_Model_Now = (_Load_Model_Now)(moduleBase + 0xA773F0);
	// Give Weapon call
	Give_Weapon = (_Give_Weapon)(moduleBase + 0x1057DC0);
	// Mark Model as no longer needed call
	Destroy_Model = (_Destroy_Model)(moduleBase + 0x10C5C80);
	
			// call vehicle spawn functions //
	// get vehicle pointer
	Get_Vehicle_Pointer = (_Get_Vehicle_Pointer)(moduleBase + 0x111A920);
	// Init_Vehicle_Model
	Init_Vehicle_Model = (_Init_Vehicle_Model)(moduleBase + 0);
	// Vehicle Model Stuff
	Vehicle_Model_Stuff = (_Vehicle_Model_Stuff)(moduleBase + 0);
	// Vehicle Model Stuff 2
	Vehicle_Model_Stuff2 = (_Vehicle_Model_Stuff2)(moduleBase + 0xAAA720);
	// Init Vehicle Physics
	Init_Vehicle_Physics = (_Init_Vehicle_Physics)(moduleBase + 0x102E3B0);
				// call car spawn functions //
	// Init CVehicle
	Init_CVehicle = (_Init_CVehicle)(moduleBase + 0x10E6DD0);
	// Update_CVehicle_Transform
	Update_CVehicle_Transform = (_Update_CVehicle_Transform)(moduleBase + 0xF8A670);
				// call bike spawn functions //
	// Init CBike
	Init_CBike = (_Init_CBike)(moduleBase + 0x10FB9F0);
	// Update CBike Transform
	Update_CBike_Transform = (_Update_CBike_Transform)(moduleBase + 0xA60940);
			// call boat spawn functions //
	// Init CBike
	Init_CBoat = (_Init_CBoat)(moduleBase + 0x1107720);
	// Update CBike Transform
	Update_CBoat_Transform = (_Update_CBoat_Transform)(moduleBase + 0xFBB100);

	Init_CHelicopter = (_Init_CHelicopter)(moduleBase + 0x1112A30);

	
	// Fly cheat hook
	//uintptr_t module_address1 = moduleBase + 0x100A761;
	//JumpBack1 = module_address1 + 15; // Address + length
	//Far_Hook((void*)module_address1, (void*)Fly, 15);
	
	//window = Get_Game_HWND();
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = Get_Game_HWND();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGISwapChain* swap_chain;
	ID3D11Device* device;

	const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		feature_levels,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		nullptr,
		nullptr) == S_OK)
	{
		void** p_vtable = *reinterpret_cast<void***>(swap_chain);
		swap_chain->Release();
		device->Release();
		//context->Release();
		p_present_target = (present)p_vtable[8];
		p_resizebuffers_target = (ResizeBuffers)p_vtable[13]; //New
		return true;
	}
	return false;
}

// ImGui event handling | message handling | allows interaction with the window's message loop
WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool ctrlKeyPressed = false;
	static bool F2Pressed = false;
	static bool F3Pressed = false;
	static bool F4Pressed = false;
	static bool F5Pressed = false;

	/*if (show == WM_KEYDOWN)
	{
		if (wParam == VK_INSERT)
		{
			show = !show;
		}
	}*/

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (show)
	{
		switch (uMsg)
		{
			case WM_KEYDOWN:
			{
				return 0;
			}

			case WM_LBUTTONDOWN:
			{
				return 0;
			}

			case WM_LBUTTONUP:
			{
				return 0;
			}

			case WM_LBUTTONDBLCLK:
			{
				return 0;
			}

			case WM_RBUTTONDOWN:
			{
				return 0;
			}

			case WM_RBUTTONUP:
			{
				return 0;
			}

			case WM_RBUTTONDBLCLK:
			{
				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				return 0;
			}

			case WM_MOUSEMOVE:
			{
				return 0;
			}

			case WM_INPUT:
			{
				return 0;
			}
		}
	}
	else
	{
		switch (uMsg)
		{
			case WM_KEYDOWN:
			{
				if (wParam == VK_CONTROL)
				{
					ctrlKeyPressed = true;
				}
				else if (ctrlKeyPressed)
				{
					if (wParam == VK_F2)
					{
						SaveLocation();
					}
					else if (wParam == VK_F3)
					{
						LoadLocation();
					}
					else if (wParam == VK_F4)
					{
						UndoLocation();
					}
					else if (wParam == VK_F5)
					{
						WaypointTeleport();
					}
				}
			}
			break;

			case WM_KEYUP:
			{
				if (wParam == VK_CONTROL)
				{
					ctrlKeyPressed = false;
				}
				else if (wParam == VK_F2)
				{
					F2Pressed = false;
				}
				else if (wParam == VK_F3)
				{
					F3Pressed = false;
				}
				else if (wParam == VK_F4)
				{
					F4Pressed = false;
				}
				else if (wParam == VK_F5)
				{
					F5Pressed = false;
				}
				break;
			}
		}
	}

	const auto result = CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	return result;
}

//void InputHandler()
//{
//	for (int i = 0; i < 5; i++) {
//		ImGui::GetIO().MouseDown[i] = false;
//	}
//
//	int Button = -1;
//	if (GetAsyncKeyState(VK_LBUTTON)) {
//		Button = 0;
//	}
//
//	if (Button != -1) {
//		ImGui::GetIO().MouseDown[Button] = true;
//	}
//}

// to align ImGui text
void align_text(const char* text, float width_avail, float padding = 0)
{
	ImVec2 size = ImGui::CalcTextSize(text);
	ImGui::SameLine(((width_avail / 1.1f) - (size.x / 1.1f)) + padding);
	ImGui::Text(text);
}

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

WEAPONS weapontogive = FIST;
VEHICLES vehicletospawn = LANDSTALKER;
__declspec(align(32)) ImVec4 primary_color(0, 0, 0, 0);
__declspec(align(32)) ImVec4 secondary_color(0, 0, 0, 0);
int selected_veh_color = 0;
int giveweapon_counter{};
int giveweapon_counter2{};
int giveweapon_counter3{};
int giveweapon_counter4{};
int spawnvehicle_counter{};
int primary_material_counter{};
int secondary_material_counter{};
int veh_colorchange_counter{};

// Prepares for rendering | initializes ImGui | intercept rendering calls, handle ImGui rendering
bool init = false;

ID3D11Device* p_device = NULL;
ID3D11DeviceContext* p_context = NULL;
ID3D11RenderTargetView* mainRenderTargetView = NULL;

static long __stdcall detour_present(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
{
	if (!init)
	{
		if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device)))
		{
			p_device->GetImmediateContext(&p_context);
			DXGI_SWAP_CHAIN_DESC sd;
			p_swap_chain->GetDesc(&sd);
			window = sd.OutputWindow;

			if (mainRenderTargetView == nullptr)
			{
				ID3D11Texture2D* pBackBuffer;
				p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
				p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
				pBackBuffer->Release();
			}

			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SegoeUI.ttf", 35.f);
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(p_device, p_context);
			init = true;
		}
		else
			return p_present(p_swap_chain, sync_interval, flags);
	}
	
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		show = !show;
	}
		
	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD0) & 1)
		{
			Godmode_flag = !Godmode_flag;  // Godmode hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			ohk_flag = !ohk_flag;  // Super Damage / One Hit Kill hotkey
		}
	}

	if (stamina_flag)
	{
		uintptr_t stamina_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8F4 });
		uintptr_t max_stamina_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8F8 });
		if (stamina_ptr && max_stamina_ptr)  // check if null ptr
		{
			*(float*)((uintptr_t)stamina_ptr) = *(float*)max_stamina_ptr;
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			uintptr_t stamina_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8F4 });
			if (stamina_ptr)  // check if null ptr
			{
				staminaValue = *(float*)((uintptr_t)stamina_ptr);
			}

			stamina_flag = !stamina_flag;  // Infinite Stamina hotkey
		}
	}
		
	if (wanted_flag)  // used to freeze wanted
	{
		uintptr_t wanted_level_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x20 });  // Wanted Level stars
		uintptr_t chaos_meter_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x0 });  // Chaos Meter
		if (wanted_level_ptr && chaos_meter_ptr)  // check if null ptr
		{
			*(DWORD*)((uintptr_t)wanted_level_ptr) = wantedLevel;
			*(DWORD*)((uintptr_t)chaos_meter_ptr) = chaosLevel;
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			uintptr_t wanted_level_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x20 });  // Wanted Level stars
			uintptr_t chaos_meter_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x0 });  // Chaos Meter
			if (wanted_level_ptr && chaos_meter_ptr)
			{
				wantedLevel = *(DWORD*)((uintptr_t)wanted_level_ptr);
				chaosLevel = *(DWORD*)((uintptr_t)chaos_meter_ptr);
			}

			wanted_flag = !wanted_flag;  // Freeze Wanted Level hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD4) & 1)
		{
			timer1_flag = !timer1_flag;  // Freeze Mission Timer hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD5) & 1)
		{
			ammo_flag = !ammo_flag;  // Infinite Ammo hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD6) & 1)
		{
			toggle1 = !toggle1;  // Player Fly/No Clip hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD7) & 1)
		{
			toggle2 = !toggle2;  // Vehicle Fly/No Clip hotkey
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD8) & 1)
		{
			Repair_Vehicle();  // Repair Vehicle hotkey
		}
	}

	if (vehicleHP_flag)
	{
		uintptr_t vehicleBase = *reinterpret_cast<uintptr_t*>(moduleBase + 0x5189090);
		if (vehicleBase)
		{
			uintptr_t vehClass = *reinterpret_cast<uintptr_t*>(vehicleBase + 0x0);

			if (vehClass == moduleBase + 0x404B310) // cars / helicopters
			{
				*(float*)(vehicleBase + 0x320) = 1000.0f;
				*(bool*)(vehicleBase + 0x3E5) = 0;
				*(bool*)(vehicleBase + 0x3E6) = 0;
				*(bool*)(vehicleBase + 0x3E7) = 0;
				*(bool*)(vehicleBase + 0x3E8) = 0;
			}
			else if (vehClass == moduleBase + 0x404CC08) // bikes
			{
				*(float*)(vehicleBase + 0x320) = 1000.0f;
				*(bool*)(vehicleBase + 0x47C) = 0;
				*(bool*)(vehicleBase + 0x47D) = 0;
			}
			else if (vehClass == moduleBase + 0x404CA98) // boats
			{
				*(float*)(vehicleBase + 0x320) = 1000.0f;
			}
		}
	}

	if (!GetAsyncKeyState(VK_MENU) & 1)
	{
		if (GetAsyncKeyState(VK_NUMPAD9) & 1)
		{
			uintptr_t vehicleHP_ptr = mem::FindAddress(moduleBase + 0x5189090, { 0x320 });
			if (vehicleHP_ptr)  // check if null ptr
			{
				vehicleHP = *(float*)((uintptr_t)vehicleHP_ptr);
			}

			vehicleHP_flag = !vehicleHP_flag;  // Infinite Vehicle Health hotkey
		}
	}
				
	if (toggle1)
	{
		PlayerFly_Function();
	}

	if (toggle2)
	{
		VehicleFly_Function();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::GetIO().MouseDrawCursor = show;
	if (show == true)
	{
		//InputHandler();
		//ImGui::GetIO().MouseDrawCursor = true;
		//ImGui::GetIO().WantCaptureMouse = true;
		ImGui::SetNextWindowSize(ImVec2(900, 850),ImGuiCond_FirstUseEver);  // sets default window size (width, height)

		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(222, 0, 0).Value);  // push red
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImColor(255, 255, 255, 180).Value);  // push transparent grey
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 210).Value);  // push transparent black
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImColor(133, 0, 0, 180).Value);  // push red
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImColor(60, 60, 60, 180).Value);  // push darker grey)
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImColor(90, 90, 90, 180).Value);  // push lighter grey)
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowTitleAlign = ImVec2(0.5, 0.5);
		
		if (ImGui::Begin("Grand Theft Auto: Vice City - The Definitive Edition (Trainer)", NULL, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PopStyleColor(6);
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255).Value);  // push white
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImColor(222, 0, 0).Value);  // push red
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(80, 80, 80, 170).Value);  // push darker grey
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor(40, 40, 40).Value);  // push darker grey
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(100, 100, 100).Value);  // push light grey
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImColor(133, 0, 0, 220).Value);  // push darker red
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImColor(133, 0, 0, 255).Value);  // push red
			ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImColor(133, 0, 0).Value);  // push red
			ImGui::PushStyleColor(ImGuiCol_Tab, ImColor(70, 70, 70, 200).Value);  // push grey
			ImGui::PushStyleColor(ImGuiCol_TabActive, ImColor(200, 0, 0, 180).Value);  // push red
			ImGui::PushStyleColor(ImGuiCol_TabHovered, ImColor(133, 0, 0, 130).Value);  // push dark red
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor(60, 60, 60, 200).Value);  // push dark grey
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(200, 0, 0, 180).Value);  // push brighter red
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(90, 90, 90).Value);  // push lighter grey
			ImGui::PushStyleColor(ImGuiCol_Header, ImColor(46, 46, 46, 200).Value);  // push grey
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImColor(200, 0, 0, 180).Value);  // push red
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImColor(133, 0, 0, 130).Value);  // push dark red

			if (ImGui::BeginTabBar("MyTabBar"))
			{
				if (ImGui::BeginTabItem("Main"))
				{
					ImGui::Checkbox("Godmode", &Godmode_flag);
					align_text("Numpad 0", ImGui::GetContentRegionAvail().x);
					
					uintptr_t health_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x4DC });
					uintptr_t armor_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x4E0 });
					if (health_ptr && armor_ptr != NULL)
					{
						float health_value = *reinterpret_cast<float*>(health_ptr);
						float armor_value = *reinterpret_cast<float*>(armor_ptr);

						ImGui::SetNextItemWidth(80.0f); // Set the width to 80 pixels
						if (ImGui::InputFloat("Health", &health_value, 0.0f, 0.0f, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							// The value has changed; update the memory
							*reinterpret_cast<float*>(health_ptr) = health_value;
						}

						ImGui::SetNextItemWidth(80.0f);
						if (ImGui::InputFloat("Armor", &armor_value, 0.0f, 0.0f, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							// The value has changed; update the memory
							*reinterpret_cast<float*>(armor_ptr) = armor_value;
						}
					}

					ImGui::Checkbox("Super Damage/One Hit Kill", &ohk_flag);
					align_text("Numpad 1", ImGui::GetContentRegionAvail().x);

					uintptr_t stamina_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8F4 });
					if (ImGui::Checkbox("Infinite Stamina", &stamina_flag))
					{
						if (stamina_ptr)
						{
							staminaValue = *(float*)stamina_ptr;
						}
					}
					align_text("Numpad 2", ImGui::GetContentRegionAvail().x);
																									
					uintptr_t wanted_level_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x20 });  // Wanted Level Stars
					uintptr_t chaos_meter_ptr = mem::FindAddress(moduleBase + 0x4D6CC70, { 0x8E0, 0x0 });  // Chaos Meter
					if (ImGui::Checkbox("Wanted Level", &wanted_flag))
					{
						if (wanted_level_ptr && chaos_meter_ptr)
						{
							wantedLevel = *(DWORD*)wanted_level_ptr;
							chaosLevel = *(DWORD*)chaos_meter_ptr;
						}
					}

					if (wanted_level_ptr != NULL)
					{
						//int wantedLevel = *reinterpret_cast<int*>(wanted_level_ptr);

						ImGui::SameLine();
						ImGui::SetNextItemWidth(300.0f); // Set the width to 300 pixels
						if (ImGui::SliderInt("##WantedLevel", (int*)(wanted_level_ptr), 0, 6))
						{
							Set_Wanted(*(int*)(wanted_level_ptr));
						}
						ImGui::SameLine();
						HelpMarker("Check the box to freeze Wanted Level. Note: If cops don't come after you (act aloof) at first,\nthen attack them or do some sort of crime for it to take effect");
					}
					align_text("Numpad 3", ImGui::GetContentRegionAvail().x);
										
					ImGui::Checkbox("Freeze Mission Timer", &timer1_flag);
					align_text("Numpad 4", ImGui::GetContentRegionAvail().x);

					int* money_ptr = reinterpret_cast<int*>(moduleBase + 0x4D6CD40);
					if (money_ptr)
					{
						int step{ 10000 };
						ImGui::SetNextItemWidth(300.0f); // Set the width to 300 pixels
						ImGui::InputScalar("Money", ImGuiDataType_S32, money_ptr, &step, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue);
					}

					static const char* items3[] = { "Clear", "Overcast", "Rain/Lightning", 
						"Fog", "Hurricane" };
					static int selectedItem3 = 0; // Index of the selected item

					ImGui::Text("Change Weather  ");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(270.0f);
					if (ImGui::Combo("Select an option", &selectedItem3, items3, IM_ARRAYSIZE(items3)))
					{
						Change_Weather(selectedItem3);
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Weapon"))
				{
					ImGui::Checkbox("Infinite Ammo & No Reload", &ammo_flag);
					ImGui::SameLine();
					HelpMarker("Make sure you have at least 1 ammo in your weapon if you're going to use this");
					align_text("Numpad 5", ImGui::GetContentRegionAvail().x);

					static const char* items2[] = { "Fist", "Brass Knuckles", "Screwdriver", "Golf Club", "Night Stick", "Knife",
						"Baseball Bat", "Hammer", "Meat Cleaver", "Machete", "Katana", "Chainsaw", "Grenade", "Remote Detonation Grenades",
						"Tear Gas", "Molotov Cocktails","Rocket", "Colt 45", "Python", "Shotgun", "Spas-12 Shotgun", "Stubby Shotgun",
						"Tec-9", "Uzi", "Ingram", "MP5", "M4", "Kruger", "Sniper Rifle", "Laser Scope Sniper Rifle",
						"Rocket Launcher", "Flame Thrower", "M60", "Minigun", "N/A", "N/A", "Camera" };
					static int selectedWeapon = 0; // Index of the selected item
					
					ImGui::SetNextItemWidth(450.0f);
					if (ImGui::BeginCombo("Select an option", items2[selectedWeapon]))
					{
						bool is_selected = false;

						for (int i = 0; i < IM_ARRAYSIZE(items2); ++i)
						{
							is_selected = false;

							if (i == (int)selectedWeapon)
								is_selected = true;

							if (_stricmp(items2[i], "N/A"))
							{
								if (ImGui::Selectable(items2[i], is_selected))
								{
									selectedWeapon = i;
								}
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					if (ImGui::Button("Give Weapon"))
					{
						weapontogive = (WEAPONS)selectedWeapon;
						giveweapon_counter++;
					}
					ImGui::NewLine();

					if (ImGui::Button("Give PROFESSIONALTOOLS set"))
					{
						giveweapon_counter2++;
					}

					if (ImGui::Button("Give NUTTERTOOLS set"))
					{
						giveweapon_counter3++;
					}

					if (ImGui::Button("Give THUGSTOOLS set"))
					{
						giveweapon_counter4++;
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Fly/Teleport"))
				{
					ImGui::Checkbox("Player Fly/No Clip", &toggle1);
					ImGui::SameLine();
					HelpMarker("Player Fly/No Clip controls: \nForwards - Left Shift key\nUpwards - Spacebar key\nDownwards - Caps Lock key\nSpeed up - T key\nSlow down - G key\n\nBest not to be moving player when launching yourself Upwards from ground\nbecause it adds momentum and messes with the camera. Afterwards, once\nyou are successfully in the air, you can use the hotkeys simultaneously to your liking");
					align_text("Numpad 6", ImGui::GetContentRegionAvail().x);

					ImGui::Checkbox("Vehicle Fly/No Clip", &toggle2);
					ImGui::SameLine();
					HelpMarker("Vehicle Fly/No Clip controls: \nForwards - Left Shift key\nUpwards - Spacebar key\nDownwards - Caps Lock key\nSpeed up - T key\nSlow down - G key\n\nBest not to be moving vehicle when launching yourself Upwards from ground\nbecause it adds momentum and messes with the camera. Afterwards, once\nyou are successfully in the air, you can use the hotkeys simultaneously to your liking");
					align_text("Numpad 7", ImGui::GetContentRegionAvail().x);

					if (ImGui::Button("Save Location"))
					{
						SaveLocation();
					}
					ImGui::SameLine();
					HelpMarker("Works for vehicle too");
					align_text("Ctrl+F2", ImGui::GetContentRegionAvail().x);

					if (ImGui::Button("Teleport to Saved Location"))
					{
						LoadLocation();
					}
					ImGui::SameLine();
					HelpMarker("Works for vehicle too.\nNote: If you're going to use this with vehicle, then I suggest lifting your vehicle in air\na bit (with Fly/No Clip) before doing so, otherwise your vehicle might flip upon teleport");
					align_text("Ctrl+F3", ImGui::GetContentRegionAvail().x);

					if (ImGui::Button("Undo Teleport"))
					{
						UndoLocation();
					}
					ImGui::SameLine();
					HelpMarker("Works for vehicle too.\nNote: If you're going to use this with vehicle, then I suggest lifting your vehicle in air\na bit (with Fly/No Clip) before doing so, otherwise your vehicle might flip upon teleport");
					align_text("Ctrl+F4", ImGui::GetContentRegionAvail().x);

					if (ImGui::Button("Teleport to Waypoint"))
					{
						WaypointTeleport();
					}
					ImGui::SameLine();
					HelpMarker("Works for vehicle too.\nNote: If you're going to use this with vehicle, then I suggest lifting your vehicle in air\na bit (with Fly/No Clip) before doing so, otherwise your vehicle might flip upon teleport");
					align_text("Ctrl+F5", ImGui::GetContentRegionAvail().x);

					ImGui::NewLine();
					if (ImGui::TreeNode("Coordinates"))
					{
						uintptr_t playerObj = Get_Player_Object();
						if (playerObj)
						{
							float* playerX_ptr = reinterpret_cast<float*>(playerObj + 0x38);
							float* playerY_ptr = reinterpret_cast<float*>(playerObj + 0x3C);
							float* playerZ_ptr = reinterpret_cast<float*>(playerObj + 0x40);
							ImGui::SetNextItemWidth(150.0f);
							ImGui::InputFloat("Player X", playerX_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
							ImGui::SetNextItemWidth(150.0f);
							ImGui::InputFloat("Player Y", playerY_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
							ImGui::SetNextItemWidth(150.0f);
							ImGui::InputFloat("Player Z", playerZ_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);

							ImGui::Spacing();
							ImGui::Spacing();
							ImGui::Spacing();
							ImGui::Spacing();

							if (waypointBase)
							{
								float* waypointX_ptr = reinterpret_cast<float*>(waypointBase + 0x518629C);
								float* waypointY_ptr = reinterpret_cast<float*>(waypointBase + 0x51862A0);
								float* waypointZ_ptr = reinterpret_cast<float*>(waypointBase + 0x51862A4);
								ImGui::SetNextItemWidth(150.0f);
								ImGui::InputFloat("Waypoint X", waypointX_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
								ImGui::SetNextItemWidth(150.0f);
								ImGui::InputFloat("Waypoint Y", waypointY_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
								ImGui::SetNextItemWidth(150.0f);
								ImGui::InputFloat("Waypoint Z", waypointZ_ptr, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
							}
						}

						ImGui::TreePop();
					}

					ImGui::EndTabItem();
				}
				
				if (ImGui::BeginTabItem("Vehicle"))
				{
					if (ImGui::Button("Fix Vehicle"))
					{
						Repair_Vehicle();
					}
					ImGui::SameLine();
					HelpMarker("This will fix any cosmetic and functional damage to your current vehicle. It will not work\nif you've exited your vehicle. Press button or hotkey for it to restore Car/Bike tires as well");
					align_text("Numpad 8", ImGui::GetContentRegionAvail().x);
					
					uintptr_t vehicleHP_ptr = mem::FindAddress(moduleBase + 0x5189090, { 0x320 });
					if (ImGui::Checkbox("Infinite Vehicle Health", &vehicleHP_flag))
					{
						if (vehicleHP_ptr)
						{
							vehicleHP = *(float*)vehicleHP_ptr;
						}
					}
					ImGui::SameLine();
					ImGui::SetNextItemWidth(90.0f);
					if (vehicleHP_ptr)
					{
						if (ImGui::InputFloat("##VehicleHP", (float*)vehicleHP_ptr, 0.0f, 0.0f, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							vehicleHP = *(float*)vehicleHP_ptr;
						}
					}
					ImGui::SameLine();
					HelpMarker("This works for your current vehicle. It will not work if you've exited your vehicle.\nCheck the box to freeze Vehicle Health and restore Car/Bike tires");
					align_text("Numpad 9", ImGui::GetContentRegionAvail().x);

					ImGui::Checkbox("Can't Fall Off Bike", &toggle10);
					
					static const char* items[] = { "Purple", "Green", "Yellow", "Gold", "Orange", "Salmon",
						"Cherry Red", "Red", "Blue", "Bright Blue", "Light Blue", "Navy", "Teal", "Grey", 
						"Black", "White" };
					static int selectedItem = 0; 

					ImGui::Text("Vehicle Color Changer  ");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(230.0f);
					if (ImGui::Combo("Select an option", &selectedItem, items, IM_ARRAYSIZE(items)))
					{
						selected_veh_color = selectedItem;
						veh_colorchange_counter++;
					}
					ImGui::SameLine();
					HelpMarker("Enter a vehicle then pick your color");

					if (ImGui::ColorEdit3("Primary Color Picker", (float*)&primary_color))
					{
						primary_material_counter++;
					}
					if (ImGui::ColorEdit3("Secondary Color Picker", (float*)&secondary_color))
					{
						secondary_material_counter++;
					}

					ImGui::NewLine();
					ImGui::PushID("MyCombo");
					static const char* items4[] = { "LANDSTALKER", "IDAHO", "STINGER", "LINERUNNER",
						"PERENNIAL", "SENTINEL", "RIO", "FIRETRUCK", "TRASHMASTER", "STRETCH",
						"MANANA", "INFERNUS", "VOODOO", "PONY", "MULE", "CHEETAH", "AMBULANCE",
						"FBI WASHINGTON", "MOONBEAM", "ESPERANTO", "TAXI", "WASHINGTON", "BOBCAT",
						"MR WHOOPEE", "BF INJECTION", "HUNTER", "POLICE", "ENFORCER", "SECURICAR",
						"BANSHEE", "PREDATOR", "BUS", "RHINO", "BARRACKS OL", "CUBAN HERMES",
						"HELICOPTER", "ANGEL", "COACH", "CABBIE", "STALLION", "RUMPO", "RC BANDIT",
						"ROMEROS HEARSE", "PACKER", "SENTINEL XS", "ADMIRAL", "SQUALO", "SEA SPARROW",
						"PIZZA BOY", "GANG BURRITO", "AIRTRAIN", "DODO", "SPEEDER", "REEFER", "TROPIC",
						"FLATBED", "YANKEE", "CADDY", "ZEBRA CAB", "TOP FUN", "SKIMMER", "PCJ 600",
						"FAGGIO", "FREEWAY", "RC BARON", "RC RAIDER", "GLENDALE", "OCEANIC", "SANCHEZ",
						"SPARROW", "PATRIOT", "LOVE FIST", "COAST GUARD", "DINGHY", "HERMES", "SABRE",
						"SABRE TURBO", "PHOENIX", "WALTON", "REGINA", "COMET", "DELUXO", "BURRITO",
						"SPAND EXPRESS", "MARQUIS", "BAGGAGE HANDLER", "KAUFMAN CAB", "MAVERICK",
						"VCN MAVERICK", "RANCHER", "FBI RANCHER", "VIRGO", "GREENWOOD", "CUBAN JETMAX",
						"HOTRING RACER", "SANDKING", "BLISTA COMPACT", "POLICE MAVERICK", "BOXVILLE",
						"BENSON", "MESA GRANDE", "RC GOBLIN", "HOTRING RACER 2", "HOTRING RACER 3",
						"BLOODRING BANGER", "BLOODRING BANGER 2", "POLICE CHEETAH" };
					static int selectedItem4 = 0;

					ImGui::Text("Vehicle Spawner  ");
										
					static ImGuiTextFilter filter;
										
					ImGui::SetNextItemWidth(350.0f);
					if (ImGui::BeginCombo("Select an option", items4[selectedItem4]))
					{
						//ImGui::SameLine();
						ImGui::SetNextItemWidth(350.0f);
						ImGui::InputText("Search Vehicle", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf));
						filter.Build();
						for (int n = 0; n < IM_ARRAYSIZE(items4); n++)
						{
							if (filter.PassFilter(items4[n]) || filter.InputBuf[0] == '\0')
							{
								bool is_selected = (selectedItem4 == n);
								if (ImGui::Selectable(items4[n], is_selected))
									selectedItem4 = n;
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					ImGui::PopID();

					if (ImGui::Button("Spawn Vehicle"))
					{
						vehicletospawn = (VEHICLES)(selectedItem4 + 130);
						spawnvehicle_counter++;
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Credits"))
				{
					ImGui::Text("Trainer by Zenbeau & seifmagdi.\nBig thanks to seifmagdi for his continuous support.");
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::PopStyleColor(17);
			ImGui::End();
		}
	}
	ImGui::EndFrame();
	ImGui::Render();

	p_context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	const auto result = p_present(p_swap_chain, sync_interval, flags);
	if (Detach_ImGui)
		Shutdown();
	return result;
}

void OrderStuff()
{
	if (giveweapon_counter > 0)
	{
		Grant_Player_Weapon(weapontogive);  // give individual weapon
		giveweapon_counter--;
	}

	if (giveweapon_counter2 > 0)
	{
		PROFESSIONALTOOLS();
		giveweapon_counter2--;
	}

	if (giveweapon_counter3 > 0)
	{
		NUTTERTOOLS();
		giveweapon_counter3--;
	}

	if (giveweapon_counter4 > 0)
	{
		THUGSTOOLS();
		giveweapon_counter4--;
	}

	if (spawnvehicle_counter > 0)
	{
		Create_Vehicle(vehicletospawn);
		spawnvehicle_counter--;
	}

	if (veh_colorchange_counter > 0)
	{
		Change_VehicleColor(selected_veh_color);
		veh_colorchange_counter--;
	}

	uintptr_t vehicleBase = *reinterpret_cast<uintptr_t*>(moduleBase + 0x5189090);

	if (primary_material_counter > 0)
	{
		if (vehicleBase)
		{
			uintptr_t material = *(uintptr_t*)(vehicleBase + 0x58);
			material = *(uintptr_t*)(material + 0x2D8);
			if (material)
			{
				material = *(uintptr_t*)(material + 0x0);
				if (material)
				{
					uintptr_t material_xF0 = *(uintptr_t*)(material + 0xF0);
					unsigned long value = *(unsigned long*)((uintptr_t)material_xF0);
					__declspec(align(32))
					unknownStruct unkstruct_ = {};
					unkstruct_.field1_ = value;
					Material_Color_Changer(material, unkstruct_, primary_color);
				}
			}
		}
		primary_material_counter--;
	}

	if (secondary_material_counter > 0)
	{
		if (vehicleBase)
		{
			uintptr_t material = *(uintptr_t*)(vehicleBase + 0x58);
			material = *(uintptr_t*)(material + 0x2D8 + 2 * 8);
			if (material)
			{
				material = *(uintptr_t*)(material + 0x0);
				if (material)
				{
					uintptr_t material_xF0 = *(uintptr_t*)(material + 0xF0);
					unsigned long value = *(unsigned long*)((uintptr_t)material_xF0);
					__declspec(align(32))
					unknownStruct unkstruct_ = {};
					unkstruct_.field1_ = value;
					Material_Color_Changer(material, unkstruct_, secondary_color);
				}
			}
		}
		secondary_material_counter--;
	}
}

HRESULT WINAPI detour_resizebuffers(IDXGISwapChain* swapchain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (mainRenderTargetView)
	{
		mainRenderTargetView->Release();
		mainRenderTargetView = nullptr;
	}

	const auto result = p_resizebuffers(swapchain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	swapchain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device);

	if (mainRenderTargetView == nullptr && p_device)
	{
		ID3D11Texture2D* pBackBuffer;
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		p_device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
		pBackBuffer->Release();
	}

	return result;
}

DWORD __stdcall EjectThread(LPVOID lpParameter)
{
	Sleep(750);
	FreeLibraryAndExitThread(dll_handle, 0);
	return 0;
}

void Shutdown()
{
	// Cleanup
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
		return;
	}
	if (MH_Uninitialize() != MH_OK) {
		return;
	}
			
			// Restore bytes back when uninjecting cheat //

	// Godmode cheat hook
	uintptr_t module_address0 = moduleBase + 0x108B2D0;
	setmemory_back((void*)module_address0, (BYTE*)"\x48\x89\x5C\x24\x18\x48\x89\x74\x24\x20\x57\x41\x54\x41\x55", 15);
	// Prevent dying when car explodes hook
	//uintptr_t module_address6 = moduleBase + 0x1060FD0;
	//setmemory_back((void*)module_address6, (BYTE*)"\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x40", 15);
	// Enable ability to get out of car when you die in exploding car hook
	//uintptr_t module_address7 = moduleBase + 0x1015543;
	//setmemory_back((void*)module_address7, (BYTE*)"\x8B\x51\x6C\x81\xE2\xF8\x00\x00\x00\x8D\x42\xD8\xA9\xF7\xFF\xFF\xFF", 17);

	// Super Damage / One Hit Kill cheat hook
	uintptr_t module_address1 = moduleBase + 0x108B2DF;
	setmemory_back((void*)module_address1, (BYTE*)"\x41\x56\x41\x57\x48\x81\xEC\xB0\x00\x00\x00\x0F\xB6\x05\x79\x3D\xF3\x03", 18);

	// wanted1 (prevents Wanted stars from pointing gun at cops) cheat hook
	uintptr_t module_address2 = moduleBase + 0x102D3A0;
	setmemory_back((void*)module_address2, (BYTE*)"\x48\x89\x5C\x24\x08\x57\x48\x83\xEC\x20\x8B\x01\x48\x8B\xD9", 15);
		
	// Free Mouse Cursor hook
	uintptr_t module_address3 = moduleBase + 0x1B2AC40;
	setmemory_back((void*)module_address3, (BYTE*)"\x48\x89\x5C\x24\x10\x48\x89\x74\x24\x18\x57\x48\x83\xEC\x20", 15);

	// Infinite Ammo cheat hook
	uintptr_t module_address4 = moduleBase + 0x1136876;
	setmemory_back((void*)module_address4, (BYTE*)"\x83\x7F\x08\x00\x7F\x29\x44\x8B\x47\x0C\x45\x85\xC0\x0F\x8E\xE8\x05\x00\x00", 19);

	// Freeze Mission Timer cheat hook
	uintptr_t module_address5 = moduleBase + 0xFF8356;
	setmemory_back((void*)module_address5, (BYTE*)"\x43\x89\x8C\x20\xD0\x71\xF7\x04\x79\x1F\x47\x89\xBC\x20\xD0\x71\xF7\x04", 18);

	// get Waypoints cheat hook
	uintptr_t module_address8 = moduleBase + 0x112EC89;
	setmemory_back((void*)module_address8, (BYTE*)"\xF3\x44\x0F\x10\x8C\x11\x9C\x62\x18\x05\xF3\x44\x0F\x10\x84\x11\xA0\x62\x18\x05", 20);

	// for calling Give Weapon from Main Thread hook
	uintptr_t module_address9 = moduleBase + 0x10059E0;
	setmemory_back((void*)module_address9, (BYTE*)"\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18", 15);

	// Can't fall off bike hook
	uintptr_t module_address10 = moduleBase + 0x10FCCD1;
	setmemory_back((void*)module_address10, (BYTE*)"\xC6\x44\x24\x28\x00\x4C\x8B\xCE\xBA\x27\x00\x00\x00\xC6\x44\x24\x20\x00", 18);

	// Fly cheat hook
	//uintptr_t module_address1 = moduleBase + 0x100A761;
	//setmemory_back((void*)module_address1, (BYTE*)"\xF3\x0F\x10\x47\x40\xF3\x0F\x10\x4F\x38\xF3\x0F\x10\x57\x3C", 15);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (mainRenderTargetView) { 
	mainRenderTargetView->Release(); 
	mainRenderTargetView = nullptr; }

	if (p_context) { 
	p_context->Release(); 
	p_context = nullptr; }

	if (p_device) {
	p_device->Release();
	p_device = nullptr; }

	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));
	CreateThread(0, 0, EjectThread, 0, 0, 0);
}

//"main" loop
int WINAPI main()
{
	while (*(HANDLE*)(moduleBase + 0x548AA50) == NULL)
	{
		Sleep(3000);
	}

	Sleep(3000);
		
	if (!get_present_pointer())
	{
		return 1;
	}

	MH_STATUS status = MH_Initialize();
	if (status != MH_OK)
	{
		return 1;
	}

	if (MH_CreateHook(reinterpret_cast<void**>(p_present_target), &detour_present, reinterpret_cast<void**>(&p_present)) != MH_OK) {
		return 1;
	}

	if (MH_CreateHook(reinterpret_cast<void**>(p_resizebuffers_target), &detour_resizebuffers, reinterpret_cast<void**>(&p_resizebuffers)) != MH_OK) {  // new
		return 1;
	}

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		return 1;
	}

	while (true)
	{
		if (GetAsyncKeyState(VK_MENU) & 0x8000 && GetAsyncKeyState(VK_END) & 0x8000)
		{
			Detach_ImGui = true;
			break;
		}
	}
}


BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		dll_handle = hModule;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, NULL, 0, NULL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{

	}
	return TRUE;
}