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

struct UserCmd;
struct Vector;
class GameEvent;

namespace Walkbot {

    //aimbot shiz
    void aimAtEvent() noexcept;
    void enemiesRadar() noexcept;
    void handleLocatedEnemies() noexcept;
    //dynamic nodes pathfinding
    void findBreakable(UserCmd* cmd) noexcept;
    void drawPathfinding()noexcept;
    void addNeighborNodes() noexcept;
    void findPath() noexcept;
    void pathfind() noexcept;
    void drawPath() noexcept;
    void reload(UserCmd* cmd) noexcept;
    void gotoBotzPos(UserCmd* cmd) noexcept;

    //static nodes pathfinding
    void addNewNode(Vector pingPos) noexcept;
    void drawPresetNodes() noexcept;
    void event(GameEvent& event) noexcept;
    void openNode(int nodeIndex) noexcept;

	//GUI
	void drawGUI(bool contentOnly) noexcept;
	void menuBarItem() noexcept;

}