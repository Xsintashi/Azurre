#include "Grief.h"
#include "HelpersAimbot.h"
#include "Misc.h"

#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PlayerResource.h"

#include "../GUI.h"
#include "../imgui/imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../imguiCustom.h"
#include "../ConfigStructs.h"
#include "../Interfaces.h"
#include "../ProtobufReader.h"

#include <pstring.h>
#include <array>
#include <algorithm>

struct FunConfig {
    std::string nick{ "*Player Name*" };

    int nickColor{ 1 };
    std::string inputNick = "";
    bool nameStealer = false,
         kickNames = false,
         unmuteAll = false;

    KeyBind blockBotKey;

    struct DisconnectMessage {
        bool enabled = false;
        std::string str = "#Player_DisconnectReason_ConvictedAccount";
    } disconnet;

    struct Spam {
        bool
            enabled = false,
            say = false,
            DidNotAsk = false;

        int
            team = 1,
            shows = 0;
        KeyBind DidNotAskKey;
    }spam;

    std::string nameSpam[4] = {
        "I'm",
        "I'm Legit,",
        "I'm Legit, I",
        "I'm Legit, I swear!"
    };

}gCfg;

std::string tempnick = "";

void chatChangeNameSpam(char const* names) noexcept {
    static auto cv{ interfaces->cvar->findVar("name") };
    cv->onChangeCallbacks.size = NULL;
    cv->setValue(names);
}

bool static spamName = false;

bool Fun::unmuteAllPlayers() noexcept {
    return gCfg.unmuteAll;
}

bool Fun::getDisconnect() noexcept {
    return gCfg.disconnet.enabled;
}

const char* Fun::getDisconnectMessage() noexcept {
    return gCfg.disconnet.str.c_str();
}

void Fun::prepareNamesSpam() noexcept {

    if (!spamName) return;
    if (!localPlayer) return;
    std::string ogName = localPlayer->getPlayerName();

    static float temp = memory->globalVars->currenttime;
    static int c = 0;

    if (c < 5 && spamName) {//true
        if (float modul = memory->globalVars->currenttime; modul - temp > 0.1) {
            if (gCfg.nameSpam[c].empty()) return;

            chatChangeNameSpam(gCfg.nameSpam[c].c_str());
            temp = memory->globalVars->currenttime;
            c++;
        }
    }
    else {
        changeName(false, ogName.c_str(), 5.f);
        spamName = false;
        c = 0;
    }

}

bool Fun::changeName(bool reconnect, const char* newName, float delay) noexcept
{
    static auto exploitInitialized{ false };

    static auto name{ interfaces->cvar->findVar("name") };

    if (reconnect) {
        exploitInitialized = false;
        return false;
    }

    if (!exploitInitialized && interfaces->engine->isInGame()) {
        if (PlayerInfo playerInfo; localPlayer && interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo) && (playerInfo.name == std::string{ "?empty" } || playerInfo.name == std::string{ "unnamed" } || playerInfo.name == std::string{ "\n\xAD\xAD\xAD" })){
            exploitInitialized = true;
        }
        else {
            name->onChangeCallbacks.size = 0;
            name->setValue("\n\xAD\xAD\xAD");
            return false;
        }
    }

    if (static auto nextChangeTime = 0.0f; nextChangeTime <= memory->globalVars->realtime) {
        name->setValue(newName);
        nextChangeTime = memory->globalVars->realtime + delay;
        return true;
    }
    return false;
}

void Fun::stealNames() noexcept
{
    if (!gCfg.nameStealer)
        return;

    if (!localPlayer)
        return;

    static std::vector<int> stolenIds;

    for (int i = 1; i <= memory->globalVars->maxClients; ++i) {
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get())
            continue;

        PlayerInfo playerInfo;
        if (!interfaces->engine->getPlayerInfo(entity->index(), playerInfo))
            continue;

        if (playerInfo.fakeplayer || std::ranges::find(stolenIds, playerInfo.userId) != stolenIds.cend())
            continue;

        if (changeName(false, (std::string{ playerInfo.name } + '\x1' + '\x1').c_str(), 1.0f))
            stolenIds.push_back(playerInfo.userId);

        return;
    }
    stolenIds.clear();
}

