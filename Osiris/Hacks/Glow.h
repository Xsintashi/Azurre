#pragma once

#include "../JsonForward.h"

#define OSIRIS_GLOW() true

namespace Glow
{
    void changeGlowThickness() noexcept;
    void render() noexcept;
    void clearCustomObjects() noexcept;
    void updateInput() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
