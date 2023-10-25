#include "AntiAim.h"
#include "Extra.h"
#include "Misc.h"
#include "Movement.h"
#include "Tickbase.h"
#include "Xsin.h"

#include <cmath>
#include <algorithm>

#include "../imgui/imgui.h"
#include "../ConfigStructs.h"
#include <imguiCustom.h>

#include "../SDK/Entity.h"
#include "../SDK/Engine.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/EntityList.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Gamemode.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Input.h"

#include <Memory.h>
#include <interfaces.h>
#include <HelpersAimbot.h>

#define min(a,b)            (((a) < (b)) ? (a) : (b))

struct AntiAimConfig {

    bool enabledLegit = false,
        enabledRage = false,
        enabledAngle = false,
        freezetime = false,
        extendedLegit = false,
        syncRollWithFakeAngles = false;

    int aamode = 0,
        yawmode = 0,
        pitchmode = 0,
        direction = 0,

        leftLimit = 60,
        rightLimit = 60;

    float
        jittermin = 0,
        jittermax = 1,
        rollAngle = 0,
        pitchcustom = 0,
        yawadd = 0;

    int peekMode = 0; //Off, Peek real, Peek fake
    int lbyMode = 0; // Normal, Opposite, sway, fake desync

    KeyBind invertangle{ KeyBind::NONE },
        invertYaw{ KeyBind::NONE },
        invertlegit{ KeyBind::NONE },
        invertRoll{ KeyBind::NONE },
        overrideleft{ KeyBind::LEFT },
        overridedown{ KeyBind::DOWN },
        overrideright{ KeyBind::RIGHT };

    struct FakeFlick {
        KeyBind keybindEnabled;
        KeyBind keybindInvert;
        int rate = 16;
    } flick;

    struct updateInputs {
        int Yaw{ 0 };
        int Angle{ 0 };
        int Legit{ 0 };
        int Roll{ 0 };
    }keyMode;

    struct Arrows {
        ColorToggle3 legitArrows, angleArrows, overrideArrows, realArrows, fakeArrows;
        float posLegit = 10.f,posAngle = 10.f, posOverride = 10.f;
    }arrows;

    struct Distort {
        bool enabled;
        float speed = 0, amount = 0;
        int triggerOn = 0;
    }dist;

} aaCfg;

static bool isRolling = false;

static ImVec2 overrideArrowsVec = {};

float AntiAim::RollValue() {
    return aaCfg.rollAngle;
}

float RandomFloatAdv(float a, float b, float multiplier) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    float result = a + r;
    return result * multiplier;
}

bool AntiAim::canRun(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return false;

    if (interfaces->engine->isHLTV())
        return false;

    updateLby(true); //Update lby timer

    if (localPlayer->flags() & (1 << 6))
        return false;

    if (localPlayer->isDefusing()) return false;

    if (*memory->gameRules && (*memory->gameRules)->freezePeriod())
        return !aaCfg.freezetime;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return true;

    if (activeWeapon->isThrowing())
        return false;

    if (activeWeapon->isGrenade())
        return true;

    if (activeWeapon->itemDefinitionIndex() == WeaponId::Revolver && activeWeapon->readyTime() <= memory->globalVars->serverTime() && cmd->buttons & (UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2))
        return false;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto() || localPlayer->waitForNoAttack())
        return true;

    if (localPlayer->nextAttack() > memory->globalVars->serverTime())
        return true;

    if (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return true;

    if (activeWeapon->nextSecondaryAttack() > memory->globalVars->serverTime())
        return true;

    if (localPlayer->nextAttack() <= memory->globalVars->serverTime() && (cmd->buttons & (UserCmd::IN_ATTACK)))
        return false;

    if (activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && (cmd->buttons & (UserCmd::IN_ATTACK)))
        return false;

    if (activeWeapon->isKnife())
    {
        if (activeWeapon->nextSecondaryAttack() <= memory->globalVars->serverTime() && cmd->buttons & (UserCmd::IN_ATTACK2))
            return false;
    }

    if (localPlayer->moveType() == MoveType::LADDER || localPlayer->moveType() == MoveType::NOCLIP)
        return false;

    if (cmd->buttons & (UserCmd::IN_USE))
        return false;

    const auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return true;

    return true;
}

static bool Angle = false;
static bool Yaw = false;
static bool Legit = false;
static bool Roll = false;

static bool runBackwards = false;

void AntiAim::updateInput() noexcept
{
    if (!aaCfg.keyMode.Angle)
        Angle = aaCfg.invertangle.isDown();
    else if (aaCfg.keyMode.Angle && aaCfg.invertangle.isPressed())
        Angle = !Angle;

    if (!aaCfg.keyMode.Yaw)
        Yaw = aaCfg.invertYaw.isDown();
    else if (aaCfg.keyMode.Yaw && aaCfg.invertYaw.isPressed())
        Yaw = !Yaw;

    if (!aaCfg.keyMode.Roll)
        Roll = aaCfg.invertRoll.isDown();
    else if (aaCfg.keyMode.Roll && aaCfg.invertRoll.isPressed())
        Roll = !Roll;

    if (!aaCfg.keyMode.Legit)
        Legit = aaCfg.invertlegit.isDown();
    else if (aaCfg.keyMode.Legit && aaCfg.invertlegit.isPressed())
        Legit = !Legit;
}

