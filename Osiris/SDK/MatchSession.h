#pragma once

#include "KeyValues.h"

class PlayerLocal {
public:
	int getXuidLow() {
		return *(int*)((DWORD)this + 0x8);
	}
	int getXuidHigh() {
		return *(int*)((DWORD)this + 0xC);
	}
private:
	virtual void unk_1() = 0; //0
	virtual void unk_2() = 0; //1
public:
	virtual const char* getName() = 0; //2
};

class PlayerManager {
private:
	virtual void unk_1() = 0; //0
public:
	virtual PlayerLocal* getLocalPlayer(int un) = 0; //1
};

class MatchSession {
public:
	virtual PlayerManager* getPlayerManager() = 0; //0
	virtual KeyValues* getSessionSettings() = 0; //1
	virtual void UpdateSessionSettings(KeyValues* pSettings) = 0; //2
	virtual void command(KeyValues* command) = 0; //3
	virtual uint64_t getLobbyID() = 0; //4
};