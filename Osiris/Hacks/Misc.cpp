#include "Misc.h"
#include "Logger.h"
#include "Clantag.h"
#include "LegitAimbot.h"
#include "AntiAim.h"
#include "Movement.h"
#include "Extra.h"
#include "EnginePrediction.h"
#include "Visuals.h"
#include "Grief.h"
#include "Tickbase.h"
#include "GrenadeHelper.h"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iomanip>
#include <mutex>
#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>

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
#include "../SDK/Gamemode.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Input.h"
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/PlayerResource.h"
#include "../SDK/Panorama.h"
#include "../SDK/Platform.h"
#include "../SDK/Surface.h"
#include "../SDK/UserCmd.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../SDK/ViewSetup.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/WeaponSystem.h"
#include "../SDK/Steam.h"
#include "../SDK/SteamAPI.h"
#include "../SDK/ViewRenderBeams.h"
#include "../SDK/isteamuser.h"
#include "../SDK/isteamuserstats.h"
#include "../SDK/isteamfriends.h"
#include "../SDK/NetworkStringTable.h"
#include "../SDK/Constants/UserMessages.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "../GUI.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"
#include "../Config.h"
#include "../ProtobufReader.h"

#include "../imguiCustom.h"
#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#pragma warning (disable : 4244)
struct OffscreenEnemies : ColorToggle {
    OffscreenEnemies() : ColorToggle{ 1.0f, 0.26f, 0.21f, 1.0f } {}
    HealthBar healthBar;
};

struct MiscConfig {
    OffscreenEnemies offscreenEnemies;
    ColorToggle accuracyCircle{ .5f, 0.5f, 0.5f, 0.3f };

    KeyBind menuKey{ KeyBind::INSERT };
    bool antiAfkKick{ false };
    bool moonwalk{ false };
    int animFuck{ 0 };
    bool autoPistol{ false };
    bool autoReload{ false };
    bool autoAccept{ false };
    int reveal{ 0 };
    bool adBlock{ false };
    bool squareRadar{ false };
    bool fixMouseDelta{ false };
    bool fixAnimationLOD{ false };
    bool fixBoneMatrix{ false };
    bool fixMovement{ true };
    bool disableModelOcclusion{ false };
    bool killMessage{ false };
    bool deadMessage{ false };
    bool deadMessageTeam{ false };
    bool nadeAnimationCancel{ false };
    bool fixTabletSignal{ false };
    bool bypassSvPure{ true };
    bool fastPlant{ false };
    bool autoDefuse{ false };
    bool oppositeHandKnife = false;
    bool forcecrosshair{ false };
    bool recoilcrosshair{ false };
    bool autodisconnect{ false };
    bool unlockInventory{ false };
    bool outofammo{ false };
    bool antiTkill{ false };
    bool unlockCvars{ false };
    bool extrapolate{ true };
    int relayCluster{ 0 };
    int flash{ 0 };

    bool faceit = false;

    struct ColorPalette {
        bool enabled{ false };
        ImVec2 pos;
        Color4 c1 { 1.f, 1.f, 1.f, 1.f };
        Color4 c2 { 1.f, 1.f, 1.f, 1.f };
        Color4 c3 { 1.f, 1.f, 1.f, 1.f };
        Color4 c4 { 1.f, 1.f, 1.f, 1.f };
        Color4 c5 { 1.f, 1.f, 1.f, 1.f };
        Color4 c6 { 1.f, 1.f, 1.f, 1.f };
        Color4 c7 { 1.f, 1.f, 1.f, 1.f };
        Color4 c8 { 1.f, 1.f, 1.f, 1.f };
    } colorPalette;

    struct FootstepESP {
        ColorToggle footstepBeams{ 0.2f, 0.5f, 1.f, 1.0f };
        float footstepBeamRadius = 230.f;
        float footstepBeamThickness = 2.f;
        int textureID = 0;
    } footsteps;

    struct PurchaseList {
        bool enabled = false;
        bool onlyDuringFreezeTime = false;
        bool showPrices = false;
        bool noTitleBar = false;
        bool print = false;

        ImVec2 pos;

        enum Mode {
            Details = 0,
            Summary
        };
        int mode = Details;
    } purchaseList;

    struct KillfeedChanger {
        std::string icon = "";
        bool enabled = false;
        bool preserve = false;
        bool preserveOnlyHeadshots = false;
        bool headshot = false;
        bool dominated = false;
        bool revenge = false;
        bool penetrated = false;
        bool noscope = false;
        bool thrusmoke = false;
        bool attackerblind = false;
        bool wipe = false;
    } killfeedChanger;

    struct BuyBot {
        bool enabled = false;

        int primaryWeapon{ 0 };
        int secondaryWeapon{ 0 };
        int armor{ 0 };
        int utility{ 0 };
        int grenades{ 0 };
    } buyBot;

    struct PlayerList {
        bool enabled = false, noTitleBar = false, hideInGame = false;
        int mode = 0;
        KeyBind plkeybind;
        ImVec2 pos;
    } playerList;

    struct Indicators {
        bool enabled = false;
        bool noTitleBar = false;
        bool noBackground = false;
        ImVec2 pos;
        int bytewise = 0;
    } indicators;

    struct SpectatorList {
        bool enabled = false;
        bool noTitleBar = false;
        bool noBackground = false;
        ImVec2 pos;
    } spectatorList;

    struct VoteRevealerS {
        bool enabled = false;
        bool noTitleBar = false;
        bool noBackground = false;
        ImVec2 pos;
    } voteRevealers;

    struct Watermark {
        bool enabled = true;
        bool noTitleBar = false;
        bool noBackground = false;
        int flags = 0;
        Color4 color;
        ImVec2 pos;
    } watermark;

    struct SmokeTimer {
        bool enabled = false;
        Color4 backgroundColor{ 1.0f, 1.0f, 1.0f, 0.5f };
        Color4 timerColor{ .01f, .44f, .01f, 1.f };
        float timerThickness{ 1.f };
        Color4 textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    } smokeTimer;

    struct MolotovTimer {
        bool enabled = false;
        Color4 backgroundColor{ 1.0f, 1.0f, 1.0f, 0.5f };
        Color4 timerColor{ .72f, .28f, .23f, 1.f };
        float timerThickness{ 1.f };
        Color4 textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    } molotovTimer;

    struct Binds {
        bool enabled = false;
        bool noTitleBar = false;
        bool noBackground = false;
        ImVec2 pos;
    };
    Binds binds;

    struct FreeCam {
        bool enabled = false;
        KeyBindToggle Key;
        int Speed = 6, keyMode = 0;
    }freeCam;

    struct Reportbot {
        bool enabled = false;
        bool textAbuse = false;
        bool griefing = false;
        bool wallhack = true;
        bool aimbot = true;
        bool other = true;
        int target = 0;
        int delay = 1;
        int rounds = 1;
    } reportbot;

    float aspectratio{ 0 };
    std::string killMessageString{ "Azurre|  Get Better hahaha!" };
    std::string deadMessageString{ "" };
    ColorToggle3 bombTimer{ 1.0f, 0.55f, 0.0f };
    struct HurtIndicator {
        ColorToggle3 color{ 0.f, 0.38f, 1.f };
        bool noTitleBar = false;
        bool noBackground = false;
        ImVec2 pos;
    }hurtIndicator;
    ImVec2 bombTimerPos;

    struct NadePredict {
        bool enabled = false;
        bool showDmg = false;
        Color4 colorCircle{ 1.f, 1.f, 1.f, 1.f };
        Color4 colorLine{ 1.0f, 1.0f, 1.0f, 1.f };
        Color4 colorDmg{ 1.0f, 0.0f, 0.0f, 1.f };
    }nadePredict;

    int hitSound{ 0 };
    int killSound{ 0 };
    std::string customVoiceKillSound;
    std::string customVoiceDieSound;
    std::string customKillSound;
    std::string customHitSound;
    std::string customMVPSound;

} mCfg;

bool Misc::nadePrediction() noexcept
{
    return mCfg.nadePredict.enabled;
}

bool Misc::nadePredictionDmg() noexcept
{
    return mCfg.nadePredict.showDmg;
}

Color4 Misc::GetNadePredictDmgColor() noexcept {
    return mCfg.nadePredict.colorDmg;
};

Color4 Misc::GetNadePredictLineColor() noexcept {
    return mCfg.nadePredict.colorLine;
};

Color4 Misc::GetNadePredictCircleColor() noexcept {
    return mCfg.nadePredict.colorCircle;
};

static bool FreeCamkeyPressed = false;

GameMode Misc::getGameMode() noexcept
{
    static auto gameType{ interfaces->cvar->findVar("game_type") };
    static auto gameMode{ interfaces->cvar->findVar("game_mode") };
    switch (gameType->getInt())
    {
    case 0:
        switch (gameMode->getInt())
        {
        case 0:
            return GameMode::Casual;
        case 1:
            return GameMode::Competitive;
        case 2:
            return GameMode::Wingman;
        case 3:
            return GameMode::WeaponsExpert;
        default:
            break;
        }
        break;
    case 1:
        switch (gameMode->getInt())
        {
        case 0:
            return GameMode::ArmsRace;
        case 1:
            return GameMode::Demolition;
        case 2:
            return GameMode::Deathmatch;
        default:
            break;
        }
        break;
    case 2:
        return GameMode::Training;
    case 3:
        return GameMode::Custom;
    case 4:
        switch (gameMode->getInt())
        {
        case 0:
            return GameMode::Guardian;
        case 1:
            return GameMode::CoopStrike;
        default:
            break;
        }
        break;
    case 5:
        return GameMode::WarGames;
    case 6:
        return GameMode::DangerZone;
    default:
        break;
    }
    return GameMode::None;
}

void Misc::updateInput() noexcept {
    if (mCfg.freeCam.keyMode == 0)
        FreeCamkeyPressed = mCfg.freeCam.Key.isDown();
    if (mCfg.freeCam.keyMode == 1 && mCfg.freeCam.Key.isPressed())
        FreeCamkeyPressed = !FreeCamkeyPressed;
}

bool Misc::AdBlockEnabled() noexcept{
    return mCfg.adBlock;
}

bool Misc::isRadarSquared() noexcept{
    return mCfg.squareRadar;
}

bool Misc::isInChat() noexcept
{
    if (!localPlayer)
        return false;

    const auto hudChat = memory->findHudElement(memory->hud, "CCSGO_HudChat");
    if (!hudChat)
        return false;

    const bool isInChat = *(bool*)((uintptr_t)hudChat + 0xD);

    return isInChat;
}

bool Misc::unlockInventory() noexcept
{
    return mCfg.unlockInventory;
}

bool Misc::FreeCamEnabled() noexcept
{
    return FreeCamkeyPressed;
}

bool Misc::FreeCamKey() noexcept
{
    if (!mCfg.freeCam.enabled)
        return false;
    return FreeCamkeyPressed;
}

bool Misc::shouldEnableSvPureBypass() noexcept
{
    return mCfg.bypassSvPure;
}

bool Misc::shouldRevealMoney() noexcept
{
    return Helpers::getByteFromBytewise(mCfg.reveal, 2);
}

bool Misc::shouldRevealSuspect() noexcept
{
    return Helpers::getByteFromBytewise(mCfg.reveal, 3);
}

bool Misc::shouldDisableModelOcclusion() noexcept
{
    return mCfg.disableModelOcclusion;
}

bool Misc::shouldFixBoneMatrix() noexcept
{
    return mCfg.fixBoneMatrix;
}

bool Misc::isRadarHackOn() noexcept
{
    return Helpers::getByteFromBytewise(mCfg.reveal, 0);
}

bool Misc::isMenuKeyPressed() noexcept
{
    return mCfg.menuKey.isPressed();
}

float Misc::aspectRatio() noexcept
{
    return mCfg.aspectratio;
}

void Misc::colorPaletteWindow() noexcept {

    if (!mCfg.colorPalette.enabled)
        return;

    if (mCfg.colorPalette.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.colorPalette.pos);
        mCfg.colorPalette.pos = {};
    }
    ImGui::SetNextWindowSize({ 256.0f, 32.f });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (!gui->isOpen()) return;

    ImGui::SetNextWindowSize({ 140.0f, 64.0f });
    ImGui::Begin("Color palette", &mCfg.colorPalette.enabled, windowFlags);

    ImGui::PushID(1);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c1); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(2);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c2); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(3);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c3); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(4);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c4); ImGui::PopID();
    ImGui::PushID(5);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c5); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(6);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c6); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(7);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c7); ImGui::PopID(); ImGui::SameLine();
    ImGui::PushID(8);  ImGuiCustom::colorPicker("", mCfg.colorPalette.c8); ImGui::PopID();

    ImGui::End();
}

void Misc::FlashGame(int idx) { //WIP 
    /*
    * Index:
    * Found Match 0
    * Round Start 1
    */
#ifdef _WIN32
    if (!mCfg.flash) return;
    auto window = FindWindowW(L"Valve001", NULL);
    if (GetActiveWindow()) return;
    FLASHWINFO flash{ sizeof(FLASHWINFO), window, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };

    if ((idx == 0 && Helpers::getByteFromBytewise(mCfg.flash, 0) ) || (idx == 1 && Helpers::getByteFromBytewise(mCfg.flash, 1))) {
        FlashWindowEx(&flash);
    }
    //ShowWindow(window, SW_RESTORE);
#endif
}

void Misc::outOfAmmoNotification() noexcept {
    static auto cvar{ interfaces->cvar->findVar("sv_outofammo_indicator") };

    cvar->onChangeCallbacks.size = 0;
    cvar->setValue(mCfg.outofammo);
}

