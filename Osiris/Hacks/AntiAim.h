#pragma once

#include "../JsonForward.h"

struct UserCmd;
struct Vector;

#define OSIRIS_ANTIAIM() true

namespace AntiAim
{
    bool isAngleInverted() noexcept;
    bool isAAInverted() noexcept;
    bool isRollInverted() noexcept;
    bool isBackwards() noexcept;
    bool isLegitEnabled() noexcept;
    bool isEnabled() noexcept;
    bool isAnglesEnabled() noexcept;
    bool hasToRoll() noexcept;
    bool isLAAInverted() noexcept;

    float RollValue();

    bool canRun(UserCmd* cmd) noexcept;

    void updateInput() noexcept;
    float breakLC(bool& sendPacket) noexcept;
    float getLBYUpdate() noexcept;

    void run(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept;
    void rage(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept;
    void legit(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept;
    void lines(ImDrawList* drawList) noexcept;
    void backwards(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept;
    void Visualize(ImDrawList* drawList) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void menuBarItemL() noexcept;
    void drawGUI(bool contentOnly) noexcept;
    void drawLegitGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}

bool updateLby(bool update) noexcept;
