#pragma once
#include "pbwrap.hpp"

enum csgoGameCoordinatorMessages {
    Base = 9100,
    MatchmakingStart = 9101,
    MatchmakingStop = 9102,
    MatchmakingClient2ServerPing = 9103,
    MatchmakingGC2ClientUpdate = 9104,
    MatchmakingServerReservationResponse = 9106,
    MatchmakingGC2ClientReserve = 9107,
    MatchmakingClient2GCHello = 9109,
    MatchmakingGC2ClientHello = 9110,
    MatchmakingGC2ClientAbandon = 9112,
    MatchmakingGCOperationalStats = 9115,
    MatchmakingOperator2GCBlogUpdate = 9117,
    ServerNotificationForUserPenalty = 9118,
    ClientReportPlayer = 9119,
    ClientReportServer = 9120,
    ClientCommendPlayer = 9121,
    ClientReportResponse = 9122,
    ClientCommendPlayerQuery = 9123,
    ClientCommendPlayerQueryResponse = 9124,
    WatchInfoUsers = 9126,
    ClientRequestPlayersProfile = 9127,
    PlayersProfile = 9128,
    PlayerOverwatchCaseUpdate = 9131,
    PlayerOverwatchCaseAssignment = 9132,
    PlayerOverwatchCaseStatus = 9133,
    GC2ClientTextMsg = 9134,
    Client2GCTextMsg = 9135,
    MatchEndRunRewardDrops = 9136,
    MatchEndRewardDropsNotification = 9137,
    ClientRequestWatchInfoFriends2 = 9138,
    MatchList = 9139,
    MatchListRequestCurrentLiveGames = 9140,
    MatchListRequestRecentUserGames = 9141,
    GC2ServerReservationUpdate = 9142,
    ClientVarValueNotificationInfo = 9144,
    MatchListRequestTournamentGames = 9146,
    MatchListRequestFullGameInfo = 9147,
    GiftsLeaderboardRequest = 9148,
    GiftsLeaderboardResponse = 9149,
    ServerVarValueNotificationInfo = 9150,
    ClientSubmitSurveyVote = 9152,
    Server2GCClientValidate = 9153,
    MatchListRequestLiveGameForUser = 9154,
    Client2GCEconPreviewDataBlockRequest = 9156,
    Client2GCEconPreviewDataBlockResponse = 9157,
    AccountPrivacySettings = 9158,
    SetMyActivityInfo = 9159,
    MatchListRequestTournamentPredictions = 9160,
    MatchListUploadTournamentPredictions = 9161,
    DraftSummary = 9162,
    ClientRequestJoinFriendData = 9163,
    ClientRequestJoinServerData = 9164,
    ClientRequestNewMission = 9165,
    GC2ClientTournamentInfo = 9167,
    GlobalGame_Subscribe = 9168,
    GlobalGame_Unsubscribe = 9169,
    GlobalGame_Play = 9170,
    AcknowledgePenalty = 9171,
    Client2GCRequestPrestigeCoin = 9172,
    GC2ClientGlobalStats = 9173,
    Client2GCStreamUnlock = 9174,
    FantasyRequestClientData = 9175,
    FantasyUpdateClientData = 9176,
    GCToClientSteamdatagramTicket = 9177,
    ClientToGCRequestTicket = 9178,
    ClientToGCRequestElevate = 9179,
    GlobalChat = 9180,
    GlobalChat_Subscribe = 9181,
    GlobalChat_Unsubscribe = 9182,
    ClientAuthKeyCode = 9183,
    GotvSyncPacket = 9184,
    ClientPlayerDecalSign = 9185,
    ClientLogonFatalError = 9187,
    ClientPollState = 9188,
    Party_Register = 9189,
    Party_Unregister = 9190,
    Party_Search = 9191,
    Party_Invite = 9192,
    Account_RequestCoPlays = 9193,
    ClientGCRankUpdate = 9194,
    ClientRequestOffers = 9195,
    ClientAccountBalance = 9196,
    ClientPartyJoinRelay = 9197,
    ClientPartyWarning = 9198,
    SetEventFavorite = 9200,
    GetEventFavorites_Request = 9201,
    ClientPerfReport = 9202,
    GetEventFavorites_Response = 9203,
    ClientRequestSouvenir = 9204,
    ClientReportValidation = 9205,
    GC2ClientRefuseSecureMode = 9206,
    GC2ClientRequestValidation = 9207,
    ClientRedeemMissionReward = 9209,
    ClientDeepStats = 9210,
    StartAgreementSessionInGame = 9211
};

