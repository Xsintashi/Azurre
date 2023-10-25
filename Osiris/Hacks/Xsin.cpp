// This file is debug/admin file only for authorized ppl

//SERVER CRASHER LOL
//pCmd->viewangles.x = std::numeric_limits<float>::quiet_NaN();
//pCmd->viewangles.y = std::numeric_limits<float>::quiet_NaN();
//pCmd->viewangles.z = std::numeric_limits<float>::quiet_NaN();

#include "Xsin.h"

#include "../tinyformat.h"
#include <Hacks/AntiAim.h>
#include <Hacks/Tickbase.h>
#include <Hacks/Panorama.h>
#include <Hacks/Notifications.h>

#include "../DiscordSDK/RPC.h"
#include "ProfileChanger/Protobuffs.h"

#include "../SDK/Constants/ClassId.h"
#include "../SDK/Client.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ClientMode.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/Constants/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/PlayerResource.h"
#include "../SDK/Panorama.h"
#include "../SDK/Platform.h"
#include "../SDK/UserCmd.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../SDK/ViewSetup.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/WeaponSystem.h"
#include "../SDK/Steam.h"
#include "../SDK/SteamAPI.h"
#include "../SDK/isteamuserstats.h"
#include "../SDK/isteamfriends.h"
#include "../SDK/Surface.h"
#include "../SDK/Panorama.h"
#include "../SDK/Input.h"
#include "../SDK/Effects.h"
#include "../SDK/NetworkStringTable.h"
#include "Extra.h"
#include "Backtrack.h"

struct XsinConfig {
    bool renderDemoWindow = false;
    bool rainbow = false;
    bool jsConsole = false;
    bool debug0 = false;
    bool debug1 = false;
    bool debug2 = false;
    bool debug3 = false;
    bool debug4 = false;
    bool debug5 = false;
    bool debugWindow =
#if defined(_DEBUG)
        false
#else
        false
#endif
        ;

    int iDebug0 = 0;
    int iDebug5 = 0;
    int iDebug4 = 0;
    int iDebug3 = 0;
    int iDebug2 = 0;
    int iDebug1 = 0;

    bool serverHitboxes = false;
    int customLvl = 0;
    int customRank = 0;
    int customColor = 0;

    std::string customKnife = "";

    Color4 colors;

    bool debugBox = false;
}xsnCfg;

std::string executeJsPath = "";
std::string executeJs = "";
static bool windowJs = false;
static bool closeWindowsAfterExec = false;

static bool windowOpen = false;

static bool sentPacket = true;
bool Xsin::SentPacket(bool& sendPacket) noexcept {
    sentPacket = sendPacket;
    return sentPacket;
}

bool Xsin::ImGuiDemo() noexcept {
    return xsnCfg.renderDemoWindow;
}

std::string Xsin::getKnifeModel() noexcept {
    return xsnCfg.customKnife;
}

bool Xsin::getDebug1(){
    return xsnCfg.debug1;
}

void Xsin::rainbowColor() noexcept {
    if (!xsnCfg.rainbow)
        return;

    if (!localPlayer) return;

    auto playerResource = *memory->playerResource;

    if (!playerResource) return;

    playerResource->teammateColor()[localPlayer->index()] = (static_cast<int>(memory->globalVars->currenttime * 10.f) % 5);
}

void getColors() {
    const auto& colors = ImGui::GetStyle().Colors;
    std::ostringstream ss;

    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        ss << "colors[ImGuiCol_" << ImGui::GetStyleColorName(i) << "] = ImVec4(";
        ss.precision(2);
        ss << std::fixed << colors[i].x;
        ss << "f, ";
        ss << colors[i].y;
        ss << "f, ";
        ss << colors[i].z;
        ss << "f, ";
        ss << colors[i].w;
        ss << "f);\n";
    }
    ImGui::SetClipboardText(ss.str().c_str());
}