bool AntiAim::isAngleInverted() noexcept {
    if (!aaCfg.enabledAngle) return false;
    return Angle;
}

bool AntiAim::isAAInverted() noexcept {
    if (!aaCfg.enabledRage) return false;
    return Yaw;
}

bool AntiAim::isRollInverted() noexcept {
    if (!aaCfg.enabledRage) return false;
    return Roll;
}

bool AntiAim::isLAAInverted() noexcept {
    if (!aaCfg.enabledLegit) return false;
    return Legit;
}

bool AntiAim::isBackwards() noexcept
{
    return runBackwards;
}

bool AntiAim::isLegitEnabled() noexcept {
    return aaCfg.enabledLegit;
}

bool AntiAim::isEnabled() noexcept {
    return aaCfg.enabledRage;
}

bool AntiAim::isAnglesEnabled() noexcept {
    return aaCfg.enabledAngle;
}

bool AntiAim::hasToRoll()noexcept {
    return isRolling;
}

float AntiAim::breakLC(bool& sendPacket) noexcept {

    if (!localPlayer) return false;
    if (!localPlayer->isAlive()) return false;

    static Vector previousOrigin = localPlayer->getAbsOrigin();
    const Vector currentOrigin = localPlayer->getAbsOrigin();

    if (sendPacket)
        previousOrigin = localPlayer->getAbsOrigin();

    return (currentOrigin - previousOrigin).squareLength();
}

bool updateLby(bool update = false) noexcept
{
    static float timer = 0.f;
    static bool lastValue = false;

    if (!update)
        return lastValue;

    if (!(localPlayer->flags() & 1) || !localPlayer->getAnimstate())
    {
        lastValue = false;
        return false;
    }

    if (localPlayer->velocity().length2D() > 0.1f || fabsf(localPlayer->velocity().z) > 100.f)
        timer = memory->globalVars->serverTime() + 0.22f;

    if (timer < memory->globalVars->serverTime())
    {
        timer = memory->globalVars->serverTime() + 1.1f;
        lastValue = true;
        return true;
    }
    lastValue = false;
    return false;
}

float AntiAim::getLBYUpdate() noexcept
{
    static float nextLBYUpdate[65];

    const float curtime = memory->globalVars->currenttime;

    if (!localPlayer || !localPlayer->isAlive())
        return 0;

    if (localPlayer->velocity().length2D() > 0.1f || fabsf(localPlayer->velocity().z) > 100.f)
        return 0;

    auto animstate = localPlayer->getAnimstate();
    if (!animstate)
        return 0;

    if (static float lastLBYUpdate; lastLBYUpdate != localPlayer->lby())
    {
        lastLBYUpdate = localPlayer->lby();
        if (fabs(curtime) > 0.3 && fabs(Helpers::normalizeYaw(localPlayer->lby())) > 35.f)
            nextLBYUpdate[localPlayer->index()] = curtime + 1.1f + memory->globalVars->intervalPerTick;
    }

    if (nextLBYUpdate[localPlayer->index()] < curtime)
    {
        nextLBYUpdate[localPlayer->index()] = curtime + 1.1f;
    }

    float timeToUpdate = nextLBYUpdate[localPlayer->index()] - localPlayer->simulationTime();

    return std::clamp(timeToUpdate, 0.f, 1.f);
}

void distortion(UserCmd* cmd) {
    const float sine = ((sin(memory->globalVars->currenttime * (aaCfg.dist.speed / 10.f)) + 1) / 2) * aaCfg.dist.amount;
    cmd->viewangles.y += sine - (aaCfg.dist.amount / 2.f);
}

bool autoDirection(Vector eyeAngle) noexcept
{
    constexpr float maxRange{ 8192.0f };

    Vector eye = eyeAngle;
    eye.x = 0.f;
    Vector eyeAnglesLeft45 = eye;
    Vector eyeAnglesRight45 = eye;
    eyeAnglesLeft45.y += 45.f;
    eyeAnglesRight45.y -= 45.f;
    eyeAnglesLeft45.toAngle();

    Vector viewAnglesLeft45 = {};
    viewAnglesLeft45 = viewAnglesLeft45.fromAngle(eyeAnglesLeft45) * maxRange;

    Vector viewAnglesRight45 = {};
    viewAnglesRight45 = viewAnglesRight45.fromAngle(eyeAnglesRight45) * maxRange;

    static Trace traceLeft45;
    static Trace traceRight45;

    Vector startPosition{ localPlayer->getEyePosition() };

    interfaces->engineTrace->traceRay({ startPosition, startPosition + viewAnglesLeft45 }, 0x4600400B, { localPlayer.get() }, traceLeft45);
    interfaces->engineTrace->traceRay({ startPosition, startPosition + viewAnglesRight45 }, 0x4600400B, { localPlayer.get() }, traceRight45);

    float distanceLeft45 = sqrtf(powf(startPosition.x - traceRight45.endpos.x, 2) + powf(startPosition.y - traceRight45.endpos.y, 2) + powf(startPosition.z - traceRight45.endpos.z, 2));
    float distanceRight45 = sqrtf(powf(startPosition.x - traceLeft45.endpos.x, 2) + powf(startPosition.y - traceLeft45.endpos.y, 2) + powf(startPosition.z - traceLeft45.endpos.z, 2));

    float mindistance = min(distanceLeft45, distanceRight45);

    if (distanceLeft45 == mindistance)
        return false;
    return true;
}

