#include "Lobby.h"

#include "../SDK/ClientState.h"
#include "../SDK/MatchSession.h"
#include "../SDK/MatchFramework.h"
#include "../SDK/isteamFriends.h"
#include "../SDK/Panorama.h"
#include "../SDK/InputSystem.h"
#include "../SDK/isteamuser.h"
#include "../SDK/UIComponents.h"

#include "Panorama.h"

#include "../Config.h"
#include "../ConfigStructs.h"
#include "../Interfaces.h"
#include "../hooks.h"
#include "../Memory.h"
#include <cmath>
#include <string>
#include <xstring>
#include <regex>

#include "../imguiCustom.h"
#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#define vectorContains(ve, va) (std::find(ve.begin(), ve.end(), va) != ve.end())

//https://github.com/ticzz/Aimware-v5-luas/blob/master/CSGOLobbyTroll.lua

struct LobbyConfig {

	float nukeLobby = 0.0000f; // 0 - 1

	bool spoofMaps = false;
	int spoofedMaps = 0;

	std::string customLobbyLoc{ "PL" };
	std::string hideInvitePlayerName{ "azurre alpha user no.#idx" };
	bool hideInvitePlayerNames = false;

	struct LobbyRanks {
		bool enabled = false;
		int playerID = 0;
		int level = 1;
		int xp = 0;
		int rank = 0;
		int rankType = 0;
		int t1 = 0;
		int t2 = 0;
		int t3 = 0;
		bool prime = true;
		int teamColor = 0;
		std::string medal = "";
		std::string loc = "PL";
	} lobbyRank;

	struct LobbyTroll {
		int message = 0;
		int messageColor = 2;
		int fakeInviteMessage = 0;
		int powerFakeInviteMessage = 1;
		int powerMessage = 1;
		int powerPopUpSpam = 1;
	} troll;

	struct AutoQueue {
		bool enabled = false;
		int minPlayers = 0;
		bool autoDisconnect = false;
		bool autoAccept = false;
		bool detectAutoAccept = false;
	} autoQueue;

	struct AutoInvite {
		bool enabled = false;
		std::vector<uint64_t> steamIDs = std::vector<uint64_t>();
		bool autoJoin = false;
		struct MassInvite {
			bool prime = false;
			int gameType = 0;
			int avgRank = 10;
		}massInvite;
	} autoInvite;

	bool lobbyRankParty = false;
	bool lobbyRetard = false;
	bool commandSystem = false;

}cfgLobby;

constexpr const char* errorMessagePrefix = "#SFUI_QMM_ERROR_";

