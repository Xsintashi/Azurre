#include "../Memory.h"
#include "../Config.h"

#include "../SDK/SteamAPI.h"
#include "../Lobby.h"

#include "MemAlloc.h"
#include "Protobuffs.h"
#include "Messages.h"
#include "ProtoWriter.h"

#include <array>
#include <Interfaces.h>
#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

constexpr const char* messagesTypes[] = {
	"Base",
	"MatchmakingStart",
	"MatchmakingStop",
	"MatchmakingClient2ServerPing",
	"MatchmakingGC2ClientUpdate",
	"MatchmakingServerReservationResponse",
	"MatchmakingGC2ClientReserve",
	"MatchmakingClient2GCHello",
	"MatchmakingGC2ClientHello",
	"MatchmakingGC2ClientAbandon",
	"MatchmakingGCOperationalStats",
	"MatchmakingOperator2GCBlogUpdate",
	"ServerNotificationForUserPenalty",
	"ClientReportPlayer",
	"ClientReportServer",
	"ClientCommendPlayer",
	"ClientReportResponse",
	"ClientCommendPlayerQuery",
	"ClientCommendPlayerQueryResponse",
	"WatchInfoUsers",
	"ClientRequestPlayersProfile",
	"PlayersProfile",
	"PlayerOverwatchCaseUpdate",
	"PlayerOverwatchCaseAssignment",
	"PlayerOverwatchCaseStatus",
	"GC2ClientTextMsg",
	"Client2GCTextMsg",
	"MatchEndRunRewardDrops",
	"MatchEndRewardDropsNotification",
	"ClientRequestWatchInfoFriends2",
	"MatchList",
	"MatchListRequestCurrentLiveGames",
	"MatchListRequestRecentUserGames",
	"GC2ServerReservationUpdate",
	"ClientVarValueNotificationInfo",
	"MatchListRequestTournamentGames",
	"MatchListRequestFullGameInfo",
	"GiftsLeaderboardRequest",
	"GiftsLeaderboardResponse",
	"ServerVarValueNotificationInfo",
	"ClientSubmitSurveyVote",
	"Server2GCClientValidate",
	"MatchListRequestLiveGameForUser",
	"Client2GCEconPreviewDataBlockRequest",
	"Client2GCEconPreviewDataBlockResponse",
	"AccountPrivacySettings",
	"SetMyActivityInfo",
	"MatchListRequestTournamentPredictions",
	"MatchListUploadTournamentPredictions",
	"DraftSummary",
	"ClientRequestJoinFriendData",
	"ClientRequestJoinServerData",
	"ClientRequestNewMission",
	"GC2ClientTournamentInfo",
	"GlobalGame_Subscribe",
	"GlobalGame_Unsubscribe",
	"GlobalGame_Play",
	"AcknowledgePenalty",
	"Client2GCRequestPrestigeCoin",
	"GC2ClientGlobalStats",
	"Client2GCStreamUnlock",
	"FantasyRequestClientData",
	"FantasyUpdateClientData",
	"GCToClientSteamdatagramTicket",
	"ClientToGCRequestTicket",
	"ClientToGCRequestElevate",
	"GlobalChat",
	"GlobalChat_Subscribe",
	"GlobalChat_Unsubscribe",
	"ClientAuthKeyCode",
	"GotvSyncPacket",
	"ClientPlayerDecalSign",
	"ClientLogonFatalError",
	"ClientPollState",
	"Party_Register",
	"Party_Unregister",
	"Party_Search",
	"Party_Invite",
	"Account_RequestCoPlays",
	"ClientGCRankUpdate",
	"ClientRequestOffers",
	"ClientAccountBalance",
	"ClientPartyJoinRelay",
	"ClientPartyWarning",
	"SetEventFavorite",
	"GetEventFavorites_Request",
	"ClientPerfReport",
	"GetEventFavorites_Response",
	"ClientRequestSouvenir",
	"ClientReportValidation",
	"GC2ClientRefuseSecureMode",
	"GC2ClientRequestValidation",
	"ClientRedeemMissionReward",
	"ClientDeepStats",
	"StartAgreementSessionInGame"
};

static std::string profile_info_changer(void* pubDest, uint32_t* pcubMsgSize) {
	MatchmakingGC2ClientHelloS msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	MatchmakingGC2ClientHelloS::PlayerRankingInfo ranking;
	MatchmakingGC2ClientHelloS::PlayerCommendationInfo commendations;

	//replace commends
	if (config->profilechanger.enabledCommends) {
		commendations.friendly().set(config->profilechanger.friendly);
		commendations.teaching().set(config->profilechanger.teach);
		commendations.leader().set(config->profilechanger.leader);
		msg.commendation().set(commendations);
	}

	if (config->profilechanger.enabledRanks) {
		ranking.rank().set(config->profilechanger.rank);
		ranking.wins().set(config->profilechanger.wins);

		//replace private level
		msg.level().set(config->profilechanger.level + 1);
		msg.currentXP().set(config->profilechanger.exp);

		msg.ranking().set(ranking);
	}

	//replace bans
	if (config->profilechanger.enabledBans && config->profilechanger.banTime){
		msg.penaltyReason().set(config->profilechanger.banType);
		msg.penaltySeconds().set(config->profilechanger.banTime);
		msg.vacBanned().set(config->profilechanger.banType == 0);
	}
	msg.ranking().set(ranking);
	return msg.serialize();
}