static bool invert = true;

void AntiAim::rage(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{
    static float keybindoverride = 0;
    bool lby = updateLby();
    Vector oldangle = cmd->viewangles;

    debug.serverAngles = cmd->viewangles;
    debug.fakeAngles = cmd->viewangles;

    if (cmd->viewangles.x == currentViewAngles.x || Tickbase::isShifting()) {
        switch (aaCfg.pitchmode) {
            default:
            case 0: //OFF
                break;
            case 1: //DOWN
                cmd->viewangles.x = 89.f;
                break;
            case 2: //UP
                cmd->viewangles.x = -89.f;
                break;
            case 3: //CUSTOM
                cmd->viewangles.x = aaCfg.pitchcustom;
                break;
            case 4: //Fake Pitch
                if ((*(memory->gameRules))->isValveDS())
                    break;
                cmd->viewangles.x = 1080.f;
                cmd->forwardmove = -cmd->forwardmove;
                break;
        }
    }
    if (cmd->viewangles.y == currentViewAngles.y || Tickbase::isShifting()) {
        static float yaw = 0.f;
        static float override = 0.f;
        float setYaw = 0.f;
        float bestFov = 255.f;
        switch (aaCfg.direction) {
            default:case 0:
                yaw = 0.f;
                break;
            case 1: {
                Vector localPlayerEyePosition = localPlayer->getEyePosition();
                const auto aimPunch = localPlayer->getAimPunch();
                for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
                    auto entity{ interfaces->entityList->getEntity(i) };
                    if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                        || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity())
                        continue;

                    const auto angle{ AimHelpers::calculateRelativeAngle(localPlayerEyePosition, entity->getAbsOrigin(), cmd->viewangles + aimPunch) };
                    const auto fov{ angle.length2D() };
                    if (fov < bestFov)
                    {
                        setYaw = angle.y;
                        bestFov = fov;
                    }
                }
                yaw = setYaw;
                break;
            }
            case 2: {
                if (aaCfg.yawmode == 4) break;

                if (aaCfg.yawmode == 4) break;

                constexpr std::array positions = { -35.0f, 0.0f, 35.0f };
                std::array active = { false, false, false };
                const auto fwd = Vector::fromAngle2D(cmd->viewangles.y);
                const auto side = fwd.crossProduct(Vector::up());

                for (std::size_t i = 0; i < positions.size(); ++i)
                {
                    const auto start = localPlayer->getEyePosition() + side * positions[i];
                    const auto end = start + fwd * 100.0f;

                    Trace trace;
                    interfaces->engineTrace->traceRay({ start, end }, CONTENTS_SOLID | CONTENTS_WINDOW, nullptr, trace);

                    if (trace.fraction != 1.0f)
                        active[i] = true;
                }

                if (active[0] && active[1] && !active[2])
                    setYaw = -1;
                else if (!active[0] && active[1] && active[2])
                    setYaw = 1;
                else
                    setYaw = 0;

                yaw = setYaw * 90.f;

                break;
            }
        }
        switch (aaCfg.yawmode) {
        default:
        case 0:
            yaw += 180.f; //backward
            break;
        case 1:
            yaw += 90.f; //left
            break;
        case 2:
            yaw += -90.f; //right
            break;
        case 3:
            yaw += 0.f; //forward
            break;
        case 4:
            yaw += 323210000.0f; //lisp
            break;
        case 5:
            break;
        }

        switch (aaCfg.aamode) {
        default://STATIC
            break;
        case 1: //Jitter
                yaw += Helpers::randomFloat(aaCfg.jittermin, aaCfg.jittermax);
            break;
        case 2: //Spin
            float server_time = localPlayer->tickBase() * memory->globalVars->intervalPerTick;
            float rate = 360.0f / 1.618033988749895f;
            yaw += fmod(static_cast<float>(server_time) * rate, 360.0f);
            break;
        }
        cmd->viewangles.y += Helpers::normalizeYaw(((yaw + aaCfg.yawadd) * (Yaw ? -1 : 1)));

        if (aaCfg.dist.enabled) {

            if (!aaCfg.dist.triggerOn)
                distortion(cmd);

            if (Helpers::getByteFromBytewise(aaCfg.dist.triggerOn, 0) && localPlayer->isOnGround() && localPlayer->velocity().length2D() < 1.2f)  // stand
                distortion(cmd);

            if (Helpers::getByteFromBytewise(aaCfg.dist.triggerOn, 1) && localPlayer->isOnGround() && (localPlayer->velocity().length2D() < Extra::getWeaponAccuracySpeed() && localPlayer->velocity().length2D() > 1.2f))  //walk
                distortion(cmd);

            if (Helpers::getByteFromBytewise(aaCfg.dist.triggerOn, 2) && localPlayer->isOnGround() && localPlayer->velocity().length2D() > Extra::getWeaponAccuracySpeed())  // run
                distortion(cmd);

            if (Helpers::getByteFromBytewise(aaCfg.dist.triggerOn, 3) && localPlayer->isOnGround() && localPlayer->isDucking())  // on duck
                distortion(cmd);

            if (Helpers::getByteFromBytewise(aaCfg.dist.triggerOn, 4) && !localPlayer->isOnGround())  // on air
                distortion(cmd);

        }

        if (aaCfg.flick.keybindEnabled.isDown()) {
            static int tickCount = memory->globalVars->tickCount;

            if ((memory->globalVars->tickCount - tickCount) > aaCfg.flick.rate) {
                cmd->viewangles.y += -90.f;
                tickCount = memory->globalVars->tickCount;
            }
        }

        debug.serverAngles.y = cmd->viewangles.y;

        if (cmd->viewangles.z == currentViewAngles.z) //Roll angle
        {

            if (localPlayer->isOnGround() && localPlayer->velocity().length2D() < 135.f) {

                if (aaCfg.syncRollWithFakeAngles) {
                    cmd->viewangles.z = invert ? aaCfg.rollAngle : -aaCfg.rollAngle;
                }
                else {
                    cmd->viewangles.z = Roll ? aaCfg.rollAngle : -aaCfg.rollAngle;
                }

                debug.serverAngles.z = cmd->viewangles.z;
            }
        }
        else {
            debug.serverAngles.z = 0.f;
        }

        if (aaCfg.enabledAngle) //Fakeangle
        {

            bool isInvertToggled = Angle;
            if (aaCfg.peekMode != 3)
                invert = isInvertToggled;
            float leftDesyncAngle = aaCfg.leftLimit * 2.f;
            float rightDesyncAngle = aaCfg.rightLimit * -2.f;

            switch (aaCfg.peekMode)
            {
            case 0:
                break;
            case 1: // Peek real
                if (!isInvertToggled)
                    invert = !autoDirection(cmd->viewangles);
                else
                    invert = autoDirection(cmd->viewangles);
                break;
            case 2: // Peek fake
                if (isInvertToggled)
                    invert = !autoDirection(cmd->viewangles);
                else
                    invert = autoDirection(cmd->viewangles);
                break;
            case 3: // Jitter
                if (sendPacket)
                    invert = !invert;
                break;
            default:
                break;
            }

            switch (aaCfg.lbyMode){
            case 0: // Normal(sidemove) ghetto micromovement
                if (fabsf(cmd->sidemove) < 5.0f)
                {
                    if (cmd->buttons & UserCmd::IN_DUCK)
                        cmd->sidemove = cmd->tickCount & 1 ? 3.25f : -3.25f;
                    else
                        cmd->sidemove = cmd->tickCount & 1 ? 1.1f : -1.1f;
                }
                break;
            case 1: // Opposite (Lby break)
                if (updateLby())
                {
                    cmd->viewangles.y += !invert ? leftDesyncAngle : rightDesyncAngle;
                    debug.serverAngles.y = cmd->viewangles.y;
                    sendPacket = false;
                    debug.fakeAngles.y = cmd->viewangles.y;
                    return;
                }
                break;
            case 2: { //Sway (flip every lby update)
                static bool flip = false;
                if (updateLby())
                {
                    cmd->viewangles.y += !flip ? leftDesyncAngle : rightDesyncAngle;
                    debug.serverAngles.y = cmd->viewangles.y;
                    sendPacket = false;
                    debug.fakeAngles.y = cmd->viewangles.y;
                    flip = !flip;
                    return;
                }
                if (!sendPacket)
                    cmd->viewangles.y += flip ? leftDesyncAngle : rightDesyncAngle;
                    debug.fakeAngles.y = cmd->viewangles.y;
                break;
            }
            case 3:
                if (updateLby())
                {
                    cmd->viewangles.y += !invert ? leftDesyncAngle : rightDesyncAngle;
                    debug.serverAngles.y = cmd->viewangles.y;
                    sendPacket = false;
                    debug.fakeAngles.y = cmd->viewangles.y;
                    return;
                }
                if (!sendPacket)
                    cmd->viewangles.y += invert ? leftDesyncAngle : rightDesyncAngle;
                debug.fakeAngles.y = cmd->viewangles.y;
                break;
            }

            if (sendPacket)
                return;

            cmd->viewangles.y += invert ? leftDesyncAngle : rightDesyncAngle;
            debug.fakeAngles.y = cmd->viewangles.y;
        }
        else {
            debug.fakeAngles.y = cmd->viewangles.y;
        }
    }
}

  