void Misc::spectatorList() noexcept
{
    if (!mCfg.spectatorList.enabled)
        return;

    if (mCfg.spectatorList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.spectatorList.pos);
        mCfg.spectatorList.pos = {};
    }
    ImGui::SetNextWindowSize({ 200.0f, 0.f });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (mCfg.spectatorList.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (mCfg.spectatorList.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    if (gui->isOpen()) {
        ImGui::Begin("Spectator list", nullptr, windowFlags);
        ImGui::TextWrapped("VALVE | 3rd");
        ImGui::TextWrapped("Azurre | 1rd");
        ImGui::TextWrapped("GOTV | Freecam");
        ImGui::End();
        return;
    }

    if (!interfaces->engine->isInGame()) return;

    if (!localPlayer) return;

    if (!localPlayer->isAlive()) return;

    GameData::Lock lock;

    const auto& observers = GameData::observers();

    if (std::ranges::none_of(observers, [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
        return;

    ImGui::Begin("Spectator list", nullptr, windowFlags);

    for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) { // int i = 1, cuz we skip localPlayer
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity->isDormant() || entity->isAlive() || entity->getObserverTarget() != localPlayer.get())
            continue;

        PlayerInfo playerInfo;

        if (!interfaces->engine->getPlayerInfo(i, playerInfo))
            continue;

        if (std::strcmp(playerInfo.name, "GOTV") == 0)
            continue;

        const char* obsMode;

        switch (entity->getObserverMode()) {
        case ObsMode::InEye:
            obsMode = "1st";
            break;
        case ObsMode::Chase:
            obsMode = "3rd";
            break;
        case ObsMode::Roaming:
            obsMode = "Freecam";
            break;
        default:
            continue;
        }
        if (const auto texture = GameData::playerByHandle(entity->handle())->getAvatarTexture()) {
            const auto textSize = ImGui::CalcTextSize(playerInfo.name);
            ImGui::Image(texture, ImVec2(textSize.y, textSize.y), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.3f));
            ImGui::SameLine();
        }
        ImGui::TextWrapped("%s | %s", playerInfo.name, obsMode);
    }
    
    ImGui::End();
}

void Misc::forceCrosshair(csgo::FrameStage stage) noexcept {

    if (!localPlayer) return;
    
    static auto shairVar = interfaces->cvar->findVar("weapon_debug_spread_show");
    shairVar->setValue(mCfg.forcecrosshair == 1 && !localPlayer->isScoped() ? 3 : 0);
}

void Misc::recoilCrosshair(csgo::FrameStage stage) noexcept {

    if (!localPlayer) return;
    static auto shairVar = interfaces->cvar->findVar("cl_crosshair_recoil");
    shairVar->setValue(mCfg.recoilcrosshair == 1 ? 1 : 0);
}

const char* getAdressIP() {

    if (!localPlayer) return "Not Connected";

    if (!interfaces->engine->isConnected()) return "Not Connected";

    if (interfaces->engine->isHLTV()) return "HLTV Demo";

    const auto gameRules = *memory->gameRules;
    if (!gameRules) return "Not available: e1";
    const char* serverIP = interfaces->engine->getNetworkChannel()->getAddress();
    if (!serverIP) return "Not available: e2";

    if (gameRules->isValveDS()) return "Valve Server";

    return serverIP;
}

void Misc::watermark() noexcept
{
    if (!mCfg.watermark.enabled)
        return;

    if (mCfg.watermark.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.watermark.pos);
        mCfg.watermark.pos = {};
    }
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (mCfg.watermark.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (mCfg.watermark.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::SetNextWindowSize({-1.f, -1.f});
    ImGui::Begin("Azurre", nullptr, windowFlags);

    const int tickRate = static_cast<int>(1 / memory->globalVars->intervalPerTick); //tps

    static auto frameRate = 1.0f;
    frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime; // fps
    constexpr float pi = std::numbers::pi_v<float>;

    ImGui::TextColored(mCfg.watermark.color.rainbow ? ImVec4(
        std::sin(mCfg.watermark.color.rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f,
        std::sin(mCfg.watermark.color.rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f,
        std::sin(mCfg.watermark.color.rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f,
        mCfg.watermark.color.color[3])
        : mCfg.watermark.color.color, "%s %ifps | %dms | %dtick | %s",
        mCfg.watermark.noTitleBar ? "Azurre |" : " ",
        frameRate != 0.0f ? static_cast<int>(1 / frameRate) : 0,
        GameData::getNetOutgoingLatency(),
        tickRate, getAdressIP());

    ImGui::End();
}

void Misc::showBinds() noexcept
{
    if (!mCfg.binds.enabled)
        return;

    if (mCfg.binds.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.binds.pos);
        mCfg.binds.pos = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (mCfg.binds.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (mCfg.binds.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowSize({ 192.0f, 0.f });
    ImGui::Begin("Keybinds", nullptr, windowFlags);

    //Aimbot
    if (config->aimbotKeyMode == 0 && config->aimbotKey.isDown() && config->legitaimbot[0].enabled) ImGui::Text("Aimbot | Hold");
    if (config->aimbotKeyMode == 1 && LegitAimbot::isPressedOrHold() && config->legitaimbot[0].enabled) ImGui::Text("Aimbot | Toggled");

    //Rage
    if (config->minDamageOverride.isDown() && config->globalEnabledRagebot) ImGui::Text("Ragebot | Dmg Override");
    if (config->visibleOnlyOverride.isDown() && config->globalEnabledRagebot) ImGui::Text("Ragebot | AutoWall Override");
    if (config->forceOnShot.isDown() && config->globalEnabledRagebot) ImGui::Text("Ragebot | On Shot");

    //AA
    if (AntiAim::isLAAInverted()) ImGui::Text("AntiAim | Legit Inverted");
    if (AntiAim::isAAInverted()) ImGui::Text("AntiAim | Rage Inverted");
    if (AntiAim::isAngleInverted()) ImGui::Text("AntiAim | Angle Inverted");
    if (AntiAim::isRollInverted()) ImGui::Text("AntiAim | Roll");

    //Chams
    if (config->chamsToggleKey.isSet() && config->chamsToggleKey.isToggled() && !config->chamsHoldKey.isDown()) ImGui::Text("Chams | Toggled");
    if (config->chamsToggleKey.isSet() && config->chamsHoldKey.isDown()) ImGui::Text("Chams | Hold");

    //Visuals

    if(memory->input->isCameraInThirdPerson)ImGui::Text("Third-Person");

    //Movement
    if (Extra::isSlowWalkKeyDown()) ImGui::Text("SlowWalk");
    if (Movement::isJumpBugKeyDown()) ImGui::Text("JumpBug");
    if (Movement::isEdgeBugKeyDown()) ImGui::Text("EdgeBug");
    if (Movement::isAutoPXKeyDown()) ImGui::Text("Auto PixelSurf");
    if (Movement::isMiniJumpKeyDown()) ImGui::Text("MiniJump");
    if (Movement::isEdgeJumpKeyDown()) ImGui::Text("LongJump");
    if (AntiAim::isBackwards()) ImGui::Text("Backwards");

    //Extras
    if (Extra::isFakeDuckKeyPressed()) ImGui::Text("FakeDuck");
    if (Extra::isDTEnabled()) ImGui::Text("DoubleTap");
    if (Extra::isHideShotsEnabled()) ImGui::Text("HideShots");
    if (Extra::isQuickPeekKeyDown()) ImGui::Text("Auto-Peek");
    if (Extra::isAirStuckKeyDown()) ImGui::Text("Air-Stuck");
    if (Extra::isFakeWalkKeyPressed()) ImGui::Text("FakeWalk");
    if (Extra::forceTeleportKey()) ImGui::Text("Tickbase Teleport");

    //Misc
    if (Nade::isKeyDown()) ImGui::Text("Grenade helper | Adjusting");
    if (mCfg.freeCam.enabled && mCfg.freeCam.keyMode == 0 && mCfg.freeCam.Key.isDown()) ImGui::Text("Freecam | Hold");
    if (mCfg.freeCam.enabled && mCfg.freeCam.keyMode == 1 && FreeCamkeyPressed) ImGui::Text("Freecam | Toggled");
    ImGui::End();

}

void Misc::antiTKilling(UserCmd* cmd) noexcept {

    if (!mCfg.antiTkill) return;

    if (!localPlayer || !localPlayer->isAlive() || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    if (activeWeapon->isGrenade())
        return;

    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles + localPlayer->getAimPunch()) * weaponData->range;

    Trace trace;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, localPlayer.get(), trace);

    if (!trace.entity || !trace.entity->isPlayer())
        return;

    if (!localPlayer->isOtherEnemy(trace.entity)){
        cmd->buttons &= ~UserCmd::IN_ATTACK;
        if(activeWeapon->itemDefinitionIndex() == WeaponId::Revolver)
            cmd->buttons &= ~UserCmd::IN_ATTACK2;
    }
}

constexpr const char* ringTextures[] = {
    "sprites/physbeam.vmt",
    "sprites/purplelaser1.vmt",
    "sprites/laserbeam.vmt",
    "sprites/white.vmt",    //draws behind the wall
};

void Misc::footstepESP(GameEvent& event) {

    if (!mCfg.footsteps.footstepBeams.enabled)
        return;

    if (!localPlayer) return;

    const auto userid = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userid) return;

    if (localPlayer->getUserId() == userid->getUserId())
        return;

    if (localPlayer->getTeamNumber() == userid->getTeamNumber())
        return;

    const auto modelIndex = interfaces->modelInfo->getModelIndex(ringTextures[mCfg.footsteps.textureID]);

    BeamInfo info;

    info.type = TE_BEAMRINGPOINT;
    info.modelName = ringTextures[mCfg.footsteps.textureID];
    info.modelIndex = modelIndex;
    info.haloIndex = -1;
    info.haloScale = 3.0f;
    info.life = 2.0f;
    info.width = mCfg.footsteps.footstepBeamThickness;
    info.fadeLength = 0.0f;
    info.amplitude = 0.0f;
    info.red = mCfg.footsteps.footstepBeams.color[0] * 255;
    info.green = mCfg.footsteps.footstepBeams.color[1] * 255;
    info.blue = mCfg.footsteps.footstepBeams.color[2] * 255;
    info.brightness = 255;
    info.speed = 0.0f;
    info.startFrame = 0.0f;
    info.frameRate = 60.0f;
    info.segments = -1;
    info.flags = FBEAM_FADEOUT;
    info.ringCenter = userid->getAbsOrigin() + Vector(0.0f, 0.0f, 5.0f);
    info.ringStartRadius = 5.0f;
    info.ringEndRadius = mCfg.footsteps.footstepBeamRadius;
    info.renderable = true;

    auto beam = memory->viewRenderBeams->createBeamRingPoints(info); //null

    if (beam)
        memory->viewRenderBeams->drawBeam(beam);
}

void Misc::GrenadeAnimationCancel(GameEvent& event) noexcept
{
    if (!mCfg.nadeAnimationCancel)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (GameData::getNetOutgoingLatency() >= 100) return; 

    if (localPlayer->getUserId() != event.getInt("userid"))
        return;

    interfaces->engine->clientCmdUnrestricted("slot3; slot2; slot1");
}

void Misc::fastPlant(UserCmd* cmd) noexcept
{
    if (!mCfg.fastPlant)
        return;

    if (static auto plantAnywhere = interfaces->cvar->findVar("mp_plant_c4_anywhere"); plantAnywhere->getInt())
        return;

    if (!localPlayer || !localPlayer->isAlive() || (localPlayer->inBombZone() && localPlayer->isOnGround()))
        return;

    if (const auto activeWeapon = localPlayer->getActiveWeapon(); !activeWeapon || activeWeapon->getClientClass()->classId != ClassId::C4)
        return;

    cmd->buttons &= ~UserCmd::IN_ATTACK;

    static constexpr auto doorRange = 200.0f;

    Trace trace;
    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles) * doorRange;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, localPlayer.get(), trace);

    if (!trace.entity || trace.entity->getClientClass()->classId != ClassId::PropDoorRotating)
        cmd->buttons &= ~UserCmd::IN_USE;
}

std::vector<conCommandBase*> dev;
std::vector<conCommandBase*> hidden;
std::vector<conCommandBase*> archive;

void Misc::initHiddenCvars() noexcept {

    conCommandBase* iterator = **reinterpret_cast<conCommandBase***>(interfaces->cvar + 0x34);

    for (auto c = iterator->next; c != nullptr; c = c->next)
    {
        conCommandBase* cmd = c;

        if (cmd->flags & FCVAR::DEVELOPMENTONLY)
            dev.push_back(cmd);

        if (cmd->flags & FCVAR::HIDDEN)
            hidden.push_back(cmd);

        if (cmd->flags & FCVAR::ARCHIVE)
            archive.push_back(cmd);
    }
}

void Misc::unlockHiddenCvars() noexcept {

    static bool trigger = false;

    if (mCfg.unlockCvars == trigger) return;

    if (!trigger) { //unhide
        for (unsigned x = 0; x < dev.size(); x++)
            dev.at(x)->flags &= ~FCVAR::DEVELOPMENTONLY;

        for (unsigned x = 0; x < hidden.size(); x++)
            hidden.at(x)->flags &= ~FCVAR::HIDDEN;

        for (unsigned x = 0; x < archive.size(); x++)
            archive.at(x)->flags &= ~FCVAR::ARCHIVE;
    }
    if (trigger) { //hide
        for (unsigned x = 0; x < dev.size(); x++)
            dev.at(x)->flags |= FCVAR::DEVELOPMENTONLY;

        for (unsigned x = 0; x < hidden.size(); x++)
            hidden.at(x)->flags |= FCVAR::HIDDEN;

        for (unsigned x = 0; x < archive.size(); x++)
            archive.at(x)->flags |= FCVAR::ARCHIVE;
    }

    trigger = mCfg.unlockCvars;
}

bool static canDefuse = false;

void Misc::autoDefuseEvents(GameEvent& event) {
    if (!mCfg.autoDefuse) return;
    if (!localPlayer) return;
    canDefuse = false;
}

void Misc::autoDefuse(UserCmd* cmd) noexcept
{
    if (!mCfg.autoDefuse) return;
    if (!localPlayer) return;

    GameData::Lock lock;

    const auto& plantedC4 = GameData::plantedC4();

    auto bombEntity = interfaces->entityList->getEntityFromHandle(plantedC4.bombHandle);

    if (!bombEntity) return;

    const float c4Time = (plantedC4.blowTime - memory->globalVars->currenttime + (float)GameData::getNetOutgoingLatency());

    if (c4Time == 0.f || c4Time > 11.f) canDefuse = false;

    if (!canDefuse && (bombEntity->origin() - localPlayer->origin()).length2D() < 60.f && (!localPlayer->hasDefuser() && (c4Time < 10.1f && c4Time > 10.f)) ||
                        localPlayer->hasDefuser() && (c4Time <  5.1f && c4Time >  5.f)) {
        canDefuse = true;
    }

    if(canDefuse) cmd->buttons |= UserCmd::IN_USE;
}

bool static  bombisPlanting = false;
int static bombSiteP = -1, plantTime = -1, idxPlayer = -1;

void Misc::isBombBeingPlanted(GameEvent& event) noexcept {

    if (!mCfg.bombTimer.enabled)
        return;
    bombSiteP = event.getInt("site");
    plantTime = memory->globalVars->currenttime + 4.f;
    idxPlayer = event.getInt("userid");

    bombisPlanting = true;
}
void Misc::isBombPlantingAborted(GameEvent& event) noexcept {

    if (!mCfg.bombTimer.enabled)
        return;
    bombisPlanting = false;
    bombSiteP = -1;
    plantTime = -1;
    idxPlayer = -1;
}

void Misc::drawBombTimer() noexcept
{
    if (!mCfg.bombTimer.enabled)
        return;

    GameData::Lock lock;

    const auto& plantedC4 = GameData::plantedC4();
    if (plantedC4.blowTime == 0.0f && !gui->isOpen())
        return;

    if (!gui->isOpen()) {
        ImGui::SetNextWindowBgAlpha(0.3f);
    }

    ImGui::SetNextWindowPos({ (ImGui::GetIO().DisplaySize.x - 200.0f) / 2.0f, 60.0f }, ImGuiCond_Once);
    ImGui::SetNextWindowSize({ 200.0f, 0 }, ImGuiCond_Once);

    if (!gui->isOpen())
        ImGui::SetNextWindowSize({ 200.0f, 0 });

    if (mCfg.bombTimerPos!= ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.bombTimerPos);
        mCfg.bombTimerPos = {};
    }

    ImGui::SetNextWindowSizeConstraints({ 0, -1 }, { FLT_MAX, -1 });
    ImGui::Begin("Bomb Timer", nullptr, ImGuiWindowFlags_NoTitleBar | (gui->isOpen() ? 0 : ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing));

    std::ostringstream ss; ss << "Bomb on " << (!plantedC4.bombsite ? 'A' : 'B') << " : " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(plantedC4.blowTime - memory->globalVars->currenttime, 0.0f) << " s";

    ImGui::textUnformattedCentered(ss.str().c_str());

    if (localPlayer) {
        bool drawDamage = true; //we want to draw the progress bar even if we cant do the damage

        auto bombEntity = interfaces->entityList->getEntityFromHandle(plantedC4.bombHandle);

        if (!bombEntity || bombEntity->isDormant() || bombEntity->getClientClass()->classId != ClassId::PlantedC4)
            drawDamage = false;

        if (!localPlayer || localPlayer->isDormant())
            drawDamage = false;

        static constexpr float bombDamage = 500.f;
        static constexpr float bombRadius = bombDamage * 3.5f; //wont work with some maps because of this i guess
        static constexpr float sigma = bombRadius / 3.0f;

        static constexpr float armorRatio = 0.5f;
        static constexpr float armorBonus = 0.5f;

        if (drawDamage) {
            const float armorValue = static_cast<float>(localPlayer->armor()); //crash
            const int health = localPlayer->health();

            float finalBombDamage = 0.f;
            float distanceToLocalPlayer = (bombEntity->origin() - localPlayer->origin()).length();
            float gaussianFalloff = exp(-distanceToLocalPlayer * distanceToLocalPlayer / (2.0f * sigma * sigma));

            finalBombDamage = bombDamage * gaussianFalloff;

            if (armorValue > 0) {
                float newRatio = finalBombDamage * armorRatio;
                float armor = (finalBombDamage - newRatio) * armorBonus;

                if (armor > armorValue) {
                    armor = armorValue * (1.f / armorBonus);
                    newRatio = finalBombDamage - armor;
                }
                finalBombDamage = newRatio;
            }

            int displayBombDamage = static_cast<int>(floor(finalBombDamage));

            if (health <= (truncf(finalBombDamage * 10) / 10)) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                ImGui::textUnformattedCentered("Fatal!!!");
                ImGui::PopStyleColor();
            }
            else {
                std::ostringstream text; text << "Damage: " << std::clamp(displayBombDamage, 0, health - 1);
                const auto color = Helpers::healthColor(std::clamp(1.f - (finalBombDamage / static_cast<float>(health)), 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::textUnformattedCentered(text.str().c_str());
                ImGui::PopStyleColor();
            }
        }
    }
    


    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Helpers::calculateColor(mCfg.bombTimer.asColor3()));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
    ImGui::progressBarFullWidth((plantedC4.blowTime - memory->globalVars->currenttime) / plantedC4.timerLength, 5.0f, false);

    if (plantedC4.defuserHandle != -1) {
        const bool canDefuse = plantedC4.blowTime >= plantedC4.defuseCountDown;

        if (plantedC4.defuserHandle == GameData::local().handle) {
            if (canDefuse) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                ImGui::textUnformattedCentered("You can defuse!");
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                ImGui::textUnformattedCentered("You can not defuse!");
            }
            ImGui::PopStyleColor();
        }
        else if (const auto defusingPlayer = GameData::playerByHandle(plantedC4.defuserHandle)) {
            std::ostringstream ss; ss << defusingPlayer->name << " is defusing: " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(plantedC4.defuseCountDown - memory->globalVars->currenttime, 0.0f) << " s";

            ImGui::textUnformattedCentered(ss.str().c_str());

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, canDefuse ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
            ImGui::progressBarFullWidth((plantedC4.defuseCountDown - memory->globalVars->currenttime) / plantedC4.defuseLength, 5.0f, false);
            ImGui::PopStyleColor();
        }
    }

    ImGui::PopStyleColor(2);
    ImGui::End();
}

void Misc::hurtIndicator() noexcept
{
    if (!mCfg.hurtIndicator.color.enabled)
        return;

    GameData::Lock lock;

    const auto& local = GameData::local();
    if ((!local.exists || !local.alive) && !gui->isOpen())
        return;

    if (local.velocityModifier >= 0.99f && !gui->isOpen())
        return;

    if (!gui->isOpen()) {
        ImGui::SetNextWindowBgAlpha(0.3f);
    }

    static float windowWidth = 140.0f;
    if (mCfg.hurtIndicator.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.hurtIndicator.pos);
        mCfg.hurtIndicator.pos = {};
    }else {
        ImGui::SetNextWindowPos({ (ImGui::GetIO().DisplaySize.x - 140.0f) / 2.0f, 260.0f }, ImGuiCond_Once);
    }
    ImGui::SetNextWindowSize({ windowWidth, 0 }, ImGuiCond_Once);

    if (!gui->isOpen())
        ImGui::SetNextWindowSize({ windowWidth, 0 });

    ImGui::SetNextWindowSizeConstraints({ 0, -1 }, { FLT_MAX, -1 });
    ImGui::Begin("Hurt Indicator", nullptr, ImGuiWindowFlags_NoTitleBar | (gui->isOpen() ? 0 : ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration));

    std::ostringstream ss; ss << "Slowed down " << static_cast<int>(local.velocityModifier * 100.f) << "%";
    ImGui::textUnformattedCentered(ss.str().c_str());

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Helpers::calculateColor(mCfg.hurtIndicator.color.asColor3()));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
    ImGui::progressBarFullWidth(local.velocityModifier, 1.0f, false);

    windowWidth = ImGui::GetCurrentWindow()->SizeFull.x;

    ImGui::PopStyleColor(2);
    ImGui::End();
}


