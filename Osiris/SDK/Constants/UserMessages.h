#pragma once

// https://github.com/SteamDatabase/Protobufs/blob/master/csgo/cstrike15_usermessages.proto
namespace csgo
{
	enum UserMessageType {
		VGUIMenu = 1,
		Geiger = 2,
		Train = 3,
		HudText = 4,
		SayText = 5,
		SayText2 = 6,
		Text = 7,
		HudMsg = 8,
		ResetHud = 9,
		GameTitle = 10,
		Shake = 12,
		Fade = 13,
		Rumble = 14,
		CloseCaption = 15,
		CloseCaptionDirect = 16,
		SendAudio = 17,
		RawAudio = 18,
		VoiceMask = 19,
		RequestState = 20,
		Damage = 21,
		RadioText = 22,
		HintText = 23,
		KeyHintText = 24,
		ProcessSpottedEntityUpdate = 25,
		ReloadEffect = 26,
		AdjustMoney = 27,
		UpdateTeamMoney = 28,
		StopSpectatorMode = 29,
		KillCam = 30,
		DesiredTimescale = 31,
		CurrentTimescale = 32,
		AchievementEvent = 33,
		MatchEndConditions = 34,
		DisconnectToLobby = 35,
		PlayerStatsUpdate = 36,
		DisplayInventory = 37,
		WarmupHasEnded = 38,
		ClientInfo = 39,
		XRankGet = 40,
		XRankUpd = 41,
		CallVoteFailed = 45,
		VoteStart = 46,
		VotePass = 47,
		VoteFailed = 48,
		VoteSetup = 49,
		ServerRankRevealAll = 50,
		SendLastKillerDamageToClient = 51,
		ServerRankUpdate = 52,
		ItemPickup = 53,
		ShowMenu = 54,
		BarTime = 55,
		AmmoDenied = 56,
		MarkAchievement = 57,
		MatchStatsUpdate = 58,
		ItemDrop = 59,
		GlowPropTurnOff = 60,
		SendPlayerItemDrops = 61,
		RoundBackupFilenames = 62,
		SendPlayerItemFound = 63,
		ReportHit = 64,
		XpUpdate = 65,
		QuestProgress = 66,
		ScoreLeaderboardData = 67,
		PlayerDecalDigitalSignature = 68,
		MAX_ECSTRIKE15USERMESSAGES
	};
}