void Xsin::printDebug(const char* x) {
    if(xsnCfg.debug0)
        printf(std::string(x).append("\n").c_str());
}

constexpr const char* panelsName[]{
    "CSGOHud",
    "CSGOMainMenu",
    "MainMenuContainerPanel"
};

int execInPanel = 0;

void Xsin::javaScriptConsole() {

    if (!xsnCfg.jsConsole) return;

    if (!gui->isOpen()) return;

    ImGui::SetNextWindowSize({ 640.0f, 720.0f });
    ImGui::Begin("V8JsPanorama", &xsnCfg.jsConsole, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(256.f);
    ImGui::Combo("Panel", &execInPanel, panelsName, IM_ARRAYSIZE(panelsName));

    ImGui::SameLine();
    ImGui::InputText("Path", &executeJsPath);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Checkbox("Close", &closeWindowsAfterExec);
    ImGui::InputTextMultiline("", &executeJs, { -1.f, -24.f }, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
    if (ImGui::Button("Execute", { 256.f, 24.f })) {
        if (!Panorama::getPanel(execInPanel)) {
            ImGui::End();
            return;
        }
        interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(execInPanel), executeJs.c_str(), "", 8, 10, false, false);
        if (closeWindowsAfterExec)
            xsnCfg.jsConsole = false;
    }


    ImGui::End();
}

void customRankss() noexcept {
    write.SendMatchmakingClient2GCHello();
    auto playerResource = *memory->playerResource;

    if (!interfaces->engine->isInGame() || !playerResource) return;

    playerResource->level()[localPlayer->index()] = xsnCfg.customLvl;
    playerResource->competitiveRanking()[localPlayer->index()] = xsnCfg.customRank;
    playerResource->teammateColor()[localPlayer->index()] = xsnCfg.customColor;
    config->profilechanger.rank = xsnCfg.customRank;
    config->profilechanger.level = xsnCfg.customLvl - 1;
}

void Xsin::renderServerHitboxes() noexcept {
    if (!xsnCfg.serverHitboxes) return;
    if (!localPlayer) return;
    if (!localPlayer->isAlive()) return;
    if (!memory->input->isCameraInThirdPerson) return;

    localPlayer->drawServerHitboxes(0.1f, 0);
}

void guiContent() {
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 288.0f);
    ImGui::Text("Debug");
    ImGui::Checkbox("Box", &debug.enable);
    ImGui::Checkbox("Dev Window", &xsnCfg.debugWindow);
    ImGui::Checkbox("V8JsPanorama Console", &xsnCfg.jsConsole);
    ImGui::Checkbox("Render Demo Window", &xsnCfg.renderDemoWindow);
    if (ImGui::Button("Update Backtrack CVars"))
        Backtrack::init();
    if (ImGui::Button("Open console"))
        interfaces->engine->clientCmdUnrestricted("con_enable 1;toggleconsole");
    if (ImGui::Button("Demo UI"))
        interfaces->engine->clientCmdUnrestricted("demoui");
    if (ImGui::Button("Fog UI"))
        interfaces->engine->clientCmdUnrestricted("fogui");
    if (ImGui::Button("Loaded textures"))
        interfaces->cvar->findVar("mat_texture_list")->setValue(true);
    if (ImGui::Button("Full Update"))
        interfaces->cvar->findVar("cl_fullupdate")->changeCallback();
    if (ImGui::Button("Send test console Notification")) {
        memory->conColorMsg({ 0, 0 ,0 ,255 }, "\n#");
        memory->conColorMsg({ 64, 64 ,64 ,255 }, "#");
        memory->conColorMsg({ 192, 192 ,192 ,255 }, "#");
        memory->conColorMsg({ 255, 255 ,255 ,255 }, "#\n");

        memory->conColorMsg({ 255 ,0   ,0   , 255 }, "#");
        memory->conColorMsg({ 255 ,106 ,0   , 255 }, "#");
        memory->conColorMsg({ 255 ,216 ,0   , 255 }, "#");
        memory->conColorMsg({ 182 ,225 ,0   , 255 }, "#");
        memory->conColorMsg({ 182 ,225 ,0   , 16 }, " #\n");
        memory->conColorMsg({ 76  ,255 ,0   , 255 }, "#");
        memory->conColorMsg({ 0   ,255 ,144 , 255 }, "#");
        memory->conColorMsg({ 0   ,255 ,255 , 255 }, "#");
        memory->conColorMsg({ 0   ,148 ,255 , 255 }, "#");
        memory->conColorMsg({ 0   ,148 ,255 , 64 }, " #\n");
        memory->conColorMsg({ 0   ,0   ,255 , 255 }, "#");
        memory->conColorMsg({ 72  ,0   ,255 , 255 }, "#");
        memory->conColorMsg({ 178 ,0   ,255 , 255 }, "#");
        memory->conColorMsg({ 255 ,0   , 220, 255 }, "#");
        memory->conColorMsg({ 255 ,0   , 220, 192 }, " #\n\n");
    }
    if (ImGui::Button("Send test notification")) {
        Notifications::addNotification(xsnCfg.colors, "Test notification!");
    }
    if (ImGui::Button("List client classes"))
    {
        for (int i = 0; i <= interfaces->entityList->getHighestEntityIndex(); i++)
        {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity) continue;

            memory->conColorMsg({ 0, 200, 0, 255 }, std::to_string(i).c_str());
            memory->debugMsg(": ");
            memory->conColorMsg({ 0, 120, 255, 255 }, entity->getClientClass()->networkName);
            memory->debugMsg(" -> ");
            memory->conColorMsg({ 255, 120, 255, 255 }, std::to_string((int)entity->getClientClass()->classId).c_str());
            memory->debugMsg("   ");
        }
    }
    if (ImGui::Button("Test chat virtual methods"))
        memory->clientMode->getHudChat()->printf(0, "\x1Z \x2Z \x3Z \x4Z \x5Z \x6Z \x7Z \x8Z \x9Z \xAZ \xBZ \xCZ \xDZ \xEZ \xFZ \x10Z \x1");

    if (ImGui::Button("Test alert message"))
        Helpers::getAlertMessage("Azurre");


    if (ImGui::Button("Test chat prefix")) {
        Helpers::CheatChatNotification(false, "Not Shorten");
        Helpers::CheatChatNotification(true, "Shorten");
    }
    if (ImGui::Button("Copy style colors"))
        getColors();
    ImGui::NextColumn();


    ImGui::Image(GameData::getLogo(), { 128.f, 128.f });
    ImGui::Checkbox("Show Player Server Hitboxes", &xsnCfg.serverHitboxes);
    ImGuiCustom::colorPicker("Color", xsnCfg.colors);
    ImGui::PushID("custom ranks");
    ImGui::PushItemWidth(100.f);
    ImGui::InputInt("Custom Level", &xsnCfg.customLvl, 1, 10);
    ImGui::InputInt("Custom Rank", &xsnCfg.customRank, 1, 10);
    ImGui::InputInt("Custom Color", &xsnCfg.customColor, 1, 10);
    ImGui::PopItemWidth();
    if (ImGui::Button("Update"))
        customRankss();
    ImGui::PopID();

