#pragma once
#include <iostream>
#include "steamtypes.h"
#include "steamclientpublic.h"


template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}

enum EGCResult
{
	k_EGCResultOK = 0,
	k_EGCResultNoMessage = 1,           // There is no message in the queue
	k_EGCResultBufferTooSmall = 2,      // The buffer is too small for the requested message
	k_EGCResultNotLoggedOn = 3,         // The client is not logged onto Steam
	k_EGCResultInvalidMessage = 4,      // Something was wrong with the message being sent with SendMessage
};

class SteamGameCoordinator
{
public:
	virtual EGCResult GCSendMessage(int unMsgType, const void* pubData, int cubData) = 0;
	virtual bool IsMessageAvailable(int* pcubMsgSize) = 0;
	virtual EGCResult GCRetrieveMessage(int* punMsgType, void* pubDest, int cubDest, int* pcubMsgSize) = 0;

};

class CSteamID
{
public:

	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	//-----------------------------------------------------------------------------
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeInvalid;
		m_steamid.m_comp.m_EUniverse = k_EUniverseInvalid;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID(uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		Set(unAccountID, eUniverse, eAccountType);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			unAccountInstance - instance 
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID(uint32 unAccountID, unsigned int unAccountInstance, EUniverse eUniverse, EAccountType eAccountType)
	{
#if defined(_SERVER) && defined(Assert)
		Assert((k_EAccountTypeIndividual != eAccountType) || (unAccountInstance == k_unSteamUserDefaultInstance));	// enforce that for individual accounts, instance is always 1
#endif // _SERVER
		InstancedSet(unAccountID, unAccountInstance, eUniverse, eAccountType);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	// Note:	Will not accept a uint32 or int32 as input, as that is a probable mistake.
	//			See the stubbed out overloads in the private: section for more info.
	//-----------------------------------------------------------------------------
	CSteamID(uint64 ulSteamID)
	{
		SetFromUint64(ulSteamID);
	}
#ifdef INT64_DIFFERENT_FROM_INT64_T
	CSteamID(uint64_t ulSteamID)
	{
		SetFromUint64((uint64)ulSteamID);
	}
#endif


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void Set(uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;

		if (eAccountType == k_EAccountTypeClan || eAccountType == k_EAccountTypeGameServer)
		{
			m_steamid.m_comp.m_unAccountInstance = 0;
		}
		else
		{
			m_steamid.m_comp.m_unAccountInstance = 1;
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void InstancedSet(uint32 unAccountID, uint32 unInstance, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;
		m_steamid.m_comp.m_unAccountInstance = unInstance;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 52 bit parts and universe/type
	// Input  : ulIdentifier - 52 bits of goodness
	//-----------------------------------------------------------------------------
	void FullSet(uint64 ulIdentifier, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = (ulIdentifier & k_unSteamAccountIDMask);						// account ID is low 32 bits
		m_steamid.m_comp.m_unAccountInstance = ((ulIdentifier >> 32) & k_unSteamAccountInstanceMask);			// account instance is next 20 bits
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 64-bit representation
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	void SetFromUint64(uint64 ulSteamID)
	{
		m_steamid.m_unAll64Bits = ulSteamID;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Clear all fields, leaving an invalid ID.
	//-----------------------------------------------------------------------------
	void Clear()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeInvalid;
		m_steamid.m_comp.m_EUniverse = k_EUniverseInvalid;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}


#if defined( INCLUDED_STEAM2_USERID_STRUCTS ) 
	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to convert
	//			eUniverse -				universe this ID belongs to
	//-----------------------------------------------------------------------------
	void SetFromSteam2(TSteamGlobalUserID* pTSteamGlobalUserID, EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits * 2 +
			pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits;
		m_steamid.m_comp.m_EUniverse = eUniverse;		// set the universe
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeIndividual; // Steam 2 accounts always map to account type of individual
		m_steamid.m_comp.m_unAccountInstance = k_unSteamUserDefaultInstance; // Steam2 only knew one instance
	}

	//-----------------------------------------------------------------------------
	// Purpose: Fills out a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to write to
	//-----------------------------------------------------------------------------
	void ConvertToSteam2(TSteamGlobalUserID* pTSteamGlobalUserID) const
	{
		// only individual accounts have any meaning in Steam 2, only they can be mapped
		// Assert( m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual );

		pTSteamGlobalUserID->m_SteamInstanceID = 0;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits = m_steamid.m_comp.m_unAccountID % 2;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits = m_steamid.m_comp.m_unAccountID / 2;
	}
#endif // defined( INCLUDED_STEAM_COMMON_STEAMCOMMON_H )

	//-----------------------------------------------------------------------------
	// Purpose: Converts steam ID to its 64-bit representation
	// Output : 64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	uint64 ConvertToUint64() const
	{
		return m_steamid.m_unAll64Bits;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Converts the static parts of a steam ID to a 64-bit representation.
	//			For multiseat accounts, all instances of that account will have the
	//			same static account key, so they can be grouped together by the static
	//			account key.
	// Output : 64-bit static account key
	//-----------------------------------------------------------------------------
	uint64 GetStaticAccountKey() const
	{
		// note we do NOT include the account instance (which is a dynamic property) in the static account key
		return (uint64)((((uint64)m_steamid.m_comp.m_EUniverse) << 56) + ((uint64)m_steamid.m_comp.m_EAccountType << 52) + m_steamid.m_comp.m_unAccountID);
	}


	//-----------------------------------------------------------------------------
	// Purpose: create an anonymous game server login to be filled in by the AM
	//-----------------------------------------------------------------------------
	void CreateBlankAnonLogon(EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeAnonGameServer;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}


	//-----------------------------------------------------------------------------
	// Purpose: create an anonymous game server login to be filled in by the AM
	//-----------------------------------------------------------------------------
	void CreateBlankAnonUserLogon(EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeAnonUser;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous game server login that will be filled in?
	//-----------------------------------------------------------------------------
	bool BBlankAnonAccount() const
	{
		return m_steamid.m_comp.m_unAccountID == 0 && BAnonAccount() && m_steamid.m_comp.m_unAccountInstance == 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a game server account id?  (Either persistent or anonymous)
	//-----------------------------------------------------------------------------
	bool BGameServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeGameServer || m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a persistent (not anonymous) game server account id?
	//-----------------------------------------------------------------------------
	bool BPersistentGameServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous game server account id?
	//-----------------------------------------------------------------------------
	bool BAnonGameServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a content server account id?
	//-----------------------------------------------------------------------------
	bool BContentServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeContentServer;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this a clan account id?
	//-----------------------------------------------------------------------------
	bool BClanAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeClan;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this a chat account id?
	//-----------------------------------------------------------------------------
	bool BChatAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeChat;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a chat account id?
	//-----------------------------------------------------------------------------
	bool IsLobby() const
	{
		return (m_steamid.m_comp.m_EAccountType == k_EAccountTypeChat)
			&& (m_steamid.m_comp.m_unAccountInstance & k_EChatInstanceFlagLobby);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this an individual user account id?
	//-----------------------------------------------------------------------------
	bool BIndividualAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual || m_steamid.m_comp.m_EAccountType == k_EAccountTypeConsoleUser;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous account?
	//-----------------------------------------------------------------------------
	bool BAnonAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonUser || m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous user account? ( used to create an account or reset a password )
	//-----------------------------------------------------------------------------
	bool BAnonUserAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonUser;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a faked up Steam ID for a PSN friend account?
	//-----------------------------------------------------------------------------
	bool BConsoleUserAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeConsoleUser;
	}

	// simple accessors
	void SetAccountID(uint32 unAccountID) { m_steamid.m_comp.m_unAccountID = unAccountID; }
	void SetAccountInstance(uint32 unInstance) { m_steamid.m_comp.m_unAccountInstance = unInstance; }

	AccountID_t GetAccountID() const { return m_steamid.m_comp.m_unAccountID; }
	uint32 GetUnAccountInstance() const { return m_steamid.m_comp.m_unAccountInstance; }
	EAccountType GetEAccountType() const { return (EAccountType)m_steamid.m_comp.m_EAccountType; }
	EUniverse GetEUniverse() const { return m_steamid.m_comp.m_EUniverse; }
	void SetEUniverse(EUniverse eUniverse) { m_steamid.m_comp.m_EUniverse = eUniverse; }
	bool IsValid() const;

	// this set of functions is hidden, will be moved out of class
	explicit CSteamID(const char* pchSteamID, EUniverse eDefaultUniverse = k_EUniverseInvalid);
	const char* Render() const;				// renders this steam ID to string
	static const char* Render(uint64 ulSteamID);	// static method to render a uint64 representation of a steam ID to a string

	void SetFromString(const char* pchSteamID, EUniverse eDefaultUniverse);
	// SetFromString allows many partially-correct strings, constraining how
	// we might be able to change things in the future.
	// SetFromStringStrict requires the exact string forms that we support
	// and is preferred when the caller knows it's safe to be strict.
	// Returns whether the string parsed correctly.
	bool SetFromStringStrict(const char* pchSteamID, EUniverse eDefaultUniverse);
	bool SetFromSteam2String(const char* pchSteam2ID, EUniverse eUniverse);

	inline bool operator==(const CSteamID& val) const { return m_steamid.m_unAll64Bits == val.m_steamid.m_unAll64Bits; }
	inline bool operator!=(const CSteamID& val) const { return !operator==(val); }
	inline bool operator<(const CSteamID& val) const { return m_steamid.m_unAll64Bits < val.m_steamid.m_unAll64Bits; }
	inline bool operator>(const CSteamID& val) const { return m_steamid.m_unAll64Bits > val.m_steamid.m_unAll64Bits; }

	// DEBUG function
	bool BValidExternalSteamID() const;

private:
	// These are defined here to prevent accidental implicit conversion of a u32AccountID to a CSteamID.
	// If you get a compiler error about an ambiguous constructor/function then it may be because you're
	// passing a 32-bit int to a function that takes a CSteamID. You should explicitly create the SteamID
	// using the correct Universe and account Type/Instance values.
	CSteamID(uint32);
	CSteamID(int32);

	// 64 bits total
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
#ifdef VALVE_BIG_ENDIAN
			EUniverse			m_EUniverse : 8;	// universe this account belongs to
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID
			uint32				m_unAccountID : 32;			// unique account identifier
#else
			uint32				m_unAccountID : 32;			// unique account identifier
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			EUniverse			m_EUniverse : 8;	// universe this account belongs to
#endif
		} m_comp;

		uint64 m_unAll64Bits;
	} m_steamid;
};

inline bool CSteamID::IsValid() const
{
	if (m_steamid.m_comp.m_EAccountType <= k_EAccountTypeInvalid || m_steamid.m_comp.m_EAccountType >= k_EAccountTypeMax)
		return false;

	if (m_steamid.m_comp.m_EUniverse <= k_EUniverseInvalid || m_steamid.m_comp.m_EUniverse >= k_EUniverseMax)
		return false;

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual)
	{
		if (m_steamid.m_comp.m_unAccountID == 0 || m_steamid.m_comp.m_unAccountInstance != 1)
			return false;
	}

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeClan)
	{
		if (m_steamid.m_comp.m_unAccountID == 0 || m_steamid.m_comp.m_unAccountInstance != 0)
			return false;
	}

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeGameServer)
	{
		if (m_steamid.m_comp.m_unAccountID == 0)
			return false;
		// Any limit on instances?  We use them for local users and bots
	}
	return true;
}

// lobby type description
enum ELobbyType
{
	k_ELobbyTypePrivate = 0,		// only way to join the lobby is to invite to someone else
	k_ELobbyTypeFriendsOnly = 1,	// shows for friends or invitees, but not in lobby list
	k_ELobbyTypePublic = 2,			// visible for friends and in lobby list
	k_ELobbyTypeInvisible = 3,		// returned by search, but not visible to other friends 
									//    useful if you want a user in two lobbies, for example matching groups together
									//	  a user can be in only one regular lobby, and up to two invisible lobbies
};

// lobby search filter tools
enum ELobbyComparison
{
	k_ELobbyComparisonEqualToOrLessThan = -2,
	k_ELobbyComparisonLessThan = -1,
	k_ELobbyComparisonEqual = 0,
	k_ELobbyComparisonGreaterThan = 1,
	k_ELobbyComparisonEqualToOrGreaterThan = 2,
	k_ELobbyComparisonNotEqual = 3,
};

// lobby search distance. Lobby results are sorted from closest to farthest.
enum ELobbyDistanceFilter
{
	k_ELobbyDistanceFilterClose,		// only lobbies in the same immediate region will be returned
	k_ELobbyDistanceFilterDefault,		// only lobbies in the same region or near by regions
	k_ELobbyDistanceFilterFar,			// for games that don't have many latency requirements, will return lobbies about half-way around the globe
	k_ELobbyDistanceFilterWorldwide,	// no filtering, will match lobbies as far as India to NY (not recommended, expect multiple seconds of latency between the clients)
};

class ISteamMatchmaking
{
public:
	// game server favorites storage
	// saves basic details about a multiplayer game server locally

	// returns the number of favorites servers the user has stored
	virtual int GetFavoriteGameCount() = 0;

	// returns the details of the game server
	// iGame is of range [0,GetFavoriteGameCount())
	// *pnIP, *pnConnPort are filled in the with IP:port of the game server
	// *punFlags specify whether the game server was stored as an explicit favorite or in the history of connections
	// *pRTime32LastPlayedOnServer is filled in the with the Unix time the favorite was added
	virtual bool GetFavoriteGame(int iGame, AppId_t* pnAppID, uint32* pnIP, uint16* pnConnPort, uint16* pnQueryPort, uint32* punFlags, uint32* pRTime32LastPlayedOnServer) = 0;

	// adds the game server to the local list; updates the time played of the server if it already exists in the list
	virtual int AddFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer) = 0;

	// removes the game server from the local storage; returns true if one was removed
	virtual bool RemoveFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags) = 0;

	CALL_RESULT(LobbyMatchList_t)
		virtual SteamAPICall_t RequestLobbyList() = 0;
	// filters for lobbies
	// this needs to be called before RequestLobbyList() to take effect
	// these are cleared on each call to RequestLobbyList()
	virtual void AddRequestLobbyListStringFilter(const char* pchKeyToMatch, const char* pchValueToMatch, ELobbyComparison eComparisonType) = 0;
	// numerical comparison
	virtual void AddRequestLobbyListNumericalFilter(const char* pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType) = 0;
	// returns results closest to the specified value. Multiple near filters can be added, with early filters taking precedence
	virtual void AddRequestLobbyListNearValueFilter(const char* pchKeyToMatch, int nValueToBeCloseTo) = 0;
	// returns only lobbies with the specified number of slots available
	virtual void AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable) = 0;
	// sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed)
	virtual void AddRequestLobbyListDistanceFilter(ELobbyDistanceFilter eLobbyDistanceFilter) = 0;
	// sets how many results to return, the lower the count the faster it is to download the lobby results & details to the client
	virtual void AddRequestLobbyListResultCountFilter(int cMaxResults) = 0;

	virtual void AddRequestLobbyListCompatibleMembersFilter(CSteamID steamIDLobby) = 0;

	// returns the CSteamID of a lobby, as retrieved by a RequestLobbyList call
	// should only be called after a LobbyMatchList_t callback is received
	// iLobby is of the range [0, LobbyMatchList_t::m_nLobbiesMatching)
	// the returned CSteamID::IsValid() will be false if iLobby is out of range
	virtual CSteamID GetLobbyByIndex(int iLobby) = 0;

	// Create a lobby on the Steam servers.
	// If private, then the lobby will not be returned by any RequestLobbyList() call; the CSteamID
	// of the lobby will need to be communicated via game channels or via InviteUserToLobby()
	// this is an asynchronous request
	// results will be returned by LobbyCreated_t callback and call result; lobby is joined & ready to use at this point
	// a LobbyEnter_t callback will also be received (since the local user is joining their own lobby)
	CALL_RESULT(LobbyCreated_t)
		virtual SteamAPICall_t CreateLobby(ELobbyType eLobbyType, int cMaxMembers) = 0;

	// Joins an existing lobby
	// this is an asynchronous request
	// results will be returned by LobbyEnter_t callback & call result, check m_EChatRoomEnterResponse to see if was successful
	// lobby metadata is available to use immediately on this call completing
	CALL_RESULT(LobbyEnter_t)
		virtual SteamAPICall_t JoinLobby(CSteamID steamIDLobby) = 0;

	// Leave a lobby; this will take effect immediately on the client side
	// other users in the lobby will be notified by a LobbyChatUpdate_t callback
	virtual void LeaveLobby(CSteamID steamIDLobby) = 0;

	// Invite another user to the lobby
	// the target user will receive a LobbyInvite_t callback
	// will return true if the invite is successfully sent, whether or not the target responds
	// returns false if the local user is not connected to the Steam servers
	// if the other user clicks the join link, a GameLobbyJoinRequested_t will be posted if the user is in-game,
	// or if the game isn't running yet the game will be launched with the parameter +connect_lobby <64-bit lobby id>
	virtual bool InviteUserToLobby(CSteamID steamIDLobby, CSteamID steamIDInvitee) = 0;

	// Lobby iteration, for viewing details of users in a lobby
	// only accessible if the lobby user is a member of the specified lobby
	// persona information for other lobby members (name, avatar, etc.) will be asynchronously received
	// and accessible via ISteamFriends interface

	// returns the number of users in the specified lobby
	virtual int GetNumLobbyMembers(CSteamID steamIDLobby) = 0;
	// returns the CSteamID of a user in the lobby
	// iMember is of range [0,GetNumLobbyMembers())
	// note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
	virtual CSteamID GetLobbyMemberByIndex(CSteamID steamIDLobby, int iMember) = 0;

	// Get data associated with this lobby
	// takes a simple key, and returns the string associated with it
	// "" will be returned if no value is set, or if steamIDLobby is invalid
	virtual const char* GetLobbyData(CSteamID steamIDLobby, const char* pchKey) = 0;
	// Sets a key/value pair in the lobby metadata
	// each user in the lobby will be broadcast this new value, and any new users joining will receive any existing data
	// this can be used to set lobby names, map, etc.
	// to reset a key, just set it to ""
	// other users in the lobby will receive notification of the lobby data change via a LobbyDataUpdate_t callback
	virtual bool SetLobbyData(CSteamID steamIDLobby, const char* pchKey, const char* pchValue) = 0;

	// returns the number of metadata keys set on the specified lobby
	virtual int GetLobbyDataCount(CSteamID steamIDLobby) = 0;

	// returns a lobby metadata key/values pair by index, of range [0, GetLobbyDataCount())
	virtual bool GetLobbyDataByIndex(CSteamID steamIDLobby, int iLobbyData, char* pchKey, int cchKeyBufferSize, char* pchValue, int cchValueBufferSize) = 0;

	// removes a metadata key from the lobby
	virtual bool DeleteLobbyData(CSteamID steamIDLobby, const char* pchKey) = 0;

	// Gets per-user metadata for someone in this lobby
	virtual const char* GetLobbyMemberData(CSteamID steamIDLobby, CSteamID steamIDUser, const char* pchKey) = 0;
	// Sets per-user metadata (for the local user implicitly)
	virtual void SetLobbyMemberData(CSteamID steamIDLobby, const char* pchKey, const char* pchValue) = 0;

	// Broadcasts a chat message to the all the users in the lobby
	// users in the lobby (including the local user) will receive a LobbyChatMsg_t callback
	// returns true if the message is successfully sent
	// pvMsgBody can be binary or text data, up to 4k
	// if pvMsgBody is text, cubMsgBody should be strlen( text ) + 1, to include the null terminator
	virtual bool SendLobbyChatMsg(CSteamID steamIDLobby, const void* pvMsgBody, int cubMsgBody) = 0;
	// Get a chat message as specified in a LobbyChatMsg_t callback
	// iChatID is the LobbyChatMsg_t::m_iChatID value in the callback
	// *pSteamIDUser is filled in with the CSteamID of the member
	// *pvData is filled in with the message itself
	// return value is the number of bytes written into the buffer
	virtual int GetLobbyChatEntry(CSteamID steamIDLobby, int iChatID, OUT_STRUCT() CSteamID* pSteamIDUser, void* pvData, int cubData, EChatEntryType* peChatEntryType) = 0;

	// Refreshes metadata for a lobby you're not necessarily in right now
	// you never do this for lobbies you're a member of, only if your
	// this will send down all the metadata associated with a lobby
	// this is an asynchronous call
	// returns false if the local user is not connected to the Steam servers
	// results will be returned by a LobbyDataUpdate_t callback
	// if the specified lobby doesn't exist, LobbyDataUpdate_t::m_bSuccess will be set to false
	virtual bool RequestLobbyData(CSteamID steamIDLobby) = 0;

	// sets the game server associated with the lobby
	// usually at this point, the users will join the specified game server
	// either the IP/Port or the steamID of the game server has to be valid, depending on how you want the clients to be able to connect
	virtual void SetLobbyGameServer(CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer) = 0;
	// returns the details of a game server set in a lobby - returns false if there is no game server set, or that lobby doesn't exist
	virtual bool GetLobbyGameServer(CSteamID steamIDLobby, uint32* punGameServerIP, uint16* punGameServerPort, OUT_STRUCT() CSteamID* psteamIDGameServer) = 0;

	// set the limit on the # of users who can join the lobby
	virtual bool SetLobbyMemberLimit(CSteamID steamIDLobby, int cMaxMembers) = 0;
	// returns the current limit on the # of users who can join the lobby; returns 0 if no limit is defined
	virtual int GetLobbyMemberLimit(CSteamID steamIDLobby) = 0;

	// updates which type of lobby it is
	// only lobbies that are k_ELobbyTypePublic or k_ELobbyTypeInvisible, and are set to joinable, will be returned by RequestLobbyList() calls
	virtual bool SetLobbyType(CSteamID steamIDLobby, ELobbyType eLobbyType) = 0;

	// sets whether or not a lobby is joinable - defaults to true for a new lobby
	// if set to false, no user can join, even if they are a friend or have been invited
	virtual bool SetLobbyJoinable(CSteamID steamIDLobby, bool bLobbyJoinable) = 0;

	// returns the current lobby owner
	// you must be a member of the lobby to access this
	// there always one lobby owner - if the current owner leaves, another user will become the owner
	// it is possible (bur rare) to join a lobby just as the owner is leaving, thus entering a lobby with self as the owner
	virtual CSteamID GetLobbyOwner(CSteamID steamIDLobby) = 0;

	// changes who the lobby owner is
	// you must be the lobby owner for this to succeed, and steamIDNewOwner must be in the lobby
	// after completion, the local user will no longer be the owner
	virtual bool SetLobbyOwner(CSteamID steamIDLobby, CSteamID steamIDNewOwner) = 0;

	// link two lobbies for the purposes of checking player compatibility
	// you must be the lobby owner of both lobbies
	virtual bool SetLinkedLobby(CSteamID steamIDLobby, CSteamID steamIDLobbyDependent) = 0;

#ifdef _PS3
	// changes who the lobby owner is
	// you must be the lobby owner for this to succeed, and steamIDNewOwner must be in the lobby
	// after completion, the local user will no longer be the owner
	virtual void CheckForPSNGameBootInvite(unsigned int iGameBootAttributes) = 0;
#endif
	CALL_BACK(LobbyChatUpdate_t)
};

inline ISteamMatchmaking* steamMatchmaking;

// Steam API export macro
#if defined( _WIN32 ) && !defined( _X360 )
#if defined( STEAM_API_EXPORTS )
#define S_API extern "C" __declspec( dllexport ) 
#elif defined( STEAM_API_NODLL )
#define S_API extern "C"
#else
#define S_API extern "C" __declspec( dllimport ) 
#endif // STEAM_API_EXPORTS
#elif defined( GNUC )
#if defined( STEAM_API_EXPORTS )
#define S_API extern "C" __attribute__ ((visibility("default"))) 
#else
#define S_API extern "C" 
#endif // STEAM_API_EXPORTS
#else // !WIN32
#if defined( STEAM_API_EXPORTS )
#define S_API extern "C"  
#else
#define S_API extern "C" 
#endif // STEAM_API_EXPORTS
#endif