constexpr const char* errorMessagesTitles[]{
	"Failed to connect to the match.",
	"Not logged in to CS:GO matchmaking service.",
	"VAC was unable to verify your game session.",
	"Cannot begin matchmaking because you have too many *player*s on your team.",
	"Cannot begin matchmaking because the mission requires you to play together with one friend.",
	"Cannot begin matchmaking because your client needs to update, please restart.",
	"Cannot begin matchmaking because your client is running CS:GO beta version, please opt out of CS:GO beta version and restart.",
	"Cannot begin matchmaking because game settings were specified incorrectly.",
	"Cannot begin matchmaking because one or more selected maps are no longer available.",
	"Cannot begin matchmaking because one or more selected maps require an active Operation Pass.",
	"Cannot begin matchmaking because every *player* must have an active Operation Pass to play the selected mode.",
	"Cannot begin matchmaking because the requested mission must first be activated using Operation Journal.",
	"Competitive Skill Groups of *player*s in your party are too high to participate in competitive matchmaking with a *player* who does not have a Skill Group.",
	"Competitive Skill Groups of *player*s in your party are too far apart to participate in competitive matchmaking unless you play with a full team of five *player*s.",
	"Cannot begin matchmaking because you need to have a full team in your matchmaking party.",
	"Cannot begin matchmaking because all party members must be members of the same tournament team.",
	"Cannot begin matchmaking because your tournament team must have 5 *player*s.",
	"Cannot begin matchmaking because your tournament team must configure tournament match settings.",
	"Cannot begin matchmaking because your team is not registered in the tournament.",
	"Cannot begin matchmaking because your team is not registered in the requested tournament.",
	"Cannot begin matchmaking because your team must be one of the teams in match settings.",
	"Cannot begin matchmaking because your team must specify opponents team in match settings.",
	"Cannot begin matchmaking because opponents team specified in match settings is not registered in the tournament.",
	"Failed to allocate a slot in matchmaking pool.",
	"Failed to locate official game servers with acceptable ping.",
	"Failed to accept match.",
	"Failed to connect to Steam Community Group server.",
	"VAC banned.",
	"*player* is not using the Perfect World CS:GO Launcher and cannot play in your party.",
	"*player* is using the Perfect World CS:GO Launcher and cannot play in your party.",
	"*player* has an ongoing match that should be rejoined from the Main Menu.",
	"You have an ongoing match that you should rejoin from the Main Menu.",
	"*player* is not logged in to CS:GO matchmaking service.",
	"You are not logged in to CS:GO matchmaking service.",
	"VAC was unable to verify the game session for *player*.",
	"VAC was unable to verify your game session.",
	"*player* failed to locate official game servers with acceptable ping. Please, try increasing your 'Max Acceptable Matchmaking Ping' in 'Options > Game Settings' and try again.",
	"You failed to locate official game servers with acceptable ping. Please, try increasing your 'Max Acceptable Matchmaking Ping' in 'Options > Game Settings' and try again.",
	"A match was found for you, but *player* failed to accept it.",
	"A match was found for you, but you failed to accept it. You have been removed from the matchmaking queue.",
	"YOU FAILED TO ACCEPT",
	"*player* has VAC ban on record.",
	"You have VAC ban on record.",
	"VAC BANNED",
	"Steam Community Group matchmaking is only available to Steam Community Group members, but *player* is not a member of the requested Steam Community Group.",
	"Steam Community Group matchmaking is only available to Steam Community Group members, but you are not a member of the requested Steam Community Group. You have been removed from the matchmaking queue.",
	"STEAM COMMUNITY GROUP",
	"*player* is in cooldown and cannot participate in competitive matchmaking yet.",
	"You are in cooldown and cannot participate in competitive matchmaking yet.",
	"*player* needs to wait before playing additional competitive matches.",
	"You need to wait before playing additional competitive matches.",
	"*player* has game files with invalid signatures and is not allowed to join VAC secure servers.",
	"*player* must unlock this game mode by completing matches in other game modes. Casual and Deathmatch are good modes to start with.",
	"This game mode unlocks after you complete matches in other game modes. Casual and Deathmatch are good modes to start with.",
	"*player* must gain a higher CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"You must gain a higher CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"*player* must gain at least 'Private Rank 2' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"You must gain at least 'Private Rank 2' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"*player* must gain at least 'Private Rank 3' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"You must gain at least 'Private Rank 3' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"*player* must gain at least 'Private Rank 4' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"You must gain at least 'Private Rank 4' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"*player* must gain at least 'Corporal Rank 5' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"You must gain at least 'Corporal Rank 5' CS:GO Profile Rank before participating in Competitive Matchmaking.",
	"Your matchmaking experience may be slightly impacted because the Trust Factor of *player* is lower than yours.",
	"Your matchmaking experience will be significantly affected because the Trust Factor of *player* is substantially lower than yours.",
	"CS:GO's matchmaking will prioritize finding opponents with a similar substantially lower Trust Factor.",
	"Matchmaking was unable to find a match for you, so your search has been canceled.",
	"*player* must restart their game before playing on official servers."
};

constexpr const char* errorMessages[] {
	"NoOngoingMatch",
	"NotLoggedIn",
	"NotVacVerified",
	"PartyTooLarge",
	"PartyRequired2",
	"ClientUpdateRequired",
	"ClientBetaVersionMismatch",
	"InvalidGameMode",
	"UnavailMapSelect",
	"OperationPassInvalid",
	"OperationPassFullTeam",
	"OperationQuestIdInactive",
	"SkillGroupMissing",
	"SkillGroupLargeDelta",
	"FullTeamRequired",
	"TournamentTeamAccounts",
	"TournamentTeamSize",
	"TournamentMatchRequired",
	"TournamentMatchNoEvent",
	"TournamentMatchInvalidEvent",
	"TournamentMatchSetupYourTeam",
	"TournamentMatchSetupSameTeam",
	"TournamentMatchSetupNoTeam",
	"FailedToSetupSearchData",
	"FailedToPingServers",
	"FailedToReadyUp",
	"FailedToVerifyClan",
	"VacBanned",
	"X_PerfectWorldRequired",
	"X_PerfectWorldDenied",
	"X_OngoingMatch",
	"1_OngoingMatch",
	"X_NotLoggedIn",
	"1_NotLoggedIn",
	"X_NotVacVerified",
	"1_NotVacVerified",
	"X_FailPingServer",
	"1_FailPingServer",
	"X_FailReadyUp",
	"1_FailReadyUp",
	"1_FailReadyUp_Title",
	"X_VacBanned",
	"1_VacBanned",
	"1_VacBanned_Title",
	"X_FailVerifyClan",
	"1_FailVerifyClan",
	"1_FailVerifyClan_Title",
	"X_PenaltySeconds",
	"1_PenaltySeconds",
	"X_PenaltySecondsGreen",
	"1_PenaltySecondsGreen",
	"X_InsecureBlocked",
	"X_InsufficientLevelNP",
	"1_InsufficientLevelNP",
	"X_InsufficientLevel",
	"1_InsufficientLevel",
	"X_InsufficientLevel02",
	"1_InsufficientLevel02",
	"X_InsufficientLevel03",
	"1_InsufficientLevel03",
	"X_InsufficientLevel04",
	"1_InsufficientLevel04",
	"X_InsufficientLevel05",
	"1_InsufficientLevel05",
	"X_AccountWarningTrustMinor",
	"X_AccountWarningTrustMajor",
	"X_AccountWarningTrustMajor_Summary",
	"X_SearchTimeExceeded",
	"X_PureFileStateDirty"
};

