
#pragma once

#include "kfglobal.h"

struct KClientInputFrame
{

	enum ClientPacketFlags
	{
		CPF_RequestingSpawn	= 1,
	};

	u32 Frame            = 0;
	u8  Flags            = 0;
	u8  MovementKeyFlags = 0;
	u8  WeaponIndex      = 0;
	u16 Pitch            = 0;
	u16 Yaw              = 0;
	u8  ViewedPlayer     = 255;

	KClientInputFrame() = default;

	void IncludeSpawnRequest() { Flags |= CPF_RequestingSpawn; }
};

// client generate diffs
struct KPackedClientInput
{
	friend class KNetInterface_Server;
private:
	enum
	{
		F_HasFlags	= 1,
		F_HasKeys	= 2,
		F_HasWeapon	= 4,
		F_HasPitch	= 8,
		F_HasYaw	= 16,
		F_HasView   = 32,
	};
	u8 Flags = 0;
	KClientInputFrame InputData;
public:

	void AddFlags()   { Flags |= F_HasFlags; }
	void AddKeys()	  { Flags |= F_HasKeys; }
	void AddWeapon()  { Flags |= F_HasWeapon; }
	void AddPitch()	  { Flags |= F_HasPitch; }
	void AddYaw()     { Flags |= F_HasYaw; }
	void AddView()    { Flags |= F_HasView; }
	void AddAll()     { Flags = 255; }

	void SetInputData(const KClientInputFrame& data) { InputData = data; }
	const KClientInputFrame& GetInputData() const { return InputData; }

	bool HasFlags() const   { return Flags & F_HasFlags; }
	bool HasKeys() const    { return Flags & F_HasKeys; }
	bool HasWeapon() const  { return Flags & F_HasWeapon; }
	bool HasPitch() const   { return Flags & F_HasPitch; }
	bool HasYaw() const	    { return Flags & F_HasYaw; }
	bool HasView() const    { return Flags & F_HasView; }
	bool HasNoFlags() const { return Flags == 0; }

	u8 GetFlags() const { return Flags; }
};

// stored for server
struct KBufferedClientInput
{
	u8 Keys = 0, WeaponIndex = 0;
	u16 Pitch = 0, Yaw = 0;

	// used for ping comp
	u32 ClientCompFrame = 0;
};