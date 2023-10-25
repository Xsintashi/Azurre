#pragma once
#include "../Singleton.h"

class CSteamID;

class LobbyClass {

public:
	// returns true if message has been overwritten
	bool InterpretLobbyMessage(CSteamID steamIdLobby, const void* pvMsgBody, int cubMsgBody);
private:
	bool OnEnterMatchmakingQueue(CSteamID Lobby, const char* pMessage, const size_t MessageSize);
	bool CallOriginalSendLobbyChatMessage(CSteamID steamIdLobby, const void* pvMsgBody, int cubMsgBody);
	bool OnChatInviteMessage(CSteamID Lobby, const char* pMessage, const size_t MessageSize);
	bool OnUpdated(CSteamID Lobby, const char* pMessage, const size_t MessageSize);
private:
	Singleton(LobbyClass);
};


struct LobbyConfig;

namespace Lobby {
	float &setUpdate() noexcept;
	bool &autoQueue() noexcept;
	bool autoJoin() noexcept;
	bool autoAccept() noexcept;
	bool detectAutoAccept() noexcept;
	std::vector<std::pair<uint64_t, uint64_t>>& getPendingInvites() noexcept;
	void commandSystemScript() noexcept;
	void retardLobby() noexcept;
	void lobbyRankParty() noexcept;
	void startMatchmaking() noexcept;
	void autoDisconnect() noexcept;
	void menuBarItem() noexcept;
	void drawGUI(bool contentOnly) noexcept;
}