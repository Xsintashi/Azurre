#pragma once

#include "../JsonForward.h"
#include "../Interfaces.h"
#include "../Memory.h"

struct ImDrawList;
struct UserCmd;
struct Vector;
enum class GameMode;

namespace Movement
{
    inline bool shouldEdgebug;
    inline float zVelBackup;
    inline float bugSpeed;
    inline int edgebugButtons;

    void updateInput() noexcept;
    bool isBackwardsPressed() noexcept;
    bool isJumpBugKeyDown() noexcept;
    bool isEdgeBugKeyDown() noexcept;
    bool isMiniJumpKeyDown() noexcept;
    bool isAutoPXKeyDown() noexcept;
    bool isEdgeJumpKeyDown() noexcept;
    bool isLadderJumpKeyDown() noexcept;

    void edgeJump(UserCmd* cmd) noexcept;
    void ladderJump(UserCmd* cmd) noexcept;
    void showVelocity(ImDrawList* drawList) noexcept;

    void gatherDataOnTick(UserCmd* cmd) noexcept;

    void handleKeyEvent(int keynum, const char* currentBinding) noexcept;

    void drawKeyStrokes(ImDrawList* drawList) noexcept;
    void jumpBug(UserCmd* cmd) noexcept;
    void miniJump(UserCmd* cmd) noexcept;
    void prePrediction(UserCmd* cmd) noexcept;
    void EdgebugDetect(float unpredictedVelocity) noexcept;
    void autoPixelSurf(UserCmd* cmd) noexcept;
    void headBounce(UserCmd* cmd) noexcept;
    void autoAlign(UserCmd* cmd) noexcept;
    void edgeBug(UserCmd* cmd, Vector& angView) noexcept;
    void gotJump() noexcept;
    void jumpStats(UserCmd* cmd) noexcept;
    void showMouseMove() noexcept;
    void Checkpoints() noexcept;
    void removeCrouchCooldown(UserCmd* cmd) noexcept;
    void fastStop(UserCmd* cmd) noexcept;
    void bunnyHop(UserCmd* cmd) noexcept;
    void duckWhenJump(UserCmd* cmd) noexcept;
    void autoStrafe(UserCmd* cmd, Vector& currentViewAngles) noexcept;
    void NullStrafe(UserCmd* cmd);

    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