constexpr const char* fancy[]{
"\x1D\x67\x0",
"\x1D\x68\xA",
"\x1D\x67\x1",
"\x1D\x68\xB",
"\x1D\x67\x2",
"\x1D\x68\xC",
"\x1D\x67\x3",
"\x1D\x68\xD",
"\x1D\x67\x4",
"\x1D\x68\xE",
"\x1D\x67\x5",
"\x1D\x68\xF",
"\x1D\x67\x6",
"\x1D\x69\x0",
"\x1D\x67\x7",
"\x1D\x69\x1",
"\x1D\x67\x8",
"\x1D\x69\x2",
"\x1D\x67\x9",
"\x1D\x69\x3",
"\x1D\x67\xA",
"\x1D\x69\x4",
"\x1D\x67\xB",
"\x1D\x69\x5",
"\x1D\x67\xC",
"\x1D\x69\x6",
"\x1D\x67\xD",
"\x1D\x69\x7",
"\x1D\x67\xE",
"\x1D\x69\x8",
"\x1D\x67\xF",
"\x1D\x69\x9",
"\x1D\x68\x0",
"\x1D\x69\xA",
"\x1D\x68\x1",
"\x1D\x69\xB",
"\x1D\x68\x2",
"\x1D\x69\xC",
"\x1D\x68\x3",
"\x1D\x69\xD",
"\x1D\x68\x4",
"\x1D\x69\xE",
"\x1D\x68\x5",
"\x1D\x69\xF",
"\x1D\x68\x6",
"\x1D\x6A\x0",
"\x1D\x68\x7",
"\x1D\x6A\x1",
"\x1D\x68\x8",
"\x1D\x6A\x2",
"\x1D\x68\x9",
"\x1D\x6A\x3"
};
constexpr const char* normal[]{
    "A", "a", "B", "b", "C", "c", "D", "d",
    "E", "e", "F", "f", "G", "g", "H", "h",
    "I", "i", "J", "j", "K", "k", "L", "l",
    "M", "m", "N", "n", "O", "o", "P", "p",
    "Q", "q", "R", "r", "S", "s", "T", "t",
    "U", "u", "V", "v", "W", "w", "X", "x",
    "Y", "y", "Z", "z"
};

void Fun::repeatSomeoneMessage(const void*& data, int& size) noexcept{

    const auto reader = ProtobufReader{ static_cast<const std::uint8_t*>(data), size };
    const auto strings = reader.readRepeatedString(4);
    if (strings.size() < 4)
        return;

    std::string message = strings[1];

    std::string editedMessage;
    Helpers::CheatChatNotification(true, message.c_str());
}