enum LobbyGameType {
	COMP = 0,
	WIGMAN,
	DANGER_ZONE,
};

float nextUpdate = 0.f;

float &Lobby::setUpdate() noexcept{
	return nextUpdate;
}

bool &Lobby::autoQueue() noexcept {
	return cfgLobby.autoQueue.enabled;
}

bool Lobby::autoJoin() noexcept {
	return cfgLobby.autoInvite.autoJoin;
}
bool Lobby::autoAccept() noexcept {
	return cfgLobby.autoQueue.autoAccept;
}

bool Lobby::detectAutoAccept() noexcept {
	return cfgLobby.autoQueue.detectAutoAccept;
}

std::vector<std::pair<uint64_t, uint64_t>> pendingInvites;

std::vector<std::pair<uint64_t, uint64_t>>& Lobby::getPendingInvites() noexcept{
	return pendingInvites;
}

constexpr int rankType[] = {
	6,7,10
};

constexpr const char* maps[] = {
	"mg_lobby_mapveto",
	"mg_de_ancient",
	"mg_de_anubis",
	"mg_de_inferno",
	"mg_de_mirage",
	"mg_de_nuke",
	"mg_de_overpass",
	"mg_de_vertigo",
	"mg_de_tuscan",
	"mg_de_breach",
	"mg_de_dust2",
	"mg_de_train",
	"mg_de_cache",
	"mg_cs_agency",
	"mg_cs_office"
};

Singleton_CPP(LobbyClass);

bool LobbyClass::CallOriginalSendLobbyChatMessage(CSteamID steamIdLobby, const void* pvMsgBody, int cubMsgBody)
{
	typedef bool(__thiscall* SendLobbyChatMessage_t)(ISteamMatchmaking*, CSteamID, const void*, int);
	static auto original = hooks->steamMatchmakingHook.get_original<SendLobbyChatMessage_t>(26);

	return original(steamMatchmaking, steamIdLobby, pvMsgBody, cubMsgBody);
}

const char* findStringEnd(const char* Message)
{
	while (*Message != 0)
		Message++;
	return Message + 1; // Include null terminator
}

const char* findString(const char* in, const char* pat, unsigned int mxdst = 1000)
{
	for (unsigned int i = 0; i < mxdst; i++)
	{
		if (strcmp(in + i, pat) == 0)
			return in + i;
	}

	return in;
}

bool isLobbyOwner = false;

bool LobbyClass::InterpretLobbyMessage(CSteamID steamIdLobby, const void* pvMsgBody, int cubMsgBody)
{
	const char* pMessage = (const char*)pvMsgBody;
	const char* MessageIterator = pMessage + 5; // 5 Bytes in we have the big SysSession::Command

	auto steamIdOwner = steamMatchmaking->GetLobbyOwner(steamIdLobby);
	auto steamIdMe = steamUser->GetSteamID();

	isLobbyOwner = steamIdOwner.GetAccountID() == steamIdMe.GetAccountID();

	auto DumpFile = std::ofstream("AzurreLobbyChatMessageDump.bin", std::ios::binary | std::ios::app);

	if (DumpFile.is_open())
	{
		DumpFile.write(pMessage, cubMsgBody);
		DumpFile.write("\n", 1);
		DumpFile.close();
	}

	if (strcmp(MessageIterator, "SysSession::Command") == 0)
	{
		MessageIterator = findStringEnd(MessageIterator) + 1;

		if (strcmp(MessageIterator, "Game::EnteringQueue") == 0) //Nuke Bomb
			return OnEnterMatchmakingQueue(steamIdLobby, pMessage, cubMsgBody);
		else if (strcmp(MessageIterator, "Game::ChatInviteMessage") == 0) //Hide invited player name
			return OnChatInviteMessage(steamIdLobby, pMessage, cubMsgBody);	
	}

	if (strcmp(MessageIterator, "SysSession::OnUpdate") == 0) //spoof map selection
		return OnUpdated(steamIdLobby, pMessage, cubMsgBody);
	
	return false;
}

