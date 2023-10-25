#include "Extra.h"
#include "Misc.h"

#include <array>
#include <algorithm>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../imguiCustom.h"

#include "../Config.h"
#include "../ConfigStructs.h"
#include "../GUI.h"
#include "../Interfaces.h"
#include "../Memory.h"

#include "../SDK/ClientMode.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/Gamemode.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/UserCmd.h"

struct ExtraConfig {
    KeyBind fakeWalkKey;
    KeyBind fakeDuckKey;
    KeyBind prepareRevolverKey;

    struct AutoPeek{
        ColorToggle enabled;
        KeyBind autoPeekKey;
    } autoPeek;

    float maxAngleDelta{ 255.f };
    bool antiUntrust = true, prepareRevolver = false;

    struct SlowWalk {
        bool enabled = false;
        float slowspeed = 135.f;
        int slowwalkMode = 0;
        KeyBind slowwalkKey;
    };
    SlowWalk slowWalk;

    struct AirStuck {
        KeyBind airStuckKey;
        int mode = 0;
    }airstuck;

    struct Tickbase {
        KeyBind doubletap;
        KeyBind hideshots;
        KeyBind forceTeleport;

        bool teleport{ false };
    }tickBase;

    struct updateInputs {
        int doubleTap{ 0 };
        int hideShots{ 0 };
        int autoPeek{ 0 };
        int fakeDuck{ 0 };
        int fakeWalk{ 0 };
        int airStuck{ 0 };
        int slowWalk{ 0 };
    }keyMode;

    struct KnifeBot
    {
        bool enabled = false;
        int mode = 0;
    } knifeBot;

}eCfg;


static bool doubleTapHandle = false;
static bool hideShotsHandle = false;
static bool autoPeekHandle = false;
static bool fakeDuckHandle = false;
static bool fakeWalkHandle = false;
static bool airStuckHandle = false;
static bool slowWalkHandle = false;


void Extra::updateInput() noexcept
{
    if (!eCfg.keyMode.doubleTap)
        doubleTapHandle = eCfg.tickBase.doubletap.isDown();
    else if (eCfg.keyMode.doubleTap && eCfg.tickBase.doubletap.isPressed())
        doubleTapHandle = !doubleTapHandle;

    if (!eCfg.keyMode.hideShots)
        hideShotsHandle = eCfg.tickBase.hideshots.isDown();
    else if (eCfg.keyMode.hideShots && eCfg.tickBase.hideshots.isPressed())
        hideShotsHandle = !hideShotsHandle;

    if (!eCfg.keyMode.fakeDuck)
        fakeDuckHandle = eCfg.fakeDuckKey.isDown();
    else if (eCfg.keyMode.fakeDuck && eCfg.fakeDuckKey.isPressed())
        fakeDuckHandle = !fakeDuckHandle;

    if (!eCfg.keyMode.fakeWalk)
        fakeWalkHandle = eCfg.fakeWalkKey.isDown();
    else if (eCfg.keyMode.fakeWalk && eCfg.fakeWalkKey.isPressed())
        fakeWalkHandle = !fakeWalkHandle;

    if (!eCfg.keyMode.autoPeek)
        autoPeekHandle = eCfg.autoPeek.autoPeekKey.isDown();
    else if (eCfg.keyMode.autoPeek && eCfg.autoPeek.autoPeekKey.isPressed())
        autoPeekHandle = !autoPeekHandle;

    if (!eCfg.keyMode.airStuck)
        airStuckHandle = eCfg.airstuck.airStuckKey.isDown();
    else if (eCfg.keyMode.airStuck && eCfg.airstuck.airStuckKey.isPressed())
        airStuckHandle = !airStuckHandle;

    if (!eCfg.keyMode.slowWalk)
        slowWalkHandle = eCfg.slowWalk.slowwalkKey.isDown();
    else if (eCfg.keyMode.slowWalk && eCfg.slowWalk.slowwalkKey.isPressed())
        slowWalkHandle = !slowWalkHandle;
}

bool Extra::isDTEnabled() noexcept {
    if (!eCfg.tickBase.doubletap.isSet()) return false;
    return doubleTapHandle;
}

bool Extra::isHideShotsEnabled() noexcept {
    if (!eCfg.tickBase.hideshots.isSet()) return false;
    return hideShotsHandle;
}

