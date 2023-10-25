#pragma once

struct UserCmd;
struct Vector;
class GameEvent;

namespace LegitAimbot
{
    bool isPressedOrHold() noexcept;
    void updateInput() noexcept;
    void run(UserCmd* cmd) noexcept;
}