void Misc::fixTabletSignal() noexcept
{
    if (mCfg.fixTabletSignal && localPlayer) {
        if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && activeWeapon->getClientClass()->classId == ClassId::Tablet)
            activeWeapon->tabletReceptionIsBlocked() = false;
    }
}

void Misc::killMessage(GameEvent& event) noexcept
{
    if (!mCfg.killMessage)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto userid = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("attacker")));

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    PlayerInfo userIdInfo;

    if (!interfaces->engine->getPlayerInfo(userid->index(), userIdInfo))
        return;

    std::string cmd = "say \"";
    cmd += mCfg.killMessageString;
    cmd += '"';

    while (cmd.find("#p") != std::string::npos) // replace #p with name of killed player
        cmd.replace(cmd.find("#p"), 2, userIdInfo.name);

    while (cmd.find("#l") != std::string::npos) // replace #l with name of last location
        cmd.replace(cmd.find("#l"), 2, userid->lastPlaceName());

    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::DeadMessage(GameEvent& event) noexcept
{
    if (!mCfg.deadMessage)
        return;

    if (!localPlayer)
        return;

    const auto userid = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("attacker")));

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") == localUserId || event.getInt("userid") != localUserId)
        return;

    PlayerInfo attackerInfo;

    if (!interfaces->engine->getPlayerInfo(attacker->index(), attackerInfo))
        return;

    std::string cmd = mCfg.deadMessageTeam ? "say_team \"" : "say \"";
    cmd += mCfg.deadMessageString;
    cmd += '"';

    while (cmd.find("#p") != std::string::npos) // replace #p with name of killed player
        cmd.replace(cmd.find("#p"), 2, attackerInfo.name);

    while (cmd.find("#l") != std::string::npos) // replace #l with name of last location
        cmd.replace(cmd.find("#l"), 2, attacker->lastPlaceName());

    while (cmd.find("#h") != std::string::npos) // replace #h with name of health
        cmd.replace(cmd.find("#h"), 2, std::to_string(attacker->health()));

    while (cmd.find("#-h") != std::string::npos) // replace #h with name of damage got
        cmd.replace(cmd.find("#-h"), 2, std::to_string(100 - attacker->health()));

    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::fixMovement(UserCmd* cmd, float yaw) noexcept
{
    if (!mCfg.fixMovement) return;

    if (!localPlayer) return;
    if (!localPlayer->isAlive()) return;

    float oldYaw = yaw + (yaw < 0.0f ? 360.0f : 0.0f);
    float newYaw = cmd->viewangles.y + (cmd->viewangles.y < 0.0f ? 360.0f : 0.0f);
    float yawDelta = newYaw < oldYaw ? fabsf(newYaw - oldYaw) : 360.0f - fabsf(newYaw - oldYaw);
    yawDelta = 360.0f - yawDelta;

    const float forwardmove = cmd->forwardmove;
    const float sidemove = cmd->sidemove;
    cmd->forwardmove = std::cos(Helpers::deg2rad(yawDelta)) * forwardmove + std::cos(Helpers::deg2rad(yawDelta + 90.0f)) * sidemove;
    cmd->sidemove = std::sin(Helpers::deg2rad(yawDelta)) * forwardmove + std::sin(Helpers::deg2rad(yawDelta + 90.0f)) * sidemove;

}

void Misc::antiAfkKick(UserCmd* cmd) noexcept
{
    if (mCfg.antiAfkKick && cmd->commandNumber % 2)
        cmd->buttons |= 1 << 27;
}

void Misc::fixMouseDelta(UserCmd* cmd) noexcept
{
    if (!mCfg.fixMouseDelta)
        return;

    if (!localPlayer) return;
    if (!localPlayer->isAlive()) return;

    static Vector delta_viewangles{ };
    Vector delta = cmd->viewangles - delta_viewangles;

    delta.x = std::clamp(delta.x, -89.0f, 89.0f);
    delta.y = std::clamp(delta.y, -180.0f, 180.0f);
    delta.z = 0.0f;
    static ConVar* sensitivity = interfaces->cvar->findVar("sensitivity");
    if (delta.x != 0.f) {
        static ConVar* m_pitch = interfaces->cvar->findVar("m_pitch");

        int final_dy = static_cast<int>((delta.x / m_pitch->getFloat()) / sensitivity->getFloat());
        if (final_dy <= 32767) {
            if (final_dy >= -32768) {
                if (final_dy >= 1 || final_dy < 0) {
                    if (final_dy <= -1 || final_dy > 0)
                        final_dy = final_dy;
                    else
                        final_dy = -1;
                }
                else {
                    final_dy = 1;
                }
            }
            else {
                final_dy = 32768;
            }
        }
        else {
            final_dy = 32767;
        }

        cmd->mousedy = static_cast<short>(final_dy);
    }

    if (delta.y != 0.f) {
        static ConVar* m_yaw = interfaces->cvar->findVar("m_yaw");

        int final_dx = static_cast<int>((delta.y / m_yaw->getFloat()) / sensitivity->getFloat());
        if (final_dx <= 32767) {
            if (final_dx >= -32768) {
                if (final_dx >= 1 || final_dx < 0) {
                    if (final_dx <= -1 || final_dx > 0)
                        final_dx = final_dx;
                    else
                        final_dx = -1;
                }
                else {
                    final_dx = 1;
                }
            }
            else {
                final_dx = 32768;
            }
        }
        else {
            final_dx = 32767;
        }

        cmd->mousedx = static_cast<short>(final_dx);
    }

    delta_viewangles = cmd->viewangles;
}

void Misc::fixAnimationLOD(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START)
        return;

#ifdef _WIN32
    if (!mCfg.fixAnimationLOD) return;

    if (!localPlayer)
        return;

    for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
        Entity* entity = interfaces->entityList->getEntity(i);
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive())
            continue;
        *reinterpret_cast<int*>(entity + 0xA28) = 0;
        *reinterpret_cast<int*>(entity + 0xA30) = memory->globalVars->framecount;
    }
#endif
}

void Misc::autoPistol(UserCmd* cmd) noexcept
{
    if (!mCfg.autoPistol)
        return;

    if(!localPlayer || !localPlayer->isAlive())
        return;

        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->isPistol() && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime()) {
            if (activeWeapon->itemDefinitionIndex() == WeaponId::Revolver)
                cmd->buttons &= ~UserCmd::IN_ATTACK2;
            else
                cmd->buttons &= ~UserCmd::IN_ATTACK;
        }
    
}

void Misc::autoReload(UserCmd* cmd) noexcept
{
    if (!mCfg.autoReload)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && getWeaponIndex(activeWeapon->itemDefinitionIndex()) && !activeWeapon->clip())
            cmd->buttons &= ~(UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2);
    
}

void Misc::revealRanks(UserCmd* cmd) noexcept
{
    if (Helpers::getByteFromBytewise(mCfg.reveal, 1) && cmd->buttons & UserCmd::IN_SCORE)
        interfaces->client->dispatchUserMessage(csgo::UserMessageType::ServerRankRevealAll, 0, 0, nullptr);
}

void Misc::moonwalk(UserCmd* cmd) noexcept
{
    if (!localPlayer || localPlayer->moveType() == MoveType::LADDER)
        return;

    cmd->buttons &= ~(UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT);
    if (cmd->forwardmove > 0.0f)
        cmd->buttons |= UserCmd::IN_FORWARD;
    else if (cmd->forwardmove < 0.0f)
        cmd->buttons |= UserCmd::IN_BACK;
    if (cmd->sidemove > 0.0f)
        cmd->buttons |= UserCmd::IN_MOVERIGHT;
    else if (cmd->sidemove < 0.0f)
        cmd->buttons |= UserCmd::IN_MOVELEFT;

    if (!mCfg.moonwalk)
        return;

    static bool toggle = false;
    static int oldTick = 0;
    if (mCfg.animFuck){
        if (memory->globalVars->tickCount - oldTick > mCfg.animFuck) {
            if (toggle) toggle = false;
            else toggle = true;

            oldTick = memory->globalVars->tickCount;
        }
    }   else toggle = true;

    if (toggle)
        cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
}

void Misc::playHitSound(GameEvent& event) noexcept
{
    if (!mCfg.hitSound)
        return;

    if (!localPlayer)
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    static constexpr std::array hitSounds{
        "physics/metal/metal_solid_impact_bullet2",
        "buttons/arena_switch_press_02",
        "training/timer_bell",
        "physics/glass/glass_impact_bullet1",
        "ui/deathnotice"
    };

    if (static_cast<std::size_t>(mCfg.hitSound - 1) < hitSounds.size())
        interfaces->engine->clientCmdUnrestricted(hitSounds[mCfg.hitSound - 1]);
    else if (mCfg.hitSound == 6)
        interfaces->engine->clientCmdUnrestricted(("play " + mCfg.customHitSound).c_str());
}

void Misc::killSound(GameEvent& event) noexcept
{
    if (!mCfg.killSound)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    static constexpr std::array killSounds{
        "physics/metal/metal_solid_impact_bullet2",
        "buttons/arena_switch_press_02",
        "training/timer_bell",
        "physics/glass/glass_impact_bullet1",
        "ui/deathnotice"
    };

    if (static_cast<std::size_t>(mCfg.killSound - 1) < killSounds.size())
        interfaces->engine->clientCmdUnrestricted(killSounds[mCfg.killSound - 1]);
    else if (mCfg.killSound == 6)
        interfaces->engine->clientCmdUnrestricted(("play " + mCfg.customKillSound).c_str());
}

void Misc::BuyBot(GameEvent* event) noexcept
{
    static std::array<std::string, 17> primary = {
        "",
        "buy mac10;buy mp9;",
        "buy mp7;",
        "buy ump45;",
        "buy p90;",
        "buy bizon;",
        "buy galilar;buy famas;",
        "buy ak47;buy m4a1;",
        "buy ssg08;",
        "buy sg556;buy aug;",
        "buy awp;",
        "buy g3sg1; buy scar20;",
        "buy nova;",
        "buy xm1014;",
        "buy sawedoff;buy mag7;",
        "buy m249;",
        "buy negev;"
    };
    static std::array<std::string, 6> secondary = {
        "",
        "buy glock;buy hkp2000",
        "buy elite;",
        "buy p250;",
        "buy tec9;buy fiveseven;",
        "buy deagle;buy revolver;"
    };
    static std::array<std::string, 3> armor = {
        "",
        "buy vest;",
        "buy vesthelm;",
    };
    static std::array<std::string, 2> utility = {
        "buy defuser;",
        "buy taser;"
    };
    static std::array<std::string, 5> nades = {
        "buy hegrenade;",
        "buy smokegrenade;",
        "buy molotov;buy incgrenade;",
        "buy flashbang;buy flashbang;",
        "buy decoy;"
    };

    if (!mCfg.buyBot.enabled)
        return;

    std::string cmd = "";

    if (event)
    {
        cmd += primary[mCfg.buyBot.primaryWeapon];
        cmd += secondary[mCfg.buyBot.secondaryWeapon];
        cmd += armor[mCfg.buyBot.armor];

        for (size_t i = 0; i < utility.size(); i++)
        {
            if ((mCfg.buyBot.utility & 1 << i) == 1 << i)
                cmd += utility[i];
        }

        for (size_t i = 0; i < nades.size(); i++)
        {
            if ((mCfg.buyBot.grenades & 1 << i) == 1 << i)
                cmd += nades[i];
        }

        interfaces->engine->clientCmdUnrestricted(cmd.c_str());
    }
}

void Misc::BlockInputWhileOpenGui(UserCmd* cmd) noexcept
{
    if (config->style.blockInput) return;

    if (gui->isOpen()) {
        cmd->buttons &= ~UserCmd::IN_ATTACK;
        cmd->buttons &= ~UserCmd::IN_ATTACK2;
    }
}

