#pragma once

#include "Inconstructible.h"
#include "../Netvars.h"
#include "VirtualMethod.h"

struct Vector;

class IPlayerResource {
public:
    INCONSTRUCTIBLE(IPlayerResource)

    VIRTUAL_METHOD_V(bool, isAlive, 5, (int index), (this, index))
    VIRTUAL_METHOD_V(const char*, getPlayerName, 8, (int index), (this, index))
    VIRTUAL_METHOD_V(int, getPlayerHealth, 14, (int index), (this, index))
};

class PlayerResource {
public:

    auto getIPlayerResource() noexcept
    {
        return reinterpret_cast<IPlayerResource*>(std::uintptr_t(this) + WIN32_LINUX(0x9D8, 0xF68));
    }
    
    NETVAR(bombsiteCenterA, "CCSPlayerResource", "m_bombsiteCenterA", Vector)
    NETVAR(bombsiteCenterB, "CCSPlayerResource", "m_bombsiteCenterB", Vector)
    NETVAR(armor, "CCSPlayerResource", "m_iArmor", int[65])
    NETVAR(activeCoinRank, "CCSPlayerResource", "m_nActiveCoinRank", int[65])
    NETVAR(competitiveWins, "CCSPlayerResource", "m_iCompetitiveWins", int[65])
    NETVAR(competitiveRanking, "CCSPlayerResource", "m_iCompetitiveRanking", int[65])
    NETVAR(teammateColor, "CCSPlayerResource", "m_iCompTeammateColor", int[65])
    NETVAR(musicID, "CCSPlayerResource", "m_nMusicID", int[65])
    NETVAR(level, "CCSPlayerResource", "m_nPersonaDataPublicLevel", int[65])

    NETVAR(kills, "CCSPlayerResource", "m_iKills", int[65])
    NETVAR(assists, "CCSPlayerResource", "m_iAssists", int[65])
    NETVAR(deaths, "CCSPlayerResource", "m_iDeaths", int[65])

    NETVAR(ping, "CCSPlayerResource", "m_iPing", int[65])
    NETVAR(BombCarrier, "CCSPlayerResource", "m_iPlayerC4", int)
    NETVAR(hasCommunicationAbuseMute, "CCSPlayerResource", "m_bHasCommunicationAbuseMute", bool[65])

    NETVAR(GetMVPs, "CCSPlayerResource", "m_iMVPs", int[65])
    NETVAR(GetScore, "CCSPlayerResource", "m_iScore", int[65])

    const char* getClan(int idx) noexcept
    {
        if (!this)
            return nullptr;
        static auto m_szClan = Netvars::get(fnv::hash("CCSPlayerResource->m_szClan"));
        return reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(this) + m_szClan + (idx * 16));
    }
};
