#include "Fakelag.h"
#include "Misc.h"
#include "Tickbase.h"
#include "Extra.h"
#include "RageAimBot.h"
#include "EnginePrediction.h"
#include "AntiAim.h"
#include "../interfaces.h"
#include "../GUI.h"

#include "../ConfigStructs.h"
#include "../imguiCustom.h"
#include "../Config.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Engine.h"
#include "../SDK/Localplayer.h"

struct FakeLagConfig {
    int tryb = 0;
    int limit = 0;
    int runOn = 0;
    bool disableOnGrenade = false;
} fCfg;

bool FakeLag::Enabled() {
    return fCfg.limit;
}

int FakeLag::Limit() {
    return fCfg.limit;
}

void FakeLag::run(bool& sendPacket) noexcept
{

    if (!localPlayer || !localPlayer->isAlive() || Extra::isFakeDuckKeyPressed() || interfaces->engine->isHLTV())
        return;

    const auto gameRules = (*memory->gameRules);

    if (!gameRules) return;

    auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon) return;
    if (activeWeapon->isGrenade() && activeWeapon->isThrowing()) return;

    const auto netChannel = interfaces->engine->getNetworkChannel();
    if (!netChannel)
        return;

    const float speed = EnginePrediction::getVelocity().length2D() >= 15.0f ? EnginePrediction::getVelocity().length2D() : 0.0f;
    auto chokedPackets = AntiAim::isLegitEnabled() || AntiAim::isEnabled() ? 2 : 0;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    if (fCfg.limit) {

        switch (fCfg.tryb) {
        case 0: //Static
            chokedPackets = fCfg.limit;
            break;
        case 1: //Adaptive
            chokedPackets = std::clamp(static_cast<int>(std::ceilf(64 / (speed * memory->globalVars->intervalPerTick))), 1, fCfg.limit);
            break;
        case 2: // Random
            srand(static_cast<unsigned int>(time(NULL)));
            chokedPackets = rand() % fCfg.limit + 1;
            break;
        }

        //"On Stand\0On Walk\0On Run\0On Duck\0On Air\0");
        chokedPackets = std::clamp(chokedPackets, 0, maxUserCmdProcessTicks - Tickbase::getTargetTickShift());

        if (!fCfg.runOn) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }
        if (Helpers::getByteFromBytewise(fCfg.runOn, 0) && localPlayer->isOnGround() && localPlayer->velocity().length2D() < 1.2f) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }
        if (Helpers::getByteFromBytewise(fCfg.runOn, 1) && localPlayer->isOnGround() && localPlayer->velocity().length2D() < Extra::getWeaponAccuracySpeed() && localPlayer->velocity().length2D() > 1.2f) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }
        if (Helpers::getByteFromBytewise(fCfg.runOn, 2) && localPlayer->isOnGround() && localPlayer->velocity().length2D() > Extra::getWeaponAccuracySpeed()) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }
        if (Helpers::getByteFromBytewise(fCfg.runOn, 3) && localPlayer->isDucking()) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }
        if (Helpers::getByteFromBytewise(fCfg.runOn, 4) && !localPlayer->isOnGround()) {
            sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
        }

        return;
    }

    chokedPackets = std::clamp(chokedPackets, 0, maxUserCmdProcessTicks - Tickbase::getTargetTickShift());
    sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= std::clamp(chokedPackets, 0, maxUserCmdProcessTicks);
}

static bool fakeLagOpen = false;

void FakeLag::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Fake Lag")) {
        fakeLagOpen = true;
        ImGui::SetWindowFocus("Fake-Lag");
        ImGui::SetWindowPos("Fake-Lag", { 100.0f, 100.0f });
    }
}

void FakeLag::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!fakeLagOpen)
            return;
        ImGui::SetNextWindowSize({ 256.f, 144.f });
        ImGui::Begin("Fake-Lag", &fakeLagOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::PushID("Fakelag");
    ImGui::Columns(2, "FakeLag", false);
    ImGui::SetColumnOffset(1, 128.f);
    ImGui::PushItemWidth(75.0f);
    ImGui::PushID("fake lag leg");
    ImGui::Combo("Mode", &fCfg.tryb, "Static\0Adaptive\0Random\0");
    ImGui::PopID();
    ImGui::PushID("fake lag slider");
    ImGui::SliderInt("Limit", &fCfg.limit, 0, 16, "%d");
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Fake lagging over 8 ticks\n on Valve Servers isn't possible!");
    ImGui::PopID();
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(100.f);
    ImGuiCustom::multiCombo("Trigger on", fCfg.runOn, "On Stand\0On Walk\0On Run\0On Duck\0On Air\0");
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::PopID();
    if (!contentOnly)
        ImGui::End();
}

static void from_json(const json& j, FakeLagConfig& fl)
{
    read(j, "Mode", fl.tryb);
    read(j, "Limit", fl.limit);
    read(j, "Triggers", fl.runOn);
}

static void to_json(json& j, const FakeLagConfig& o)
{
    const FakeLagConfig dummy;
    WRITE("Mode", tryb);
    WRITE("Limit", limit);
    WRITE("Triggers", runOn);
}

json FakeLag::toJson() noexcept
{
    json j;
    to_json(j, fCfg);
    return j;
}

void FakeLag::fromJson(const json& j) noexcept
{
    from_json(j, fCfg);
}

void FakeLag::resetConfig() noexcept
{
    fCfg = {};
}