void AntiAim::legit(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{
    if (cmd->viewangles.y == currentViewAngles.y || Tickbase::isShifting())
    {
        float desyncAngle = localPlayer->getMaxDesyncAngle() * 2.f;
        if (updateLby() && aaCfg.extendedLegit)
        {
            cmd->viewangles.y += !AntiAim::isLAAInverted() ? desyncAngle : -desyncAngle;
            sendPacket = false;
            return;
        }

        if (fabsf(cmd->sidemove) < 5.0f && !aaCfg.extendedLegit)
        {
            if (cmd->buttons & UserCmd::IN_DUCK)
                cmd->sidemove = cmd->tickCount & 1 ? 3.25f : -3.25f;
            else
                cmd->sidemove = cmd->tickCount & 1 ? 1.1f : -1.1f;
        }

        if (sendPacket)
            return;

        cmd->viewangles.y += invert ? desyncAngle : -desyncAngle;
    }
}

void AntiAim::lines(ImDrawList* drawList) noexcept
{
    if (!debug.enable)
        return;

    if (!localPlayer) return;

    if (!localPlayer->isAlive()) return;

    auto AngleVectors = [](const Vector& angles, Vector* forward)
    {
        float	sp, sy, cp, cy;

        sy = sin(Helpers::deg2rad(angles.y));
        cy = cos(Helpers::deg2rad(angles.y));

        sp = sin(Helpers::deg2rad(angles.x));
        cp = cos(Helpers::deg2rad(angles.x));

        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    };

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (!interfaces->engine->isInGame() || !interfaces->engine->isConnected())
        return;

    if (!memory->input->isCameraInThirdPerson)
        return;


    float lbyAngle = localPlayer->lby();
    float realAngle = debug.serverAngles.y;
    float fakeAngle = debug.fakeAngles.y;

    Vector src3D, dst3D, forward;
    ImVec2 src, dst;
    //
    // LBY
    //

    AngleVectors(Vector{ 0, lbyAngle, 0 }, &forward);
    src3D = localPlayer->getAbsOrigin();
    dst3D = src3D + (forward * 40.f);

    if (!Helpers::worldToScreen(src3D, src) || !Helpers::worldToScreen(dst3D, dst))
        return;

    drawList->AddLine(src, dst, ImGui::GetColorU32({ 0.f, 1.f, 0.f, 1.f }), 1.f);

     //
     // Fake
     //

    AngleVectors(Vector{ 0, fakeAngle, 0 }, &forward);
    dst3D = src3D + (forward * 40.f);


    if (!Helpers::worldToScreen(src3D, src) || !Helpers::worldToScreen(dst3D, dst))
        return;

    drawList->AddLine(src, dst, ImGui::GetColorU32({0.f, 0.f, 1.f, 1.f }), 1.f);

    //
    // Real
    //

    AngleVectors(Vector{ 0, realAngle, 0 }, &forward);
    dst3D = src3D + (forward * 40.f);

    if (!Helpers::worldToScreen(src3D, src) || !Helpers::worldToScreen(dst3D, dst))
        return;

    drawList->AddLine(src, dst, ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), 1.f);
}

