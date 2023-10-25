#include <Hacks/ProfileChanger/Protobuffs.h>
#include "Memory.h"
#include "../imguiCustom.h"
#include <Interfaces.h>
#include <SDK/Cvar.h>
#include <SDK/Convar.h>
#include <SDK/Engine.h>
#include <SDK/Entity.h>
#include <SDK/GlobalVars.h>
#include <SDK/LocalPlayer.h>
#include <SDK/PlayerResource.h>
#include <SDK/steamtypes.h>
#include <SDK/isteamUserStats.h>
#include <Config.h>
#include "ProfileChanger.h"

static int site = 0,

compranktemp = 0,
compwinstemp = 10,
wingranktemp = 0,
wingwinstemp = 10,
dangranktemp = 0,
dangwinstemp = 10, /*Ranks*/

colortemp = 0, /*Team Color*/

statusTemp = 0,
lvlTemp = 1,
XPTemp = 0, /*Level*/

teachtemp = 0,
frientemp = 0,
leadstemp = 0, /*Commends*/

banTypeTemp = 0,
banTimeTemp = 0; /*Ban Alerts*/

bool hasCommunicationAbuseMute = false;

constexpr const char* accountPrivileges[] = {
    "None (NonPrime)",
    "Not Identifying",
    "Awaiting Cooldown",
    "Eligible",
    "Eligible With Takeover",
    "Elevated (Prime)",
    "Account Cooldown"
};
constexpr const char* privateLevels[] = {
    "Recruit (Level 1)",
    "Private (Level 2)",
    "Private (Level 3)",
    "Private (Level 4)",
    "Corporal (Level 5)",
    "Corporal (Level 6)",
    "Corporal (Level 7)",
    "Corporal (Level 8)",
    "Sergeant (Level 9)",
    "Sergeant (Level 10)",
    "Sergeant (Level 11)",
    "Sergeant (Level 12)",
    "Master Sergeant (Level 13)",
    "Master Sergeant (Level 14)",
    "Master Sergeant (Level 15)",
    "Master Sergeant (Level 16)",
    "Sergeant Major (Level 17)",
    "Sergeant Major (Level 18)",
    "Sergeant Major (Level 19)",
    "Sergeant Major (Level 20)",
    "Lieutenant (Level 21)",
    "Lieutenant (Level 22)",
    "Lieutenant (Level 23)",
    "Lieutenant (Level 24)",
    "Captain (Level 25)",
    "Captain (Level 26)",
    "Captain (Level 27)",
    "Captain (Level 28)",
    "Major (Level 29)",
    "Major (Level 30)",
    "Major (Level 31)",
    "Major (Level 32)",
    "Colonel (Level 33)",
    "Colonel (Level 34)",
    "Colonel (Level 35)",
    "Brigadier General (Level 36)",
    "Major General (Level 37)",
    "Lieutenant General (Level 38)",
    "General (Level 39)",
    "Global General (Level 40)"
};
constexpr const char* bans_gui[] = {
    "VAC ( Valve Anti-Cheat )",
    "You were kicked from the last match (competitive cooldown)",
    "You killed too many teammates (competitive cooldown)",
    "You killed a teammate at round start (competitive cooldown)",
    "You failed to reconnect to the last match (competitive cooldown)",
    "You abandoned the last match (competitive cooldown)",
    "You did too much damage to your teammates (competitive cooldown)",
    "You did too much damage to your teammates at round start (competitive cooldown)",
    "This account is permanently untrusted (global cooldown)",
    "You were kicked from too many recent matches (competitive cooldown)",
    "Convicted by overwatch - majorly disruptive (global cooldown)",
    "Convicted by overwatch - minorly disruptive (global cooldown)",
    "Resolving matchmaking state for your account (temporary cooldown)",
    "Resolving matchmaking state after the last match (temporary cooldown)",
    "This account is permanently untrusted (global cooldown)",
    "Global Cooldown",
    "You failed to connect by match start. (competitive cooldown)",
    "You have kicked too many teammates in recent matches (competitive cooldown)",
    "Congratulations on your recent competitive wins! before you play competitive matches further please wait for\nmatchmaking servers to calibrate your skill group placement based on your lastest performance. (temporary cooldown)",
    "You have recived significantly more griefing reports than most players (global cooldown)",
    "You have recived significantly more griefing reports than most players (competitive cooldown)" //20
};
constexpr const char* ranks_gui[] = {
    "Hidden / Expired",
    "Silver 1",
    "Silver 2",
    "Silver 3",
    "Silver 4",
    "Silver elite",
    "Silver elite master",
    "Gold nova 1",
    "Gold nova 2",
    "Gold nova 3",
    "Gold nova master",
    "Master guardian 1",
    "Master guardian 2",
    "Master guardian elite",
    "Distinguished master guardian",
    "Legendary eagle",
    "Legendary eagle master",
    "Supreme master first class",
    "The global elite"
};
constexpr const char* gz_ranks_gui[] = {
    "Hidden / Expired",
    "Lab Rat 1",
    "Lab Rat 2",
    "Sprinting Hare 1",
    "Sprinting Hare 2",
    "Wild Scout 1",
    "Wild Scout 2",
    "Wild Scout Elite",
    "Hunter Fox 1",
    "Hunter Fox 2",
    "Hunter Fox 3",
    "Hunter Fox Elite",
    "Timer Wolf",
    "Ember Wolf",
    "Wildfire Wolf",
    "The Howling Alpha"
};
constexpr const char* colorTeam[] = {
    "Gray",
    "Blank",
    "Yellow",
    "Purple",
    "Green",
    "Blue",
    "Orange"
};
constexpr const char* achievementsIdx[] = {
    "AVENGE_FRIEND",
    "BASE_SCAMPER",
    "BLOODLESS_VICTORY",
    "BOMB_DEFUSE_CLOSE_CALL",
    "BOMB_DEFUSE_LOW",
    "BOMB_MULTIKILL",
    "BOMB_PLANT_IN_25_SECONDS",
    "BOMB_PLANT_LOW",
    "BORN_READY",
    "BREAK_WINDOWS",
    "CAUSE_FRIENDLY_FIRE_WITH_FLASHBANG",
    "CONCURRENT_DOMINATIONS",
    "DAMAGE_NO_KILL",
    "DEAD_GRENADE_KILL",
    "DEFUSE_DEFENSE",
    "DOMINATIONS_HIGH",
    "DOMINATIONS_LOW",
    "DOMINATION_OVERKILLS_HIGH",
    "DOMINATION_OVERKILLS_LOW",
    "DOMINATION_OVERKILLS_MATCH",
    "DONATE_WEAPONS",
    "EARN_MONEY_HIGH",
    "EARN_MONEY_LOW",
    "EARN_MONEY_MED",
    "EXTENDED_DOMINATION",
    "FAST_HOSTAGE_RESCUE",
    "FAST_ROUND_WIN",
    "FLAWLESS_VICTORY",
    "GIVE_DAMAGE_HIGH",
    "GIVE_DAMAGE_LOW",
    "GIVE_DAMAGE_MED",
    "GOOSE_CHASE",
    "GRENADE_MULTIKILL",
    "GUN_GAME_CONSERVATIONIST",
    "GUN_GAME_FIRST_KILL",
    "GUN_GAME_FIRST_THING_FIRST",
    "GUN_GAME_KILL_KNIFER",
    "GUN_GAME_KNIFE_KILL_KNIFER",
    "GUN_GAME_RAMPAGE",
    "GUN_GAME_ROUNDS_HIGH",
    "GUN_GAME_ROUNDS_LOW",
    "GUN_GAME_ROUNDS_MED",
    "GUN_GAME_SMG_KILL_KNIFER",
    "GUN_GAME_TARGET_SECURED",
    "HEADSHOTS",
    "HEADSHOTS_IN_ROUND",
    "HIP_SHOT",
    "IMMOVABLE_OBJECT",
    "KILLED_DEFUSER_WITH_GRENADE",
    "KILLER_AND_ENEMY_IN_AIR",
    "KILLING_SPREE",
    "KILLING_SPREE_ENDER",
    "KILLS_ENEMY_WEAPON",
    "KILLS_WITH_MULTIPLE_GUNS",
    "KILL_BOMB_DEFUSER",
    "KILL_BOMB_PICKUP",
    "KILL_ENEMIES_WHILE_BLIND",
    "KILL_ENEMIES_WHILE_BLIND_HARD",
    "KILL_ENEMY_AK47",
    "KILL_ENEMY_AUG",
    "KILL_ENEMY_AWP",
    "KILL_ENEMY_BIZON",
    "KILL_ENEMY_BLINDED",
    "KILL_ENEMY_DEAGLE",
    "KILL_ENEMY_ELITE",
    "KILL_ENEMY_FAMAS",
    "KILL_ENEMY_FIVESEVEN",
    "KILL_ENEMY_G3SG1",
    "KILL_ENEMY_GALILAR",
    "KILL_ENEMY_GLOCK",
    "KILL_ENEMY_HEGRENADE",
    "KILL_ENEMY_HIGH",
    "KILL_ENEMY_HKP2000",
    "KILL_ENEMY_IN_AIR",
    "KILL_ENEMY_KNIFE",
    "KILL_ENEMY_LAST_BULLET",
    "KILL_ENEMY_LOW",
    "KILL_ENEMY_M249",
    "KILL_ENEMY_M4A1",
    "KILL_ENEMY_MAC10",
    "KILL_ENEMY_MAG7",
    "KILL_ENEMY_MED",
    "KILL_ENEMY_MOLOTOV",
    "KILL_ENEMY_MP7",
    "KILL_ENEMY_MP9",
    "KILL_ENEMY_NEGEV",
    "KILL_ENEMY_NOVA",
    "KILL_ENEMY_P250",
    "KILL_ENEMY_P90",
    "KILL_ENEMY_RELOADING",
    "KILL_ENEMY_SAWEDOFF",
    "KILL_ENEMY_SCAR20",
    "KILL_ENEMY_SG556",
    "KILL_ENEMY_SSG08",
    "KILL_ENEMY_TASER",
    "KILL_ENEMY_TEAM",
    "KILL_ENEMY_TEC9",
    "KILL_ENEMY_UMP45",
    "KILL_ENEMY_XM1014",
    "KILL_HOSTAGE_RESCUER",
    "KILL_LOW_DAMAGE",
    "KILL_SNIPERS",
    "KILL_SNIPER_WITH_KNIFE",
    "KILL_SNIPER_WITH_SNIPER",
    "KILL_TWO_WITH_ONE_SHOT",
    "KILL_WHEN_AT_LOW_HEALTH",
    "KILL_WHILE_IN_AIR",
    "KILL_WITH_EVERY_WEAPON",
    "KILL_WITH_OWN_GUN",
    "LAST_PLAYER_ALIVE",
    "LOSSLESS_EXTERMINATION",
    "MEDALIST",
    "META_PISTOL",
    "META_RIFLE",
    "META_SHOTGUN",
    "META_SMG",
    "META_WEAPONMASTER",
    "ONE_SHOT_ONE_KILL",
    "PISTOL_ROUND_KNIFE_KILL",
    "PLAY_EVERY_GUNGAME_MAP",
    "RESCUE_ALL_HOSTAGES",
    "RESCUE_HOSTAGES_LOW",
    "RESCUE_HOSTAGES_MED",
    "REVENGES_HIGH",
    "REVENGES_LOW",
    "SILENT_WIN",
    "STILL_ALIVE",
    "SURVIVE_GRENADE",
    "SURVIVE_MANY_ATTACKS",
    "TR_BOMB_DEFUSE_LOW",
    "TR_BOMB_PLANT_LOW",
    "UNSTOPPABLE_FORCE",
    "WIN_BOMB_DEFUSE",
    "WIN_BOMB_PLANT",
    "WIN_BOMB_PLANT_AFTER_RECOVERY",
    "WIN_DUAL_DUEL",
    "WIN_EVERY_GUNGAME_MAP",
    "WIN_GUN_GAME_ROUNDS_EXTREME",
    "WIN_GUN_GAME_ROUNDS_HIGH",
    "WIN_GUN_GAME_ROUNDS_LOW",
    "WIN_GUN_GAME_ROUNDS_MED",
    "WIN_GUN_GAME_ROUNDS_ULTIMATE",
    "WIN_KNIFE_FIGHTS_HIGH",
    "WIN_KNIFE_FIGHTS_LOW",
    "WIN_MAP_AR_BAGGAGE",
    "WIN_MAP_AR_SHOOTS",
    "WIN_MAP_CS_ITALY",
    "WIN_MAP_CS_OFFICE",
    "WIN_MAP_DE_AZTEC",
    "WIN_MAP_DE_BANK",
    "WIN_MAP_DE_DUST",
    "WIN_MAP_DE_DUST2",
    "WIN_MAP_DE_INFERNO",
    "WIN_MAP_DE_LAKE",
    "WIN_MAP_DE_NUKE",
    "WIN_MAP_DE_SAFEHOUSE",
    "WIN_MAP_DE_SHORTTRAIN",
    "WIN_MAP_DE_STMARC",
    "WIN_MAP_DE_SUGARCANE",
    "WIN_MAP_DE_TRAIN",
    "WIN_PISTOLROUNDS_HIGH",
    "WIN_PISTOLROUNDS_LOW",
    "WIN_PISTOLROUNDS_MED",
    "WIN_ROUNDS_HIGH",
    "WIN_ROUNDS_LOW",
    "WIN_ROUNDS_MED",
    "WIN_ROUNDS_WITHOUT_BUYING"
};