void Misc::purchaseList(GameEvent* event) noexcept
{
    static std::mutex mtx;
    std::scoped_lock _{ mtx };

    struct PlayerPurchases {
        int totalCost;
        std::unordered_map<std::string, int> items;
    };

    static std::unordered_map<int, PlayerPurchases> playerPurchases;
    static std::unordered_map<std::string, int> purchaseTotal;
    static int totalCost;

    static auto freezeEnd = 0.0f;

    if (event) {
        switch (fnv::hashRuntime(event->getName())) {
        case fnv::hash("item_purchase"): {
            if (const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid"))); player && localPlayer && localPlayer->isOtherEnemy(player)) {
                if (const auto definition = memory->itemSystem()->getItemSchema()->getItemDefinitionByName(event->getString("weapon"))) {
                    auto& purchase = playerPurchases[player->handle()];
                    if (const auto weaponInfo = memory->weaponSystem->getWeaponInfo(definition->getWeaponId())) {
                        purchase.totalCost += weaponInfo->price;
                        totalCost += weaponInfo->price;
                    }
                    const std::string weapon = interfaces->localize->findAsUTF8(definition->getItemBaseName());
                    ++purchaseTotal[weapon];
                    ++purchase.items[weapon];
                }
            }
            break;
        }
        case fnv::hash("round_start"):
            freezeEnd = 0.0f;
            playerPurchases.clear();
            purchaseTotal.clear();
            totalCost = 0;
            break;
        case fnv::hash("round_freeze_end"):
            freezeEnd = memory->globalVars->realtime;
            break;
        }
    }
    else {
        if (!mCfg.purchaseList.enabled)
            return;

        if (static const auto mp_buytime = interfaces->cvar->findVar("mp_buytime"); (!interfaces->engine->isInGame() || freezeEnd != 0.0f && memory->globalVars->realtime > freezeEnd + (!mCfg.purchaseList.onlyDuringFreezeTime ? mp_buytime->getFloat() : 0.0f) || playerPurchases.empty() || purchaseTotal.empty()) && !gui->isOpen())
            return;

        ImGui::SetNextWindowSize({ 200.0f, 200.0f }, ImGuiCond_Once);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!gui->isOpen())
            windowFlags |= ImGuiWindowFlags_NoInputs;
        if (mCfg.purchaseList.noTitleBar)
            windowFlags |= ImGuiWindowFlags_NoTitleBar;

        if (mCfg.purchaseList.pos != ImVec2{}) {
            ImGui::SetNextWindowPos(mCfg.purchaseList.pos);
            mCfg.purchaseList.pos = {};
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
        ImGui::Begin("Purchases", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (mCfg.purchaseList.mode == MiscConfig::PurchaseList::Details) {
            GameData::Lock lock;

            for (const auto& [handle, purchases] : playerPurchases) {
                std::string s;
                s.reserve(std::accumulate(purchases.items.begin(), purchases.items.end(), 0, [](int length, const auto& p) { return length + p.first.length() + 2; }));
                for (const auto& purchasedItem : purchases.items) {
                    if (purchasedItem.second > 1)
                        s += std::to_string(purchasedItem.second) + "x ";
                    s += purchasedItem.first + ", ";
                }

                if (s.length() >= 2)
                    s.erase(s.length() - 2);

                if (const auto player = GameData::playerByHandle(handle)) {
                    if (mCfg.purchaseList.showPrices)
                        ImGui::TextWrapped("%s $%d: %s", player->name.c_str(), purchases.totalCost, s.c_str());
                    else
                        ImGui::TextWrapped("%s: %s", player->name.c_str(), s.c_str());
                }
            }
        }
        else if (mCfg.purchaseList.mode == MiscConfig::PurchaseList::Summary) {
            for (const auto& purchase : purchaseTotal)
                ImGui::TextWrapped("%d x %s", purchase.second, purchase.first.c_str());

            if (mCfg.purchaseList.showPrices && totalCost > 0) {
                ImGui::Separator();
                ImGui::TextWrapped("Total: $%d", totalCost);
            }
        }
        ImGui::End();
    }
}

static int buttons = 0;

void Misc::runFreeCam(UserCmd* cmd, Vector viewAngles) noexcept
{
    static Vector currentViewAngles = Vector{ };
    static Vector realViewAngles = Vector{ };
    static bool wasCrouching = false;
    static bool hasSetAngles = false;

    buttons = cmd->buttons;

    if (!mCfg.freeCam.enabled || !FreeCamkeyPressed || !localPlayer || !localPlayer->isAlive())
    {
        if (hasSetAngles)
        {
            interfaces->engine->setViewAngles(realViewAngles);
            cmd->viewangles = currentViewAngles;
            if (wasCrouching)
                cmd->buttons |= UserCmd::IN_DUCK;
            wasCrouching = false;
            hasSetAngles = false;
        }
        currentViewAngles = Vector{};
        return;
    }

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (currentViewAngles.null())
    {
        currentViewAngles = cmd->viewangles;
        realViewAngles = viewAngles;
        wasCrouching = cmd->buttons & UserCmd::IN_DUCK;
    }

    cmd->forwardmove = 0;
    cmd->sidemove = 0;
    if (wasCrouching)
        cmd->buttons = UserCmd::IN_DUCK;
    else
        cmd->buttons = 0;
    cmd->viewangles = currentViewAngles;
    hasSetAngles = true;
}

void Misc::freeCam(ViewSetup* setup) noexcept
{
    static Vector newOrigin = Vector{ };

    if (!mCfg.freeCam.enabled || !FreeCamkeyPressed)
    {
        newOrigin = Vector{ };
        return;
    }

    if (!localPlayer || !localPlayer->isAlive())
        return;

    float freeCamSpeed = fabsf(static_cast<float>(mCfg.freeCam.Speed));

    if (newOrigin.null())
        newOrigin = setup->origin;

    Vector forward{ }, right{ }, up{ };

    Vector::fromAngleAll(setup->angles, &forward, &right, &up);

    const bool backBtn = buttons & UserCmd::IN_BACK;
    const bool forwardBtn = buttons & UserCmd::IN_FORWARD;
    const bool rightBtn = buttons & UserCmd::IN_MOVERIGHT;
    const bool leftBtn = buttons & UserCmd::IN_MOVELEFT;
    const bool shiftBtn = buttons & UserCmd::IN_SPEED;
    const bool duckBtn = buttons & UserCmd::IN_DUCK;
    const bool jumpBtn = buttons & UserCmd::IN_JUMP;

    if (duckBtn)
        freeCamSpeed *= 0.45f;

    if (shiftBtn)
        freeCamSpeed *= 1.65f;

    if (forwardBtn)
        newOrigin += forward * freeCamSpeed;

    if (rightBtn)
        newOrigin += right * freeCamSpeed;

    if (leftBtn)
        newOrigin -= right * freeCamSpeed;

    if (backBtn)
        newOrigin -= forward * freeCamSpeed;

    if (jumpBtn)
        newOrigin += up * freeCamSpeed;

    setup->origin = newOrigin;
}


void Misc::oppositeHandKnife(csgo::FrameStage stage) noexcept
{
    if (!mCfg.oppositeHandKnife)
        return;

    if (!localPlayer)
        return;

    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    static const auto cl_righthand = interfaces->cvar->findVar("cl_righthand");
    static bool original;

    if (stage == csgo::FrameStage::RENDER_START) {
        original = cl_righthand->getInt();

        if (const auto activeWeapon = localPlayer->getActiveWeapon()) {
            if (const auto classId = activeWeapon->getClientClass()->classId; classId == ClassId::Knife || classId == ClassId::KnifeGG)
                cl_righthand->setValue(!original);
        }
    }
    else {
        cl_righthand->setValue(original);
    }
}

static std::vector<std::uint64_t> reportedPlayers;
static int reportbotRound;

[[nodiscard]] static std::string generateReportString()
{
    std::string report;
    if (mCfg.reportbot.textAbuse)
        report += "textabuse,";
    if (mCfg.reportbot.griefing)
        report += "grief,";
    if (mCfg.reportbot.wallhack)
        report += "wallhack,";
    if (mCfg.reportbot.aimbot)
        report += "aimbot,";
    if (mCfg.reportbot.other)
        report += "speedhack,";
    return report;
}

[[nodiscard]] static bool isPlayerReported(std::uint64_t xuid)
{
    return std::ranges::find(std::as_const(reportedPlayers), xuid) != reportedPlayers.cend();
}

[[nodiscard]] static std::vector<std::uint64_t> getXuidsOfCandidatesToBeReported()
{
    std::vector<std::uint64_t> xuids;

    for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
        const auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity == localPlayer.get())
            continue;

        if (mCfg.reportbot.target != 2 && (localPlayer->isOtherEnemy(entity) ? mCfg.reportbot.target != 0 : mCfg.reportbot.target != 1))
            continue;

        if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(i, playerInfo) && !playerInfo.fakeplayer)
            xuids.push_back(playerInfo.xuid);
    }

    return xuids;
}

void Misc::disableExtrapolation() noexcept {
    auto extrapolate = interfaces->cvar->findVar("cl_extrapolate");
    extrapolate->setValue(!mCfg.extrapolate);
}

void Misc::forceRelayCluster() noexcept
{
    const std::string dataCentersList[] = { "", "syd", "vie", "gru", "scl", "dxb", "par", "fra", "hkg",
    "maa", "bom", "tyo", "lux", "ams", "limc", "man", "waw", "sgp", "jnb",
    "mad", "sto", "lhr", "atl", "eat", "ord", "lax", "mwh", "okc", "sea", "iad" };

    *memory->relayCluster = dataCentersList[mCfg.relayCluster];
}

void Misc::runReportbot() noexcept
{
    if (!mCfg.reportbot.enabled)
        return;

    if (!localPlayer)
        return;

    static auto lastReportTime = 0.0f;

    if (lastReportTime + mCfg.reportbot.delay > memory->globalVars->realtime)
        return;

    if (reportbotRound >= mCfg.reportbot.rounds)
        return;

    for (const auto& xuid : getXuidsOfCandidatesToBeReported()) {
        if (isPlayerReported(xuid))
            continue;

        if (const auto report = generateReportString(); !report.empty()) {
            memory->submitReport(std::to_string(xuid).c_str(), report.c_str());
            lastReportTime = memory->globalVars->realtime;
            reportedPlayers.push_back(xuid);
			return;
        }
    }

    reportedPlayers.clear();
    ++reportbotRound;
}

void Misc::resetReportbot() noexcept
{
    reportbotRound = 0;
    reportedPlayers.clear();
}

