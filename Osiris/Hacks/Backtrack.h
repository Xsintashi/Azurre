#pragma once

#include <array>
#include <deque>

#include "../JsonForward.h"
#include "../ConfigStructs.h"

#include "../SDK/NetworkChannel.h"
#include "../SDK/matrix3x4.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/Vector.h"

namespace csgo { enum class FrameStage; }
struct UserCmd;

namespace Backtrack
{
    void run(UserCmd*) noexcept;

    void addLatencyToNetwork(NetworkChannel*, float) noexcept;
    void updateIncomingSequences() noexcept;

    bool DisableFakeLagWhileBeingDead() noexcept;

    int TimeLimitRecive() noexcept;

    int fakeLatencyAmountRecive() noexcept;

    bool DrawAllTick() noexcept;

    float getLerp() noexcept;

    struct incomingSequence {
        int inreliablestate;
        int sequencenr;
        float servertime;
    };

    bool valid(float simtime) noexcept;
    void init() noexcept;
    float getMaxUnlag() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