void AntiAim::Visualize(ImDrawList* drawList) noexcept
{

    if (!localPlayer) return;

    if (!localPlayer->isAlive()) return;

    if (aaCfg.arrows.angleArrows.enabled && aaCfg.enabledRage) //angle
    {
        const auto color = Helpers::calculateColor(aaCfg.arrows.angleArrows.asColor3());

        if (invert)
            ImGuiCustom::drawTriangleFromCenter(drawList, { -aaCfg.arrows.posAngle, 0 }, color, false);
        else
            ImGuiCustom::drawTriangleFromCenter(drawList, { aaCfg.arrows.posAngle, 0 }, color, false);

    }

    if (aaCfg.arrows.realArrows.enabled && aaCfg.enabledRage) //angle
    {
        const auto color = Helpers::calculateColor(aaCfg.arrows.realArrows.asColor3());

        const auto engineAngles = interfaces->engine->getViewAngles();

        const float normalized = -Helpers::normalizeYaw(debug.serverAngles.y - engineAngles.y - 225.f); //dont ask why 225.f cuz idk
        const auto yaw = Helpers::deg2rad(normalized);

        auto x = std::cos(yaw) - std::sin(yaw);
        auto y = std::cos(yaw) + std::sin(yaw);
        if (const auto len = std::sqrt(x * x + y * y); len != 0.0f) {
            x /= len;
            y /= len;
        }
        ImGuiCustom::drawTriangleFromCenter(drawList, { x, y }, color, false);
    }

    if (aaCfg.arrows.fakeArrows.enabled && aaCfg.enabledRage && aaCfg.enabledAngle && localPlayer->isOnGround()) //angle
    {

        if (!localPlayer) return;

        if (!localPlayer->isAlive()) return;

        const auto color = Helpers::calculateColor(aaCfg.arrows.fakeArrows.asColor3());

        const auto engineAngles = interfaces->engine->getViewAngles();

        const float normalized = -Helpers::normalizeYaw(debug.fakeAngles.y - engineAngles.y - 225.f);
        const auto yaw = Helpers::deg2rad(normalized);

        auto x = std::cos(yaw) - std::sin(yaw);
        auto y = std::cos(yaw) + std::sin(yaw);
        if (const auto len = std::sqrt(x * x + y * y); len != 0.0f) {
            x /= len;
            y /= len;
        }

        ImGuiCustom::drawTriangleFromCenter(drawList, { x, y }, color, false);
    }

    if (aaCfg.arrows.legitArrows.enabled && aaCfg.enabledLegit) //legit desync
    {

        if (!localPlayer) return;

        if (!localPlayer->isAlive()) return;

        const auto color = Helpers::calculateColor(aaCfg.arrows.legitArrows.asColor3());

        if (AntiAim::isLAAInverted())
            ImGuiCustom::drawTriangleFromCenter(drawList, { -aaCfg.arrows.posLegit, 0 }, color, false);
        else
            ImGuiCustom::drawTriangleFromCenter(drawList, { aaCfg.arrows.posLegit, 0 }, color, false);

    }

    if (aaCfg.yawmode == 4) {
        const auto color = Helpers::calculateColor(aaCfg.arrows.overrideArrows.asColor3());

        ImGuiCustom::drawTriangleFromCenter(drawList, overrideArrowsVec, color, false);
    }
}