bool LobbyClass::OnUpdated(CSteamID Lobby, const char* pMessage, const size_t MessageSize)
{
	if (!cfgLobby.spoofMaps)
		return false;

	std::string Message;
	std::copy(pMessage, pMessage + MessageSize, std::back_inserter(Message));

	unsigned messageFirst = Message.find("gamemodeflags") - 2u;
	unsigned messageLast = Message.size();

	std::string choosedMaps;

	if (cfgLobby.spoofedMaps) {
		for (int t = 0; t < 15; t++) {
			if (Helpers::getByteFromBytewise(cfgLobby.spoofedMaps, t))
				choosedMaps += maps[t];

			if (t != 15)
				choosedMaps += ",";
		}
	}

	if (Message.find("mapgroupname") != std::string::npos)
		Message.replace(Message.find("mapgroupname") + 13, 1 + choosedMaps.size(), std::string(choosedMaps + Message.substr(messageFirst, messageLast - messageFirst)));

	return CallOriginalSendLobbyChatMessage(Lobby, Message.data(), Message.size());
}

bool LobbyClass::OnChatInviteMessage(CSteamID Lobby, const char* pMessage, const size_t MessageSize)
{
	if (!cfgLobby.hideInvitePlayerNames)
		return false;

	std::vector<char> Message;

	// For remembering players :O
	static std::map<uint64_t, unsigned int> Players;
	static unsigned int PlayerCount = 0;

	// The Friends steamid
	const auto FriendXUID = *(uint64_t*)(pMessage + 0x51);

	// If we havnt seen this player before assign him a his own number
	if (!Players.count(FriendXUID))
	{
		Players[FriendXUID] = ++PlayerCount;
	}

	// Player's ID
	const auto PlayerIndex = Players[FriendXUID];

	// new playername
	std::string PlayerName = std::string(cfgLobby.hideInvitePlayerName);

	while (PlayerName.find("#idx") != std::string::npos) // replace #idx with player i
		PlayerName.replace(PlayerName.find("#idx"), 4, std::to_string(PlayerIndex));

	// all those messages ending in ZERO BEE BEE BEE
	const std::vector<char> MessageEnd =
	{
		0x00, 0x0b, 0x0b, 0x0b
	};

	// Copy the original message upto the players name
	std::copy(pMessage, pMessage + 0x65, std::back_inserter(Message));
	std::copy(PlayerName.begin(), PlayerName.end(), std::back_inserter(Message));
	std::copy(MessageEnd.begin(), MessageEnd.end(), std::back_inserter(Message));

	// Set XUID to a random steam account of mine so they can never tell which
	// nonamer was invited even if they really wanted to!
	*(uint64_t*)(Message.data() + 0x51) = 76561198397576094u;

	return CallOriginalSendLobbyChatMessage(Lobby, Message.data(), Message.size());
}

bool LobbyClass::OnEnterMatchmakingQueue(CSteamID Lobby, const char* pMessage, const size_t MessageSize)
{
	if (cfgLobby.nukeLobby)
	{
		// This is a big RIP to people
		const auto NukePower = std::powf(2, cfgLobby.nukeLobby * 10);

		for (auto i = 0.f; i < NukePower; i++)
			LobbyClass::CallOriginalSendLobbyChatMessage(Lobby, pMessage, MessageSize);

		return true;
	}

	return false;
}

void Lobby::commandSystemScript() noexcept {

	if (!cfgLobby.commandSystem) return;

	constexpr const char* script =
	#include "PanoramaJs/lobbyCommandSystem.js" // not my code. Its obvious 
	;

	interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script, "panorama/layout/mainmenu.xml", 8, 10, false, false);
}

void Lobby::retardLobby() noexcept {
	if (!cfgLobby.lobbyRetard)
		return;
	static float update = 0.f;
	std::string script = "";
	if (update < memory->globalVars->realtime) {
		script += "azurreRetardLobby();";
		interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
		update = memory->globalVars->realtime + 0.2f;
	}
}
void Lobby::lobbyRankParty() noexcept {
	static float update = 0.f;
	std::string script = "aLobbyRankParty = ";
	if (update < memory->globalVars->realtime) {
		script += cfgLobby.lobbyRankParty ? "true;" : "false; ";
		script += "azurreLobbyRankParty();";
		interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
		update = memory->globalVars->realtime + 0.2f;
	}
}

int GetMaxLobbySlotsForGameMode(const char* mode) {
	switch (fnv::hashRuntime(mode))
	{
	case fnv::hash("scrimcomp2v2"):
	case fnv::hash("cooperative"):
	case fnv::hash("coopmission"):
	case fnv::hash("survival"):
		return 2;
	default:
		return 5;
	}
}

