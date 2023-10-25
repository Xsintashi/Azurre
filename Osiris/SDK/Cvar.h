#pragma once

#include "Inconstructible.h"
#include "VirtualMethod.h"

struct ConVar;

class Cvar {
public:
    INCONSTRUCTIBLE(Cvar)

    VIRTUAL_METHOD(ConVar*, findVar, 15, (const char* name), (this, name))
};
class conCommandBase
{
public:
	void* vmt;
	conCommandBase* next;
	bool registered;
	const char* name;
	const char* helpString;
	int flags;
	conCommandBase* conCommandBases;
	void* accessor;
};