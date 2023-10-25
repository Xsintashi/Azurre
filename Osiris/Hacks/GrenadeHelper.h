#pragma once

#include "../Helpers.h"
#include "../GUI.h"
#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../JsonForward.h"
#include "../imguiCustom.h"
#include "../ConfigStructs.h"
#include "../config.h"
#include "../Interfaces.h"
#include "../SDK/Entity.h"
#include "../SDK/Engine.h"
#include "../SDK/LocalPlayer.h"

struct ImDrawList;
struct UserCmd;
struct Vector;
class GameEvent;
namespace Nade{
    void Run(ImDrawList* drawList) noexcept;
	void Move(UserCmd* cmd) noexcept;
    bool isKeyDown() noexcept;
	void drawGUI(bool contentOnly) noexcept;


    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;

}

enum Throws {
    StandLeft = 0,
    JumpLeft,
    StandRight,
    JumpRight,
};
enum NadesTypes {
    He = 0,
    Flash,
    Smoke,
    Molotov
};