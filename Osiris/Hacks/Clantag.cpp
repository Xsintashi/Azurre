#include "Clantag.h"

#include "../Interfaces.h"
#include "../Memory.h"

#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Entity.h"
#include "../SDK/Engine.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/PlayerResource.h"
#include <GameData.h>

struct ClanTagConfig {
    
    struct Custom {
        std::string prefix = "";
        std::string tag = "";
        std::string teamTag = "";
        std::string postfix = "";
        bool hideName = false;
        int mode = 0;
        float speed = .5f;
    } custom;

    int mode = { 0 };
    bool stealerShowAll = false;
} clanCfg;

void azurreClanTag() noexcept {

    const char* clantagAzurre[] = {
    
    "rre            ",
    "re             ",
    "e              ",
    "               ",
    "              a",
    "             az",
    "            azu",
    "           azur",
    "          azurr",
    "         azurre",
    "        azurre ",
    "       azurre  ",
    "      azurre   ",
    "     azurre    ",
    "    azurre     ",
    "   azurre      ",
    "  azurre       ",
    " azurre        ",
    "azurre         ",
    "zurre          ",
    "urre           ",
    };

    float latency = interfaces->engine->getNetworkChannel()->getLatency(0) + interfaces->engine->getNetworkChannel()->getLatency(1);

    int serverTime = static_cast<int>(((memory->globalVars->currenttime / 0.296875f) + 6.60925f - 0.07f) - (interfaces->engine->getNetworkChannel()->getLatency(0) + interfaces->engine->getNetworkChannel()->getLatency(1)));
    memory->setClanTag(std::string(clantagAzurre[serverTime % 21]).append("\n").c_str(), clantagAzurre[serverTime % 21]);
}

void gamesenzeClanTag() noexcept {

    const char* clantagGamesense[] = {

        "sense          ",
        "ense           ",
        "nse            ",
        "se             ",
        "e              ",
        "               ",
        "             ga",
        "            gam",
        "           game",
        "          games",
        "         gamese",
        "        gamesen",
        "       gamesens",
        "      gamesense",
        "      gamesense",
        "     gamesense ",
        "     gamesense ",
        "    gamesense  ",
        "    gamesense  ",
        "   gamesense   ",
        "   gamesense   ",
        "  gamesense    ",
        "  gamesense    ",
        " gamesense     ",
        " gamesense     ",
        "gamesense      ",
        "gamesense      ",
        "amesense       ",
        "mesense        ",
        "esense         "

    };

    float latency = interfaces->engine->getNetworkChannel()->getLatency(0) + interfaces->engine->getNetworkChannel()->getLatency(1);

    int serverTime = static_cast<int>(((memory->globalVars->currenttime / 0.296875f) + 6.60925f - 0.07f) - (interfaces->engine->getNetworkChannel()->getLatency(0) + interfaces->engine->getNetworkChannel()->getLatency(1)));
    memory->setClanTag(clantagGamesense[serverTime % 30], clantagGamesense[serverTime % 30]);
}

struct neededVars {
    int index;
    std::string name;
};

static  int stealFromIdx = -1;
static bool stealEnabled = false;

std::vector<neededVars>players;

void ClanTagStealer::update() noexcept{
    players.clear();
    if (!localPlayer) return;
    std::vector<std::reference_wrapper<const PlayerData>> playersOrdered{ GameData::players().begin(), GameData::players().end() };
    for (const PlayerData& player : playersOrdered) {
        players.push_back({ player.index , player.name});
    }
}