void Fun::blockBot(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static int blockTargetHandle = 0;

    if (!gCfg.blockBotKey.isDown())
    {
        blockTargetHandle = 0;
        return;
    }

    float best = 1024.0f;
    if (!blockTargetHandle)
    {
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
        {
            Entity* entity = interfaces->entityList->getEntity(i);

            if (!entity || !entity->isPlayer() || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive())
                continue;

            const auto distance = entity->getAbsOrigin().distTo(localPlayer->getAbsOrigin());
            if (distance < best)
            {
                best = distance;
                blockTargetHandle = entity->handle();
            }
        }
    }

    const auto target = interfaces->entityList->getEntityFromHandle(blockTargetHandle);
    if (target && target->isPlayer() && target != localPlayer.get() && !target->isDormant() && target->isAlive())
    {
        const auto targetVec = (target->getAbsOrigin() + target->velocity() * memory->globalVars->intervalPerTick - localPlayer->getAbsOrigin());
        const auto z1 = target->getAbsOrigin().z - localPlayer->getEyePosition().z;
        const auto z2 = target->getEyePosition().z - localPlayer->getAbsOrigin().z;
        if (z1 >= 0.0f || z2 <= 0.0f)
        {
            Vector fwd = Vector::fromAngle2D(cmd->viewangles.y);
            Vector side = fwd.crossProduct(Vector::up());
            Vector move = Vector{ fwd.dotProduct2D(targetVec), side.dotProduct2D(targetVec), 0.0f };
            move *= 45.0f;

            const float l = move.length2D();
            if (l > 450.0f)
                move *= 450.0f / l;

            cmd->forwardmove = move.x;
            cmd->sidemove = move.y;
        }
        else
        {
            Vector fwd = Vector::fromAngle2D(cmd->viewangles.y);
            Vector side = fwd.crossProduct(Vector::up());
            Vector tar = (targetVec / targetVec.length2D()).crossProduct(Vector::up());
            tar = tar.snapTo4();
            tar *= tar.dotProduct2D(targetVec);
            Vector move = Vector{ fwd.dotProduct2D(tar), side.dotProduct2D(tar), 0.0f };
            move *= 45.0f;

            const float l = move.length2D();
            if (l > 450.0f)
                move *= 450.0f / l;

            cmd->forwardmove = move.x;
            cmd->sidemove = move.y;
        }
    }
}

void Fun::changeMyName(bool set) noexcept
{
    static bool shouldSet = false;

    if (set)
        shouldSet = set;

    if (shouldSet && interfaces->engine->isInGame() && changeName(false, std::string{ Helpers::TextColors(gCfg.nickColor)}.append(tempnick).append("\x1\x1").c_str(), 5.0f))
        shouldSet = false;
}

void Fun::ChatSpammer() noexcept
{

    if (!localPlayer) return;

    if (Misc::isInChat()) return;

    if (gCfg.spam.DidNotAskKey.isPressed()) {


        std::vector<std::string> prefix = { "no name","yo buddy", "yo pal", "listen here,", "basically...", "hahaha", "yeah well", "yeah ok however", "k", "lol", "dude", "ok but", "ok", "lmaooo ok", "lol ok", "funny uh", "uh ok", "tahts funny", "that is funny", "that's funny", "uh welp ok", "ok but like", "k but", "kk", ":)", "nigga", "cool dog", "puppy", "imagine" };
        std::vector<std::string> middle = { "did i ask", "ask i did not", "name one person who asked", "didnt ask", "dont recall asking", "i didnt ask", "i really dont remember asking", "xd who ask", "did not ask", "times i asked = 0", "when i asked = never", "me asking didnt happen", "never asked ", "i never asked", "did i ask", "who asked", ", i didn’t ask", "I wish I asked", ". My teammate has 8 apples, his train is 18 minutes late. Calculate probability of me asking, taking into account, I didn't ask tho" };
        std::vector<std::string> suffix = { "so yeah", "do you understand", "so shut up", " bro", "nn", "nn boi", "dude", "lmao", "lol", "lmfao", "haha", "tho", "though lol", " tho lmao", "jajajja", "xaxaxa", "xddd", ", lol", "lmao", "XDD", "ezzzzz", "mi amigo", "xd jajaja", "hahaha", ":/", "so plz be quiet", "hehe", "so shush", "boo hoo", "ayayayaya", "dawg"};

        auto say = "say ";
        std::string reply = say + prefix[rand() % prefix.size()] + " ";
        if (rand() > 0.8)
            reply += middle[rand() % middle.size()] + " ";
        if (rand() > 0.5)
            reply += suffix[rand() % suffix.size()];


        interfaces->engine->clientCmdUnrestricted(reply.c_str());
    }

    if (!gCfg.spam.enabled)
        return;

    int ServerTime = static_cast<int>(localPlayer->tickBase() * 3.5f * memory->globalVars->intervalPerTick);
    int repeat = ServerTime % 2;
    static int lastId = 1;
    auto playerResource = *memory->playerResource;
    if (!gCfg.spam.shows) return;

    if (repeat)
    {
        static int lastId = 1;
        for (int i = lastId; i < interfaces->engine->getMaxClients(); i++)
        {
            const auto Player = interfaces->entityList->getEntity(i);

            lastId++;
            if (lastId == interfaces->engine->getMaxClients())
                lastId = 1;
            if (!Player || !Player->isAlive())
                continue;

            if (gCfg.spam.team == 0 && Player->getTeamNumber() != localPlayer->getTeamNumber())
                continue;

            if (gCfg.spam.team == 1 && Player->getTeamNumber() == localPlayer->getTeamNumber())
                continue;

            pstring str;

            str = gCfg.spam.say ? "say_team" : "say";
            str << " \"";

            str << "\xE2\x80\xA8";

            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 0)) {
                str << "Name: " << Player->getPlayerName();
            }
            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 1)) {
                str << " | ";
                str << "Rank: " << Helpers::ConvertRankFromNumber(true, playerResource->competitiveRanking()[i]);
                str << "(" << playerResource->competitiveWins()[i] << ")";
            }

            str << "\xE2\x80\xA8";

            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 2)) {
                str << "HP: " << Player->health();
            }
            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 3)) {
                str << " | ";
                str << "Armor: " << Player->armor() << (Player->hasHelmet() ? "+H" : "");
            }
            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 4)) {
                str << " | ";
                str << "$" << Player->account();
            }

            str << "\xE2\x80\xA8";

            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 5)) {
                str << std::string(Player->getActiveWeapon()->getWeaponData()->name).substr(13, std::string(Player->getActiveWeapon()->getWeaponData()->name).length() - 13);
            }

            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 6)) {
                str << " | ";
                str << "At: " << Player->lastPlaceName();
            }

            if (Helpers::getByteFromBytewise(gCfg.spam.shows, 7)) {
                str << " | ";
                str << (Player->hasDefuser() ? "Has Defuser" : "");
            }
            str << "\"";

            interfaces->engine->clientCmdUnrestricted(str.c_str());
            break;
        }
    }
    
}