bool Extra::isFakeDuckKeyPressed() noexcept
{
    if (!eCfg.fakeDuckKey.isSet()) return false;
    return fakeDuckHandle;
}

bool Extra::isFakeWalkKeyPressed() noexcept
{
    if (!eCfg.fakeWalkKey.isSet()) return false;
    return fakeWalkHandle;
}

bool Extra::isAirStuckKeyDown() noexcept
{
    if (eCfg.airstuck.airStuckKey.isSet()) return false;
    return airStuckHandle;
}

bool Extra::isQuickPeekKeyDown() noexcept
{
    if (!eCfg.autoPeek.autoPeekKey.isSet() || !eCfg.autoPeek.enabled.enabled) return false;
    return autoPeekHandle;
}

bool Extra::isSlowWalkKeyDown() noexcept
{
    if (!eCfg.slowWalk.enabled) return false;
    return slowWalkHandle;
}

int Extra::knifeBotMode() noexcept {
    return eCfg.knifeBot.mode;
}
bool Extra::knifeBotEnabled() noexcept {
    return eCfg.knifeBot.enabled;
}

bool Extra::forceTeleportKey() noexcept {
    return eCfg.tickBase.forceTeleport.isDown();
}

bool Extra::isTeleportEnabled() noexcept {
    return eCfg.tickBase.teleport;
}

float Extra::getWeaponAccuracySpeed() noexcept {

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return 0.f;

    if (activeWeapon->isKnife()) return 0.f;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return 0.f;

    const float maxSpeed = eCfg.slowWalk.slowwalkMode == 0 ? ((localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3) : eCfg.slowWalk.slowspeed;
    
    return maxSpeed;
}

static Vector autoPeekStartPos;

void Extra::quickPeek(UserCmd* cmd) noexcept
{
    static bool hasShot = false;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER || !localPlayer->isOnGround())
        return;

    if (!autoPeekHandle)
    {
        hasShot = false;
        autoPeekStartPos = Vector{};
        return;
    }

    if (!autoPeekStartPos.notNull())
        autoPeekStartPos = localPlayer->getAbsOrigin();

    if (cmd->buttons & UserCmd::IN_ATTACK) {
        hasShot = true;
    }

    if (hasShot)
    {
        const float yaw = cmd->viewangles.y;
        const auto delta = autoPeekStartPos - localPlayer->getAbsOrigin();

        if (delta.length2D() > 5.0f)
        {
            Vector fwd = Vector::fromAngle2D(cmd->viewangles.y);
            Vector side = fwd.crossProduct(Vector::up());
            Vector move = Vector{ fwd.dotProduct2D(delta), side.dotProduct2D(delta), 0.0f };
            move *= 45.0f;

            const float l = move.length2D();
            if (l > 450.0f)
                move *= 450.0f / l;

            cmd->forwardmove = move.x;
            cmd->sidemove = move.y;
        }
        else
        {
            hasShot = false;
            autoPeekStartPos = Vector{};
        }
    }
}

void Extra::visualizeQuickPeek(ImDrawList* drawList) noexcept
{
    if (!autoPeekHandle)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (autoPeekStartPos.notNull())
    {
        static const auto circumference = []
        {
            std::array<Vector, 32> points;
            for (std::size_t i = 0; i < points.size(); ++i)
            {
                constexpr auto radius = 25.0f;
                points[i] = Vector{ radius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
                    radius * std::sin(Helpers::deg2rad(i * (360.0f / points.size()))),
                    0.0f };
            }
            return points;
        }();

        std::array<ImVec2, circumference.size()> screenPoints;
        std::size_t count = 0;

        for (const auto& point : circumference)
        {
            if (Helpers::worldToScreen(autoPeekStartPos + point, screenPoints[count]))
                ++count;
        }

        if (count < 1)
            return;

        std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

        constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c)
        {
            return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
        };
        std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });

        const auto color = Helpers::calculateColor(eCfg.autoPeek.enabled.asColor4());

        drawList->AddConvexPolyFilled(screenPoints.data(), count, color);
    }
}

void Extra::airStuck(UserCmd* cmd) noexcept{
    if (!eCfg.airstuck.airStuckKey.isDown()) return;
    if (!interfaces->engine->isInGame()) return;
    if (!localPlayer->isAlive()) return;

    const auto gameRules = (*memory->gameRules);

    if (gameRules->isValveDS()) return;

    switch (eCfg.airstuck.mode) {
    default:
    case 0: 
        cmd->commandNumber = INT_MAX;
        cmd->tickCount = INT_MAX;
        break;

    case 1: 
        cmd->tickCount = 16777216;
        break;
    }

}

