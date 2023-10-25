#pragma once
#include "../JsonForward.h"
#include "../imgui/imgui.h"
#include "../imguiCustom.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../ConfigStructs.h"
#include "../Memory.h"
#include "../Interfaces.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/GameEvent.h"


class GameEvent;

namespace Log {
    Color4 LogColor() noexcept;
    void DrawGUI() noexcept;
    void damageLog(GameEvent& event) noexcept;
    void droppedBomb(GameEvent& event) noexcept;
    void pickedUpBomb(GameEvent& event) noexcept;
    void bombBeginPlaningLog(GameEvent& event) noexcept;
    void bombAbortedPlantingLog(GameEvent& event) noexcept;
    void bombBeginDefuseLog(GameEvent& event) noexcept;
    void bombAbortDefuseLog(GameEvent& event) noexcept;

    void pickedUpItem(GameEvent& event) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}