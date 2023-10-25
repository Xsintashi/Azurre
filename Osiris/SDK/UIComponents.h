#pragma once

#include "../Memory.h"

class PartyBrowser {
public:
	inline bool hasBeenInvited(int64_t id) {
		typedef void* (__thiscall* OriginalFn)(PVOID, int64_t);
		return ((OriginalFn)memory->beenInvited)(this, id) != NULL;
	}
};

class PartyList {
private:
	virtual void unk1() = 0;
public:
	virtual int GetCount() = 0;
};