void Lobby::startMatchmaking() noexcept{

	if (cfgLobby.autoQueue.enabled && Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel)) {
		if (*memory->partyList && memory->clientState->signonState == 0
			&& (*memory->matchmakingUIStatusString == NULL || **memory->matchmakingUIStatusString == '\0') //panorama uses the same check btw
			&& (memory->currentMatchState & 2) == 0 && memory->globalVars->currenttime >= nextUpdate + 1.f) {

			nextUpdate = memory->globalVars->currenttime; //no need to spam

			bool okToStart = false;
			bool OpenPlayMenu = false;

			MatchSession* matchSession = interfaces->matchFramework->getMatchSession();

			if (cfgLobby.autoQueue.minPlayers == 1 && (!matchSession || matchSession->getLobbyID() == 0)) {//solo, no lobby required. If we have a lobby and we are still solo the next check will pass
				OpenPlayMenu = true;
				okToStart = true;
			}
			else if (matchSession && matchSession->getLobbyID() != 0) {//check for if we have an "online" lobby
				if (isLobbyOwner) { //check if we are lobby owner
					KeyValues* settings = matchSession->getSessionSettings();
					const char* gamemode = settings->getString("game/mode");

					int slots = GetMaxLobbySlotsForGameMode(gamemode);
					int players = (*memory->partyList)->GetCount();

					if (cfgLobby.autoQueue.minPlayers == 0 && slots == players)
						okToStart = true;
					else if (cfgLobby.autoQueue.minPlayers >= 1 && players >= cfgLobby.autoQueue.minPlayers && players <= slots)
						okToStart = true;
				}

			}

			if (okToStart) {
				const char* script;
				if (OpenPlayMenu) //if we are alone we need to open PlayMenu first to initialise the offline lobby, there is a better way to do this(manualy) but i'm too lazy
					script = "LobbyAPI.CreateSession(); LobbyAPI.StartMatchmaking('', '', '', '')";
				else
					script = "LobbyAPI.StartMatchmaking('', '', '', '')";

				interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel), script, "panorama/layout/mainmenu.xml", 8, 10, false, false);
			}
		}

	}

	if (cfgLobby.autoInvite.autoJoin && *memory->partyBrowser) {
		for (size_t i = 0; i < pendingInvites.size(); i++) {
			auto& invite = pendingInvites[i];
			pendingInvites.erase(pendingInvites.begin() + i);
			*memory->joinLobbyID = invite.first;
			memory->joinLobby();
			break;
		}
	}
	else if (!cfgLobby.autoInvite.autoJoin)
		pendingInvites.clear();

}

void Lobby::autoDisconnect() noexcept
{
	if (!cfgLobby.autoQueue.autoDisconnect)
		return;

	interfaces->engine->clientCmdUnrestricted("disconnect");
	interfaces->inputSystem->resetInputState();
}

void sendErrorMessage(const char* message, int player, int color) {
	
	if (!Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel)) return;

	constexpr const char* colors[]{
		"ChatReportGreen",
		"ChatReportYellow",
		"ChatReportError"
	};

	constexpr const char* colorType[]{
	"green",
	"yellow",
	"error"
	};

	std::string chatReportColor = colors[color];
	std::string startMessage = "PartyListAPI.SessionCommand('Game::" + chatReportColor;
	std::string target = std::string("(LobbyAPI.BIsHost()) ? PartyListAPI.GetXuidByIndex(" + std::to_string(player) + ") : MyPersonaAPI.GetXuid()");
	std::string	col = colorType[color];
	std::string extraMessage = std::string(errorMessagePrefix).append(message);
	std::string finalMessage = std::string(startMessage + "', `run all xuid ${" + target + "} " + col + " " + extraMessage + "`);");
	interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel), finalMessage.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
}

std::unordered_map<char, uint64_t> ralnum = {
	{'A', 0 }, {'B', 1 }, {'C', 2 }, {'D', 3 }, {'E', 4 },
	{'F', 5 }, {'G', 6 }, {'H', 7 }, {'J', 8 }, {'K', 9 },
	{'L', 10 }, {'M', 11 }, {'N', 12 }, {'P', 13 }, {'Q', 14 },
	{'R', 15 }, {'S', 16 }, {'T', 17 }, {'U', 18 }, {'V', 19 },
	{'W', 20 }, {'X', 21 }, {'Y', 22 }, {'Z', 23 }, {'2', 24 },
	{'3', 25 }, {'4', 26 }, {'5', 27 }, {'6', 28 }, {'8', 30 },
	{'9', 31 },
};