#ifdef _WIN32
    ImGui::Text("Windows NT");
#elif __linux__
    ImGui::Text("Linux");
#endif // WIN32
    ImGui::Checkbox("Debug0", &debug.customKnife.enabled);
    ImGuiCustom::colorPicker("Color 1", debug.customKnife.color1.color.data());
    ImGuiCustom::colorPicker("Color 2", debug.customKnife.color2.color.data());
    ImGuiCustom::colorPicker("Color 3", debug.customKnife.color3.color.data());
    ImGuiCustom::colorPicker("Color 4", debug.customKnife.color4.color.data());

    ImGui::Checkbox("Debug0", &xsnCfg.debug0);
    ImGui::Checkbox("Debug1", &xsnCfg.debug1);
    ImGui::Checkbox("Debug2", &xsnCfg.debug2);
    ImGui::Checkbox("Debug3", &xsnCfg.debug3);
    ImGui::Checkbox("Debug4", &xsnCfg.debug4);
    ImGui::Checkbox("Debug5", &xsnCfg.debug5);
    ImGui::InputInt("iDebug0", &xsnCfg.iDebug0);
    ImGui::InputInt("iDebug1", &xsnCfg.iDebug1);
    ImGui::InputInt("iDebug2", &xsnCfg.iDebug2);
    ImGui::InputInt("iDebug3", &xsnCfg.iDebug3);
    ImGui::InputInt("iDebug4", &xsnCfg.iDebug4);
    ImGui::InputInt("iDebug5", &xsnCfg.iDebug5);
    ImGui::Checkbox("Rainbow Color", &xsnCfg.rainbow);
    const char* name = steamIFriends->GetPersonaName();
    ImGui::Text("Name: %s", name);
    ImGui::Text("CurTime: %.1f", memory->globalVars->currenttime);
    ImGui::Text("RealTime: %.1f", memory->globalVars->realtime);
    ImGui::Text("LocalPlayer: "); ImGui::SameLine();
    ImGui::TextColored({ 0.0f, 0.38f, 1.0f, 1.0f }, "0x%p", localPlayer ? localPlayer.get() : 0);

    if (localPlayer) {
        ImGui::Text("Origin: %.1f %.1f %.1f", localPlayer->getAbsOrigin().x, localPlayer->getAbsOrigin().y, localPlayer->getAbsOrigin().z); ImGui::SameLine();
        if (ImGui::Button("Cout Origin")) {
            std::cout << "{" << localPlayer->getAbsOrigin().x << "f, " << localPlayer->getAbsOrigin().y << "f, " << localPlayer->getAbsOrigin().z << "f }" << std::endl;
        }
        ImGui::Text("Angle: %.1f %.1f %.1f", interfaces->engine->getViewAngles().x, interfaces->engine->getViewAngles().y, interfaces->engine->getViewAngles().z); ImGui::SameLine();
        if (ImGui::Button("Cout Angle")) {
            std::cout << "{" << interfaces->engine->getViewAngles().x << "f, " << interfaces->engine->getViewAngles().y << "f, " << "0.f }" << std::endl;
        }
        float Velocity = localPlayer->velocity().length2D();
        ImGui::Text("Max Desync Angle: %.2f", localPlayer->getMaxDesyncAngle());
        ImGui::Text("vel: %.2f", Velocity);
        ImGui::Text("LBY: %.2f", AntiAim::getLBYUpdate());

        ImGui::Text("Duck Speed: %.2f", localPlayer->duckSpeed());
        ImGui::Text("Duck Amount: %.2f", localPlayer->duckAmount());
        ImGui::Text("Paused Ticks: %i", Tickbase::pausedTicks());

        ImGui::Text("Weapon Accuracy Speed: %.2f", Extra::getWeaponAccuracySpeed());
    }
    ImGui::Text("Console Enabled: %s", interfaces->engine->isConsoleVisible() ? "true" : "false");
    ImGui::Text("Breaking LC?: %s (%.2f)", AntiAim::breakLC(sentPacket) > 4096.f ? "true" : "false", AntiAim::breakLC(sentPacket));

    if (ImGui::Button("Custom Discord Rich Presense")) {
        ImGui::OpenPopup("l");
    }

    if (ImGui::BeginPopup("l")) {
        static std::string state;
        static std::string details;
        static int endTimestamp = 0;
        static std::string smallImageKey;
        static std::string largeImageKey;
        static std::string largeImageText;

        ImGui::InputText("State: ", &state);
        ImGui::InputText("Details: ", &details);
        ImGui::InputInt("End Time stamp: ", &endTimestamp);
        ImGui::InputText("Small Image Key: ", &smallImageKey);
        ImGui::InputText("Large Image Key: ", &largeImageKey);
        ImGui::InputText("Large Image Text: ", &largeImageText);

        if (ImGui::Button("Update"))
            Discord::customRichPresenseTest(
                state.c_str(),
                details.c_str(),
                endTimestamp,
                smallImageKey.c_str(),
                largeImageKey.c_str(),
                largeImageText.c_str());

        ImGui::EndPopup();
    }

    ImGui::Columns(1);
}