//FORMAT: constexpr static Tag field_name = {field_id, field_type};

using namespace pbwrap;

struct CMsgClientHello : pbmsg<8> {
    PBMSG_CTOR;
    PBFIELD(3, types::Uint32, clientSessionNeed);
};

// ProfileChanger
struct MatchmakingGC2ClientHelloS : pbmsg<20> {
    struct PlayerRankingInfo : pbmsg<6> {
        PBMSG_CTOR;
        PBFIELD(1, types::Uint32, account);
        PBFIELD(2, types::Uint32, rank);
        PBFIELD(3, types::Uint32, wins);
        PBFIELD(6, types::Uint32, rankType);
    };
    struct PlayerCommendationInfo : pbmsg<4> {
        PBMSG_CTOR;
        PBFIELD(1, types::Uint32, friendly);
        PBFIELD(2, types::Uint32, teaching);
        PBFIELD(4, types::Uint32, leader);
    };

    PBMSG_CTOR;
    PBFIELD(7, PlayerRankingInfo, ranking);
    PBFIELD(8, PlayerCommendationInfo, commendation);
    PBFIELD(17, types::Int32, level);
    PBFIELD(18, types::Int32, currentXP);
    PBFIELD(4, types::Int32, penaltySeconds);
    PBFIELD(5, types::Int32, penaltyReason);
    PBFIELD(6, types::Int32, vacBanned);

};

struct CMsgGCCStrike15_v2_Party_Invite {
    bool has_accountid;
    uint32_t accountid;
    bool has_lobbyid;
    uint32_t lobbyid;
};

struct CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate {
    bool hasMatchmaking;
    int32_t matchmaking;
    void* waiting_account_id_sessions;
    void* error;
    void* ongoingmatch_account_id_sessions;
    bool has_global_stats;
    void* global_stats;
    void* failping_account_id_sessions;
    void* penalty_account_id_sessions;
    void* failready_account_id_sessions;
    void* vacbanned_account_id_sessions;
    bool has_server_ipaddress_mask;
    void* server_ipaddress_mask;
    void* notes;
    void* penalty_account_id_sessions_green;
    void* insufficientlevel_sessions;
    void* vsncheck_account_id_sessions;
    void* launcher_mismatch_sessions;
    void* insecure_account_id_sessions;
};

struct CMsgGCCStrike15_v2_ClientGCRankUpdate : pbmsg<1> {
    PBMSG_CTOR;
    PBFIELD(1, MatchmakingGC2ClientHelloS::PlayerRankingInfo, ranking);
};

struct CMsgGCCStrike15_v2_PlayerOverwatchCaseAssignment : pbmsg<11> {
    PBFIELD(1, types::Uint64, caseid);
    PBFIELD(2, types::String, caseurl);
    PBFIELD(3, types::Uint32, verdict);
    PBFIELD(4, types::Uint32, timestamp);
    PBFIELD(5, types::Uint32, throttleseconds);
    PBFIELD(6, types::Uint32, suspectid);
    PBFIELD(7, types::Uint32, fractionid);
    PBFIELD(8, types::Uint32, numrounds);
    PBFIELD(9, types::Uint32, fractionrounds);
    PBFIELD(10, types::Int32, streakconvictions);
    PBFIELD(11, types::Uint32, reason);
};