void Profile::ChangeInGame() {
    auto playerResource = *memory->playerResource;

    if (!interfaces->engine->isInGame() || !playerResource) return;

    if (config->profilechanger.enabledRanks) {
        playerResource->level()[localPlayer->index()] = config->profilechanger.level + 1;
        playerResource->teammateColor()[localPlayer->index()] = config->profilechanger.colorTeam - 2;
        playerResource->hasCommunicationAbuseMute()[localPlayer->index()] = config->profilechanger.hasCommunicationAbuseMute;

        switch (interfaces->cvar->findVar("game_mode")->getInt()) {
        case 1:
            playerResource->competitiveRanking()[localPlayer->index()] = config->profilechanger.rank;
            break;
        case 2:
            playerResource->competitiveRanking()[localPlayer->index()] = config->profilechanger.wmrank;
            break;
        }
    }
}

void Profile::updateInfo() {
    if ((config->profilechanger.rank != compranktemp) || (config->profilechanger.wins != compwinstemp) && config->profilechanger.enabledRanks) {
        write.SendClientGcRankUpdate(6); //mm
        compranktemp = config->profilechanger.rank;
        compwinstemp = config->profilechanger.wins;
        ChangeInGame();
    }
    if ((config->profilechanger.wmrank != wingranktemp) || (config->profilechanger.wmwins != wingwinstemp) && config->profilechanger.enabledRanks) {
        write.SendClientGcRankUpdate(7); //wm
        wingranktemp = config->profilechanger.wmrank;
        wingwinstemp = config->profilechanger.wmwins;
        ChangeInGame();
    }
    if ((config->profilechanger.dzrank != dangranktemp) || (config->profilechanger.dzwins != dangwinstemp) && config->profilechanger.enabledRanks) {
        write.SendClientGcRankUpdate(10); //dz
        dangranktemp = config->profilechanger.dzrank;
        dangwinstemp = config->profilechanger.dzwins;
        ChangeInGame();
    }

    if ((config->profilechanger.level != lvlTemp) || (config->profilechanger.exp != XPTemp) && config->profilechanger.enabledRanks) {
        write.SendMatchmakingClient2GCHello(); //level
        lvlTemp = config->profilechanger.level;
        XPTemp = config->profilechanger.exp;
        ChangeInGame();
    }
    if (config->profilechanger.colorTeam != colortemp && config->profilechanger.enabledRanks) {
        ChangeInGame(); //Team Color
        colortemp = config->profilechanger.colorTeam;
    }
    if (config->profilechanger.hasCommunicationAbuseMute != hasCommunicationAbuseMute && config->profilechanger.enabledRanks) {
        ChangeInGame(); //Mute
        hasCommunicationAbuseMute = config->profilechanger.hasCommunicationAbuseMute;
    }
    if (config->profilechanger.accountStatus != statusTemp && config->profilechanger.enabledRanks) {
        write.SendMatchmakingClient2GCHello(); //prime
        statusTemp = config->profilechanger.accountStatus;
    }
    if (config->profilechanger.leader != leadstemp && config->profilechanger.enabledCommends) {
        write.SendMatchmakingClient2GCHello();
        leadstemp = config->profilechanger.leader;
    }
    if (config->profilechanger.teach != teachtemp && config->profilechanger.enabledCommends) {
        write.SendMatchmakingClient2GCHello();
        teachtemp = config->profilechanger.teach;
    }
    if (config->profilechanger.friendly != frientemp && config->profilechanger.enabledCommends) {
        write.SendMatchmakingClient2GCHello();
        frientemp = config->profilechanger.friendly;
    }
    if (config->profilechanger.banTime != banTimeTemp && config->profilechanger.enabledBans) {
        write.SendMatchmakingClient2GCHello();
        banTimeTemp = config->profilechanger.banTime;
    }
    if (config->profilechanger.banType != banTypeTemp && config->profilechanger.enabledBans) {
        write.SendMatchmakingClient2GCHello();
        banTypeTemp = config->profilechanger.banType;
    }
}

