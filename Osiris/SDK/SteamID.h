#pragma once

#include "steamtypes.h"

class CSteamID
{
public:
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = 0;
		m_steamid.m_comp.m_EUniverse = 0;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}
	uint32_t GetAccountID() const { return m_steamid.m_comp.m_unAccountID; }

	inline bool operator==(const CSteamID& val) const { return m_steamid.m_unAll64Bits == val.m_steamid.m_unAll64Bits; }
	inline bool operator!=(const CSteamID& val) const { return !operator==(val); }
	inline bool operator<(const CSteamID& val) const { return m_steamid.m_unAll64Bits < val.m_steamid.m_unAll64Bits; }
	inline bool operator>(const CSteamID& val) const { return m_steamid.m_unAll64Bits > val.m_steamid.m_unAll64Bits; }

private:
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
			uint32_t			m_unAccountID : 32;			// unique account identifier
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID (used for multiseat type accounts only)
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			int					m_EUniverse : 8;	// universe this account belongs to
		} m_comp;

		uint64_t m_unAll64Bits;
	} m_steamid;
};