void Misc::drawAimbotFov(ImDrawList* drawList) noexcept
{
    if (!config->legitbotFov.enabled || !config->legitbotDeadzone.enabled && !config->aimbotKey.isActive())
        return;

    if (!localPlayer || !localPlayer->isAlive()) return;

    GameData::Lock lock;
    const auto& local = GameData::local();

    if (!local.exists || !local.alive || local.aimPunch.null())
        return;

    if (memory->input->isCameraInThirdPerson)
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    const auto& cfg = config->legitaimbot;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
    if (!cfg[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!cfg[weaponIndex].enabled)
        weaponIndex = 0;

    if (ImVec2 pos; Helpers::worldToScreen(local.aimPunch, pos))
    {
        const auto& displaySize = ImGui::GetIO().DisplaySize;
        const auto radiusFov = std::tan(Helpers::deg2rad(cfg[weaponIndex].fov / 1.411764705882353f) / (16.0f / 6.0f)) / std::tan(Helpers::deg2rad(localPlayer->isScoped() ? localPlayer->fov() : (Visuals::fov() + 90.0f)) / 2.0f) * displaySize.x;
        const auto radiusDeadZone = std::tan(Helpers::deg2rad(cfg[weaponIndex].deadzone / 1.411764705882353f) / (16.0f / 6.0f)) / std::tan(Helpers::deg2rad(localPlayer->isScoped() ? localPlayer->fov() : (Visuals::fov() + 90.0f)) / 2.0f) * displaySize.x;
        if (radiusFov > displaySize.x || radiusFov > displaySize.y || !std::isfinite(radiusFov))
            return;

        if (radiusDeadZone > displaySize.x || radiusDeadZone > displaySize.y || !std::isfinite(radiusDeadZone))
            return;

        const auto color = Helpers::calculateColor(config->legitbotFov);
        const auto colorDz = Helpers::calculateColor(config->legitbotDeadzone);
        if (config->legitbotFov.enabled) drawList->AddCircleFilled(localPlayer->shotsFired() > 1 ? pos : displaySize / 2.0f, radiusFov, color);
        if (config->legitbotDeadzone.enabled) drawList->AddCircleFilled(localPlayer->shotsFired() > 1 ? pos : displaySize / 2.0f, radiusDeadZone, colorDz);
    }
}

void Misc::visualizeAccuracy(ImDrawList* drawList) noexcept
{

    if (!mCfg.accuracyCircle.enabled)
        return;

    GameData::Lock lock;
    const auto& local = GameData::local();

    if (!local.exists || !local.alive || local.inaccuracy.null())
        return;

    if (ImVec2 edge; Helpers::worldToScreen(local.inaccuracy, edge))
    {
        const auto& displaySize = ImGui::GetIO().DisplaySize;
        const auto radius = std::sqrtf(ImLengthSqr(edge - displaySize / 2.0f));

        if (radius > displaySize.x || radius > displaySize.y || !std::isfinite(radius))
            return;

        const auto color = Helpers::calculateColor(mCfg.accuracyCircle.asColor4());
        drawList->AddCircleFilled(displaySize / 2.f, radius, color);
    }
}

struct nadesData
{
    float destructionTime;
    Vector pos;
};

static std::vector<nadesData> smokes;
static std::vector<nadesData> molotovs;

void Misc::drawSmokeTimerEvent(GameEvent* event) noexcept
{
    if (!event)
        return;

    if (!localPlayer) return;


    nadesData data{};
    const auto time = memory->globalVars->realtime + 17.5f;
    const auto pos = Vector(event->getFloat("x"), event->getFloat("y"), event->getFloat("z"));
    data.destructionTime = time;
    data.pos = pos;
    smokes.push_back(data);
}

void Misc::drawMolotovTimerEvent(GameEvent* event) noexcept
{
    if (!event)
        return;

    if (!localPlayer) return;

    nadesData data{};
    const float lifeTime = interfaces->cvar->findVar("inferno_flame_lifetime")->getFloat();
    const auto time = memory->globalVars->realtime + lifeTime;
    const auto pos = Vector(event->getFloat("x"), event->getFloat("y"), event->getFloat("z"));
    data.destructionTime = time;
    data.pos = pos;
    molotovs.push_back(data);
}

void Misc::drawSmokeTimer(ImDrawList* drawList) noexcept
{
    if (!mCfg.smokeTimer.enabled)
        return;

    if (!interfaces->engine->isInGame())
        return;

    if (!localPlayer) return;

    for (size_t i = 0; i < smokes.size(); i++) {
        const auto& smoke = smokes[i];

        auto time = smoke.destructionTime - memory->globalVars->realtime;
        std::ostringstream text; text << std::fixed << std::showpoint << std::setprecision(1) << time << " sec.";
        auto textSize = ImGui::CalcTextSize(text.str().c_str());

        ImVec2 pos;

        if (time >= 0.0f) {
            if (Helpers::worldToScreen(smoke.pos, pos)) {
                const auto radius = 16.f + mCfg.smokeTimer.timerThickness;
                const auto fraction = std::clamp(time / 17.5f, 0.0f, 1.0f);

                drawList->AddCircle(pos, radius, Helpers::calculateColor(mCfg.smokeTimer.backgroundColor), 40, 3.0f + mCfg.smokeTimer.timerThickness);
                if (fraction == 1.0f) {
                    drawList->AddCircle(pos, radius, Helpers::calculateColor(mCfg.smokeTimer.timerColor), 40, 2.0f + mCfg.smokeTimer.timerThickness);
                }
                else {
                    constexpr float pi = std::numbers::pi_v<float>;
                    const auto arc270 = (3 * pi) / 2;
                    drawList->PathArcTo(pos, radius - 0.5f, arc270 - (2 * pi * fraction), arc270, 40);
                    drawList->PathStroke(Helpers::calculateColor(mCfg.smokeTimer.timerColor), false, 4.0f + mCfg.smokeTimer.timerThickness);
                }
                drawList->AddText(ImVec2(pos.x - (textSize.x / 2.f), pos.y + (mCfg.smokeTimer.timerThickness * 4.5f) + (textSize.y / 2.f)), Helpers::calculateColor(mCfg.smokeTimer.textColor), text.str().c_str());
                drawList->AddImage(GameData::getNade(2), { pos.x - 4.f, pos.y - 16.f }, { pos.x + 8.f, pos.y + 10.f });
                Helpers::setAlphaFactor(1.f);
            }
        }
        else
            smokes.erase(smokes.begin() + i);
    }
}

void Misc::drawMolotovTimer(ImDrawList* drawList) noexcept
{
    if (!mCfg.molotovTimer.enabled)
        return;

    if (!interfaces->engine->isInGame())
        return;

    if (!localPlayer) return;

    const float lifeTime = interfaces->cvar->findVar("inferno_flame_lifetime")->getFloat();

    for (size_t i = 0; i < molotovs.size(); i++) {
        const auto& molo = molotovs[i];

        auto time = molo.destructionTime - memory->globalVars->realtime;
        std::ostringstream text; text << std::fixed << std::showpoint << std::setprecision(1) << time << " sec.";
        auto textSize = ImGui::CalcTextSize(text.str().c_str());

        ImVec2 pos;

        if (time >= 0.0f) {
            if (Helpers::worldToScreen(molo.pos, pos)) {
                const auto radius = 16.f + mCfg.molotovTimer.timerThickness;
                const auto fraction = std::clamp(time / lifeTime, 0.0f, 1.0f);

                drawList->AddCircle(pos, radius, Helpers::calculateColor(mCfg.molotovTimer.backgroundColor), 40, 3.0f + mCfg.molotovTimer.timerThickness);
                if (fraction == 1.0f) {
                    drawList->AddCircle(pos, radius, Helpers::calculateColor(mCfg.molotovTimer.timerColor), 40, 2.0f + mCfg.molotovTimer.timerThickness);
                }
                else {
                    constexpr float pi = std::numbers::pi_v<float>;
                    const auto arc270 = (3 * pi) / 2;
                    drawList->PathArcTo(pos, radius - 0.5f, arc270 - (2 * pi * fraction), arc270, 40);
                    drawList->PathStroke(Helpers::calculateColor(mCfg.molotovTimer.timerColor), false, 4.0f + mCfg.molotovTimer.timerThickness);
                }
                drawList->AddText(ImVec2(pos.x - (textSize.x / 2), pos.y + (mCfg.molotovTimer.timerThickness * 4.5f) + (textSize.y / 2)), Helpers::calculateColor(mCfg.molotovTimer.textColor), text.str().c_str());
                drawList->AddImage(GameData::getNade(3), { pos.x - 8.f, pos.y - 17.f }, { pos.x + 8.f, pos.y + 10.f });
                Helpers::setAlphaFactor(1.f);
            }
        }
        else
            molotovs.erase(molotovs.begin() + i);
    }
}

void Misc::drawPlayerList() noexcept
{
    if (!mCfg.playerList.enabled) return;

    if (mCfg.playerList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.playerList.pos);
        mCfg.playerList.pos = {};
    }

    if (mCfg.playerList.hideInGame && !gui->isOpen()) return;

    if (mCfg.playerList.plkeybind.isSet() && !mCfg.playerList.plkeybind.isDown() && !mCfg.playerList.hideInGame) return;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    if (!gui->isOpen() || isInChat())
        windowFlags |= ImGuiWindowFlags_NoInputs;

    if (mCfg.playerList.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    
    ImGui::Begin("Player List", nullptr, windowFlags);

    if (ImGui::BeginTable("table", 7)){

        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("  ");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("HP");
        ImGui::TableSetupColumn("Weapon");
        if (!mCfg.playerList.mode) ImGui::TableSetupColumn("Team");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();
        if (!localPlayer) {
            ImGui::EndTable();
            ImGui::End();
            return;
        }
        std::vector<std::reference_wrapper<const PlayerData>> playersOrdered{ GameData::players().begin(), GameData::players().end() };
        std::ranges::sort(playersOrdered, [](const PlayerData& a, const PlayerData& b) {
            // enemies first
            if (a.enemy != b.enemy)
                return a.enemy && !b.enemy;

            return a.handle < b.handle;
            });

        for (const PlayerData& player : playersOrdered)
        {
            ImGui::TableNextRow();
            ImGui::PushID(ImGui::TableGetRowIndex());

            //name level ranking team hp armor money place

            switch (mCfg.playerList.mode) {
            default:
            case 0: //ALL
                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted(std::to_string(player.index).c_str()); //Name
                if (ImGui::TableNextColumn())
                    ImGui::Image(player.getAvatarTexture(), ImVec2(16, 16)); //Avatar

                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted(player.name.c_str()); //Name

                if (ImGui::TableNextColumn())
                    player.health > 0 ? ImGui::Text("%i", player.health) : ImGui::TextColored(ImVec4{ 1.f, 0.f, 0.f, 1.f }, "Dead"); //HP

                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted(player.activeWeapon.c_str()); //Weapon

                if (ImGui::TableNextColumn())
                    switch ((int)player.team) {
                    case 1: ImGui::Text("SPEC"); break;
                    case 2: ImGui::Text("T"); break;
                    case 3: ImGui::Text("CT"); break;
                    }
                if (ImGui::TableNextColumn()) {
                    if (ImGui::Button("More")) {
                        ImGui::OpenPopup("##yoink");
                    }
                }
                break;
            case 1: //Allies
                if (player.team == localPlayer->getTeamNumber()) {
                    if (ImGui::TableNextColumn())
                        ImGui::Image(player.getAvatarTexture(), ImVec2(16, 16)); //Avatar

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(player.name.c_str()); //Name

                    if (ImGui::TableNextColumn())
                        player.health > 0 ? ImGui::Text("%i", player.health) : ImGui::TextColored(ImVec4{ 1.f, 0.f, 0.f, 1.f }, "Dead"); //HP

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(player.activeWeapon.c_str()); //Weapon

                    if (ImGui::TableNextColumn()) {
                        if (ImGui::Button("More")) {
                            ImGui::OpenPopup("##yoink");
                        }
                    }
                }
                break;

            case 2: //Ememies
                if (player.team != localPlayer->getTeamNumber()) {
                    if (ImGui::TableNextColumn())
                        ImGui::Image(player.getAvatarTexture(), ImVec2(16, 16)); //Avatar

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(player.name.c_str()); //Name

                    if (ImGui::TableNextColumn())
                        player.health > 0 ? ImGui::Text("%i", player.health) : ImGui::TextColored(ImVec4{ 1.f, 0.f, 0.f, 1.f }, "Dead"); //HP

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(player.activeWeapon.c_str()); //Weapon

                    if (ImGui::TableNextColumn()) {
                        if (ImGui::Button("More")) {
                            ImGui::OpenPopup("##yoink");
                        }
                    }
                }
                break;
            }

            if (ImGui::BeginPopup("##yoink"))
            {
                ImGui::Text("Information about %s:", player.name.c_str()); ImGui::Separator();
                ImGui::Text("\n");

                ImGui::PushID("steal name");
                ImGui::Text("Name: %s", player.name.c_str());
                if (ImGui::Button("Copy", ImVec2(64.f, 24.f))) {
                    ImGui::SetClipboardText(player.name.c_str());
                    Helpers::CheatChatNotification(false, std::string("| Copied to clipboard: ").append(player.name).c_str());
                }
                ImGui::SameLine();
                if (ImGui::Button("Use", ImVec2(64.f, 24.f))) {
                    Fun::changeName(false, std::string(player.name + '\x1' + '\x1').c_str(), 2.f);
                    Helpers::CheatChatNotification(false, std::string("Set your name to: ").append(player.name).c_str());
                }
                ImGui::PopID();

                ImGui::PushID("steal clantag");
                ImGui::Text("Clantag %s:", player.clanTag.c_str());
                if (ImGui::Button("Copy", ImVec2(64.f, 24.f))) {
                    ImGui::SetClipboardText(player.clanTag.c_str());
                    Helpers::CheatChatNotification(false, std::string("Copied to clipboard: ").append(player.clanTag).c_str());
                }
                ImGui::SameLine();
                if (ImGui::Button("Use", ImVec2(64.f, 24.f))) {
                    Clan::update(true);
                    memory->setClanTag(player.clanTag.c_str(), player.clanTag.c_str());
                    Helpers::CheatChatNotification(false, std::string("Set your clantag to: ").append(player.clanTag).c_str());
                }
                ImGui::PopID();
                ImGui::PushID("steal steamid64");
                ImGui::Text("SteamID64 %i:", player.steamID);
                if (ImGui::Button("Copy", ImVec2(64.f, 24.f))) ImGui::SetClipboardText(std::to_string(player.steamID).c_str());
                ImGui::PopID();
                ImGui::Separator();
                ImGui::PushID("useless");
                ImGui::Text("Origin x:%f.1 y:%f.1 z:%f.1", player.origin.x, player.origin.y, player.origin.z);
                ImGui::Text("Angle x:%f.1 y:%f.1 z:%f.1", player.angle.x, player.angle.y, player.angle.z);
                ImGui::Text("Delta %2.f", player.maxDesyncAngle);
                ImGui::Text("Health %i:", player.health);
                ImGui::Text("Armor %i:", player.armor);
                ImGui::Text("Velocity %.0f:", player.velocity);
                ImGui::Text("Weapon %s:", player.activeWeapon.c_str());
                ImGui::Text("Audible? %s:", player.audible ? "true" : "false");
                ImGui::Text("Alive? %s:", player.alive ? "true" : "false");
                ImGui::Text("Spotted? %s:", player.spotted ? "true" : "false");
                ImGui::Text("Flash Duration: %.2f", player.flashDuration);
                ImGui::Text("Rank: %s", Helpers::ConvertRankFromNumber(false, player.competitiveRank));
                ImGui::Text("Wins: %i", player.competitiveWins);
                ImGui::Image(player.getAvatarTexture(), ImVec2(64, 64)); //Avatar
                ImGui::PopID();
                if (player.team == localPlayer->getTeamNumber() && player.steamID != 0)
                {
                    ImGui::Separator();
                    if (ImGui::Button("Kick"))
                    {
                        const std::string cmd = "callvote kick " + std::to_string(player.userId);
                        interfaces->engine->clientCmdUnrestricted(cmd.c_str());
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (gui->isOpen()) {
        ImGui::Separator();
        switch (mCfg.playerList.mode) {
        default:
        case 0: //ALL
            if (ImGui::Button("All", ImVec2(64.f, 24.f))) mCfg.playerList.mode++;
            break;
        case 1: //ALLIES
            if (ImGui::Button("Allies", ImVec2(64.f, 24.f))) mCfg.playerList.mode++;
            break;
        case 2: //ENEMIES
            if (ImGui::Button("Enemies", ImVec2(64.f, 24.f))) mCfg.playerList.mode = 0;
            break;
        }
    }

    ImGui::End();
}

void Misc::preserveKillfeed(bool roundStart) noexcept
{
    if (!mCfg.killfeedChanger.preserve)
        return;

    static auto nextUpdate = 0.0f;

    if (roundStart) {
        nextUpdate = memory->globalVars->realtime + 10.0f;
        return;
    }

    if (nextUpdate > memory->globalVars->realtime)
        return;

    nextUpdate = memory->globalVars->realtime + 2.0f;

    const auto deathNotice = std::uintptr_t(memory->findHudElement(memory->hud, "CCSGO_HudDeathNotice"));
    if (!deathNotice)
        return;

    const auto deathNoticePanel = (*(UIPanel**)(*reinterpret_cast<std::uintptr_t*>(deathNotice WIN32_LINUX(-20 + 88, -32 + 128)) + sizeof(std::uintptr_t)));

    const auto childPanelCount = deathNoticePanel->GetChildCount();

    for (int i = 0; i < childPanelCount; ++i) {
        const auto child = deathNoticePanel->GetChild(i);
        if (!child)
            continue;

        if (child->HasClass("DeathNotice_Killer") && (mCfg.killfeedChanger.preserveOnlyHeadshots|| child->HasClass("DeathNoticeHeadShot")))
            child->SetAttributeFloat("SpawnTime", memory->globalVars->currenttime);
    }
}

void Misc::killfeedChanger(GameEvent& event) noexcept
{
    if (!mCfg.killfeedChanger.enabled)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    if (!mCfg.killfeedChanger.icon.empty())
        event.setString("weapon", mCfg.killfeedChanger.icon.c_str());

    if (mCfg.killfeedChanger.headshot)
        event.setInt("headshot", 1);

    if (mCfg.killfeedChanger.dominated)
        event.setInt("Dominated", 1);

    if (mCfg.killfeedChanger.revenge)
        event.setInt("Revenge", 1);

    if (mCfg.killfeedChanger.penetrated)
        event.setInt("penetrated", 1);

    if (mCfg.killfeedChanger.noscope)
        event.setInt("noscope", 1);

    if (mCfg.killfeedChanger.thrusmoke)
        event.setInt("thrusmoke", 1);

    if (mCfg.killfeedChanger.attackerblind)
        event.setInt("attackerblind", 1);

    if (mCfg.killfeedChanger.wipe)
        event.setInt("wipe", 1);
}

void Misc::customMVPSound(GameEvent& event) noexcept {

    if (mCfg.customMVPSound == "") return;

    auto volume = interfaces->cvar->findVar("snd_mvp_volume")->getFloat();

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (localPlayer->getUserId() == event.getInt("userid"))
    interfaces->engine->clientCmdUnrestricted(("playvol " + mCfg.customMVPSound + " " + std::to_string(volume)).c_str());

    return;
}

struct VoteRevealerArray {
    Entity* entity;
    bool vote;
};
struct VoteRevealer {
    int remain = -1;
    Entity* callEntity;
    const char* callVoteType = "";
    bool passed;
    bool voteEnded;
}voteReveal;



std::vector<VoteRevealerArray> vote{};

void Misc::voteRevealer(GameEvent& event) noexcept
{
    if (!Helpers::getByteFromBytewise(mCfg.reveal, 4))
        return;

    const auto entity = interfaces->entityList->getEntity(event.getInt("entityid"));
    if (!entity || !entity->isPlayer())
        return;

    const auto votedYes = event.getInt("vote_option") == 0;
    const auto isLocal = localPlayer && entity == localPlayer.get();
    const char color = votedYes ? '\x06' : '\x07';
    const char* prefixColor = Helpers::TextColors(config->style.prefixColor);

    memory->clientMode->getHudChat()->printf(0, " %sAzurre \x08| %c%s\x01 voted %c%s\x01", prefixColor, isLocal ? '\x01' : color, isLocal ? "You" : entity->getPlayerName().c_str(), color, votedYes ? "Yes" : "No");
    vote.push_back({entity, static_cast<bool>(votedYes)});
}

void Misc::onVoteStart(const void* data, int size) noexcept
{
    if (!Helpers::getByteFromBytewise(mCfg.reveal, 4))
        return;

    constexpr auto voteName = [](int index) {
        switch (index) {
        case 0: return "Kick";
        case 1: return "Change Level";
        case 6: return "Surrender";
        case 13: return "Start TimeOut";
        default: return "";
        }
    };

    const auto reader = ProtobufReader{ static_cast<const std::uint8_t*>(data), size };
    const auto entityIndex = reader.readInt32(2);

    const auto entity = interfaces->entityList->getEntity(entityIndex);
    if (!entity || !entity->isPlayer())
        return;

    const auto isLocal = localPlayer && entity == localPlayer.get();

    const auto voteType = reader.readInt32(3);
    const char* prefixColor = Helpers::TextColors(config->style.prefixColor);

    memory->clientMode->getHudChat()->printf(0, " %sAzurre \x08| %c%s\x01  call vote (\x06%s\x01)", prefixColor ,isLocal ? '\x01' : '\x06', isLocal ? "You" : entity->getPlayerName().c_str(), voteName(voteType));
    voteReveal.callEntity = entity;
    voteReveal.callVoteType = voteName(voteType);
    voteReveal.voteEnded = false;
    voteReveal.remain = memory->globalVars->currenttime + 120.f;
}

void Misc::onVotePass() noexcept
{
    if (Helpers::getByteFromBytewise(mCfg.reveal, 4)) {
        Helpers::CheatChatNotification(false, "Vote\x06 PASSED");
        vote.clear();
        voteReveal.voteEnded = true;
        voteReveal.passed = true;
        voteReveal.callEntity = nullptr;
        voteReveal.callVoteType = "";
        voteReveal.remain = memory->globalVars->currenttime + 10.f;
    }
}

void Misc::onVoteFailed() noexcept
{
    if (Helpers::getByteFromBytewise(mCfg.reveal, 4)) {
        Helpers::CheatChatNotification(false, "Vote\x07 FAILED");
        vote.clear();
        voteReveal.voteEnded = true;
        voteReveal.passed = false;
        voteReveal.callEntity = nullptr;
        voteReveal.callVoteType = "";
        voteReveal.remain = memory->globalVars->currenttime + 10.f;
    }
}

void Misc::guiVoteRevealer() noexcept
{
    //if (!Helpers::getByteFromBytewise(mCfg.reveal, 4))
        return;

    static ImVec4 yesColor{ 0.f, 1.f, 0.f, 1.f };
    static ImVec4 noColor{ 1.f, 0.f, 0.f, 1.f };

    GameData::Lock lock;

    if (mCfg.voteRevealers.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.voteRevealers.pos);
        mCfg.voteRevealers.pos = {};
    }
    ImGui::SetNextWindowSize({ 200.0f, 0.f });

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (mCfg.voteRevealers.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (mCfg.voteRevealers.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    if (gui->isOpen()) {
        ImGui::Begin("Vote Reveal", nullptr, windowFlags);
        ImGui::Text("Timeout vote by: You");
        ImGui::Text("VALVE voted: "); ImGui::SameLine(); ImGui::TextColored( noColor, "No");
        ImGui::Text("Azurre voted: "); ImGui::SameLine(); ImGui::TextColored( yesColor , "Yes");
        ImGui::Text("GOTV voted: "); ImGui::SameLine(); ImGui::TextColored( yesColor, "Yes");
        ImGui::End();
        return;
    }

    if (!interfaces->engine->isInGame()) return;

    if (!localPlayer) return;

    if (!voteReveal.voteEnded) return;

    auto curr = memory->globalVars->currenttime;

    ImGui::Begin("Vote Reveal", nullptr, windowFlags);
    if (voteReveal.callEntity) {
        ImGui::Text("%s vote by: %s\n", voteReveal.callVoteType, voteReveal.callEntity->getPlayerName().c_str());
    }

    for (auto i = std::begin(vote); i != std::end(vote); ++i) {
        ImGui::Text("%s voted: ", i->entity->getPlayerName().c_str()); ImGui::SameLine(); ImGui::TextColored(i->vote ? yesColor : noColor, i->vote ? "Yes" : "No");
    }
    if (voteReveal.voteEnded == true) {
        ImGui::Text("Vote: "); ImGui::SameLine(); ImGui::TextColored(voteReveal.passed ? yesColor : noColor, "%s", voteReveal.passed ? "YES" : "NO");
    }
    if (voteReveal.remain < curr) {
        voteReveal.voteEnded = false;
        vote.clear();
    }
    ImGui::End();
}

// ImGui::ShadeVertsLinearColorGradientKeepAlpha() modified to do interpolation in HSV
static void shadeVertsHSVColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;

    ImVec4 col0HSV = ImGui::ColorConvertU32ToFloat4(col0);
    ImVec4 col1HSV = ImGui::ColorConvertU32ToFloat4(col1);
    ImGui::ColorConvertRGBtoHSV(col0HSV.x, col0HSV.y, col0HSV.z, col0HSV.x, col0HSV.y, col0HSV.z);
    ImGui::ColorConvertRGBtoHSV(col1HSV.x, col1HSV.y, col1HSV.z, col1HSV.x, col1HSV.y, col1HSV.z);
    ImVec4 colDelta = col1HSV - col0HSV;

    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);

        float h = col0HSV.x + colDelta.x * t;
        float s = col0HSV.y + colDelta.y * t;
        float v = col0HSV.z + colDelta.z * t;

        ImVec4 rgb;
        ImGui::ColorConvertHSVtoRGB(h, s, v, rgb.x, rgb.y, rgb.z);
        vert->col = (ImGui::ColorConvertFloat4ToU32(rgb) & ~IM_COL32_A_MASK) | (vert->col & IM_COL32_A_MASK);
    }
}

void Misc::drawOffscreenEnemies(ImDrawList* drawList) noexcept
{
    if (!mCfg.offscreenEnemies.enabled)
        return;

    const auto yaw = Helpers::deg2rad(interfaces->engine->getViewAngles().y);

    GameData::Lock lock;
    for (auto& player : GameData::players()) {
        if ((player.dormant && player.fadingAlpha() == 0.0f) || !player.alive || !player.enemy || player.inViewFrustum)
            continue;

        const auto positionDiff = GameData::local().origin - player.origin;

        auto x = std::cos(yaw) * positionDiff.y - std::sin(yaw) * positionDiff.x;
        auto y = std::cos(yaw) * positionDiff.x + std::sin(yaw) * positionDiff.y;
        if (const auto len = std::sqrt(x * x + y * y); len != 0.0f) {
            x /= len;
            y /= len;
        }

        constexpr auto avatarRadius = 13.0f;
        constexpr auto triangleSize = 10.0f;

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } *200;
        const auto trianglePos = pos + ImVec2{ x, y } *(avatarRadius + (mCfg.offscreenEnemies.healthBar.enabled ? 5 : 3));

        Helpers::setAlphaFactor(player.fadingAlpha());
        const auto white = Helpers::calculateColor(255, 255, 255, 255);
        const auto background = Helpers::calculateColor(0, 0, 0, 80);
        const auto color = Helpers::calculateColor(mCfg.offscreenEnemies);
        const auto healthBarColor = mCfg.offscreenEnemies.healthBar.type == HealthBar::HealthBased ? Helpers::healthColor(std::clamp(player.health / 100.0f, 0.0f, 1.0f)) : Helpers::calculateColor(mCfg.offscreenEnemies.healthBar);
        Helpers::setAlphaFactor(1.0f);

        const ImVec2 trianglePoints[]{
            trianglePos + ImVec2{  0.4f * y, -0.4f * x } *triangleSize,
            trianglePos + ImVec2{  1.0f * x,  1.0f * y } *triangleSize,
            trianglePos + ImVec2{ -0.4f * y,  0.4f * x } *triangleSize
        };

        drawList->AddConvexPolyFilled(trianglePoints, 3, color);
        drawList->AddCircleFilled(pos, avatarRadius + 1, white & IM_COL32_A_MASK, 40);

        const auto texture = player.getAvatarTexture();

        const bool pushTextureId = drawList->_TextureIdStack.empty() || texture != drawList->_TextureIdStack.back();
        if (pushTextureId)
            drawList->PushTextureID(texture);

        const int vertStartIdx = drawList->VtxBuffer.Size;
        drawList->AddCircleFilled(pos, avatarRadius, white, 40);
        const int vertEndIdx = drawList->VtxBuffer.Size;
        ImGui::ShadeVertsLinearUV(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ avatarRadius, avatarRadius }, pos + ImVec2{ avatarRadius, avatarRadius }, { 0, 0 }, { 1, 1 }, true);

        if (pushTextureId)
            drawList->PopTextureID();

        if (mCfg.offscreenEnemies.healthBar.enabled) {
            const auto radius = avatarRadius + 2;
            const auto healthFraction = std::clamp(player.health / 100.0f, 0.0f, 1.0f);

            drawList->AddCircle(pos, radius, background, 40, 3.0f);

            const int vertStartIdx = drawList->VtxBuffer.Size;
            if (healthFraction == 1.0f) { // sometimes PathArcTo is missing one top pixel when drawing a full circle, so draw it with AddCircle
                drawList->AddCircle(pos, radius, healthBarColor, 40, 2.0f);
            }
            else {
                constexpr float pi = std::numbers::pi_v<float>;
                drawList->PathArcTo(pos, radius - 0.5f, pi / 2 - pi * healthFraction, pi / 2 + pi * healthFraction, 40);
                drawList->PathStroke(healthBarColor, false, 2.0f);
            }
            const int vertEndIdx = drawList->VtxBuffer.Size;

            if (mCfg.offscreenEnemies.healthBar.type == HealthBar::Gradient)
                shadeVertsHSVColorGradientKeepAlpha(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ 0.0f, radius }, pos + ImVec2{ 0.0f, radius }, IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 0, 255));
        }
    }
}

