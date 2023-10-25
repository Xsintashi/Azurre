#pragma once

#include <type_traits>

#include "Inconstructible.h"
#include "Pad.h"
#include "Platform.h"
#include "UtlVector.h"
#include "VirtualMethod.h"

enum FCVAR
{
	NONE = 0,
	UNREGISTERED = (1 << 0),
	DEVELOPMENTONLY = (1 << 1),
	GAMEDLL = (1 << 2),
	CLIENTDLL = (1 << 3),
	HIDDEN = (1 << 4),
	PROTECTED = (1 << 5),
	SPONLY = (1 << 6),
	ARCHIVE = (1 << 7),
	NOTIFY = (1 << 8),
	USERINFO = (1 << 9),
	CHEAT = (1 << 14),
	PRINTABLEONLY = (1 << 10),
	UNLOGGED = (1 << 11),
	NEVER_AS_STRING = (1 << 12),
	REPLICATED = (1 << 13),
	DEMO = (1 << 16),
	DONTRECORD = (1 << 17),
	NOT_CONNECTED = (1 << 22),
	ARCHIVE_XBOX = (1 << 24),
	SERVER_CAN_EXECUTE = (1 << 28),
	SERVER_CANNOT_QUERY = (1 << 29),
	CLIENTCMD_CAN_EXECUTE = (1 << 30)
};

struct ConVar {
    INCONSTRUCTIBLE(ConVar)

    VIRTUAL_METHOD(float, getFloat, WIN32_LINUX(12, 15), (), (this))
    VIRTUAL_METHOD(int, getInt, WIN32_LINUX(13, 16), (), (this))
    VIRTUAL_METHOD(void, setValue, WIN32_LINUX(14, 17), (const char* value), (this, value))
    VIRTUAL_METHOD(void, setValue, WIN32_LINUX(15, 18), (float value), (this, value))
    VIRTUAL_METHOD(void, setValue, WIN32_LINUX(16, 19), (int value), (this, value))

    PAD(WIN32_LINUX(24, 48))
    std::add_pointer_t<void CDECL_()> changeCallback;
    ConVar* parent;
    const char* defaultValue;
    char* string;
	//int stringLength;
	//float floatValue;
	//int intValue;
	//int hasMin;
	//float minValue;
	//int hasMax;
	//float maxValue; // if u wanna use it, u have to change PAD under
	PAD(WIN32_LINUX(44, 56))
    UtlVector<void(CDECL_*)()> onChangeCallbacks;
};