uint64_t DecodeFriendCode(std::string friend_code) {

	if (friend_code.substr(0, 5) != "AAAA-")
		friend_code = "AAAA-" + friend_code;

	if (friend_code.size() != 15 || friend_code.at(4) != '-' || friend_code.at(10) != '-')
		return 0;

	uint64_t res = 0;
	for (size_t i = 0, j = 0; j < 15; j++) {
		if (j == 4 || j == 10)
			continue;
		res |= ralnum[friend_code[j]] << (5 * i);
		i++;
	}

	uint64_t val = 0;
	for (size_t i = 0; i < 8; i++) {
		val <<= 8;
		val |= res & 0xFF;
		res >>= 8;
	}

	uint64_t id = 0;
	for (size_t i = 0; i < 8; i++) {
		val >>= 1;
		uint64_t id_nibble = val & 0xF;
		val >>= 4;

		id <<= 4;
		id |= id_nibble;
	}

	return id | 0x110000100000000;
}

void CreateOnlineLobby() {
	if (!memory->createSession()) return;
	if (!memory->createSessionUI()) return;
	MatchSession* matchSession = interfaces->matchFramework->getMatchSession();
	if (matchSession && matchSession->getLobbyID() == 0) {
		KeyValues* keyValues = new KeyValues();
		memory->initKeyValues(keyValues, "MakeOnline");
		matchSession->command(keyValues);
	}
}

void invitePlayer(const uint64_t& id){
	typedef void(__thiscall* fInvitePlayer)(uintptr_t, uint64_t);
	static const auto ptr = *(uintptr_t**)(memory->invitePlayer);
	static const auto fn = (fInvitePlayer)(memory->invitePlayerFunction);
	if (*ptr)
		fn(*ptr, id);
}

static bool windowOpen = false;

void Lobby::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Lobby")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Lobby");
        ImGui::SetWindowPos("Lobby", { 100.0f, 100.0f });
    }
}