struct Faceit {
    Entity* entity = nullptr;
    int damageGiven = 0;
    int damageTaken = 0;
    int hitGiven = 0;
    int hitTaken = 0;
    int remainingHealth = 100;
};

Faceit collectedData[64];

void Misc::faceitSimulator(GameEvent* event, int eventID) noexcept { //WIP

    // Event ID:
    // 0 - Round Start
    // 1 - Collect Info
    // 2 - End of Round
    
    GameData::Lock lock;

    if (!mCfg.faceit) return;

    if (!localPlayer) return;

    if (event) {

        const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));
        const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("attacker")));


        switch (eventID)
        {
        default: return;
        case 0:
            memory->clientMode->getHudChat()->printf(0, std::string("").append("\x4").append("[FACEIT^] LIVE!").c_str());
            memory->clientMode->getHudChat()->printf(0, std::string("").append("\x4").append("[FACEIT^] LIVE!").c_str());
            memory->clientMode->getHudChat()->printf(0, std::string("").append("\x4").append("[FACEIT^] LIVE!").c_str());
            memory->clientMode->getHudChat()->printf(0, std::string("").append("\x4").append("[FACEIT^] Please be aware that this match has overtime enabled, there is no tie.").c_str());
            break;
        case 1:
            if (!userID || !attacker) return;

            if (localPlayer->getUserId() != attacker->getUserId() && localPlayer->getUserId() != userID->getUserId()) return;

            if (attacker->getTeamNumber() == userID->getTeamNumber()) return;

            if (localPlayer->getUserId() == attacker->getUserId()) { //You attacked enemy
                collectedData[userID->index()].damageGiven += event->getInt("dmg_health");
                collectedData[userID->index()].entity = userID;
                collectedData[userID->index()].remainingHealth -= event->getInt("health");
                collectedData[userID->index()].hitGiven++;
            }

            if (localPlayer->getUserId() == userID->getUserId()) { //Enemy attacked you
               collectedData[localPlayer->index()].damageGiven += event->getInt("dmg_health");
               collectedData[localPlayer->index()].entity = localPlayer.get();
               collectedData[localPlayer->index()].remainingHealth -= event->getInt("health");
               collectedData[localPlayer->index()].hitGiven++;
            }
            break;

            case 2:
                for (int x = 0; x < interfaces->entityList->getHighestEntityIndex(); x++) {

                    if (collectedData[x].entity == nullptr) continue;

                    if (collectedData[x].entity->index() == localPlayer->index()) continue;

                    std::string message;
                    message += '\x4';
                    message += "[FACEIT^] To: [";
                    message += std::to_string(collectedData[x].damageGiven);
                    message += " / ";
                    message += std::to_string(collectedData[x].hitGiven);
                    message += "hits] From: [";
                    message += std::to_string(collectedData[x].damageTaken);
                    message += " / ";
                    message += std::to_string(collectedData[x].hitTaken);
                    message += "hits] - ";
                    message += collectedData[x].entity->getPlayerName();
                    message += "(";
                    message += std::to_string(collectedData[x].remainingHealth);
                    message += ")";

                    collectedData[x].entity = nullptr;
                    collectedData[x].damageGiven = 0;
                    collectedData[x].damageTaken = 0;
                    collectedData[x].hitGiven = 0;
                    collectedData[x].hitTaken = 0;
                    collectedData[x].remainingHealth = 100;

                    memory->clientMode->getHudChat()->printf(0, message.c_str());
                }
        }

        /*
        
        [FACEIT^] Available Commands:
        - !gg to launch a forfeit vote during the match
        - !report to reprot a teammate
        
        [FACEIT^] team_teamname [15-15] team_azurre
        [FACEIT^] To: [52 / 2 hits] From: [0 / 0 hits] - sisupastilli (42 hp)
        [FACEIT^] To: [120 / 1 hits] From: [0 / 0 hits] - playername (0 hp)
        [FACEIT^] To: [120 / 1 hits] From: [0 / 0 hits] - playername (0 hp)
        [FACEIT^] To: [120 / 1 hits] From: [0 / 0 hits] - playername (0 hp)
        */
    }
}

void Misc::autoAccept(const char* soundEntry) noexcept //OG from Osiris
{
    if (!mCfg.autoAccept || interfaces->engine->isInGame())
        return;

    if (std::strcmp(soundEntry, "UIPanorama.popup_accept_match_beep"))
        return;

    if (const auto idx = memory->registeredPanoramaEvents->find(memory->makePanoramaSymbol("MatchAssistedAccept")); idx != -1) {
        if (const auto eventPtr = memory->registeredPanoramaEvents->memory[idx].value.makeEvent(nullptr))
            interfaces->panoramaUIEngine->accessUIEngine()->DispatchEvent(eventPtr);
    }
    FlashGame(0);
}

void Misc::overheadInfo() noexcept {
    if (Helpers::getByteFromBytewise(mCfg.reveal, 5)) {
        DWORD oldProtect = 0;
        VirtualProtect(memory->overheadInfo, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
        BYTE ovbytes[2] = { 0x90, 0x90 };
        memcpy(memory->overheadInfo, ovbytes, 2);
        VirtualProtect(memory->overheadInfo, 2, oldProtect, NULL);
    }
    else
    {
        DWORD oldProtect = 0;
        VirtualProtect(memory->overheadInfo, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
        BYTE ovbytes[2] = { 0x3B, 0xC6 };
        memcpy(memory->overheadInfo, ovbytes, 2);
        VirtualProtect(memory->overheadInfo, 2, oldProtect, NULL);
    }
}

float roll = 0.f;

void Misc::getRoll(UserCmd* cmd) noexcept {
    roll = cmd->viewangles.z;
}

void Misc::indicators() noexcept {
    if (!mCfg.indicators.enabled)
        return;

    if (!localPlayer)
        return;

    if (mCfg.indicators.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(mCfg.indicators.pos);
        mCfg.indicators.pos = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;

    if (mCfg.indicators.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (mCfg.indicators.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::SetNextWindowSize({ 192.0f, 0.0f });
    ImGui::Begin("Indicators", &mCfg.indicators.enabled, windowFlags);

    const auto networkChannel = interfaces->engine->getNetworkChannel();
    if (networkChannel && Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 0))
    {
        ImGui::TextUnformatted("Choked packets");
        ImGui::progressBarFullWidth(static_cast<float>(networkChannel->chokedPackets) / 16, 5.f, true);
    }

    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 1)) {
        ImGui::TextUnformatted("Height");
        ImGui::progressBarFullWidth((localPlayer->getEyePosition().z - localPlayer->origin().z - PLAYER_EYE_HEIGHT_CROUCH) / (PLAYER_EYE_HEIGHT - PLAYER_EYE_HEIGHT_CROUCH), 5.f, true);
    }
    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 2)) {
        ImGui::TextUnformatted("LBY Update");
        ImGui::progressBarFullWidth(1.f - AntiAim::getLBYUpdate(), 5.f, true);
    }
    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 3)) {
    ImGui::TextUnformatted("Roll");
    ImGui::progressBarFullWidth((roll + 90.f) / 180.f, 5.f, true);
    }
    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 4)) {
        ImGui::TextUnformatted("Paused Ticks");
        //If you have dt enabled, you need to shift 13 ticks, so it will return 13 ticks
        //If you have hs enabled, you need to shift 9 ticks, so it will return 7 ticks
        ImGui::progressBarFullWidth(Tickbase::pausedTicks() / 13.f, 5.f, true);
    }
    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 5)) {
        ImGui::TextUnformatted("Velocity");
        float velocity = localPlayer->velocity().length2D() / 300.f;
        ImGui::progressBarFullWidth(std::clamp(velocity, 0.f, 1.f), 5.f, true);
    }
    if (Helpers::getByteFromBytewise(mCfg.indicators.bytewise, 6)) {
        ImGui::TextUnformatted("Grenade Throw Strength");

        auto activeWeapon = localPlayer->getActiveWeapon();
        if (!activeWeapon) {
            ImGui::progressBarFullWidth(0 / 100.f, 5.f, true);
            ImGui::End();
            return;

        }
        if (!activeWeapon->isGrenade()){
            ImGui::progressBarFullWidth(0 / 100.f, 5.f, true);
            ImGui::End();
            return;
        }
        ImGui::progressBarFullWidth(activeWeapon->throwStrength() / 1.f, 5.f, true);
    }
    //ImGui::TextColored(AntiAim::LBYIndicator(false) ? ImVec4{.0f, 1.f, .0f, 1.f } : ImVec4{ 1.f, .0f, .0f, 1.f }, "LBY");
    //ImGui::Text("Last shot: %s", Backtrack::lastShotLagRecord() ? "Lag Record" : "Modern Record");

    ImGui::End();
}

void Misc::updateEventListeners(bool forceRemove) noexcept
{
    class PurchaseEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override { purchaseList(event); }
    };

    static PurchaseEventListener listener;
    static bool listenerRegistered = false;

    if (mCfg.purchaseList.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "item_purchase");
        listenerRegistered = true;
    }
    else if ((!mCfg.purchaseList.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

static bool windowOpen = false;

void Misc::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Misc")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Misc");
        ImGui::SetWindowPos("Misc", { 100.0f, 100.0f });
    }
}