static bool windowOpen = false;

void Profile::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Profile Changer")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Profile Changer");
        ImGui::SetWindowPos("Profile Changer", { 100.0f, 100.0f });
    }
}

void Profile::drawGUI(bool contentOnly) noexcept
{
    auto playerResource = *memory->playerResource;
    ImGuiWindowFlags windowPCFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 400.0f, 240.0f });
        ImGui::Begin("Profile Changer", &windowOpen, windowPCFlags);
    }

    if (ImGui::Button("Ranks", { 64.000f, 32.000f })) site = 0;

    ImGui::SameLine();
    if (ImGui::Button("Commends", { 0.000f, 32.000f })) site = 1;

    ImGui::SameLine();
    if (ImGui::Button("Ban Alerts", { 0.000f, 32.000f })) site = 2;

    ImGui::SameLine();
    if (ImGui::Button("Stats", { 56.000f, 32.000f })) site = 3;

    ImGui::SameLine();
    if (ImGui::Button("Achievements", { 0.000f, 32.000f })) site = 4;
    ImGui::Separator();

    switch (site) {
    default:
    case 0:
        ImGui::PushItemWidth(100.f);
        ImGui::Checkbox("Enable Ranks", &config->profilechanger.enabledRanks);
        ImGui::Text("Competitive Rank");
        ImGui::Combo("##Competitive Rank", &config->profilechanger.rank, ranks_gui, ARRAYSIZE(ranks_gui));
        ImGui::SameLine();
        ImGui::InputInt("##Competitive Wins", &config->profilechanger.wins);

        ImGui::Text("Wingman Rank");
        ImGui::Combo("##Wingman Rank", &config->profilechanger.wmrank, ranks_gui, ARRAYSIZE(ranks_gui));
        ImGui::SameLine();
        ImGui::InputInt("##Wingman Wins", &config->profilechanger.wmwins);

        ImGui::Text("Danger Zone Rank");
        ImGui::Combo("##Danger Zone Rank", &config->profilechanger.dzrank, gz_ranks_gui, ARRAYSIZE(gz_ranks_gui));
        ImGui::SameLine();
        ImGui::InputInt("##Danger Zone Wins", &config->profilechanger.dzwins);

        ImGui::Text("Level"); ImGui::SameLine();
        ImGui::Dummy({ 64.f, 0.f }); ImGui::SameLine();
        ImGui::Text("XP");
        ImGui::Combo("##Level", &config->profilechanger.level, privateLevels, ARRAYSIZE(privateLevels));
        ImGui::SameLine();
        ImGui::SliderInt("##Xp##level", &config->profilechanger.exp, 0, 5000);
        ImGui::Text("Account Status"); ImGui::SameLine();
        ImGui::Dummy({ 12.f, 0.f }); ImGui::SameLine();
        ImGui::Text("Team Color");
        ImGui::Combo("##statusacc", &config->profilechanger.accountStatus, accountPrivileges, ARRAYSIZE(accountPrivileges));
        ImGui::SameLine();
        ImGui::Combo("##colorTeam", &config->profilechanger.colorTeam, colorTeam, ARRAYSIZE(colorTeam));
        ImGui::PopItemWidth();
        ImGui::Checkbox("Communication Abuse Mute", &config->profilechanger.hasCommunicationAbuseMute);

        break;
    case 1:
        ImGui::PushItemWidth(100.f);
        ImGui::Checkbox("Enable Commends", &config->profilechanger.enabledCommends);
        ImGui::Text("Friend");
        ImGui::InputInt("##Friend", &config->profilechanger.friendly);
        ImGui::Text("Teach");
        ImGui::InputInt("##Teach", &config->profilechanger.teach);
        ImGui::Text("Leader");
        ImGui::InputInt("##Leader", &config->profilechanger.leader);
        ImGui::PopItemWidth();
        break;
    case 2:
        ImGui::Checkbox("Enable Ban Alerts", &config->profilechanger.enabledBans);
        ImGui::PushItemWidth(100.f);
        ImGui::Text("Fake ban type");
        ImGui::Combo("##fake-ban", &config->profilechanger.banType, bans_gui, IM_ARRAYSIZE(bans_gui));
        ImGui::Text("Fake ban time");
        ImGui::InputInt("##fake-ban-time", &config->profilechanger.banTime, 0, 60);
        ImGui::PopItemWidth();
        break;
    case 3:
        if (ImGui::Button("Update")) {
            if (!config->profilechanger.enabledStats) return;
            if (!interfaces->engine->isInGame()) return;
            playerResource->GetMVPs()[localPlayer->index()] = config->profilechanger.mvp;
            playerResource->GetScore()[localPlayer->index()] = config->profilechanger.score;
            playerResource->kills()[localPlayer->index()] = config->profilechanger.kills;
            playerResource->assists()[localPlayer->index()] = config->profilechanger.assists;
            playerResource->deaths()[localPlayer->index()] = config->profilechanger.deaths;
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(100.f);
        ImGui::Checkbox("Enable Game Stats", &config->profilechanger.enabledStats);
        ImGui::Text("Kills");
        ImGui::InputInt("##kills", &config->profilechanger.kills, 0, 999);
        ImGui::Text("Assists");
        ImGui::InputInt("##assists", &config->profilechanger.assists, 0, 999);
        ImGui::Text("Deaths");
        ImGui::InputInt("##deaths", &config->profilechanger.deaths, 0, 999);
        ImGui::Text("MVPs");
        ImGui::InputInt("##mvps", &config->profilechanger.mvp, 0, 999);
        ImGui::Text("Score");
        ImGui::InputInt("##score", &config->profilechanger.score, 0, 9999);
        ImGui::PopItemWidth();
        break;
    case 4:
        static int aidx = 0;
        ImGui::PushID(01);
        if (ImGui::Button("Grant All Achievements"))
        {
            steamUserStats->RequestCurrentStats();
            for (uint32 i = 0; i < steamUserStats->GetNumAchievements(); i++)
                steamUserStats->SetAchievement(steamUserStats->GetAchievementName(i));
            steamUserStats->StoreStats();
        }
        ImGui::PopID();
        ImGui::PushID(02);
        ImGui::SameLine();
        if (ImGui::Button("Revoke All Achievements"))
        {
            steamUserStats->RequestCurrentStats();
            for (uint32 i = 0; i < steamUserStats->GetNumAchievements(); i++)
                steamUserStats->ClearAchievement(steamUserStats->GetAchievementName(i));
            steamUserStats->StoreStats();
        }
        ImGui::PopID();
        ImGui::PushID(03);
        ImGui::SetNextItemWidth(120.f);
        ImGui::Combo("##achievementsName", &aidx, achievementsIdx, ARRAYSIZE(achievementsIdx));
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID(04);
        if (ImGui::Button("Grant Achievement")){
            steamUserStats->RequestCurrentStats();
            steamUserStats->SetAchievement(steamUserStats->GetAchievementName(aidx));
            steamUserStats->StoreStats();
        }
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID(05);
        if (ImGui::Button("Revoke Achievement")){
            steamUserStats->RequestCurrentStats();
            steamUserStats->ClearAchievement(steamUserStats->GetAchievementName(aidx));
            steamUserStats->StoreStats();
        }
        ImGui::PopID();
    }

    if (!contentOnly)
        ImGui::End();
}