void Clan::update(bool reset, bool update) noexcept
{
    if (!localPlayer) return;

    static auto clanId = interfaces->cvar->findVar("cl_clanid");
    static bool wasEnabled = false;

    if (wasEnabled && !clanCfg.mode)
    {
        interfaces->engine->clientCmdUnrestricted(("cl_clanid " + std::to_string(clanId->getInt())).c_str());
        wasEnabled = false;
        return;
    }

    if (!clanCfg.mode) return;

    wasEnabled = clanCfg.mode;

    if (reset) {
        interfaces->engine->clientCmdUnrestricted(("cl_clanid " + std::to_string(clanId->getInt())).c_str());
        return;
    }

    static float prevTime = 0.f;
    static float timeToUpdate = 0.f;
    float realTime = memory->globalVars->realtime;

    static auto steal = []() {
        if (!stealEnabled || stealFromIdx == -1) return;

        if (!localPlayer) {
            stealFromIdx = -1;
            stealEnabled = false;
            return;
        }

        auto playerResource = *memory->playerResource;

        if (!playerResource) return;

        static std::string clanTmp;
        std::string clan = (playerResource->getClan(stealFromIdx));

        if (clanTmp == clan) return;

        memory->setClanTag(clan.c_str(), clan.c_str());
        clanTmp = clan;
    };

    static auto custom = [](bool updateCustom) {

        static std::string clan;
        static std::string clanTemp;
        static std::string addRemoveTemp;
        float realTime = memory->globalVars->realtime;
        static float realTimeSwitcher = 0.f;
        static unsigned int addRemoveMod = 0;

        auto upd = []() {

            if (clanTemp == clanCfg.custom.tag) return;

            clanTemp = clanCfg.custom.tag;
            clan = clanCfg.custom.tag;


            while (clan.find("\\a") != std::string::npos)
                clan.replace(clan.find("\\a"), 2, "\a");

            while (clan.find("\\b") != std::string::npos)
                clan.replace(clan.find("\\b"), 2, "\b");

            while (clan.find("\\f") != std::string::npos)
                clan.replace(clan.find("\\f"), 2, "\f");

            while (clan.find("\\n") != std::string::npos)
                clan.replace(clan.find("\\n"), 2, "\n");

            while (clan.find("\\r") != std::string::npos)
                clan.replace(clan.find("\\r"), 2, "\r");

            while (clan.find("\\t") != std::string::npos)
                clan.replace(clan.find("\\t"), 2, "\t");

            while (clan.find("\\v") != std::string::npos)
                clan.replace(clan.find("\\v"), 2, "\v");

            return;
        };

        upd();

        if (realTime - realTimeSwitcher < clanCfg.custom.speed) return;
        addRemoveMod++;

        switch (clanCfg.custom.mode) {
            default:
            case 0: // static
                break;
            case 1: // rotate
                if (const auto offset = Helpers::utf8SeqLen(clan[0]); offset <= clan.length())
                    std::rotate(clan.begin(), clan.begin() + offset, clan.end());
                break;
            case 2: // rotate backwards
                if (const auto offset = Helpers::utf8SeqLen(clan[0]); offset <= clan.length())
                    std::rotate(clan.rbegin(), clan.rbegin() + offset, clan.rend());
                break;
            case 3: // add
                addRemoveTemp = clanTemp;
                clan = addRemoveTemp.substr(0, addRemoveMod % addRemoveTemp.size() + 1);
                break;
            case 4: // remove
                std::string addRemoveTemp;
                addRemoveTemp = clanTemp;
                clan = addRemoveTemp.substr(0, clanTemp.size() - addRemoveMod % addRemoveTemp.size());
                break;
        }

        realTimeSwitcher = realTime;

        memory->setClanTag(std::string(clanCfg.custom.prefix).append(clan).append(clanCfg.custom.postfix).append(clanCfg.custom.hideName ? "\xE2\x80\xA9" : "").c_str(), clanCfg.custom.teamTag.c_str());
    };

    static auto string = [](const char* clanText) {
        memory->setClanTag(clanText, "| azurre |");
    };
    static auto clock = []() {
        if (clanCfg.mode != 2) return;

        static auto lastTime = 0.0f;

        if (memory->globalVars->realtime - lastTime < 1.0f)
            return;

        const auto time = std::time(nullptr);
        const auto localTime = std::localtime(&time);
        char s[11];
        s[0] = '\0';
        snprintf(s, sizeof(s), "[%02d:%02d:%02d]", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
        lastTime = memory->globalVars->realtime;
        memory->setClanTag(s, "| azurre |");
    };
    static auto velocity = []() {
        const auto vel = localPlayer->velocity().length2D();
        std::string velstring = std::to_string((int)vel);
        memory->setClanTag(velstring.c_str(), std::string(velstring).append("km/h WPIERDOL PEEK").c_str());
    };
    static auto position = []() {
        const auto place = localPlayer->lastPlaceName();
        memory->setClanTag(place, "| azurre |");
    };
    static auto health = []() {
        const int hp = localPlayer->health();
        std::string result = std::to_string(hp) + "HP";
        memory->setClanTag(result.c_str(), "| azurre |");
    };

    if (realTime - timeToUpdate > 5.f) {
        ClanTagStealer::update();
        timeToUpdate = realTime;
    }

    if (realTime - prevTime < 0.15f) return;

    prevTime = realTime;

    switch (clanCfg.mode) {
    default: return;

    case 1: azurreClanTag(); return;
    case 2: clock(); return;
    case 3: string("\u202e\u202e"); return;
    case 4: velocity(); return;
    case 5: position(); return;
    case 6: health(); return;
    case 7: string("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"); return;
    case 8: gamesenzeClanTag(); return;
    case 9: custom(update); return;
    case 10: steal(); return;
    }

}

//0 - Off
//1 - Azurre
//2 - Clock
//3 - Reverse
//4 - Velocity
//5 - Last Place
//6 - HP
//7 - \n overflow
//8 - gamesense
//10 - Custom

void Clan::drawGUI() {
    ImGui::PushID("ClanTag");
    
    auto playerResource = *memory->playerResource;
    GameData::Lock lock;

    ImGui::SetNextItemWidth(80.0f);
    ImGui::Combo("ClanTag", &clanCfg.mode, "None\0Azurre\0Clock\0Reverse\0Velocity\0Position\0HP\0\\n Overflow\0gamesense\0Custom\0Stealer\0");
    if (clanCfg.mode == 9) {
        ImGui::PushID("ClanTagCustom");
        ImGui::SameLine();
        if (ImGui::Button("..."))
            ImGui::OpenPopup("##custom");

        if (ImGui::BeginPopup("##custom")) {
            ImGui::PushItemWidth(120.f);

            ImGui::Combo("Type", &clanCfg.custom.mode, "Static\0Rotate\0Rotate Backwards\0Add\0Remove\0");
            if (ImGui::InputText("Name", &clanCfg.custom.tag))
                update(false, true);
            if (ImGui::InputText("Team", &clanCfg.custom.teamTag))
                update(false, true);
            if (ImGui::InputText("Prefix", &clanCfg.custom.prefix))
                update(false, true);
            if (ImGui::InputText("Postfix", &clanCfg.custom.postfix))
                update(false, true);
            ImGui::SliderFloat("##speed", &clanCfg.custom.speed, 0.01f , 1.f, "Speed: %.2f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
            ImGui::Checkbox("Hide Name", &clanCfg.custom.hideName);
            ImGui::Separator();
            ImGui::PopItemWidth();

            ImGui::Text("%s", playerResource ? playerResource->getClan(localPlayer->index()) : "unknown");
            ImGui::SameLine();
            ImGui::Text("%s", GameData::local().name.c_str());
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    if (clanCfg.mode == 10) {
        ImGui::PushID("ClanTagStealer");
        ImGui::SameLine();
        if (ImGui::Button("..."))
            ImGui::OpenPopup("##stealer");

        if (ImGui::BeginPopup("##stealer")) {
            ImGui::PushItemWidth(120.f);
            ImGui::Checkbox("Show All", &clanCfg.stealerShowAll);
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                stealFromIdx = -1;
                stealEnabled = false;
                update(true);
            }
            if (!players.empty() && localPlayer && playerResource) 
            {
                if (ImGui::BeginTable("", 3)) {
                    ImGui::TableSetupColumn("Player");
                    ImGui::TableSetupColumn("Clantag");
                    ImGui::TableSetupColumn("");
                    ImGui::TableHeadersRow();

                    for (unsigned int n = 0; n < players.size(); n++) {

                        if (players[n].index == localPlayer->index()) continue;

                        auto c = playerResource->getClan(players[n].index);

                        if (!clanCfg.stealerShowAll && c && !c[0]) continue;

                        ImGui::TableNextRow();
                        ImGui::PushID(ImGui::TableGetRowIndex());

                        if (ImGui::TableNextColumn())
                            ImGui::Text("%s", players[n].name.c_str());

                        if (ImGui::TableNextColumn())
                            ImGui::Text("%s", c);

                        if (ImGui::TableNextColumn())
                            if (ImGui::Button("Steal")) {
                                stealFromIdx = players[n].index;
                                stealEnabled = true;
                            }

                        ImGui::PopID();

                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

static void from_json(const json& j, ClanTagConfig::Custom& c)
{
    read<value_t::string>(j, "Prefix", c.prefix);
    read<value_t::string>(j, "Tag", c.tag);
    read<value_t::string>(j, "Team Tag", c.teamTag);
    read<value_t::string>(j, "Postfix", c.postfix);
    read(j, "Hide Name", c.hideName);
    read(j, "Mode", c.mode);
    read(j, "Speed", c.speed);
}

static void from_json(const json& j, ClanTagConfig& ct)
{
    read(j, "Mode", ct.mode);
    read(j, "Stealer Show All Players", ct.stealerShowAll);
    read<value_t::object>(j, "Custom", ct.custom);
}

static void to_json(json& j, const ClanTagConfig::Custom& o, const ClanTagConfig::Custom& dummy = {})
{
    WRITE("Prefix", prefix);
    WRITE("Tag", tag);
    WRITE("Team Tag", teamTag);
    WRITE("Postfix", postfix);
    WRITE("Hide Name", hideName);
    WRITE("Mode", mode);
    WRITE("Speed", speed);
}

static void to_json(json& j, const ClanTagConfig& o)
{
    const ClanTagConfig dummy;
    WRITE("Mode", mode);
    WRITE("Stealer Show All Players", stealerShowAll);
    WRITE("Custom", custom);
}

json Clan::toJson() noexcept
{
    json j;
    to_json(j, clanCfg);
    return j;
}

void Clan::fromJson(const json& j) noexcept
{
    from_json(j, clanCfg);
}

void Clan::resetConfig() noexcept
{
    clanCfg = {};
}