void Lobby::drawGUI(bool contentOnly) noexcept
{

	if (!contentOnly) {
		if (!windowOpen)
			return;
		ImGui::SetNextWindowSize({ 400.0f, 240.0f });
		ImGui::Begin("Lobby", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	}
	ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "WARNING: These features are glitched out and may not work as intended!");
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnOffset(1, 288.0f);
	ImGui::Text("Lobby Owner: %s", isLobbyOwner ? "true" : "false");
	ImGui::Checkbox("Lobby Chat Commands", &cfgLobby.commandSystem);
	ImGui::Checkbox("Rank Party", &cfgLobby.lobbyRankParty);
	ImGui::Checkbox("Retard Lobby", &cfgLobby.lobbyRetard);
	ImGui::PushItemWidth(180.f);
	ImGui::PushID("nukelobbys");
	ImGui::SliderFloat("##nukeLobby", &cfgLobby.nukeLobby, 0.00f, 1.00f, "Nuke Lobby: %.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("You need to be host of the lobby");
	ImGui::PopID();

	ImGui::PushID("SpoofsPush");
	ImGui::BeginChild("Spoofs", ImVec2(-1.f, 0.f), true, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Spoofers");
		if (ImGui::Checkbox("Ranks", &cfgLobby.lobbyRank.enabled)) {
			ImGui::SameLine();
			if (ImGui::Button("Update")) {
				std::string script;
				script += "";

				script += "\n var playerID = parseInt(" + std::to_string(cfgLobby.lobbyRank.playerID) + ");";
				script += "\n var level = parseInt(" + std::to_string(cfgLobby.lobbyRank.level) + ");";
				script += "\n var xp = parseInt(" + std::to_string(cfgLobby.lobbyRank.xp) + ");";
				script += "\n var rank = parseInt(" + std::to_string(cfgLobby.lobbyRank.rank) + ");";
				script += "\n var rankType = parseInt(" + std::to_string(rankType[cfgLobby.lobbyRank.rankType]) + ");";
				script += "\n var t1 = parseInt(" + std::to_string(cfgLobby.lobbyRank.t1) + ");";
				script += "\n var t2 = parseInt(" + std::to_string(cfgLobby.lobbyRank.t2) + ");";
				script += "\n var t3 = parseInt(" + std::to_string(cfgLobby.lobbyRank.t3) + ");";
				script += "\n var prime = parseInt(" + std::to_string(cfgLobby.lobbyRank.prime) + ");";
				script += "\n var teamColor = parseInt(" + std::to_string(cfgLobby.lobbyRank.teamColor) + ");";
				script += "\n var medal = parseInt(" + cfgLobby.lobbyRank.medal + "); ";
				script += "\n var loc = \"" + cfgLobby.lobbyRank.loc + "\";";

				script +=
					#include "PanoramaJs/changeLobbyRank.js"
				;
				interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script.c_str(), "", 8, 10, false, false);
			}
			ImGui::SliderInt("##playerID", &cfgLobby.lobbyRank.playerID, -1, 9, cfgLobby.lobbyRank.playerID < 0 ? "All Players" : "Player: %d");
			ImGui::SliderInt("##level", &cfgLobby.lobbyRank.level, 1, 40, "Level: %d");
			ImGui::SliderInt("##xp", &cfgLobby.lobbyRank.xp, 1, 5000, "XP: %d");
			ImGui::SliderInt("##rank", &cfgLobby.lobbyRank.rank, 0, 18, "Rank: %d");
			ImGui::Combo("##ranktype", &cfgLobby.lobbyRank.rankType, "Competitive\0Wingman\0Danger Zone\0");
			ImGui::SliderInt("##friendly", &cfgLobby.lobbyRank.t1, 0, 999, "Friendly: %d");
			ImGui::SliderInt("##teacher", &cfgLobby.lobbyRank.t2, 0, 999, "Teacher: %d");
			ImGui::SliderInt("##leader", &cfgLobby.lobbyRank.t3, 0, 999, "Leader: %d");
			ImGui::Checkbox("Prime", &cfgLobby.lobbyRank.prime);
			ImGui::SliderInt("##color", &cfgLobby.lobbyRank.teamColor, -1, 4, "Team Color: %d");
			ImGui::InputText("Medal", &cfgLobby.lobbyRank.medal);
			ImGui::InputText("Location", &cfgLobby.lobbyRank.loc);
			ImGui::Separator();
		}

		if (ImGui::Checkbox("Invited player name", &cfgLobby.hideInvitePlayerNames)) {
			ImGui::PushID("hiddenplayername");
			ImGui::InputText("Invite player name", &cfgLobby.hideInvitePlayerName);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("#idx - Prints index of the invited Player");
			ImGui::PopID();
		}

		ImGui::PushID("Custom Loc");
		ImGui::SetNextItemWidth(48.f);
		ImGui::InputText("Custom Lobby Location", &cfgLobby.customLobbyLoc);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Don't use spaces!");
		ImGui::SameLine();
		if (ImGui::Button("Update")) {
			std::string script = "var azurreUpdateLoc = \"Update/game/loc ";
			script += cfgLobby.customLobbyLoc;
			script += " \";";
			script += "PartyListAPI.UpdateSessionSettings(azurreUpdateLoc);";
			interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script.c_str(), "", 8, 10, false, false);
		}
		ImGui::PopID();

		if (ImGui::Checkbox("Selected Maps", &cfgLobby.spoofMaps)) {
		ImGui::PushID("Selected maps spoofer");
		ImGuiCustom::multiCombo("Selected maps", cfgLobby.spoofedMaps, "Premier\0Ancient\0Anubis\0Inferno\0Mirage\0Nuke\0Overpass\0Vertigo\0Tuscan\0Breach\0Dust2\0Train\0Cache\0Agency\0Office\0");
		ImGui::PopID();
		}
	ImGui::EndChild();
	ImGui::PopID();

	ImGui::Checkbox("Auto Disconnect", &cfgLobby.autoQueue.autoDisconnect);
	ImGui::Checkbox("Auto Join", &cfgLobby.autoInvite.autoJoin);
	//ImGui::Checkbox("Auto Accept", &cfgLobby.autoQueue.autoAccept);

	ImGui::NextColumn();

	ImGui::Text("Auto Queue");
	ImGui::PushID("auto queue");
	ImGui::Checkbox("Enabled", &cfgLobby.autoQueue.enabled);
	ImGui::SliderInt("Min players", &cfgLobby.autoQueue.minPlayers, 0, 5, cfgLobby.autoQueue.minPlayers == 0 ? "Full lobby" : "%d");
	ImGui::PopID();
	ImGui::Text("Inviter");


	MatchSession* matchSession = interfaces->matchFramework->getMatchSession();
	CSteamID lobbyID;
	if (matchSession)
		lobbyID.SetFromUint64(matchSession->getLobbyID());
	unsigned int numberPlayersInLobby = lobbyID.IsLobby() ? steamMatchmaking->GetNumLobbyMembers(lobbyID) : 0;
	std::vector<CSteamID> playersInLobby;
	for (unsigned int i = 0; i < numberPlayersInLobby; i++)
		playersInLobby.push_back(steamMatchmaking->GetLobbyMemberByIndex(lobbyID, i));

	auto& ids = cfgLobby.autoInvite.steamIDs;

	ImGui::SetNextItemWidth(180.f);
	if (ImGui::BeginTable("autoInviteIDs", 2)) {
		ImGui::TableSetupColumn("Player");
		ImGui::TableSetupColumn("Actions");
		ImGui::TableHeadersRow();
		int i = 0;
		for (uint64_t id : ids) {
			CSteamID steamID(id);
			bool inLobby = vectorContains(playersInLobby, steamID);
			steamIFriends->RequestUserInformation(steamID, true);
			ImGui::PushID(i);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const char* name = steamIFriends->GetFriendPersonaName(steamID);
			if (inLobby)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.32f, .7f, .07f, 1.f });
			ImGui::Text(name);
			if (inLobby)
				ImGui::PopStyleColor();
			ImGui::TableSetColumnIndex(1);
			if (ImGui::SmallButton("Invite")) {
				CreateOnlineLobby();
				invitePlayer(steamID.ConvertToUint64());
			}
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));
			if (ImGui::SmallButton("X")) {
				ids.erase(ids.begin() + i);
			}
			ImGui::PopStyleVar();
			ImGui::PopID();
			i++;
		}
		ImGui::EndTable();
	}

	static std::string addText = "";
	static const std::regex friendCodeRegex("(?:[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{4}-)?[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{5}-[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{4}");
	static const std::regex steamIDRegex("76\\d{15}");
	std::smatch fMatch;
	std::regex_search(addText, fMatch, friendCodeRegex);
	std::smatch iMatch;
	std::regex_search(addText, iMatch, steamIDRegex);
	const bool matched = fMatch.size() == 1 || iMatch.size() == 1;
	ImGui::PushStyleColor(ImGuiCol_Text, matched ? ImVec4{ 0.32f, .7f, .07f, 1.f } : ImVec4{1.f, 0.f, 0.f, 1.f});
	ImGui::InputText("##add_auto_invite", &addText, ImGuiInputTextFlags_AutoSelectAll);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if (ImGui::Button("Add")) {
		uint64_t id = 0;
		if (fMatch.size() == 1)
			id = DecodeFriendCode(fMatch[0]);
		else if (iMatch.size() == 1)
			id = std::stoull(iMatch[0]);

		if (id != 0) {
			addText = "";
			if (!vectorContains(ids, id))
				ids.push_back(id);
		}
	}
	ImGui::SameLine();
	ImGui::PushID("steamid64orfriendcode");
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("This can be either CS:GO friend code or steamID64");
	ImGui::PopID();

	if (ImGui::Button("Invite missing", ImVec2(128.f, 0.f))) {
		CreateOnlineLobby();
		for (uint64_t id : ids) {
			CSteamID steamID(id);
			if (!vectorContains(playersInLobby, steamID)) {
				invitePlayer(steamID.ConvertToUint64());
			}
		}
	}
	ImGui::PopItemWidth();
	if (Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel)) {
		ImGui::PushID("LobbySpammers");
		ImGui::PushItemWidth(180.f);
		ImGui::Text("Lobby Error Messages");
		ImGui::Combo("Color", &cfgLobby.troll.messageColor, "Green\0Yellow\0Red\0");
		ImGui::SameLine();
		ImGui::PushID("colorWarning");
		ImGui::TextDisabled("?");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Color Green & Yellow work only\nif You are a Lobby Owner");
		ImGui::PopID();
		ImGui::Combo("Message", &cfgLobby.troll.message, errorMessagesTitles, ARRAYSIZE(errorMessages));
		ImGui::SliderInt("Repeat##messages", &cfgLobby.troll.powerMessage, 1, 100);
		if (ImGui::Button("Send")) {
			for (int i = 0; i < cfgLobby.troll.powerMessage; i++)
				sendErrorMessage(errorMessages[cfgLobby.troll.message], 0, cfgLobby.troll.messageColor);
		}
		ImGui::Text("Popup Spammer");
		ImGui::SliderInt("Repeat##popup", &cfgLobby.troll.powerPopUpSpam, 1, 100);
		if (ImGui::Button("Spam Popup")) {
			for (int i = 0; i < cfgLobby.troll.powerPopUpSpam; i++)
				interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), "PartyListAPI.SessionCommand(\"Game::HostEndGamePlayAgain\", `run all xuid ${MyPersonaAPI.GetXuid()}`);", "panorama/layout/mainmenu.xml", 8, 10, false, false);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove Popups")) {
			interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), "UiToolkitAPI.CloseAllVisiblePopups();", "panorama/layout/mainmenu.xml", 8, 10, false, false);
			interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOHud), "UiToolkitAPI.CloseAllVisiblePopups();", "panorama/layout/mainmenu.xml", 8, 10, false, false);
		}
		ImGui::PopItemWidth();
		ImGui::PopID();
	}else {
		ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "Couldn't get the Menu Panel, panorama features are disabled. Restart game and try again to enable them.");
	}
	ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}
