#pragma once

#include "Inconstructible.h"
#include "VirtualMethod.h"
#include "BitBuffer.h"
#include "MemAlloc.h"
#include "Pad.h"

struct ClientClass;

class NetworkChannel {
public:
	VIRTUAL_METHOD(const char*, getAddress, 1, (), (this))
	VIRTUAL_METHOD(float, getLatency, 9, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getAvgLatency, 10, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getAvgLoss, 11, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getAvgChoke, 12, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getAvgData, 13, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getAvgPackets, 14, (int flow), (this, flow))
	VIRTUAL_METHOD(float, getTimeoutSeconds, 25, (), (this))
	VIRTUAL_METHOD(bool, sendNetMsg, 40, (void* msg, bool forceReliable = false, bool voice = false), (this, msg, forceReliable, voice))

	std::byte pad[24];
	int outSequenceNr;
	int inSequenceNr;
	int outSequenceNrAck;
	int outReliableState;
	int inReliableState;
	int chokedPackets;
};

class NetworkMessage
{
public:
	VIRTUAL_METHOD(int, getType, 7, (), (this))
};

struct clMsgMove
{
	clMsgMove() {
		netMessageVtable = *reinterpret_cast<std::uint32_t*>(memory->clSendMove + 126);
		clMsgMoveVtable = *reinterpret_cast<std::uint32_t*>(memory->clSendMove + 138);
		allocatedMemory = *reinterpret_cast<void**>(memory->clSendMove + 131);
		unknown = 15;

		flags = 3;

		unknown1 = 0;
		unknown2 = 0;
		unknown3 = 0;
		unknown4 = 0;
		unknown5 = 0;

	}

	~clMsgMove() {
		memory->clMsgMoveDescontructor(this);
	}

	inline auto setNumBackupCommands(int backupCommands) {
		this->backupCommands = backupCommands;
	}

	inline auto setNumNewCommands(int newCommands) {
		this->newCommands = newCommands;
	}

	inline auto setData(unsigned char* data, int numBytesWritten) {

		flags |= 4;

		//Why does this work??
		if (allocatedMemory == reinterpret_cast<void*>(memory->clSendMove + 131)) {

			void* newMemory = memory->memalloc->Alloc(24);
			if (newMemory) {
				*((unsigned long*)newMemory + 0x14) = 15;
				*((unsigned long*)newMemory + 0x10) = 0;
				*(unsigned char*)newMemory = 0;
			}

			allocatedMemory = newMemory;
		}

		return memory->clMsgMoveSetData(allocatedMemory, data, numBytesWritten);
	}

	std::uint32_t netMessageVtable; // 0x58 88 0
	std::uint32_t clMsgMoveVtable; // 0x54 84 4
	int unknown1; // 0x4c 80 8
	int backupCommands; // 0x4c 76 12
	int newCommands; // 0x48 72 16
	void* allocatedMemory; // 0x44 68 20
	int unknown2; // 0x40 64 24
	int flags; // 0x3c 60 28
	char unknown3; // 0x38 64 32
	PAD(3); // 65 33
	char unknown4; // 0x34 68 36
	PAD(15); // 69 37
	int unknown5; // 0x24 84 52
	int unknown; // 0x20 88 56
	bufferWrite dataOut;
};