float Extra::maxAngleDelta() noexcept
{
    return eCfg.maxAngleDelta;
}

void Extra::slowWalk(UserCmd* cmd) noexcept
{
    if (!eCfg.slowWalk.enabled || !slowWalkHandle)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    const float maxSpeed = eCfg.slowWalk.slowwalkMode == 0 ? ((localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3) : eCfg.slowWalk.slowspeed;

    if (cmd->forwardmove && cmd->sidemove) {
        const float maxSpeedRoot = maxSpeed * static_cast<float>(M_SQRT1_2);
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
    }
    else if (cmd->forwardmove) {
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeed : maxSpeed;
    }
    else if (cmd->sidemove) {
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeed : maxSpeed;
    }
}

bool Extra::AntiUntrusted() noexcept {
    return eCfg.antiUntrust;
}

void Extra::prepareRevolver(UserCmd* cmd) noexcept
{
    constexpr auto timeToTicks = [](float time) {  return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick); };
    constexpr float revolverPrepareTime{ 0.234375f };
    //constexpr float revolverPrepareTime{ 0.242f };

    static float readyTime;
    if (!eCfg.prepareRevolver)
        return;

    if (!eCfg.prepareRevolverKey.isDown() && eCfg.prepareRevolverKey.isSet())
        return;

    if (!localPlayer)
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (activeWeapon && activeWeapon->itemDefinitionIndex() == WeaponId::Revolver) {
        if (!readyTime) readyTime = memory->globalVars->serverTime() + revolverPrepareTime;
        auto ticksToReady = timeToTicks(readyTime - memory->globalVars->serverTime() - interfaces->engine->getNetworkChannel()->getLatency(0));
        if (ticksToReady > 0 && ticksToReady <= timeToTicks(revolverPrepareTime))
            cmd->buttons |= UserCmd::IN_ATTACK;
        else
            readyTime = 0.0f;
    }
}

void Extra::fakeWalk(UserCmd* cmd, bool &sendPacket) noexcept
{
    if (!localPlayer || !localPlayer->isAlive() || !(localPlayer->flags() & 1))
        return;

    if (!fakeWalkHandle) return;

    static int choked = 0;
    choked = choked > 7 ? 0 : choked + 1;
    cmd->forwardmove = choked < 2 || choked > 5 ? 0 : cmd->forwardmove;
    cmd->sidemove = choked < 2 || choked > 5 ? 0 : cmd->sidemove;
    sendPacket = choked < 1;
}

void Extra::fakeDuck(UserCmd* cmd, bool& sendPacket) noexcept
{
    if (!fakeDuckHandle)
        return;

    if (!localPlayer || !localPlayer->isAlive() || !localPlayer->isOnGround())
        return;

    if (const auto gameRules = (*memory->gameRules); gameRules)
        if (Misc::getGameMode() != GameMode::Competitive && gameRules->isValveDS())
            return;

    const auto netChannel = interfaces->engine->getNetworkChannel();
    if (!netChannel)
        return;

    cmd->buttons |= UserCmd::IN_BULLRUSH;
    const bool crouch = netChannel->chokedPackets >= (maxUserCmdProcessTicks / 2);
    if (crouch)
        cmd->buttons |= UserCmd::IN_DUCK;
    else
        cmd->buttons &= ~UserCmd::IN_DUCK;
    sendPacket = netChannel->chokedPackets >= maxUserCmdProcessTicks;
}

static bool windowOpen = false;

void Extra::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Extra")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Extra");
        ImGui::SetWindowPos("Extra", { 100.0f, 100.0f });
    }
}