void Protobuffs::WritePacket(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	auto g_MemAlloc = memory->memalloc;
	if ((uint32_t)packet.size() <= cubDest - 8)
	{
		memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
		*pcubMsgSize = packet.size() + 8;
	}
	else if (g_MemAlloc)
	{
		auto memPtr = *CAST(void**, thisPtr, 0x18);
		auto memPtrSize = *CAST(uint32_t*, thisPtr, 0x1C);
		auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

		auto memory = g_MemAlloc->Realloc(memPtr, newSize + 4);

		*CAST(void**, thisPtr, 0x18) = memory;
		*CAST(uint32_t*, thisPtr, 0x1C) = newSize;
		*CAST(void**, oldEBP, -0x14) = memory;

		memcpy(CAST(void*, memory, 0x1C), (void*)packet.data(), packet.size());

		*pcubMsgSize = packet.size() + 8;
	}
}

static bool onceChanger = false;

void Protobuffs::ReceiveMessage(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	if (messageType == csgoGameCoordinatorMessages::MatchmakingGC2ClientHello)
	{
		auto packet = profile_info_changer(pubDest, pcubMsgSize);
		WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
	}
	else if (messageType == csgoGameCoordinatorMessages::ClientGCRankUpdate)
	{
		CMsgGCCStrike15_v2_ClientGCRankUpdate msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);

		if (!config->profilechanger.enabledRanks) return;

		const auto ranking = msg.ranking().get();
		if (ranking.rankType().get() == 6) // competitive 
		{
			ranking.rank().set(config->profilechanger.rank);
			ranking.wins().set(config->profilechanger.wins);
			msg.ranking().set(ranking);
			auto packet = msg.serialize();
			WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
		if (ranking.rankType().get() == 7) // wingman 
		{
			ranking.rank().set(config->profilechanger.wmrank);
			ranking.wins().set(config->profilechanger.wmwins);
			msg.ranking().set(ranking);
			auto packet = msg.serialize();
			WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
		if (ranking.rankType().get() == 10) // danger zone 
		{
			ranking.rank().set(config->profilechanger.dzrank);
			ranking.wins().set(config->profilechanger.dzwins);
			msg.ranking().set(ranking);
			auto packet = msg.serialize();
			WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
	}

	if (messageType == csgoGameCoordinatorMessages::Party_Invite && Lobby::autoJoin()) {

		CMsgGCCStrike15_v2_Party_Invite message = { false, 0, false, 0 };

		CSteamID lobby_id;
		lobby_id.InstancedSet(message.lobbyid, k_EChatInstanceFlagMMSLobby | k_EChatInstanceFlagLobby, k_EUniversePublic, k_EAccountTypeChat);
		uint64_t lobby_id_uint = lobby_id.ConvertToUint64();

		if (std::find_if(Lobby::getPendingInvites().begin(), Lobby::getPendingInvites().end(),
			[&lobby_id_uint](const std::pair<uint64_t, uint64_t> x) {
				return x.first == lobby_id_uint;
			}) == Lobby::getPendingInvites().end()) {
			Lobby::getPendingInvites().push_back(std::make_pair(lobby_id_uint, (uint64_t)GetTickCount64()));
		}
		return;
	}
	if (messageType == csgoGameCoordinatorMessages::MatchmakingGC2ClientUpdate && Lobby::autoQueue()) {

		CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate message = { false, 0, NULL, NULL, NULL, false, NULL, NULL, NULL, NULL, NULL, false, NULL, NULL };

		if (message.hasMatchmaking) { //matchmaking stopped
			if (message.matchmaking == 0) {
				Lobby::autoQueue() = false;
			}
			else if (message.matchmaking == 4) {//Match confirmed
				Lobby::setUpdate() += 20.f; //prevent auto queue from running for next 20 seconds if we found a match
			}
		}
		return;
	}

	printf("Azurre | .GC Receive: %i\n", messageType);
}

bool Protobuffs::PreSendMessage(uint32_t& unMsgType, void* pubData, uint32_t& cubData)
{
	uint32_t messageType = unMsgType & 0x7FFFFFFF;
	printf("Azurre | .GC Sent: %i\n", messageType);
	return true;
}

///////////////////////////////////
/******** Custom Messages ********/
///////////////////////////////////
bool Protobuffs::SendClientHello() // inventory changer
{
	CMsgClientHello msg;
	msg.clientSessionNeed().set(1);
	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = csgoGameCoordinatorMessages::MatchmakingGC2ClientHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->GCSendMessage(csgoGameCoordinatorMessages::MatchmakingGC2ClientHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuffs::SendMatchmakingClient2GCHello()
{
	ProtoWriter msg(0);
	auto packet = msg.serialize();
	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = csgoGameCoordinatorMessages::MatchmakingClient2GCHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->GCSendMessage(csgoGameCoordinatorMessages::MatchmakingClient2GCHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuffs::SendClientGcRankUpdate(int rankType) // 6 - mm, 7 - wingman, 10 - dangerzone
{
	MatchmakingGC2ClientHelloS::PlayerRankingInfo rank;
	rank.rankType().set(rankType);

	CMsgGCCStrike15_v2_ClientGCRankUpdate msg;
	msg.ranking().set(rank);

	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = csgoGameCoordinatorMessages::ClientGCRankUpdate | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->GCSendMessage(csgoGameCoordinatorMessages::ClientGCRankUpdate | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}