void Xsin::debugBox(ImDrawList* drawList) noexcept {
    if (!debug.enable) return;
    if (!localPlayer) return;
    if (!localPlayer->isAlive()) return;

    if (!memory->input->isCameraInThirdPerson) return;

    auto ioDisplay = ImGui::GetIO().DisplaySize;
    const Vector getPosition = localPlayer->getEyePosition();
    ImVec2 eyePos;
    ImVec2 pos{ (ioDisplay.x / 8) * 5 , (ioDisplay.y / 4) };

    const float normalizedReal = Extra::AntiUntrusted() ? Helpers::normalizeYaw(debug.serverAngles.y) : debug.serverAngles.y;
    const float normalizedFake = Extra::AntiUntrusted() ? Helpers::normalizeYaw(debug.fakeAngles.y) : debug.fakeAngles.y;

    if (Helpers::worldToScreen(getPosition, eyePos)) {
        drawList->AddLine(eyePos, pos, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }));
        drawList->AddRectFilled({ pos.x, pos.y }, { pos.x + 256.f, pos.y + 192.f }, ImGui::GetColorU32({ 0.f ,0.f ,0.f , 0.5f }));
        drawList->AddText({ pos.x + 4.f, pos.y + 04.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("Real: ").append(std::to_string(normalizedReal)).c_str());
        drawList->AddText({ pos.x + 4.f, pos.y + 20.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("Fake: ").append(std::to_string(normalizedFake)).c_str());
        drawList->AddText({ pos.x + 4.f, pos.y + 36.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("LBY: ").append(std::to_string(localPlayer->lby())).c_str());
        drawList->AddText({ pos.x + 4.f, pos.y + 52.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("Roll: ").append(std::to_string(debug.serverAngles.z)).c_str());
        drawList->AddText({ pos.x + 4.f, pos.y + 68.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("Max Desync: ").append(std::to_string(localPlayer->getMaxDesyncAngle())).c_str());
    
        if (debug.targetedEntity) {
            drawList->AddText({ pos.x + 4.f, pos.y + 100.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("MinDamage: ").append(std::to_string(debug.minDamage)).c_str());
            drawList->AddText({ pos.x + 4.f, pos.y + 116.f }, ImGui::GetColorU32({ 1.f ,1.f ,1.f , 1.f }), std::string("Damage: ").append(std::to_string(debug.damage)).c_str());
        }
    }
}

void Xsin::backgroundDebugDraw(ImDrawList* dw) {
    if (interfaces->engine->getNetworkChannel()) {
        float rotate = interfaces->engine->getNetworkChannel()->chokedPackets / 16.f;
        ImGuiCustom::circularProgressBar(dw, { 76.f, 76.f }, 16.f, 90.f, 360.f * rotate, Helpers::calculateColor(Color3{ 1.f, 1.f, 1.f }), false);
    }
}

void Xsin::drawDebugWindow() {

    if (!xsnCfg.debugWindow) return;
    if (!localPlayer) return;

    ImGui::Begin("dev", &xsnCfg.debugWindow);
    guiContent();
    ImGui::End();
}

void Xsin::drawGUI(bool contentOnly) noexcept
{

    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 560.0f, 480.0f });
        ImGui::Begin("Dev Window", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }
    
    guiContent();

    if (!contentOnly)
        ImGui::End();
}