void Misc::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 560.0f, 480.0f });
        ImGui::Begin("Misc", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 288.0f);
    ImGui::hotkey("Menu Key", mCfg.menuKey);
    ImGui::Checkbox("Color Palette", &mCfg.colorPalette.enabled);
    ImGui::Checkbox("Anti AFK Kick", &mCfg.antiAfkKick);
    ImGui::Checkbox("Anti TKill", &mCfg.antiTkill);
    ImGui::PushID("moonwalk");
    ImGui::Checkbox("Slide Walk", &mCfg.moonwalk); ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::SliderInt("Break Animation Speed", &mCfg.animFuck, 0, 10);
        ImGui::EndPopup();
    }
        ImGui::PopID();
    ImGui::Checkbox("Unlock Inventory", &mCfg.unlockInventory);
    ImGui::Checkbox("Auto Pistol", &mCfg.autoPistol);
    ImGui::Checkbox("Auto Reload", &mCfg.autoReload);
    ImGui::Checkbox("Auto Accept", &mCfg.autoAccept);
    ImGui::SetNextItemWidth(100.f);
    ImGuiCustom::multiCombo("Reveal", mCfg.reveal, "Radar\0Ranks\0Money\0Suspect\0Votes\0Overhead Info");
    ImGui::Checkbox("Spectator List", &mCfg.spectatorList.enabled);
    ImGui::SameLine();
    ImGui::PushID("Spectator list");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &mCfg.spectatorList.noTitleBar);
        ImGui::Checkbox("No Background", &mCfg.spectatorList.noBackground);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Keybinds List", &mCfg.binds.enabled);
    ImGui::SameLine();
    ImGui::PushID("Keybinds");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("kbbb");

    if (ImGui::BeginPopup("kbbb")) {
        ImGui::Checkbox("No Title Bar", &mCfg.binds.noTitleBar);
        ImGui::Checkbox("No Background", &mCfg.binds.noBackground);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PushID("wm");
    ImGui::Checkbox("Watermark", &mCfg.watermark.enabled);
	ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        //ImGuiCustom::multiCombo("", mCfg.watermark.flags, "FPS\0Ping\0Tickrate\0Server IP\0Time\0");
        ImGuiCustom::colorPicker("Color", mCfg.watermark.color.color.data(), &mCfg.watermark.color.color[3], &mCfg.watermark.color.rainbow, &mCfg.watermark.color.rainbowSpeed);
        ImGui::Checkbox("No Title Bar", &mCfg.watermark.noTitleBar);
        ImGui::Checkbox("No Background", &mCfg.watermark.noBackground);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGuiCustom::colorPicker("Accuracy circle", mCfg.accuracyCircle.asColor4(), &mCfg.accuracyCircle.enabled);
    ImGuiCustom::colorPicker("Offscreen Enemies", mCfg.offscreenEnemies.asColor4(), &mCfg.offscreenEnemies.enabled);
    ImGui::SameLine();
    ImGui::PushID("Offscreen Enemies");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Health Bar", &mCfg.offscreenEnemies.healthBar.enabled);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(95.0f);
        ImGui::Combo("Type", &mCfg.offscreenEnemies.healthBar.type, "Gradient\0Solid\0Health-based\0");
        if (mCfg.offscreenEnemies.healthBar.type == HealthBar::Solid) {
            ImGui::SameLine();
            ImGuiCustom::colorPicker("", mCfg.offscreenEnemies.healthBar.asColor4());
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Smoke Timer");
    ImGui::Checkbox("Smoke Timer", &mCfg.smokeTimer.enabled);
    ImGui::SameLine();
    if (ImGui::Button("...##smoke_timer"))
        ImGui::OpenPopup("##popup_smokeTimer");

    if (ImGui::BeginPopup("##popup_smokeTimer"))
    {
        ImGuiCustom::colorPicker("Background color", mCfg.smokeTimer.backgroundColor);
        ImGuiCustom::colorPicker("Text color", mCfg.smokeTimer.textColor);
        ImGuiCustom::colorPicker("Timer color", mCfg.smokeTimer.timerColor, nullptr, &mCfg.smokeTimer.timerThickness);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PushID("Molotov Timer");
    ImGui::Checkbox("Molotov Timer", &mCfg.molotovTimer.enabled);
    ImGui::SameLine();
    if (ImGui::Button("...##molo_timer"))
        ImGui::OpenPopup("##popup_moloTimer");

    if (ImGui::BeginPopup("##popup_moloTimer"))
    {
        ImGuiCustom::colorPicker("Background color", mCfg.molotovTimer.backgroundColor);
        ImGuiCustom::colorPicker("Text color", mCfg.molotovTimer.textColor);
        ImGuiCustom::colorPicker("Timer color", mCfg.molotovTimer.timerColor, nullptr, &mCfg.molotovTimer.timerThickness);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Disable Community Screen Ads", &mCfg.adBlock);
    //ImGui::Checkbox("Simulate Faceit", &mCfg.faceit);
    ImGui::Checkbox("Unlock Hidden Cvars", &mCfg.unlockCvars);
    ImGui::Checkbox("Disable Extrapolation", &mCfg.extrapolate);
    ImGui::Checkbox("Fix Mouse Delta", &mCfg.fixMouseDelta);
    ImGui::Checkbox("Fix animation LOD", &mCfg.fixAnimationLOD);
    ImGui::Checkbox("Fix bone matrix", &mCfg.fixBoneMatrix);
#if defined(_DEBUG)
    ImGui::Checkbox("Fix movement", &mCfg.fixMovement);
#endif
    ImGui::Checkbox("Force Crosshair", &mCfg.forcecrosshair);
    ImGui::Checkbox("Recoil Crosshair", &mCfg.recoilcrosshair);
    ImGui::Checkbox("Disable model occlusion", &mCfg.disableModelOcclusion);
    ImGui::SetNextItemWidth(90.0f);
    ImGui::SliderFloat("Aspect Ratio", &mCfg.aspectratio, 0.0f, 5.0f, "%.2f");
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("as");
    }
    if (ImGui::BeginPopup("as")) {
        if (ImGui::Button("4:3")) mCfg.aspectratio = 1.33f;
        if (ImGui::Button("16:9")) mCfg.aspectratio = 1.77f;
        if (ImGui::Button("16:10"))mCfg.aspectratio = 1.6f;

        ImGui::EndPopup();
    }
    ImGui::PushID("Flash Window");
    ImGui::SetNextItemWidth(90.0f);
    ImGuiCustom::multiCombo("Flash While", mCfg.flash, "Match Accept\0New Round\0");
    ImGui::PopID();
    Log::DrawGUI();
    ImGui::NextColumn();                                            /*NEW COLUMN*/
    ImGui::PushID("adfuse");
    ImGui::Checkbox("Auto Defuse", &mCfg.autoDefuse);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Not recommended with high ping");
    ImGui::PopID();
    ImGui::Checkbox("Fast plant", &mCfg.fastPlant);
    //ImGui::Checkbox("Square Radar", &mCfg.squareRadar);
    ImGuiCustom::colorPicker("Bomb timer", mCfg.bombTimer);
    ImGui::PushID("indicator velocity");
    ImGuiCustom::colorPicker("Velocity Slowdown", mCfg.hurtIndicator.color);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &mCfg.hurtIndicator.noTitleBar);
        ImGui::Checkbox("No Background", &mCfg.hurtIndicator.noBackground);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::SetNextItemWidth(100.f);
    ImGui::Combo("Hit Sound", &mCfg.hitSound, "None\0Metal\0Arena Switch\0Bell\0Glass\0Overwatch\0Custom\0");
    if (mCfg.hitSound == 6) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputText("Hit Sound filename", &mCfg.customHitSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PushID(5);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("Kill Sound", &mCfg.killSound, "None\0Metal\0Arena Switch\0Bell\0Glass\0Overwatch\0Custom\0");
    if (mCfg.killSound == 6) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputText("Kill Sound filename", &mCfg.customKillSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PopID();

    ImGui::SetNextItemWidth(100.f);
    ImGui::InputText("MVP Sound filename", &mCfg.customMVPSound);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    ImGui::PushID("NadePred");
    ImGui::Checkbox("Grenade Prediction", &mCfg.nadePredict.enabled);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::Checkbox("Show Damage", &mCfg.nadePredict.showDmg);
        ImGuiCustom::colorPicker("Trail Color", mCfg.nadePredict.colorLine, nullptr);
        ImGuiCustom::colorPicker("Circle Color", mCfg.nadePredict.colorCircle, nullptr);
        ImGuiCustom::colorPicker("Damage Text Color", mCfg.nadePredict.colorDmg, nullptr);
        ImGui::EndPopup();
    }

    ImGui::PopID();


    ImGui::Checkbox("Autobuy", &mCfg.buyBot.enabled);
    ImGui::SameLine();

    ImGui::PushID("Autobuy");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::Combo("Primary weapon", &mCfg.buyBot.primaryWeapon, "None\0MAC-10 | MP9\0MP7 | MP5-SD\0UMP-45\0P90\0PP-Bizon\0Galil AR | FAMAS\0AK-47 | M4A4 | M4A1-S\0SSG 08\0SG553 |AUG\0AWP\0G3SG1 | SCAR-20\0Nova\0XM1014\0Sawed-Off | MAG-7\0M249\0Negev\0");
        ImGui::Combo("Secondary weapon", &mCfg.buyBot.secondaryWeapon, "None\0Glock-18 | P2000 | USP-S\0Dual Berettas\0P250\0CZ75-Auto | Five-SeveN | Tec-9\0Desert Eagle | R8 Revolver\0");
        ImGui::Combo("Armor", &mCfg.buyBot.armor, "None\0Kevlar\0Kevlar + Helmet\0");

        static bool utilities[2]{ false, false };
        static const char* utility[]{ "Defuser","Taser" };
        static std::string previewvalueutility = "";
        for (size_t i = 0; i < ARRAYSIZE(utilities); i++)
        {
            utilities[i] = (mCfg.buyBot.utility & 1 << i) == 1 << i;
        }
        if (ImGui::BeginCombo("Utility", previewvalueutility.c_str()))
        {
            previewvalueutility = "";
            for (size_t i = 0; i < ARRAYSIZE(utilities); i++)
            {
                ImGui::Selectable(utility[i], &utilities[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
            }
            ImGui::EndCombo();
        }
        for (size_t i = 0; i < ARRAYSIZE(utilities); i++)
        {
            if (i == 0)
                previewvalueutility = "";

            if (utilities[i])
            {
                previewvalueutility += previewvalueutility.size() ? std::string(", ") + utility[i] : utility[i];
                mCfg.buyBot.utility |= 1 << i;
            }
            else
            {
                mCfg.buyBot.utility &= ~(1 << i);
            }
        }

        static bool nading[5]{ false, false, false, false, false };
        static const char* nades[]{ "HE Grenade","Smoke Grenade","Molotov","Flashbang","Decoy" };
        static std::string previewvaluenades = "";
        for (size_t i = 0; i < ARRAYSIZE(nading); i++)
        {
            nading[i] = (mCfg.buyBot.grenades & 1 << i) == 1 << i;
        }
        if (ImGui::BeginCombo("Nades", previewvaluenades.c_str()))
        {
            previewvaluenades = "";
            for (size_t i = 0; i < ARRAYSIZE(nading); i++)
            {
                ImGui::Selectable(nades[i], &nading[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
            }
            ImGui::EndCombo();
        }
        for (size_t i = 0; i < ARRAYSIZE(nading); i++)
        {
            if (i == 0)
                previewvaluenades = "";

            if (nading[i])
            {
                previewvaluenades += previewvaluenades.size() ? std::string(", ") + nades[i] : nades[i];
                mCfg.buyBot.grenades |= 1 << i;
            }
            else
            {
                mCfg.buyBot.grenades &= ~(1 << i);
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Freecam");
    ImGui::Checkbox("Freecam", &mCfg.freeCam.enabled);
    ImGui::SameLine();
    if(ImGui::Button("..."))
        ImGui::OpenPopup("freecampop");

    if (ImGui::BeginPopup("freecampop")) {
        ImGui::PushItemWidth(100.0f);
        ImGui::hotkey("Key", mCfg.freeCam.Key);
        ImGui::SameLine();
        ImGui::Combo("##ss", &mCfg.freeCam.keyMode, "Hold\0Toggle\0");
        ImGui::SliderInt("##speed", &mCfg.freeCam.Speed, 1, 10, "Freecam speed: %d");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Footstep");
    ImGuiCustom::colorPicker("Footsteps", mCfg.footsteps.footstepBeams);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");
    if (ImGui::BeginPopup("")) {
        ImGui::SliderFloat("Thickness", &mCfg.footsteps.footstepBeamThickness, 0.01f, 30.00f, "Thickness: %.2f");
        ImGui::SliderFloat("Radius", &mCfg.footsteps.footstepBeamRadius, 0.01f, 230.00f, "Radius: %.2f");
        ImGui::Combo("Texture", &mCfg.footsteps.textureID, ringTextures, ARRAYSIZE(ringTextures));
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Grenade Animation Cancel", &mCfg.nadeAnimationCancel);
    ImGui::Checkbox("Fix tablet signal", &mCfg.fixTabletSignal);
    ImGui::Checkbox("Out Of Ammo Notification", &mCfg.outofammo);
    ImGui::Checkbox("Bypass sv pure", &mCfg.bypassSvPure);
    ImGui::Checkbox("Opposite Hand Knife", &mCfg.oppositeHandKnife);
    ImGui::PushID("Killfeed changer");
    ImGui::Checkbox("Killfeed Changer", &mCfg.killfeedChanger.enabled);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");
    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::Checkbox("Preserve Killfeed", &mCfg.killfeedChanger.preserve);
        ImGui::Checkbox("Only Headshots", &mCfg.killfeedChanger.preserveOnlyHeadshots);
        ImGui::Separator();
        ImGui::InputText("Icon", &mCfg.killfeedChanger.icon);

        ImGui::Checkbox("Headshot", &mCfg.killfeedChanger.headshot);
        ImGui::Checkbox("Dominated", &mCfg.killfeedChanger.dominated);
        ImGui::SameLine();
        ImGui::Checkbox("Revenge", &mCfg.killfeedChanger.revenge);

        ImGui::Checkbox("Penetrated", &mCfg.killfeedChanger.penetrated);
        ImGui::Checkbox("Noscope", &mCfg.killfeedChanger.noscope);
        ImGui::Checkbox("Thrusmoke", &mCfg.killfeedChanger.thrusmoke);
        ImGui::Checkbox("Attackerblind", &mCfg.killfeedChanger.attackerblind);
        ImGui::Checkbox("Squad Wipeout", &mCfg.killfeedChanger.wipe);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Purchase List", &mCfg.purchaseList.enabled);
    ImGui::SameLine();

    ImGui::PushID("Purchase List");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(75.0f);
        ImGui::Combo("Mode", &mCfg.purchaseList.mode, "Details\0Summary\0");
        ImGui::Checkbox("Only During Freeze Time", &mCfg.purchaseList.onlyDuringFreezeTime);
        ImGui::Checkbox("Show Prices", &mCfg.purchaseList.showPrices);
        ImGui::Checkbox("No Title Bar", &mCfg.purchaseList.noTitleBar);
        ImGui::Checkbox("Print", &mCfg.purchaseList.print);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("plist");
    ImGui::Checkbox("Player List", &mCfg.playerList.enabled);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Hide In Game", &mCfg.playerList.hideInGame);
        ImGui::Checkbox("No Title Bar", &mCfg.playerList.noTitleBar);
        ImGui::hotkey("", mCfg.playerList.plkeybind);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Reportbot", &mCfg.reportbot.enabled);
    ImGui::SameLine();
    ImGui::PushID("Reportbot");

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::PushItemWidth(80.0f);
        ImGui::Combo("Target", &mCfg.reportbot.target, "Enemies\0Allies\0All\0");
        ImGui::InputInt("Delay (s)", &mCfg.reportbot.delay);
        mCfg.reportbot.delay = (std::max)(mCfg.reportbot.delay, 1);
        ImGui::InputInt("Rounds", &mCfg.reportbot.rounds);
        mCfg.reportbot.rounds = (std::max)(mCfg.reportbot.rounds, 1);
        ImGui::PopItemWidth();
        ImGui::Checkbox("Abusive Communications", &mCfg.reportbot.textAbuse);
        ImGui::Checkbox("Griefing", &mCfg.reportbot.griefing);
        ImGui::Checkbox("Wall Hacking", &mCfg.reportbot.wallhack);
        ImGui::Checkbox("Aim Hacking", &mCfg.reportbot.aimbot);
        ImGui::Checkbox("Other Hacking", &mCfg.reportbot.other);
        if (ImGui::Button("Reset"))
            Misc::resetReportbot();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PushID("indi");
    ImGui::Checkbox("Indicators", &mCfg.indicators.enabled);
	ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &mCfg.indicators.noTitleBar);
        ImGui::Checkbox("No Background", &mCfg.indicators.noBackground);
        ImGui::SetNextItemWidth(80.0f);
        ImGuiCustom::multiCombo("Flags", mCfg.indicators.bytewise, "Chocked Packets\0Height\0LBY\0Roll\0Paused Ticks\0Velocity\0Grenade Throw Strength\0");
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Auto Disconnect", &mCfg.autodisconnect);

    ImGui::PushID("relay");
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("Force region", &mCfg.relayCluster, "Off\0Australia\0Austria\0Brazil\0Chile\0Dubai\0France\0Germany\0Hong Kong\0India (Chennai)\0India (Mumbai)\0Japan\0Luxembourg\0Netherlands\0Peru\0Philipines\0Poland\0Singapore\0South Africa\0Spain\0Sweden\0UK\0USA (Atlanta)\0USA (Seattle)\0USA (Chicago)\0USA (Los Angeles)\0USA (Moses Lake)\0USA (Oklahoma)\0USA (Seattle)\0USA (Washington DC)\0");
    ImGui::PopID();

    ImGui::PushID("PrefixColor");
    ImGui::SetNextItemWidth(80.0f);
    ImGui::Combo("Cheat Message Prefix Color", &config->style.prefixColor, "White\0Red\0Pale Purple\0Green\0Pale green\0Money green\0Light Red\0Gray\0Yellow\0Pale Blue Gray\0Turquoise\0Blue\0Purple\0Pink\0Light Red 2\0Gold\0");
    ImGui::PopID();

    Clan::drawGUI();

    ImGui::Checkbox("Kill message", &mCfg.killMessage);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::PushID(1);
    ImGui::InputText("", &mCfg.killMessageString);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("#p - Name of Killed Player, #l - Location where He got Killed");
    ImGui::PopID();

    ImGui::Checkbox("Dead message", &mCfg.deadMessage);
    ImGui::SameLine();
    ImGui::PushID(2);
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("dead");
    }

    if (ImGui::BeginPopup("dead")) {
        ImGui::Checkbox("Team Chat", &mCfg.deadMessageTeam);
        ImGui::SetNextItemWidth(120.0f);
        ImGui::InputText("", &mCfg.deadMessageString);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("#p - Name of Killed Player, #l - Location where He got Killed");
        ImGui::EndPopup();
    }

    ImGui::PopID();

    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

static void from_json(const json& j, ImVec2& v)
{
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, MiscConfig::SmokeTimer& s)
{
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Background color", s.backgroundColor);
    read<value_t::object>(j, "Timer color", s.timerColor);
    read(j, "Timer thickness", s.timerThickness);
    read<value_t::object>(j, "Text color", s.textColor);
}

static void from_json(const json& j, MiscConfig::MolotovTimer& s)
{
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Background color", s.backgroundColor);
    read<value_t::object>(j, "Timer color", s.timerColor);
    read(j, "Timer thickness", s.timerThickness);
    read<value_t::object>(j, "Text color", s.textColor);
}

static void from_json(const json& j, MiscConfig::PurchaseList& pl)
{
    read(j, "Enabled", pl.enabled);
    read(j, "Only During Freeze Time", pl.onlyDuringFreezeTime);
    read(j, "Show Prices", pl.showPrices);
    read(j, "No Title Bar", pl.noTitleBar);
    read(j, "Mode", pl.mode);
    read<value_t::object>(j, "Pos", pl.pos);
}

static void from_json(const json& j, OffscreenEnemies& o)
{
    from_json(j, static_cast<ColorToggle&>(o));

    read<value_t::object>(j, "Health Bar", o.healthBar);
}

static void from_json(const json& j, MiscConfig::FootstepESP& ft)
{
    read<value_t::object>(j, "Enabled", ft.footstepBeams);
    read(j, "Thickness", ft.footstepBeamThickness);
    read(j, "Radius", ft.footstepBeamRadius);
    read(j, "Texture", ft.textureID);
}

static void from_json(const json& j, MiscConfig::Binds& b)
{
    read(j, "Enabled", b.enabled);
    read(j, "No Title Bar", b.noTitleBar);
    read(j, "No Background", b.noBackground);
    read<value_t::object>(j, "Pos", b.pos);
}

static void from_json(const json& j, MiscConfig::SpectatorList& sl)
{
    read(j, "Enabled", sl.enabled);
    read(j, "No Title Bar", sl.noTitleBar);
    read(j, "No Background", sl.noBackground);
    read<value_t::object>(j, "Pos", sl.pos);
}

static void from_json(const json& j, MiscConfig::Indicators& in)
{
    read(j, "Enabled", in.enabled);
    read(j, "No Title Bar", in.noTitleBar);
    read(j, "No Background", in.noBackground);
    read(j, "Flags", in.bytewise);
    read<value_t::object>(j, "Pos", in.pos);
}

static void from_json(const json& j, MiscConfig::BuyBot& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Primary weapon", o.primaryWeapon);
    read(j, "Secondary weapon", o.secondaryWeapon);
    read(j, "Armor", o.armor);
    read(j, "Utility", o.utility);
    read(j, "Grenades", o.grenades);
}

static void from_json(const json& j, MiscConfig::HurtIndicator& o)
{
    read<value_t::object>(j, "Color", o.color);
    read(j, "No Title Bar", o.noTitleBar);
    read(j, "No Background", o.noBackground);
    read<value_t::object>(j, "Pos", o.pos);
}

static void from_json(const json& j, MiscConfig::FreeCam& fc){
    read(j, "Enabled", fc.enabled);
    read(j, "Key", fc.Key);
    read(j, "Key Mode", fc.keyMode);
    read(j, "Speed", fc.Speed);
}

static void from_json(const json& j, MiscConfig::NadePredict& p) {
    read(j, "Enabled", p.enabled);
    read(j, "Show Damage", p.showDmg);
    read<value_t::object>(j, "Circle Color", p.colorCircle);
    read<value_t::object>(j, "Line Color", p.colorLine);
    read<value_t::object>(j, "Damage Color", p.colorDmg);
}

static void from_json(const json& j, MiscConfig::Watermark& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "No Title Bar", o.noTitleBar);
    read(j, "No Background", o.noBackground);
    read<value_t::object>(j, "Pos", o.pos);
    read<value_t::object>(j, "Color", o.color);
}

static void from_json(const json& j, MiscConfig::PlayerList& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "No Title Bar", o.noTitleBar);
    read(j, "Hide In Game", o.hideInGame);
    read(j, "Mode", o.mode);
    read(j, "Show Key", o.plkeybind);
    read<value_t::object>(j, "Pos", o.pos);
}

static void from_json(const json& j, MiscConfig::KillfeedChanger& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Preserve", o.preserve);
    read(j, "Preserve Only Headshots", o.preserveOnlyHeadshots);
    read(j, "Headshot", o.headshot);
    read(j, "Dominated", o.dominated);
    read(j, "Revenge", o.revenge);
    read(j, "Penetrated", o.penetrated);
    read(j, "Noscope", o.noscope);
    read(j, "Thrusmoke", o.thrusmoke);
    read(j, "Attackerblind", o.attackerblind);
    read(j, "Wipe", o.wipe);
}

static void from_json(const json& j, MiscConfig::ColorPalette& c)
{
    read(j, "Enabled", c.enabled);
    read<value_t::object>(j, "Color 1", c.c1);
    read<value_t::object>(j, "Color 2", c.c2);
    read<value_t::object>(j, "Color 3", c.c3);
    read<value_t::object>(j, "Color 4", c.c4);
    read<value_t::object>(j, "Color 5", c.c5);
    read<value_t::object>(j, "Color 6", c.c6);
    read<value_t::object>(j, "Color 7", c.c7);
    read<value_t::object>(j, "Color 8", c.c8);
}

static void from_json(const json& j, MiscConfig& m)
{
    read(j, "Menu key", m.menuKey);
    read(j, "Anti AFK kick", m.antiAfkKick);
    read(j, "Slidewalk", m.moonwalk);
    read(j, "Break Slidewalk Animation Speed", m.animFuck);
    read(j, "Unlock Inventory", m.unlockInventory);
    read(j, "Auto pistol", m.autoPistol);
    read(j, "Auto reload", m.autoReload);
    read(j, "Auto accept", m.autoAccept);
    read(j, "Reveal", m.reveal);
    read<value_t::object>(j, "Spectator list", m.spectatorList);
    read<value_t::object>(j, "Watermark", m.watermark);
    read<value_t::object>(j, "Accuracy Circle", m.accuracyCircle);
    read<value_t::object>(j, "Offscreen Enemies", m.offscreenEnemies);
    read(j, "Fix Mouse Delta", m.fixMouseDelta);
    read(j, "Fix animation LOD", m.fixAnimationLOD);
    read(j, "Fix bone matrix", m.fixBoneMatrix);
    read(j, "Fix movement", m.fixMovement);
    read(j, "Disable model occlusion", m.disableModelOcclusion);
    read(j, "Aspect Ratio", m.aspectratio);
    read(j, "Kill message", m.killMessage);
    read<value_t::string>(j, "Kill message string", m.killMessageString);
    read(j, "Auto Defuse", m.autoDefuse);
    read(j, "Fast plant", m.fastPlant);
    read<value_t::object>(j, "Bomb timer", m.bombTimer);
    read<value_t::object>(j, "Hurt indicator", m.hurtIndicator);
    read(j, "Hit sound", m.hitSound);
    read<value_t::object>(j, "Grenade predict", m.nadePredict);
    read(j, "Fix tablet signal", m.fixTabletSignal);
    read(j, "Bypass sv pure", m.bypassSvPure);
    read<value_t::string>(j, "Custom Hit Sound", m.customHitSound);
    read(j, "Kill sound", m.killSound);
    read<value_t::string>(j, "Custom Kill Sound", m.customKillSound);
    read<value_t::string>(j, "Custom MVP Sound", m.customMVPSound);
    read<value_t::object>(j, "Purchase List", m.purchaseList);
    read<value_t::object>(j, "Reportbot", m.reportbot);
    read(j, "Opposite Hand Knife", m.oppositeHandKnife);
    read<value_t::object>(j, "Keybinds", m.binds);
    read<value_t::object>(j, "Indicators", m.indicators);
    read<value_t::object>(j, "Player List", m.playerList);
    read<value_t::object>(j, "BuyBot", m.buyBot);
    read(j, "Force Crosshair", m.forcecrosshair);
    read(j, "Recoil Crosshair", m.recoilcrosshair);
    read(j, "Auto Disconnect", m.autodisconnect);
    read(j, "Ad Block", m.adBlock);
    read(j, "Out of Ammo Notification", m.outofammo);
    read(j, "Grenade Animation Cancel", m.nadeAnimationCancel);
    read(j, "Anti Tkill", m.antiTkill);
    read(j, "Flash", m.flash);
    read<value_t::object>(j, "Freecam",  m.freeCam);
    read<value_t::object>(j, "Killfeed changer", m.killfeedChanger);
    read<value_t::object>(j, "Smoke timer", m.smokeTimer);
    read<value_t::object>(j, "Molotov timer", m.molotovTimer);
    read(j, "Unlock Hidden Convars", m.unlockCvars);
    read(j, "Disable Extrapolation", m.extrapolate);
    read<value_t::object>(j, "Footsteps", m.footsteps);
    read<value_t::object>(j, "Color Palette", m.colorPalette);
}

static void from_json(const json& j, MiscConfig::Reportbot& r)
{
    read(j, "Enabled", r.enabled);
    read(j, "Target", r.target);
    read(j, "Delay", r.delay);
    read(j, "Rounds", r.rounds);
    read(j, "Abusive Communications", r.textAbuse);
    read(j, "Griefing", r.griefing);
    read(j, "Wall Hacking", r.wallhack);
    read(j, "Aim Hacking", r.aimbot);
    read(j, "Other Hacking", r.other);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}


static void to_json(json& j, const MiscConfig::SmokeTimer& o, const MiscConfig::SmokeTimer& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Background color", backgroundColor);
    WRITE("Timer color", timerColor);
    WRITE("Timer thickness", timerThickness);
    WRITE("Text color", textColor);
}

static void to_json(json& j, const MiscConfig::MolotovTimer& o, const MiscConfig::MolotovTimer& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Background color", backgroundColor);
    WRITE("Timer color", timerColor);
    WRITE("Timer thickness", timerThickness);
    WRITE("Text color", textColor);
}

static void to_json(json& j, const MiscConfig::FreeCam& o, const MiscConfig::FreeCam& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Key", Key);
    WRITE("Key Mode", keyMode);
    WRITE("Speed", Speed);
}

static void to_json(json& j, const MiscConfig::NadePredict& o, const MiscConfig::NadePredict& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Show Damage", showDmg);
    WRITE("Circle Color", colorCircle);
    WRITE("Line Color", colorLine);
    WRITE("Damage Color", colorDmg);
}

static void to_json(json& j, const MiscConfig::Binds& o, const MiscConfig::Binds& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    if (const auto window = ImGui::FindWindowByName("Keybinds")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::Reportbot& o, const MiscConfig::Reportbot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Target", target);
    WRITE("Delay", delay);
    WRITE("Rounds", rounds);
    WRITE("Abusive Communications", textAbuse);
    WRITE("Griefing", griefing);
    WRITE("Wall Hacking", wallhack);
    WRITE("Aim Hacking", aimbot);
    WRITE("Other Hacking", other);
}

static void to_json(json& j, const MiscConfig::PurchaseList& o, const MiscConfig::PurchaseList& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Only During Freeze Time", onlyDuringFreezeTime);
    WRITE("Show Prices", showPrices);
    WRITE("No Title Bar", noTitleBar);
    WRITE("Mode", mode);

    if (const auto window = ImGui::FindWindowByName("Purchases")) {
        j["Pos"] = window->Pos;
    }
}
static void to_json(json& j, const OffscreenEnemies& o, const OffscreenEnemies& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);

    WRITE("Health Bar", healthBar);
}

static void to_json(json& j, const MiscConfig::FootstepESP& o, const MiscConfig::FootstepESP& dummy)
{
    WRITE("Enabled", footstepBeams);
    WRITE("Thickness", footstepBeamThickness);
    WRITE("Radius", footstepBeamRadius);
    WRITE("Texture", textureID);
}

static void to_json(json& j, const MiscConfig::SpectatorList& o, const MiscConfig::SpectatorList& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    if (const auto window = ImGui::FindWindowByName("Spectator list")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::Watermark& o, const MiscConfig::Watermark& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    WRITE("Color", color);
    if (const auto window = ImGui::FindWindowByName("Azurre")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::BuyBot& o, const MiscConfig::BuyBot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Primary weapon", primaryWeapon);
    WRITE("Secondary weapon", secondaryWeapon);
    WRITE("Armor", armor);
    WRITE("Utility", utility);
    WRITE("Grenades", grenades);
}

static void to_json(json& j, const MiscConfig::HurtIndicator& o, const MiscConfig::HurtIndicator& dummy = {})
{
    WRITE("Color", color);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    if (const auto window = ImGui::FindWindowByName("Hurt Indicator")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::PlayerList& o, const MiscConfig::PlayerList& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("Hide In Game", hideInGame);
    WRITE("Mode", mode);
    WRITE("Show Key", plkeybind);

    if (const auto window = ImGui::FindWindowByName("Player List")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::Indicators& o, const MiscConfig::Indicators& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    WRITE("Flags", bytewise);
    if (const auto window = ImGui::FindWindowByName("Indicators")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MiscConfig::KillfeedChanger& o, const MiscConfig::KillfeedChanger& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Preserve Only Headshots", preserve);
    WRITE("Preserve Only Headshots", preserveOnlyHeadshots);
    WRITE("Headshot", headshot);
    WRITE("Dominated", dominated);
    WRITE("Revenge", revenge);
    WRITE("Penetrated", penetrated);
    WRITE("Noscope", noscope);
    WRITE("Thrusmoke", thrusmoke);
    WRITE("Attackerblind", attackerblind);
    WRITE("Wipe", wipe);
}

static void to_json(json& j, const MiscConfig::ColorPalette& o, const MiscConfig::ColorPalette& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Color 1", c1);
    WRITE("Color 2", c2);
    WRITE("Color 3", c3);
    WRITE("Color 4", c4);
    WRITE("Color 5", c5);
    WRITE("Color 6", c6);
    WRITE("Color 7", c7);
    WRITE("Color 8", c8);
}

static void to_json(json& j, const MiscConfig& o)
{
    const MiscConfig dummy;

    WRITE("Menu key", menuKey);
    WRITE("Anti AFK kick", antiAfkKick);
    WRITE("Slidewalk", moonwalk);
    WRITE("Break Slidewalk Animation Speed", animFuck);
    WRITE("Unlock Inventory", unlockInventory);
    WRITE("Auto pistol", autoPistol);
    WRITE("Auto reload", autoReload);
    WRITE("Auto accept", autoAccept);
    WRITE("Reveal", reveal);
    WRITE("Spectator list", spectatorList);
    WRITE("Watermark", watermark);
    WRITE("Accuracy Circle", accuracyCircle);
    WRITE("Offscreen Enemies", offscreenEnemies);
    WRITE("Fix Mouse Delta", fixMouseDelta);
    WRITE("Fix animation LOD", fixAnimationLOD);
    WRITE("Fix bone matrix", fixBoneMatrix);
    WRITE("Fix movement", fixMovement);
    WRITE("Disable model occlusion", disableModelOcclusion);
    WRITE("Aspect Ratio", aspectratio);
    WRITE("Kill message", killMessage);
    WRITE("Kill message string", killMessageString);
    WRITE("Auto Defuse", autoDefuse);
    WRITE("Fast plant", fastPlant);
    WRITE("Bomb timer", bombTimer);
    WRITE("Hurt indicator", hurtIndicator);
    WRITE("Hit sound", hitSound);
    WRITE("Grenade predict", nadePredict);
    WRITE("Fix tablet signal", fixTabletSignal);
    WRITE("Bypass sv pure", bypassSvPure);
    WRITE("Custom Hit Sound", customHitSound);
    WRITE("Kill sound", killSound);
    WRITE("Custom Kill Sound", customKillSound);
    WRITE("Custom MVP Sound", customMVPSound);
    WRITE("Purchase List", purchaseList);
    WRITE("Reportbot", reportbot);
    WRITE("Opposite Hand Knife", oppositeHandKnife);
    WRITE("Auto Disconnect", autodisconnect);
    WRITE("Force Crosshair", forcecrosshair);
    WRITE("Recoil Crosshair", recoilcrosshair);
    WRITE("Indicators", indicators);
    WRITE("Keybinds", binds);
    WRITE("BuyBot", buyBot);
    WRITE("Player List", playerList);
    WRITE("Ad Block", adBlock);
    WRITE("Out of Ammo Notification", outofammo);
    WRITE("Grenade Animation Cancel", nadeAnimationCancel);
    WRITE("Anti Tkill", antiTkill);
    WRITE("Flash", flash);
    WRITE("Freecam", freeCam);
    WRITE("Killfeed changer", killfeedChanger);
    WRITE("Smoke timer", smokeTimer);
    WRITE("Molotov timer", molotovTimer);
    WRITE("Unlock Hidden Convars", unlockCvars);
    WRITE("Disable Extrapolation", extrapolate);
    WRITE("Footsteps", footsteps);
    WRITE("Color Palette", colorPalette);
}

json Misc::toJson() noexcept
{
    json j;
    to_json(j, mCfg);
    return j;
}

void Misc::fromJson(const json& j) noexcept
{
    from_json(j, mCfg);
}

void Misc::resetConfig() noexcept
{
    mCfg = {};
}