void Extra::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        ImGui::Begin("Exploits", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    }
    ImGui::SetNextItemWidth(100.0f);
    ImGui::PushID("delta");
    ImGui::SliderFloat("Delta", &eCfg.maxAngleDelta, 0.0f, 255.0f, "%.2f"); //30 casual
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Values higher than 45 may cause untrusted or VAC error, use at own risk!");
    ImGui::PopID();

    ImGui::PushID("fakewalk");
    ImGui::hotkey("FakeWalk", eCfg.fakeWalkKey);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.fakeWalk, "Hold\0Toggle\0");
    ImGui::PopID();

    ImGui::PushID("fakeduck");
    ImGui::hotkey("FakeDuck", eCfg.fakeDuckKey);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.fakeDuck, "Hold\0Toggle\0");
    ImGui::PopID();

    ImGui::PushID("Auto-Peek");
    ImGuiCustom::colorPicker("Auto-Peek", eCfg.autoPeek.enabled);
    ImGui::SameLine();
    ImGui::PushID("Auto-Peek key");
    ImGui::hotkey("", eCfg.autoPeek.autoPeekKey);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.autoPeek, "Hold\0Toggle\0");
    ImGui::PopID();

    ImGui::PushID("Slowwalk");
    ImGui::Checkbox("Slow Walk", &eCfg.slowWalk.enabled);
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("slowwasd");
    }
    if (ImGui::BeginPopup("slowwasd")) {
        ImGui::SetNextItemWidth(100);
        ImGui::Combo("Mode", &eCfg.slowWalk.slowwalkMode, "Accurate\0Custom\0");
        if (eCfg.slowWalk.slowwalkMode == 1) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::SliderFloat(" ", &eCfg.slowWalk.slowspeed, 1.0f, 250.0f, "Speed: %.0f u/s");
        }
        ImGui::hotkey("", eCfg.slowWalk.slowwalkKey);
        ImGui::SameLine();
        ImGui::PushID("slowwalkcombo");
        ImGui::SetNextItemWidth(70.f);
        ImGui::Combo("", &eCfg.keyMode.slowWalk, "Hold\0Toggle\0");
        ImGui::PopID();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Prepare revolver Key");
    ImGui::Checkbox("Prepare revolver", &eCfg.prepareRevolver);
    ImGui::SameLine();
    ImGui::hotkey("", eCfg.prepareRevolverKey);
    ImGui::PopID();

    ImGui::PushID("Anti");
    ImGui::Checkbox("Anti-Untrusted", &eCfg.antiUntrust);
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Switching this option off may cause untrusted\nwithin 20 minutes on valve servers!"); 
    ImGui::PopID();

    ImGui::PushID("AirStuck");
    ImGui::hotkey("Air-Stuck", eCfg.airstuck.airStuckKey);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.f);
    ImGui::Combo("Mode", &eCfg.airstuck.mode, "INT_MAX\00016777216\0");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.airStuck, "Hold\0Toggle\0");
    ImGui::PopID();
    //ImGui::Checkbox("Resolver", &config->resolver);
    ImGui::Separator();
    ImGui::PushItemWidth(70.0f);
    ImGui::PushID("Tickbase");
    ImGui::PushID("dt");
    ImGui::hotkey("Doubletap", eCfg.tickBase.doubletap);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.doubleTap, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::PushID("hs");
    ImGui::hotkey("Hideshots", eCfg.tickBase.hideshots);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &eCfg.keyMode.hideShots, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::Checkbox("Teleport on shift", &eCfg.tickBase.teleport); ImGui::SameLine(); ImGui::hotkey("Force Teleport", eCfg.tickBase.forceTeleport);
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::SetNextItemWidth(120.0f);
    ImGui::PushID("Zeus bot");

    ImGui::Checkbox("Enable Zeusbot", &config->zeusbot.enable);
    ImGui::Checkbox("Silent", &config->zeusbot.silent);
    ImGui::SetNextItemWidth(100.f);
    ImGui::SliderInt("Zeus Max Wall Penetration Distance", &config->zeusbot.autoZeusMaxPenDist, 0, 50);
    ImGui::SetNextItemWidth(100.f);
    ImGui::SliderInt("Hitchance", &config->zeusbot.hitchance, 0, 100);
    ImGui::PopID();
    ImGui::Separator();
    ImGui::PushID("Knifebot");
    ImGui::Checkbox("Knifebot", &eCfg.knifeBot.enabled);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.f);
    ImGui::Combo("Mode", &eCfg.knifeBot.mode, "Trigger\0Rage\0");
    ImGui::PopID();

    if (!contentOnly)
        ImGui::End();
}

static void from_json(const json& j, ImVec2& v)
{
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, ExtraConfig::updateInputs& ui) {

    read(j, "DoubleTap",ui.doubleTap);
    read(j, "HideShots",ui.hideShots);
    read(j, "Auto-Peek",ui.autoPeek);
    read(j, "Fake-Duck",ui.fakeDuck);
    read(j, "Fake-Walk",ui.fakeWalk);
    read(j, "AirStuck", ui.airStuck);
    read(j, "SlowWalk", ui.slowWalk);
}