void AntiAim::backwards(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{
    if (!localPlayer) return;

    if (!localPlayer->isAlive()) return;

    if (Movement::isBackwardsPressed()) runBackwards = !runBackwards;

    if (cmd->viewangles.y == currentViewAngles.y)
    {
        float ct = memory->globalVars->currenttime;
        float static ctLastState = ct;
        int static i = 0;

        if (runBackwards) {
            if (!(ct == ctLastState) && i < 180) {
                ct = ctLastState;
                i += 8;
            }
            cmd->viewangles.y += i;
            if (localPlayer->moveType() != MoveType::LADDER)
                cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
        }
        if (!runBackwards) {
            if (!(ct == ctLastState) && i > 0) {
                ct = ctLastState;
                i -= 8;
            }
            cmd->viewangles.y += i;
        }
    }
}

void AntiAim::run(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{

    if (aaCfg.enabledLegit) {
        AntiAim::legit(cmd, previousViewAngles, currentViewAngles, sendPacket);

    }
    else if (aaCfg.enabledRage) {

        AntiAim::rage(cmd, previousViewAngles, currentViewAngles, sendPacket);
    }
}

static bool antiAimOpen = false;
static bool lantiAimOpen = false;

void AntiAim::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Anti aim")) {
        antiAimOpen = true;
        ImGui::SetWindowFocus("Anti aim");
        ImGui::SetWindowPos("Anti aim", { 100.0f, 100.0f });
    }
}

void AntiAim::menuBarItemL() noexcept
{
    if (ImGui::MenuItem("Legit Anti Aim")) {
        lantiAimOpen = true;
        ImGui::SetWindowFocus("Legit Anti Aim");
        ImGui::SetWindowPos("Legit Anti Aim", { 100.0f, 100.0f });
    }
}

