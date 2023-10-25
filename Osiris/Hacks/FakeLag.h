#pragma once

#include "../JsonForward.h"

namespace FakeLag
{
    bool Enabled();
    int Limit();
    void run(bool& sendPacket) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;

}