static void from_json(const json& j, ExtraConfig::SlowWalk& sw) {
    read(j, "Enabled", sw.enabled);
    read(j, "SlowWalk Accurate", sw.slowwalkMode);
    read(j, "SlowWalk Key", sw.slowwalkKey);
    read(j, "SlowWalk Speed", sw.slowspeed);
}

static void from_json(const json& j, ExtraConfig::Tickbase& t) {
    read(j, "Doubletap", t.doubletap);
    read(j, "Hideshots", t.hideshots);
    read(j, "Teleport", t.teleport);
    read(j, "Force Teleport", t.forceTeleport);
}

static void from_json(const json& j, ExtraConfig::AutoPeek& ap) {
    read<value_t::object>(j, "Color", ap.enabled);
    read(j, "Key", ap.autoPeekKey);
}

static void from_json(const json& j, ExtraConfig::KnifeBot& kb) {
    read(j, "Enabled", kb.enabled);
    read(j, "Mode", kb.mode);
}

static void from_json(const json& j, ExtraConfig& x)
{
    read(j, "AimStep", x.maxAngleDelta);
    read(j, "FakeDuck", x.fakeDuckKey);
    read(j, "FakeWalk", x.fakeWalkKey);
    read<value_t::object>(j, "SlowWalk", x.slowWalk);
    read<value_t::object>(j, "Tickbase", x.tickBase);
    read(j, "Anti-Untrust", x.antiUntrust);
    read(j, "Prepare revolver", x.prepareRevolver);
    read(j, "Prepare revolver key", x.prepareRevolverKey);
    read<value_t::object>(j, "Auto-Peek", x.autoPeek);
    read<value_t::object>(j, "KnifeBot", x.knifeBot);
    read<value_t::object>(j, "KeyModes", x.keyMode);

}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const ExtraConfig::updateInputs& o, const ExtraConfig::updateInputs& dummy = {}){

    WRITE("DoubleTap", doubleTap);
    WRITE("HideShots", hideShots);
    WRITE("Auto-Peek", autoPeek);
    WRITE("Fake-Duck", fakeDuck);
    WRITE("Fake-Walk", fakeWalk);
    WRITE("AirStuck", airStuck);
    WRITE("SlowWalk", slowWalk);
}

static void to_json(json& j, const ExtraConfig::SlowWalk& o, const ExtraConfig::SlowWalk& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("SlowWalk Accurate", slowwalkMode);
    WRITE("SlowWalk Key", slowwalkKey);
    WRITE("SlowWalk Speed", slowspeed);
}

static void to_json(json& j, const ExtraConfig::Tickbase& o, const ExtraConfig::Tickbase& dummy = {})
{
    WRITE("Doubletap", doubletap);
    WRITE("Hideshots", hideshots);
    WRITE("Teleport", teleport);
    WRITE("Force Teleport", forceTeleport);
}

static void to_json(json& j, const ExtraConfig::AutoPeek& o, const ExtraConfig::AutoPeek& dummy = {})
{
    WRITE("Color", enabled);
    WRITE("Key", autoPeekKey);
}

static void to_json(json& j, const ExtraConfig::KnifeBot& o, const ExtraConfig::KnifeBot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Mode", mode);
}


static void to_json(json& j, const ExtraConfig& o)
{
    const ExtraConfig dummy;

    WRITE("AimStep", maxAngleDelta);
    WRITE("FakeDuck", fakeDuckKey);
    WRITE("FakeWalk", fakeWalkKey);
    WRITE("SlowWalk", slowWalk);
    WRITE("Tickbase", tickBase);
    WRITE("Anti-Untrust", antiUntrust);
    WRITE("Prepare revolver", prepareRevolver);
    WRITE("Prepare revolver key", prepareRevolverKey);
    WRITE("Auto-Peek", autoPeek);
    WRITE("KnifeBot", knifeBot);
    WRITE("KeyModes", keyMode);
}

json Extra::toJson() noexcept
{
    json j;
    to_json(j, eCfg);
    return j;
}

void Extra::fromJson(const json& j) noexcept
{
    from_json(j, eCfg);
}

void Extra::resetConfig() noexcept
{
    eCfg = {};
}