static bool windowOpen = false;

void Fun::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Trolling")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Trolling");
        ImGui::SetWindowPos("Trolling", { 100.0f, 100.0f });
    }
}

void Fun::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        ImGui::Begin("Trolling", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    ImGui::PushID("Chat spam");
    if (!gCfg.spam.enabled) {
        if (ImGui::Button("Start")) {
            if (localPlayer)
                gCfg.spam.enabled = true;
        }
    }
    else {
        if (ImGui::Button("Stop")) {
            gCfg.spam.enabled = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("xsd");

    if (ImGui::BeginPopup("xsd")) {
        ImGui::Checkbox("Team Chat (?)###SAY_TEAM2", &gCfg.spam.say);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Only spam messages in team chat");
        ImGui::SetNextItemWidth(100.f);
        ImGui::Combo("###POSITIONSTEAM", &gCfg.spam.team, "Allies\0Enemies\0Both\0");
        ImGui::SetNextItemWidth(100.f);
        ImGuiCustom::multiCombo("Show", gCfg.spam.shows, "Name\0Rank\0Health\0Armor\0Money\0Weapon\0Last Place\0Defuser\0");
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::hotkey("Ask Key", gCfg.spam.DidNotAskKey);

    ImGui::Separator();
    if (ImGui::Button("Change Name")) {
    tempnick = gCfg.inputNick;
    Fun::changeMyName(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Fake Ban Yourself")) {
        memory->setClanTag("", "");
        tempnick = std::string("\n\x1" + gCfg.inputNick + " has been permanently banned from official CS:GO servers." + "\n\x0Player" + gCfg.inputNick).c_str();
        changeMyName(true);
    }
    ImGui::SetNextItemWidth(80.0f);
    ImGui::PushID("fake nick");
    ImGui::InputText("", &gCfg.inputNick);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID("fake nick color");
    ImGui::SetNextItemWidth(80.0f);
    ImGui::Combo("", &gCfg.nickColor, "White\0Red\0Pale Purple\0Green\0Pale green\0Money green\0Light Red\0Gray\0Yellow\0Pale Blue Gray\0Turquoise\0Blue\0Purple\0Pink\0Light Red 2\0Gold\0");
    ImGui::PopID();
    ImGui::Checkbox("Name stealer", &gCfg.nameStealer);
    ImGui::PushID("chatnamespam");

    if (ImGui::Button("Spam")) {
        spamName = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("spamNamer");

    if (ImGui::BeginPopup("spamNamer")) {
        ImGui::Text("1:"); ImGui::SameLine(); ImGui::InputText("##1", &gCfg.nameSpam[0]);
        ImGui::Text("2:"); ImGui::SameLine(); ImGui::InputText("##2", &gCfg.nameSpam[1]);
        ImGui::Text("3:"); ImGui::SameLine(); ImGui::InputText("##3", &gCfg.nameSpam[2]);
        ImGui::Text("4:"); ImGui::SameLine(); ImGui::InputText("##4", &gCfg.nameSpam[3]);
        ImGui::EndPopup();
    }

    ImGui::PopID();
    ImGui::Separator();
    ImGui::PushID("Block Bot Key");
    ImGui::hotkey("Block Bot", gCfg.blockBotKey);
    ImGui::PopID();
    ImGui::Separator();
    ImGui::PushID("Disconnect");
    ImGui::Checkbox("Disconnect Message", &gCfg.disconnet.enabled);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);
    ImGui::InputText("", &gCfg.disconnet.str);
    ImGui::PopID();

    if (!contentOnly)
        ImGui::End();
}

static void from_json(const json& j, FunConfig::Spam& s) {

    read(j, "Enabled", s.enabled);
    read(j, "Say Team", s.say);
    read(j, "Shows", s.shows);
    read(j, "Team", s.team);
    read(j, "Didnt Ask Enabled", s.DidNotAsk);
    read(j, "Didnt Ask Key", s.DidNotAskKey);
}



static void to_json(json& j, const FunConfig::Spam& o, const FunConfig::Spam& dummy = {}) {

   WRITE("Enabled", enabled);
   WRITE("Say Team",say);
   WRITE("Shows", shows);
   WRITE("Team", team);
   WRITE("Didnt Ask Enabled", DidNotAsk);
   WRITE("Didnt Ask Key", DidNotAskKey);
}

static void to_json(json& j, const FunConfig& o)
{
    const FunConfig dummy;

    WRITE("Block Bot Key", blockBotKey);
    WRITE("Name Stealer", nameStealer);
    WRITE("Name Spam 1 Message", nameSpam[0]);
    WRITE("Name Spam 2 Message", nameSpam[1]);
    WRITE("Name Spam 3 Message", nameSpam[2]);
    WRITE("Name Spam 4 Message", nameSpam[3]);
    WRITE("Spam", spam);
}

static void from_json(const json& j, FunConfig& fun)
{
    read(j, "Block Bot Key", fun.blockBotKey);
    read(j, "Name Stealer", fun.nameStealer);
    read<value_t::string>(j, "Name Spam 1 Message", fun.nameSpam[0]);
    read<value_t::string>(j, "Name Spam 2 Message", fun.nameSpam[1]);
    read<value_t::string>(j, "Name Spam 3 Message", fun.nameSpam[2]);
    read<value_t::string>(j, "Name Spam 4 Message", fun.nameSpam[3]);
    read<value_t::object>(j, "Spam", fun.spam);
}

json Fun::toJson() noexcept
{
    json j;
    to_json(j, gCfg);
    return j;
}

void Fun::fromJson(const json& j) noexcept
{
    from_json(j, gCfg);
}

void Fun::resetConfig() noexcept
{
    gCfg = {};
}