void AntiAim::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!antiAimOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Anti aim", &antiAimOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    }
    ImGui::PushID("AA stuff");
    ImGui::Toggle("Enabled", &aaCfg.enabledRage);
    ImGui::SameLine();
    ImGui::PushID("arrs");
    if (ImGui::Button("Arrows"))
        ImGui::OpenPopup("arrowsvis");

    if (ImGui::BeginPopup("arrowsvis")) {
        ImGuiCustom::colorPicker("Real Angle", aaCfg.arrows.realArrows);
        ImGuiCustom::colorPicker("Fake Angle", aaCfg.arrows.fakeArrows);
        ImGuiCustom::colorPicker("Invert Angle", aaCfg.arrows.angleArrows); ImGui::SameLine(); ImGui::PushItemWidth(80.f); ImGui::InputFloat("##posa", &aaCfg.arrows.posAngle);
        ImGui::Separator();
        ImGuiCustom::colorPicker("Override Yaw", aaCfg.arrows.overrideArrows); ImGui::SameLine(); ImGui::PushItemWidth(80.f); ImGui::InputFloat("##poso", &aaCfg.arrows.posOverride);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Toggle("Disable when Freeze time", &aaCfg.freezetime);
    ImGui::PushItemWidth(80.f);
    ImGui::Combo("Pitch", &aaCfg.pitchmode, 
        "Off\0Down\0Up\0Custom\0Fake Pitch\0"
    );
    ImGui::SameLine();
    ImGui::Combo("Yaw", &aaCfg.yawmode, "Backward\0Left\0Right\0Forward\0Lisp");
    if (aaCfg.pitchmode == 3) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::SliderFloat("##custom", &aaCfg.pitchcustom, -89.f, 89.f, "Custom: %.2f");
    }
    if (aaCfg.yawmode == 4) {
        ImGui::PushID("KeyWin");
        ImGui::SameLine();
        if (ImGui::Button("...")) {
            ImGui::OpenPopup("Keybinds");
        }
        if (ImGui::BeginPopup("Keybinds")) {
            ImGui::hotkey("Left", aaCfg.overrideleft);
            ImGui::hotkey("Down", aaCfg.overridedown);
            ImGui::hotkey("Right", aaCfg.overrideright);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::Combo("Mode", &aaCfg.aamode, "Static\0Jitter\0Spin\0");
    ImGui::SameLine();
    ImGui::Combo("Direction", &aaCfg.direction, "OFF\0At Targets\0Freestanding\0");
    ImGui::PopItemWidth();
    ImGui::PushID("aaMode");
    ImGui::PushItemWidth(200.0f);
    if (aaCfg.aamode == 1) {
        ImGui::SliderFloat("##min", &aaCfg.jittermin, -180.f, aaCfg.jittermax, "Min: %.2f");
        ImGui::SameLine();
        ImGui::SliderFloat("##max", &aaCfg.jittermax, aaCfg.jittermin, 180.f, "Max: %.2f");
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderFloat("##yawadd", &aaCfg.yawadd, -180.f, 180.f, "Yaw add: %.2f");
    ImGui::SameLine();
    ImGui::PushID("inverYaw");
    ImGui::hotkey("Invert Yaw", aaCfg.invertYaw);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &aaCfg.keyMode.Yaw, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::PushID("Lean ANTI");
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("##lean", &aaCfg.rollAngle, -45.f, 45.f, "Roll: %.2f");
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Values over 50 and -50 may untrust you!!!");

    ImGui::hotkey("Invert Key", aaCfg.invertRoll);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &aaCfg.keyMode.Roll, "Hold\0Toggle\0");
    ImGui::SameLine();
    ImGui::Toggle("Sync", &aaCfg.syncRollWithFakeAngles);
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Sync with Fake Angles");
    ImGui::PopID();
    ImGui::Toggle("Distortion", &aaCfg.dist.enabled);
    if (aaCfg.dist.enabled) {
        ImGui::PushItemWidth(100.0f);
        ImGui::SliderFloat("Speed", &aaCfg.dist.speed, 1.f, 100.f, "%1.fHz"); ImGui::SameLine();
        ImGui::SliderFloat("Amount", &aaCfg.dist.amount, 1.f, 360.f, "%1.f");
        ImGuiCustom::multiCombo("Triggers", aaCfg.dist.triggerOn, "On Stand\0On SlowWalk\0On Run\0On Duck\0On Air\0");
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::hotkey("Fake Flick", aaCfg.flick.keybindEnabled);
    ImGui::hotkey("Fake Flick Flip", aaCfg.flick.keybindInvert);
    ImGui::SetNextItemWidth(80.0f);
    ImGui::SliderInt("##flick", &aaCfg.flick.rate, 1, 64, "Rate: %i");

    ImGui::Separator();//FAKE ANGLE

    ImGui::PushID("fakeangel");
    ImGui::Toggle("Enabled", &aaCfg.enabledAngle);
    ImGui::SameLine();
    ImGui::PushID("invertangle");
    ImGui::hotkey("Invert Angle", aaCfg.invertangle);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &aaCfg.keyMode.Angle, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::PushItemWidth(100.0f);
    ImGui::SliderInt("##leftlimit", &aaCfg.leftLimit, 0, 60, "Left limit: %d");
    ImGui::SameLine();
    ImGui::SliderInt("##rightlimit", &aaCfg.rightLimit, 0, 60, "Right limit: %d");
    ImGui::Combo("Mode", &aaCfg.peekMode, "Off\0Peek real\0Peek fake\0Jitter\0");
    ImGui::SameLine();
    ImGui::Combo("Lby mode", &aaCfg.lbyMode, "Micromovement\0Opposite\0Sway\0Fake Desync\0");
    ImGui::PopItemWidth();
    ImGui::PopID();

    if (!contentOnly)
        ImGui::End();
}

void AntiAim::drawLegitGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!lantiAimOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Legit Anti Aim", &lantiAimOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    ImGui::Toggle("Enabled", &aaCfg.enabledLegit);
    ImGui::Toggle("Disable when Freeze time", &aaCfg.freezetime);
    ImGui::Toggle("Extend", &aaCfg.extendedLegit);
    ImGui::hotkey("Invert Key", aaCfg.invertlegit);
    ImGui::SameLine();
    ImGui::PushID("key");
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &aaCfg.keyMode.Legit, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGuiCustom::colorPicker("Fake", aaCfg.arrows.legitArrows); ImGui::SameLine(); ImGui::PushItemWidth(80.f); ImGui::InputFloat("##posl", &aaCfg.arrows.posLegit);

    if (!contentOnly)
        ImGui::End();
}

static void to_json(json& j, const AntiAimConfig::Distort& o, const AntiAimConfig::Distort& dummy = {}) {
    WRITE("Enabled" ,enabled);
    WRITE("Speed" ,speed);
    WRITE("Amount" ,amount);
    WRITE("Trigger" ,triggerOn);
}

static void to_json(json& j, const AntiAimConfig::Arrows& o, const AntiAimConfig::Arrows& dummy = {}) {
    WRITE("Legit Arrow Width", posLegit);
    WRITE("Angle Arrow Width", posAngle);
    WRITE("Override Arrow Width", posOverride);
    WRITE("Legit Arrow", legitArrows);
    WRITE("Angle Arrow", angleArrows);
    WRITE("Real Arrow", realArrows);
    WRITE("Fake Arrow", fakeArrows);
    WRITE("Override Arrow", overrideArrows);
}

static void to_json(json& j, const AntiAimConfig::updateInputs& o, const AntiAimConfig::updateInputs& dummy = {}) {
    WRITE("Yaw", Yaw);
    WRITE("Angle", Angle);
    WRITE("Legit", Legit);
    WRITE("Roll", Roll);
}

static void to_json(json& j, const AntiAimConfig& o, const AntiAimConfig& dummy = {})
{
    WRITE("Enabled", enabledRage);
    WRITE("Enabled Angle", enabledAngle);
    WRITE("Enabled Legit", enabledLegit);
    WRITE("Freeze Time", freezetime);
    WRITE("Direction", direction);
    WRITE("Mode", aamode);
    WRITE("Yaw Mode", yawmode);
    WRITE("Pitch Mode", pitchmode);
    WRITE("Pitch Custom", pitchcustom);
    WRITE("Yaw Custom", yawadd);
    WRITE("Roll Angle", rollAngle);
    WRITE("Sync Roll Angle", syncRollWithFakeAngles);
    WRITE("Jitter Min", jittermin);
    WRITE("Jitter Max", jittermax);
    WRITE("Left Limit", leftLimit);
    WRITE("Right Limit", rightLimit);
    WRITE("Peek Mode", peekMode);
    WRITE("LbyMode", lbyMode);
    WRITE("Extend Legit", extendedLegit);
    WRITE("Invert Angle", invertangle);
    WRITE("Invert Angle Legit", invertlegit);
    WRITE("Invert Anti Aim", invertYaw);
    WRITE("Arrows", arrows);
    WRITE("Distortion", dist);
    WRITE("KeyModes", keyMode);
}

static void from_json(const json& j, AntiAimConfig::Distort& d){
    read(j, "Enabled", d.enabled);
    read(j, "Speed", d.speed);
    read(j, "Amount", d.amount);
    read(j, "Trigger", d.triggerOn);
}

static void from_json(const json& j, AntiAimConfig::Arrows& r)
{
    read(j, "Legit Arrow Width", r.posLegit);
    read(j, "Angle Arrow Width", r.posAngle);
    read(j, "Override Arrow Width", r.posOverride);
    read<value_t::object>(j, "Legit Arrow", r.legitArrows);
    read<value_t::object>(j, "Angle Arrow", r.angleArrows);
    read<value_t::object>(j, "Override Arrow", r.overrideArrows);
    read<value_t::object>(j, "Real Arrow", r.realArrows);
    read<value_t::object>(j, "Fake Arrow", r.fakeArrows);
}

static void from_json(const json& j, AntiAimConfig::updateInputs& km) {

    read(j, "Yaw", km.Yaw);
    read(j, "Angle", km.Angle);
    read(j, "Legit", km.Legit);
    read(j, "Roll", km.Roll);
}

static void from_json(const json& j, AntiAimConfig& a)
{
    read(j, "Enabled", a.enabledRage);
    read(j, "Enabled Angle", a.enabledAngle);
    read(j, "Enabled Legit", a.enabledLegit);
    read(j, "Freeze Time", a.freezetime);
    read(j, "Direction", a.direction);
    read(j, "Mode", a.aamode);
    read(j, "Yaw Mode", a.yawmode);
    read(j, "Pitch Mode", a.pitchmode);
    read(j, "Pitch Custom", a.pitchcustom);
    read(j, "Yaw Custom", a.yawadd);
    read(j, "Roll Angle", a.rollAngle);
    read(j, "Sync Roll Angle", a.syncRollWithFakeAngles);
    read(j, "Jitter Min", a.jittermin);
    read(j, "Jitter Max", a.jittermax);
    read(j, "Left Limit", a.leftLimit);
    read(j, "Right Limit", a.rightLimit);
    read(j, "Peek Mode", a.peekMode);
    read(j, "LbyMode", a.lbyMode);
    read(j, "Extend Legit", a.extendedLegit);
    read(j, "Invert Angle", a.invertangle);
    read(j, "Invert Angle Legit", a.invertlegit);
    read(j, "Invert Anti Aim", a.invertYaw);
    read<value_t::object>(j, "Arrows", a.arrows);
    read<value_t::object>(j, "Distortion", a.dist);
    read<value_t::object>(j, "KeyModes", a.keyMode);
}

json AntiAim::toJson() noexcept
{
    json j;
    to_json(j, aaCfg);
    return j;
}

void AntiAim::fromJson(const json& j) noexcept
{
    from_json(j, aaCfg);
}

void AntiAim::resetConfig() noexcept
{
    aaCfg = { };
}
