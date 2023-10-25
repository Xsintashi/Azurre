#pragma once

#include "../JsonForward.h"

struct UserCmd;
struct Vector;
struct ImDrawList;

namespace Extra
{
    void updateInput() noexcept;

    int knifeBotMode() noexcept;

    bool knifeBotEnabled() noexcept;
    bool forceTeleportKey() noexcept;
    bool isDTEnabled() noexcept;
    bool isHideShotsEnabled() noexcept;
    bool isTeleportEnabled() noexcept;
    bool isSlowWalkKeyDown() noexcept;
    bool isFakeDuckKeyPressed() noexcept;
    bool isAirStuckKeyDown() noexcept;
    bool isQuickPeekKeyDown() noexcept;
    bool isFakeWalkKeyPressed() noexcept;
    bool AntiUntrusted() noexcept;

    float getWeaponAccuracySpeed() noexcept;
    float maxAngleDelta() noexcept;

    void quickPeek(UserCmd* cmd) noexcept;
    void visualizeQuickPeek(ImDrawList* drawList) noexcept;
    void airStuck(UserCmd* cmd) noexcept;
    void slowWalk(UserCmd* cmd) noexcept;
    void fakeDuck(UserCmd* cmd, bool& sendPacket) noexcept;
    void fakeWalk(UserCmd* cmd, bool& sendPacket) noexcept;
    void prepareRevolver(UserCmd*) noexcept;

    // GUI

    void drawGUI(bool contentOnly) noexcept;

    void menuBarItem() noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
