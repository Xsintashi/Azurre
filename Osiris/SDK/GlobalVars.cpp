#include "../Interfaces.h"

#include "GlobalVars.h"
#include "UserCmd.h"
#include "Engine.h"
#include "Entity.h"
#include "EntityList.h"

float GlobalVars::serverTime(UserCmd* cmd) const noexcept
{
    static int tick;
    static UserCmd* lastCmd;

    if (cmd) {
        if (localPlayer && (!lastCmd || lastCmd->hasbeenpredicted))
            tick = localPlayer->tickBase();
        else
            tick++;
        lastCmd = cmd;
    }
    return tick * intervalPerTick;
}

bool GlobalVars::newTick() const noexcept
{
    static float lastTick = 0.f;
    if (lastTick != tickCount) {
        lastTick = static_cast<float>(tickCount);
        return true;
    }
    return false;
}