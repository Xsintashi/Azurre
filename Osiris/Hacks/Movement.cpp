#include "Movement.h"
#include "Misc.h"
#include "EnginePrediction.h"

#include "../SDK/Constants/ClassId.h"
#include "../SDK/Client.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ClientMode.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/Constants/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/Gamemode.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Input.h"
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Panorama.h"
#include "../SDK/Platform.h"
#include "../SDK/Prediction.h"
#include "../SDK/UserCmd.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/WeaponSystem.h"

#include "../InputUtil.h"
#include "../GUI.h"
#include "../GameData.h"
#include "../Config.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../imguiCustom.h"
#include "../ConfigStructs.h"

bool shouldEdgebug = false;
float zVelBackup = 0.0f;
float bugSpeed = 0.0f;
int edgebugButtons = 0;

#define max(a,b)            (((a) > (b)) ? (a) : (b))

struct MovementConfig {
    bool autoStrafe = false,
        strafeOptimizer = false,
        bunnyHop = false,
        jumpbug = false,
        duckjump = false,
        edgejump = false,
        reverse = false,
        fastDuck = false,
        fastStop = false,
        ladderlj = false,
        jumpstats = false,
        nulls = false,
        headBounce = false,
        autoAlign = false;

    int bhophitchance = 100,
        strafeMode = 0;

    KeyBind edgejumpkey;
    KeyBind jumpbugKey;
    KeyBind ladderljKey;
    KeyBind headBounceKey;
    KeyBind backwards;

    struct JumpStats {
        bool enabled = false;
        bool showFails = true;
        bool showColorOnFail = false;
        bool simplifyNaming = false;
    } jumpStats;

    struct PixelSurf {
        bool enabled{ false };
        KeyBind autoPixelSurfKey{KeyBind::NONE};
        int autoPixelSurfPredAmnt{ 2 };
    }px;

    struct MiniJump {
        bool enabled{ false };
        KeyBind miniJumpKey{ KeyBind::NONE };
    }mj;

    struct EdgeBug {
        bool enabled = false,
            detect = false,
            playSound = true,
            screeneffect = true,
            print = true;
        int sound{ 0 }, predAmnt{ 20 };
        KeyBind edgeBugKey;
    } edgeBug;

    struct VelocityWindow {
        bool enabled = false, plotWin = false;
        ImVec2 pos2;

        float position{ 0.9f };
        float alpha{ 1.0f };
        ColorToggle color{ 1.0f, 1.0f, 1.0f, 1.0f };
    }velocityWindow;

    struct Strokes {
        bool enabled = false;
        float position{ 0.8f };
        bool showKeyTiles = false;
        Color4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
    }strokesWindow;

    struct Mouse {
        bool noBackground = false, noTitleBar = false;
        ColorToggle3 enabled;
        ImVec2 pos;
    }mouseWindow;

    struct Checkpoints{
        bool enabled = false, noTitleBar = false, noBackground = false;
        ImVec2 pos;

        KeyBind save, load;
    }checkpoints;

} movementConfig;

bool bugou = false;

bool Movement::isBackwardsPressed() noexcept
{
    return movementConfig.backwards.isPressed();
}

bool Movement::isJumpBugKeyDown() noexcept
{
    if (!movementConfig.jumpbug) return false;
    return movementConfig.jumpbugKey.isDown();
}

bool Movement::isEdgeBugKeyDown() noexcept
{
    if (!movementConfig.edgeBug.enabled || !localPlayer) return false;
    return movementConfig.edgeBug.edgeBugKey.isDown();
}

bool Movement::isMiniJumpKeyDown() noexcept
{
    if (!movementConfig.mj.enabled || !localPlayer) return false;
    return movementConfig.mj.miniJumpKey.isDown();
}

bool Movement::isAutoPXKeyDown() noexcept
{
    if (!movementConfig.px.enabled || !localPlayer) return false;
    return movementConfig.px.autoPixelSurfKey.isDown();
}

bool Movement::isEdgeJumpKeyDown() noexcept
{
    if (!movementConfig.edgejump) return false;
    return movementConfig.edgejumpkey.isDown();
}

bool Movement::isLadderJumpKeyDown() noexcept
{
    if (!movementConfig.ladderlj) return false;
    return movementConfig.ladderljKey.isDown();
}

void Movement::edgeJump(UserCmd* cmd) noexcept
{
    if (!movementConfig.edgejump || !movementConfig.edgejumpkey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    if ((EnginePrediction::getFlags() & 1) && !(localPlayer->flags() & 1)) {
        cmd->buttons |= UserCmd::IN_JUMP;
        cmd->buttons &= ~UserCmd::IN_FORWARD;
    }
}

void Movement::ladderJump(UserCmd* cmd) noexcept{

    if (movementConfig.ladderlj && !movementConfig.ladderljKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    static int savedTick;

    const auto mtPred = EnginePrediction::getMoveType();
    const auto mtPost = localPlayer->moveType();

    if (mtPred == MoveType::LADDER) {
        if (mtPost != MoveType::LADDER) {
            savedTick = memory->globalVars->tickCount;
            cmd->buttons |= UserCmd::IN_JUMP;
            cmd->forwardmove = 0.f;
            cmd->sidemove = 0.f;
            cmd->buttons = cmd->buttons & ~(UserCmd::IN_FORWARD| UserCmd::IN_BACK | UserCmd::IN_MOVERIGHT | UserCmd::IN_MOVELEFT);
        }
    }

    if (memory->globalVars->tickCount - savedTick > 1 && memory->globalVars->tickCount - savedTick < 15) {
        cmd->forwardmove = 0.f;
        cmd->sidemove = 0.f;
        cmd->buttons = cmd->buttons & ~(UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVERIGHT | UserCmd::IN_MOVELEFT);
        cmd->buttons |= UserCmd::IN_DUCK;
    }
}

void Movement::removeCrouchCooldown(UserCmd* cmd) noexcept
{
    if (const auto gameRules = (*memory->gameRules); gameRules)
        if (Misc::getGameMode() != GameMode::Competitive && gameRules->isValveDS())
            return;

    if (movementConfig.fastDuck)
        cmd->buttons |= UserCmd::IN_BULLRUSH;
}

void Movement::fastStop(UserCmd* cmd) noexcept
{
    if (!movementConfig.fastStop)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER || !localPlayer->isOnGround() || cmd->buttons & UserCmd::IN_JUMP)
        return;

    if (cmd->buttons & (UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT | UserCmd::IN_FORWARD | UserCmd::IN_BACK))
        return;

    const auto velocity = localPlayer->velocity();
    const auto speed = velocity.length2D();
    if (speed < 15.0f)
        return;

    Vector direction = velocity.toAngle();
    direction.y = cmd->viewangles.y - direction.y;

    const auto negatedDirection = Vector::fromAngle(direction) * -speed;
    cmd->forwardmove = negatedDirection.x;
    cmd->sidemove = negatedDirection.y;
}

void Movement::bunnyHop(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    if (bugou)
        return;

    int hopsRestricted = 0,
        hopsHit = 0;
    static auto wasLastTimeOnGround{ localPlayer->isOnGround() };

    if (movementConfig.bunnyHop && !localPlayer->isOnGround() && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround) {
        cmd->buttons &= ~UserCmd::IN_JUMP;
    }
    else if ((rand() % 100 > movementConfig.bhophitchance && hopsRestricted < 12)) {
        cmd->buttons &= ~UserCmd::IN_JUMP;
        hopsRestricted++;
        hopsHit = 0;
    }
    else hopsHit++;

    wasLastTimeOnGround = localPlayer->isOnGround();
}

void Movement::duckWhenJump(UserCmd* cmd) noexcept
{
    if (!localPlayer || !movementConfig.duckjump) {
        return;
    }

    if (!localPlayer->isOnGround()) {
        cmd->buttons |= UserCmd::IN_DUCK;
    }
}

void Movement::autoStrafe(UserCmd* cmd, Vector& currentViewAngles) noexcept
{
    if (!movementConfig.autoStrafe) return;


    if (!localPlayer || !localPlayer->isAlive())
        return;

    auto vel = localPlayer->velocity().length2D();

    if (vel < 5.f) return;

    if (!movementConfig.strafeMode) {
        if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP) return;
        if (!localPlayer->isOnGround()) {
            if (cmd->mousedx < 0)
                cmd->sidemove = -450.0f;
            else if (cmd->mousedx > 0)
                cmd->sidemove = 450.0f;
        }
    }
    else {
        static float angle = 0.f;

        const bool back = cmd->buttons & UserCmd::IN_BACK;
        const bool forward = cmd->buttons & UserCmd::IN_FORWARD;
        const bool right = cmd->buttons & UserCmd::IN_MOVERIGHT;
        const bool left = cmd->buttons & UserCmd::IN_MOVELEFT;
        if (back) {
            angle = -180.f;
            if (left)
                angle -= 45.f;
            else if (right)
                angle += 45.f;
        }
        else if (left) {
            angle = 90.f;
            if (back)
                angle += 45.f;
            else if (forward)
                angle -= 45.f;
        }
        else if (right) {
            angle = -90.f;
            if (back)
                angle -= 45.f;
            else if (forward)
                angle += 45.f;
        }
        else {
            angle = 0.f;
        }

        //If we are on ground, noclip or in a ladder return
        if ((EnginePrediction::getFlags() & 1) || localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER)
            return;

        currentViewAngles.y += angle;

        cmd->forwardmove = 0.f;
        cmd->sidemove = 0.f;
        const auto delta = Helpers::normalizeYaw(currentViewAngles.y - Helpers::rad2deg(std::atan2(EnginePrediction::getVelocity().y, EnginePrediction::getVelocity().x)));
        cmd->sidemove = delta > 0.f ? -450.f : 450.f;
        currentViewAngles.y = Helpers::normalizeYaw(currentViewAngles.y - delta);
    }
}

void Movement::NullStrafe(UserCmd* cmd)
{
    if (!movementConfig.nulls || !localPlayer ||
        localPlayer->isOnGround())
        return;

    if (cmd->mousedx > 0 && cmd->buttons & UserCmd::IN_MOVERIGHT && cmd->buttons & UserCmd::IN_MOVELEFT)
        cmd->sidemove = -450.f;
    else if (cmd->mousedx < 0 && cmd->buttons & UserCmd::IN_MOVELEFT && cmd->buttons & UserCmd::IN_MOVERIGHT)
        cmd->sidemove = 450.f;
}

void Movement::jumpBug(UserCmd* cmd) noexcept {

    bool assist = false;

    bugou = false;

    if (!movementConfig.jumpbug || !movementConfig.jumpbugKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (assist)
    {
        if (localPlayer->isOnGround())
            cmd->buttons |= UserCmd::IN_DUCK;

    }

    bugou = true;
    if (localPlayer->isOnGround())
    {
        cmd->buttons &= ~UserCmd::IN_JUMP;
        if (!(EnginePrediction::getFlags() & 1)){
            cmd->buttons |= UserCmd::IN_DUCK;
        }
        assist = true;
    }
}

void Movement::miniJump(UserCmd* cmd) noexcept
{
    if (!movementConfig.mj.enabled || (movementConfig.mj.miniJumpKey.isSet() && !movementConfig.mj.miniJumpKey.isDown()))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if ((EnginePrediction::getFlags() & 1) && !(localPlayer->flags() & 1))
    {
        cmd->buttons |= UserCmd::IN_JUMP;
        cmd->buttons |= UserCmd::IN_DUCK;
    }
}

void Movement::autoPixelSurf(UserCmd* cmd) noexcept
{
    if (!movementConfig.px.enabled || (movementConfig.px.autoPixelSurfKey.isSet() && !movementConfig.px.autoPixelSurfKey.isDown()))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (EnginePrediction::getFlags() & 1)
        return;

    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER)
        return;

    //Restore before prediction
    memory->restoreEntityToPredictedFrame(0, interfaces->prediction->split->commandsPredicted - 1);

    bool detectedPixelSurf = false;

    for (int i = 0; i < movementConfig.px.autoPixelSurfPredAmnt; i++)
    {
        auto backupButtons = cmd->buttons;
        cmd->buttons |= UserCmd::IN_DUCK;

        EnginePrediction::run(cmd);

        cmd->buttons = backupButtons;

        detectedPixelSurf = (localPlayer->velocity().z == -6.25f || localPlayer->velocity().z == -6.f || localPlayer->velocity().z == -3.125) && !(localPlayer->flags() & 1) && localPlayer->moveType() != MoveType::LADDER;

        if (detectedPixelSurf)
            break;
    }

    if (detectedPixelSurf)
    {
        cmd->buttons |= UserCmd::IN_DUCK;
    }

    memory->restoreEntityToPredictedFrame(0, interfaces->prediction->split->commandsPredicted - 1);
    EnginePrediction::run(cmd);
}

void Movement::headBounce(UserCmd* cmd) noexcept {

    if (!movementConfig.headBounce)
        return;

    if (movementConfig.headBounceKey.isSet() && !movementConfig.headBounceKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive() || localPlayer->isOnGround())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    Vector wishdir;
    bool done = false;

    if ((cmd->buttons & UserCmd::IN_FORWARD) && !(cmd->buttons & UserCmd::IN_BACK) && !(cmd->buttons & UserCmd::IN_MOVELEFT) && !(cmd->buttons & UserCmd::IN_MOVERIGHT)) {
        wishdir = { std::cos(Helpers::deg2rad(cmd->viewangles.y)) * 17.f, std::sin(Helpers::deg2rad(cmd->viewangles.y)) * 17.f, 0.f };
        done = true;
    }
    if ((cmd->buttons & UserCmd::IN_BACK) && !(cmd->buttons & UserCmd::IN_FORWARD) && !(cmd->buttons & UserCmd::IN_MOVELEFT) && !(cmd->buttons & UserCmd::IN_MOVERIGHT)) {
        wishdir = { std::cos(Helpers::deg2rad(cmd->viewangles.y + 180.f)) * 17.f, std::sin(Helpers::deg2rad(cmd->viewangles.y + 180.f)) * 17.f, 0.f };
        done = true;
    }
    if ((cmd->buttons & UserCmd::IN_MOVELEFT) && !(cmd->buttons & UserCmd::IN_BACK) && !(cmd->buttons & UserCmd::IN_FORWARD) && !(cmd->buttons & UserCmd::IN_MOVERIGHT)) {
        wishdir = { std::cos(Helpers::deg2rad(cmd->viewangles.y + 90.f)) * 17.f, std::sin(Helpers::deg2rad(cmd->viewangles.y + 90.f)) * 17.f, 0.f };
        done = true;
    }
    if ((cmd->buttons & UserCmd::IN_MOVERIGHT) && !(cmd->buttons & UserCmd::IN_BACK) && !(cmd->buttons & UserCmd::IN_MOVELEFT) && !(cmd->buttons & UserCmd::IN_FORWARD)) {
        wishdir = { std::cos(Helpers::deg2rad(cmd->viewangles.y - 90.f)) * 17.f, std::sin(Helpers::deg2rad(cmd->viewangles.y - 90.f)) * 17.f, 0.f };
        done = true;
    }

    if (!done)
        return;

    const auto startPos = localPlayer->getAbsOrigin();
    const auto endPos = startPos + wishdir;

    Trace trace;
    interfaces->engineTrace->traceRay({ startPos, endPos }, MASK_PLAYERSOLID, localPlayer.get(), trace);

    if ((trace.fraction < 1.f) && (trace.plane.normal.z == 0.f)) {
        Vector Angles{ trace.plane.normal.x * -16.005f, trace.plane.normal.y * -16.005f, 0.f };
        const auto endPos2 = startPos + Angles;
        Trace trace2;
        interfaces->engineTrace->traceRay({ startPos, endPos2 }, MASK_PLAYERSOLID, localPlayer.get(), trace2);
        if (trace.plane != trace2.plane) {
            Vector to_wall = Angles.toAngle();
            float mVel = hypotf(localPlayer->velocity().x, localPlayer->velocity().y);
            float ideal = Helpers::rad2deg(atanf(29.56f / mVel));
            Vector dvelo = localPlayer->velocity();
            dvelo.z = 0.f;
            Vector velo_angle = dvelo.toAngle();
            Vector delta = velo_angle - to_wall;
            delta.normalize();

            if (delta.y >= 0.f)
                to_wall.y += ideal;
            else
                to_wall.y -= ideal;

            float rotation = Helpers::deg2rad(to_wall.y - cmd->viewangles.y);
            float cos_rot = cos(rotation);
            float sin_rot = sin(rotation);

            float forwardmove = cos_rot * 450.f;
            float sidemove = -sin_rot * 450.f;
            cmd->forwardmove = forwardmove;
            cmd->sidemove = sidemove;
        }
    }
}


void autoAlignAdjustFSmove(UserCmd* cmd, Vector alignAngle) // TODO: MAKE THIS EASY TO READ
{
    Vector align_vector_forward = { 0.f,0.f,0.f }, align_vector_right = { 0.f,0.f,0.f }, align_vector_up = { 0.f,0.f,0.f };
    Vector::fromAngleAll(alignAngle, &align_vector_forward, &align_vector_right, &align_vector_up);
    float nor_length2d_alignvecforward = 1.f / (align_vector_forward.length2D() + FLT_EPSILON); // normalized length2d of forward align vector
    float nor_length2d_alignvecright = 1.f / (align_vector_right.length2D() + FLT_EPSILON); // normalized length2d of forward align vector
    float up_vecz_alignAngle = align_vector_up.z;

    Vector va_vector_forward = { 0.f,0.f,0.f }, va_vector_right = { 0.f,0.f,0.f }, va_vector_up = { 0.f,0.f,0.f };
    Vector::fromAngleAll(cmd->viewangles, &va_vector_forward, &va_vector_right, &va_vector_up);
    //do tha shit with forward
    float nor_length2d_vavecforward = 1.f / (va_vector_forward.length2D() + FLT_EPSILON);
    float v35 = nor_length2d_vavecforward * va_vector_forward.x;
    float v32 = nor_length2d_vavecforward * va_vector_forward.y;
    //do tha shit with right
    float nor_length2d_vavecright = 1.f / (va_vector_right.length2D() + FLT_EPSILON);
    float v34 = nor_length2d_vavecright * va_vector_right.x;
    float v39 = nor_length2d_vavecright * va_vector_right.y;
    //do tha shit with up
    float nor_length2d_vavecup = 1.f / (va_vector_up.z + FLT_EPSILON); // this is v27

    //save fmove, smove
    float saved_fmove = cmd->forwardmove;
    float saved_smove = cmd->sidemove;

    //calculate the desired fmove, smove, umove

    float modified_smove_by_y = saved_smove * nor_length2d_alignvecright * align_vector_right.y; // this is v2
    float modified_smove_by_x = saved_smove * nor_length2d_alignvecright * align_vector_right.x; // this is v45.m128i_i64[1]

    float modified_fmove_by_y = saved_fmove * nor_length2d_alignvecforward * align_vector_forward.y; // this is v29.m128i_i64[1]
    float modified_fmove_by_x = saved_fmove * nor_length2d_alignvecforward * align_vector_forward.x; // this is v30.m128i_i64[1]

    float modified_umove_by_z = cmd->upmove * (1.f / (up_vecz_alignAngle + FLT_EPSILON)) * up_vecz_alignAngle;

    //further calculation

    float calculated_forwardmove = v35 * modified_fmove_by_x + v35 * modified_smove_by_x + v32 * modified_fmove_by_y + v32 * modified_fmove_by_y;
    float calculated_sidemove = v39 * modified_smove_by_y + v34 * modified_smove_by_x + v34 * modified_fmove_by_x + v39 * modified_fmove_by_y;

    //TODO: DETERMINE WHEN TO ACTUALLY SET THIS

    if ((alignAngle.y == 90.f || alignAngle.y == -90.f) && fabsf(saved_smove) > 0.01f)
    {
        cmd->forwardmove = 450.f + calculated_forwardmove;
    }
    else
    {
        cmd->forwardmove = calculated_forwardmove;
    }

    //cmd->forwardmove = calculated_forwardmove

    if ((alignAngle.y == 0.f || alignAngle.y == 180.f) && fabsf(saved_fmove) > 0.01f)
    {
        cmd->sidemove = 450.f + calculated_sidemove;
    }
    else
    {
        cmd->sidemove = calculated_sidemove;
    }

    cmd->forwardmove = std::clamp(cmd->forwardmove, -450.f, 450.f);
    cmd->sidemove = std::clamp(cmd->sidemove, -450.f, 450.f);
}

std::array<std::pair<bool, Vector>, 4> points;

std::pair<int, Vector> getAlignSide()
{
    const Vector origin = localPlayer->origin();
    const float oneMinusXfloor = 1.f - (origin.x - floor(origin.x));
    const float oneMinusYfloor = 1.f - (origin.y - floor(origin.y));
    const float floorX = origin.x - floor(origin.x);
    const float floorY = origin.y - floor(origin.y);

    Vector mins = localPlayer->mins();
    Vector maxs = localPlayer->maxs();

    Trace info[4];

    for (int i = 0; i < 4; i++)
    {
        Vector startpos = origin;
        Vector endpos = origin;
        switch (i)
        {
        case 0:
            startpos.y += mins.y; // adding mins.y so were on the edge of bbox
            endpos.y = floor(startpos.y); // flooring so we trace to the nearest whole hammer unit
            points.at(i).second = origin + Vector{ mins.x, mins.y, 0.f };
            break;
        case 1:
            startpos.x += maxs.x;
            endpos.x = floor(startpos.x) + 1.f;
            points.at(i).second = origin + Vector{ maxs.x, mins.y, 0.f };
            break;
        case 2:
            startpos.y += maxs.y;
            endpos.y = floor(startpos.y) + 1.f; //flooring y and adding 1 so we trace to opposite side
            points.at(i).second = origin + Vector{ maxs.x, maxs.y, 0.f };
            break;
        case 3: //flooring x
            startpos.x += mins.x;
            endpos.x = floor(startpos.x); // negative x from origin
            points.at(i).second = origin + Vector{ mins.x, maxs.y, 0.f };
            break;
        }

        Trace tr;
        interfaces->engineTrace->traceRay({ origin, endpos }, MASK_PLAYERSOLID, localPlayer.get(), tr);
        info[i] = tr;
    }

    float min_frac = 1.f;
    int bestind = -1;
    for (int i = 0; i < 4; i++)
    {
        auto& tr = info[i];
        if ((tr.fraction < 1.f || tr.allSolid || tr.startSolid) && (tr.entity ? !tr.entity->isPlayer() : true))
        {
            min_frac = tr.fraction;
            bestind = i;
            switch (i)
            {
            case 0:
                if (floorY < 0.03125f)
                    points.at(i).first = true;
                else
                    points.at(i).first = false;
                break;

            case 1:
                if (oneMinusXfloor < 0.03125f)
                    points.at(i).first = true;
                else
                    points.at(i).first = false;
                break;
            case 2:
                if (oneMinusYfloor < 0.03125f)
                    points.at(i).first = true;
                else
                    points.at(i).first = false;
                break;
            case 3:
                if (floorX < 0.03125f)
                    points.at(i).first = true;
                else
                    points.at(i).first = false;
                break;
            }
        }
        else
        {
            points.at(i).first = false;
        }
    }

    if (bestind != -1)
    {
        return { bestind, info[bestind].endpos };
    }


    return { bestind, {0,0,0} };
}

void Movement::autoAlign(UserCmd* cmd) noexcept {

    if (!movementConfig.autoAlign)
        return;

    if (!localPlayer || !localPlayer->isAlive() || (localPlayer->flags() & 1))
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    const Vector origin = localPlayer->origin();
    const float oneMinusXfloor = 1.f - (origin.x - floor(origin.x));
    const float oneMinusYfloor = 1.f - (origin.y - floor(origin.y));
    const float floorX = origin.x - floor(origin.x);
    const float floorY = origin.y - floor(origin.y);
    const float forwardMove = cmd->forwardmove;
    const float sideMove = cmd->sidemove;

    int alignside;

    auto alignRes = getAlignSide();

    alignside = alignRes.first;

    if ((alignside == 0 && floorY >= 0.03125) || (alignside == 1 && oneMinusXfloor >= 0.03125) || (alignside == 2 && oneMinusYfloor >= 0.03125) || (alignside == 3 && floorX >= 0.03125))
    {
        Vector orig = cmd->viewangles;

        float yAngle = 0;

        switch (alignside)
        {
        case 0:
            yAngle = -90.f;
            break;
        case 1:
            yAngle = 0.f;
            break;
        case 2:
            yAngle = 90.f;
            break;
        case 3:
            yAngle = 180.f;
            break;
        }

        Vector angleDiff = { cmd->viewangles.x, yAngle, 0.f };

        autoAlignAdjustFSmove(cmd, angleDiff);
    }
}

void Movement::edgeBug(UserCmd* cmd, Vector& angView) noexcept {
    
    if (!movementConfig.edgeBug.enabled || !movementConfig.edgeBug.edgeBugKey.isDown())
        return;
    
    if (!localPlayer || !localPlayer->isAlive())
        return;
    
    if (localPlayer->flags() & 1)
        return;
    
    shouldEdgebug = zVelBackup < -bugSpeed && round(localPlayer->velocity().z) == -round(bugSpeed) && localPlayer->moveType() != MoveType::LADDER;
    if (shouldEdgebug)
        return;

    const int commandsPredicted = interfaces->prediction->split->commandsPredicted;

    const Vector angViewOriginal = angView;
    const Vector angCmdViewOriginal = cmd->viewangles;
    const int buttonsOriginal = cmd->buttons;
    Vector vecMoveOriginal;
    vecMoveOriginal.x = cmd->sidemove;
    vecMoveOriginal.y = cmd->forwardmove;
    
    static Vector vecMoveLastStrafe;
    static Vector angViewLastStrafe;
    static Vector angViewOld = angView;
    static Vector angViewDeltaStrafe;
    static bool appliedStrafeLast = false;
    if (!appliedStrafeLast)
    {
        angViewLastStrafe = angView;
        vecMoveLastStrafe = vecMoveOriginal;
        angViewDeltaStrafe = (angView - angViewOld);
        angViewDeltaStrafe;
    }
    appliedStrafeLast = false;
    angViewOld = angView;
    
    for (int t = 0; t < 4; t++)
    {
        static int lastType = 0;
        if (lastType)
        {
            t = lastType;
            lastType = 0;
        }
        memory->restoreEntityToPredictedFrame(0, commandsPredicted - 1);
        if (buttonsOriginal& UserCmd::IN_DUCK&& t < 2)
            t = 2;
        bool applyStrafe = !(t % 2);
        bool applyDuck = t > 1;
    
        cmd->viewangles = angViewLastStrafe;
        cmd->buttons = buttonsOriginal;
        cmd->sidemove = vecMoveLastStrafe.x;
        cmd->forwardmove = vecMoveLastStrafe.y;
    
        for (int i = 0; i < movementConfig.edgeBug.predAmnt; i++)
        {
            if (applyDuck)
                cmd->buttons |= UserCmd::IN_DUCK;
            else
                cmd->buttons &= ~UserCmd::IN_DUCK;
            if (applyStrafe)
            {
                cmd->viewangles += angViewDeltaStrafe;
                cmd->viewangles.normalize();
                cmd->viewangles.clamp();
            }
            else
            {
                cmd->sidemove = 0.f;
                cmd->forwardmove = 0.f;
            }
            EnginePrediction::run(cmd);
            shouldEdgebug = zVelBackup < -bugSpeed && round(localPlayer->velocity().z) == -round(bugSpeed) && localPlayer->moveType() != MoveType::LADDER;
            zVelBackup = localPlayer->velocity().z;
            if (shouldEdgebug)
            {
                edgebugButtons = cmd->buttons;
                if (applyStrafe)
                {
                    appliedStrafeLast = true;
                    angView = (angViewLastStrafe + angViewDeltaStrafe);
                    angView.normalize();
                    angView.clamp();
                    angViewLastStrafe = angView;
                    cmd->sidemove = vecMoveLastStrafe.x;
                    cmd->forwardmove = vecMoveLastStrafe.y;
                }
                cmd->viewangles = angCmdViewOriginal;
                lastType = t;
                memory->restoreEntityToPredictedFrame(0, interfaces->prediction->split->commandsPredicted - 1);
                EnginePrediction::run(cmd);
                return;
            }
    
            if (localPlayer->flags() & 1 || localPlayer->moveType() == MoveType::LADDER)
                break;
        }
    }
    cmd->viewangles = angCmdViewOriginal;
    angView = angViewOriginal;
    cmd->buttons = buttonsOriginal;
    cmd->sidemove = vecMoveOriginal.x;
    cmd->forwardmove = vecMoveOriginal.y;

    memory->restoreEntityToPredictedFrame(0, interfaces->prediction->split->commandsPredicted - 1);
    EnginePrediction::run(cmd);
}

void Movement::prePrediction(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    zVelBackup = localPlayer->velocity().z;

    if (shouldEdgebug)
        cmd->buttons = edgebugButtons;

    static auto gravity = interfaces->cvar->findVar("sv_gravity");
    bugSpeed = (gravity->getFloat() * 0.5f * memory->globalVars->intervalPerTick);

    shouldEdgebug = zVelBackup < -bugSpeed && round(localPlayer->velocity().z) == -round(bugSpeed) && localPlayer->moveType() != MoveType::LADDER;
}

void Movement::EdgebugDetect(float unpredictedVelocity) noexcept
{
    if (!movementConfig.edgeBug.detect) return;

    if (!localPlayer || !localPlayer->isAlive() || localPlayer->moveType() == MoveType::LADDER || localPlayer->moveType() == MoveType::NOCLIP)
        return;

    auto predictedVelocity = std::floor(localPlayer->velocity().z);
    static float cooldown = 0.f;

    constexpr std::array sounds{
    "play ui/beep07",
    "play buttons/arena_switch_press_02",
    "play training/timer_bell",
    "play ui/counter_beep",
    "play ui/deathnotice",
    "play survival/money_collect_01.wav"
    };

    if (unpredictedVelocity >= -7.f || predictedVelocity != -7.f || localPlayer->isOnGround())
        return;

    if (memory->globalVars->currenttime > cooldown) {
        cooldown = memory->globalVars->currenttime + 0.5f;
        if (movementConfig.edgeBug.playSound) interfaces->engine->clientCmdUnrestricted(sounds[movementConfig.edgeBug.sound]);
        if (movementConfig.edgeBug.screeneffect) localPlayer->healthShotBoostExpirationTime() = memory->globalVars->currenttime + 0.5f;
        if (movementConfig.edgeBug.print) Helpers::CheatChatNotification(false, "edgebugged");
    }
}

//JUMP STATS

class JumpStatsCalculations
{
private:
    static const auto white = '\x01';
    static const auto violet = '\x03';
    static const auto green = '\x04';
    static const auto red = '\x07';
    static const auto golden = '\x09';
public:
    void resetStats() noexcept
    {
        units = 0.0f;
        strafes = 0;
        pre = 0.0f;
        maxVelocity = 0.0f;
        maxHeight = 0.0f;
        jumps = 0;
        bhops = 0;
        sync = 0.0f;
        startPosition = Vector{ };
        landingPosition = Vector{ };
    }

    bool show() noexcept
    {
        if (!onGround || jumping || jumpbugged)
            return false;

        if (!shouldShow)
            return false;

        units = (startPosition - landingPosition).length2D() + (isLadderJump ? 0.0f : 32.0f);

        const float z = fabsf(startPosition.z - landingPosition.z) - (isJumpbug ? 9.0f : 0.0f);
        const bool fail = z >= (isLadderJump ? 32.0f : (jumps > 0 ? (jumps > 1 ? 46.0f : 2.0f) : 46.0f));
        const bool simplifyNames = movementConfig.jumpStats.simplifyNaming;

        std::string jump = "null";

        //Values taken from
        //https://github.com/KZGlobalTeam/gokz/blob/33a3a49bc7a0e336e71c7f59c14d26de4db62957/cfg/sourcemod/gokz/gokz-jumpstats-tiers.cfg
        auto color = white;
        switch (jumps)
        {
        case 1:
            if (!isJumpbug)
            {
                jump = simplifyNames ? "LJ" : "Long jump";
                if (units < 230.0f)
                    color = white;
                else if (units >= 235.0f && units < 240.0f)
                    color = violet;
                else if (units >= 240.0f && units < 245.0f)
                    color = green;
                else if (units >= 245.0f && units < 248.0f)
                    color = red;
                else if (units >= 248.0f)
                    color = golden;
            }
            else
            {
                jump = simplifyNames ? "JB" : "Jump bug";
                if (units < 260.0f)
                    color = white;
                else if (units >= 260.0f && units < 265.0f)
                    color = violet;
                else if (units >= 265.0f && units < 270.0f)
                    color = green;
                else if (units >= 270.0f && units < 273.0f)
                    color = red;
                else if (units >= 273.0f)
                    color = golden;
            }
            break;
        case 2:
            jump = simplifyNames ? "BH" : "Bunnyhop";
            if (units < 230.0f)
                color = white;
            else if (units >= 230.0f && units < 233.0f)
                color = violet;
            else if (units >= 233.0f && units < 235.0f)
                color = green;
            else if (units >= 235.0f && units < 240.0f)
                color = red;
            else if (units >= 240.0f)
                color = golden;
            break;
        default:
            if (jumps >= 3)
            {
                jump = simplifyNames ? "MBH" : "Multi Bunnyhop";
                if (units < 230.0f)
                    color = white;
                else if (units >= 230.0f && units < 233.0f)
                    color = violet;
                else if (units >= 233.0f && units < 235.0f)
                    color = green;
                else if (units >= 235.0f && units < 240.0f)
                    color = red;
                else if (units >= 240.0f)
                    color = golden;
            }
            break;
        }

        if (isLadderJump)
        {
            jump = simplifyNames ? "LAJ" : "Ladder jump";
            if (units < 80.0f)
                color = white;
            else if (units >= 80.0f && units < 90.0f)
                color = violet;
            else if (units >= 90.0f && units < 105.0f)
                color = green;
            else if (units >= 105.0f && units < 109.0f)
                color = red;
            else if (units >= 109.0f)
                color = golden;
        }

        if (!movementConfig.jumpStats.showColorOnFail && fail)
            color = white;

        if (fail)
            jump += simplifyNames ? "-F" : " Failed";

        const bool show = (isLadderJump ? units >= 50.0f : units >= 186.0f) && (!(!movementConfig.jumpStats.showFails && fail) || (movementConfig.jumpStats.showFails));
        const char* prefixColor = Helpers::TextColors(config->style.prefixColor);
        if (show && movementConfig.jumpStats.enabled)
        {
            //Certain characters are censured on printf
            if (jumps > 2)
                memory->clientMode->getHudChat()->printf(0,
                    " %sAzurre \x08| %c%s: %.2f units \x01[\x05%d\x01 Strafes | \x05%.0f\x01 Pre | \x05%.0f\x01 Max | \x05%.1f\x01 Height | \x05%d\x01 Bhops | \x05%.0f\x01 Sync]",
                    prefixColor, color, jump.c_str(),
                    jumpStatsCalculations.units, jumpStatsCalculations.strafes, jumpStatsCalculations.pre, jumpStatsCalculations.maxVelocity, jumpStatsCalculations.maxHeight, jumpStatsCalculations.jumps, jumpStatsCalculations.sync);
            else
                memory->clientMode->getHudChat()->printf(0,
                    " %sAzurre \x08| %c%s: %.2f units \x01[\x05%d\x01 Strafes | \x05%.0f\x01 Pre | \x05%.0f\x01 Max | \x05%.1f\x01 Height | \x05%.0f\x01 Sync]",
                    prefixColor, color, jump.c_str(),
                    jumpStatsCalculations.units, jumpStatsCalculations.strafes, jumpStatsCalculations.pre, jumpStatsCalculations.maxVelocity, jumpStatsCalculations.maxHeight, jumpStatsCalculations.sync);
        }

        shouldShow = false;
        return true;
    }

    void run(UserCmd* cmd) noexcept
    {
        velocity = localPlayer->velocity().length2D();
        origin = localPlayer->getAbsOrigin();
        onGround = localPlayer->flags() & 1;
        onLadder = localPlayer->moveType() == MoveType::LADDER;
        jumping = cmd->buttons & UserCmd::IN_JUMP && !(lastButtons & UserCmd::IN_JUMP) && onGround;
        jumpbugged = !jumpped && hasJumped;

        //We jumped so we should show this jump
        if (jumping || jumpbugged)
            shouldShow = true;

        if (onLadder)
        {
            startPosition = origin;
            pre = velocity;
            startedOnLadder = true;
        }

        if (onGround)
        {
            if (!onLadder)
            {
                if (jumping)
                {
                    //We save pre velocity and the starting position
                    startPosition = origin;
                    pre = velocity;
                    jumps++;
                    startedOnLadder = false;
                    isLadderJump = false;
                }
                else
                {
                    landingPosition = origin;
                    //We reset our jumps after logging them, and incase we do log our jumps and need to reset anyways we do this
                    if (!shouldShow)
                        jumps = 0;

                    if (startedOnLadder)
                    {
                        isLadderJump = true;
                        shouldShow = true;
                    }
                    startedOnLadder = false;
                }
            }

            //Calculate sync
            if (ticksInAir > 0 && !jumping)
                sync = (static_cast<float>(ticksSynced) / static_cast<float>(ticksInAir)) * 100.0f;

            //Reset both counters used for calculating sync
            ticksInAir = 0;
            ticksSynced = 0;
        }
        else if (!onGround && !onLadder)
        {
            if (jumpbugged)
            {
                if (oldOrigin.notNull())
                    startPosition = oldOrigin;
                pre = oldVelocity;
                jumps = 1;
                isJumpbug = true;
                jumpbugged = false;
            }
            //Check for strafes
            if (cmd->mousedx != 0 && cmd->sidemove != 0.0f)
            {
                if (cmd->mousedx > 0 && lastMousedx <= 0.0f && cmd->sidemove > 0.0f)
                {
                    strafes++;
                }
                if (cmd->mousedx < 0 && lastMousedx >= 0.0f && cmd->sidemove < 0.0f)
                {
                    strafes++;
                }
            }

            //If we gain velocity, we gain more sync
            if (oldVelocity != 0.0f)
            {
                float deltaSpeed = velocity - oldVelocity;
                bool gained = deltaSpeed > 0.000001f;
                bool lost = deltaSpeed < -0.000001f;
                if (gained)
                {
                    ticksSynced++;
                }
            }

            //Get max height and max velocity
            maxHeight = max(fabsf(startPosition.z - origin.z), maxHeight);
            maxVelocity = max(velocity, maxVelocity);

            ticksInAir++; //We are in air
            sync = 0; //We dont calculate sync yet
        }

        lastMousedx = cmd->mousedx;
        lastOnGround = onGround;
        lastButtons = cmd->buttons;
        oldVelocity = velocity;
        oldOrigin = origin;
        jumpped = jumping;
        hasJumped = false;

        if (show())
            resetStats();

        if (onGround && !onLadder)
        {
            isJumpbug = false;
        }
        isLadderJump = false;
    }

    //Last values
    short lastMousedx{ 0 };
    bool lastOnGround{ false };
    int lastButtons{ 0 };
    float oldVelocity{ 0.0f };
    bool jumpped{ false };
    Vector oldOrigin{ };
    Vector startPosition{ };

    //Current values
    float velocity{ 0.0f };
    bool onLadder{ false };
    bool onGround{ false };
    bool jumping{ false };
    bool jumpbugged{ false };
    bool isJumpbug{ false };
    bool hasJumped{ false };
    bool startedOnLadder{ false };
    bool isLadderJump{ false };
    bool shouldShow{ false };
    int jumps{ 0 };
    Vector origin{ };
    Vector landingPosition{ };
    int ticksInAir{ 0 };
    int ticksSynced{ 0 };

    //Final values
    float units{ 0.0f };
    int strafes{ 0 };
    float pre{ 0.0f };
    float maxVelocity{ 0.0f };
    float maxHeight{ 0.0f };
    int bhops{ 0 };
    float sync{ 0.0f };
} jumpStatsCalculations;

void Movement::gotJump() noexcept
{
    jumpStatsCalculations.hasJumped = true;
}

void Movement::jumpStats(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    if (!localPlayer->isAlive())
    {
        jumpStatsCalculations = { };
        return;
    }

    static bool once = true;
    if ((!*memory->gameRules || (*memory->gameRules)->freezePeriod()) || localPlayer->flags() & (1 << 6))
    {
        if (once)
        {
            jumpStatsCalculations = { };
            once = false;
        }
        return;
    }

    jumpStatsCalculations.run(cmd);

    once = true;
}

//JUMP STATS

void drawVelWindow(ImDrawList* drawList) noexcept {

    const auto entity = localPlayer->isAlive() ? localPlayer.get() : localPlayer->getObserverTarget();
    if (!entity)
        return;

    int screenSizeX, screenSizeY;
    interfaces->engine->getScreenSize(screenSizeX, screenSizeY);
    const float Ypos = static_cast<float>(screenSizeY) * movementConfig.velocityWindow.position;

    static float colorTime = 0.f;
    static float takeOffTime = 0.f;

    static auto lastVelocity = 0;
    const auto velocity = static_cast<int>(round(entity->velocity().length2D()));

    static auto takeOffVelocity = 0;
    static bool lastOnGround = true;
    const bool onGround = entity->flags() & 1;
    if (lastOnGround && !onGround)
    {
        takeOffVelocity = velocity;
        takeOffTime = memory->globalVars->realtime + 2.f;
    }

    const bool shouldDrawTakeOff = !onGround || (takeOffTime > memory->globalVars->realtime);
    const std::string finalText = std::to_string(velocity);

    const Color4 trueColor = movementConfig.velocityWindow.color.enabled ? Color4{ movementConfig.velocityWindow.color.color[0], movementConfig.velocityWindow.color.color[1], movementConfig.velocityWindow.color.color[2], movementConfig.velocityWindow.alpha, movementConfig.velocityWindow.color.rainbowSpeed, movementConfig.velocityWindow.color.rainbow }
    : (velocity == lastVelocity ? Color4{ 1.0f, 0.78f, 0.34f, movementConfig.velocityWindow.alpha } : velocity < lastVelocity ? Color4{ 1.0f, 0.46f, 0.46f, movementConfig.velocityWindow.alpha } : Color4{ 0.11f, 1.0f, 0.42f, movementConfig.velocityWindow.alpha });

    ImGui::PushFont(gui->getTahoma28Font());

    const auto size = ImGui::CalcTextSize(finalText.c_str());
    drawList->AddText(ImVec2{ (static_cast<float>(screenSizeX) / 2 - size.x / 2) + 1, Ypos + 1.0f }, Helpers::calculateColor(Color4{ 0.0f, 0.0f, 0.0f, movementConfig.velocityWindow.alpha }), finalText.c_str());
    drawList->AddText(ImVec2{ static_cast<float>(screenSizeX) / 2 - size.x / 2, Ypos }, Helpers::calculateColor(trueColor), finalText.c_str());

    if (shouldDrawTakeOff)
    {
        const std::string bottomText = "(" + std::to_string(takeOffVelocity) + ")";
        const Color4 bottomTrueColor = movementConfig.velocityWindow.color.enabled ? Color4{ movementConfig.velocityWindow.color.color[0], movementConfig.velocityWindow.color.color[1], movementConfig.velocityWindow.color.color[2], movementConfig.velocityWindow.alpha, movementConfig.velocityWindow.color.rainbowSpeed, movementConfig.velocityWindow.color.rainbow }
        : (takeOffVelocity <= 250.0f ? Color4{ 0.75f, 0.75f, 0.75f, movementConfig.velocityWindow.alpha } : Color4{ 0.11f, 1.0f, 0.42f, movementConfig.velocityWindow.alpha });
        const auto bottomSize = ImGui::CalcTextSize(bottomText.c_str());
        drawList->AddText(ImVec2{ (static_cast<float>(screenSizeX) / 2 - bottomSize.x / 2) + 1, Ypos + 20.0f + 1 }, Helpers::calculateColor(Color4{ 0.0f, 0.0f, 0.0f, movementConfig.velocityWindow.alpha }), bottomText.c_str());
        drawList->AddText(ImVec2{ static_cast<float>(screenSizeX) / 2 - bottomSize.x / 2, Ypos + 20.0f }, Helpers::calculateColor(bottomTrueColor), bottomText.c_str());
    }

    ImGui::PopFont();

    if (colorTime <= memory->globalVars->realtime)
    {
        colorTime = memory->globalVars->realtime + 0.1f;
        lastVelocity = velocity;
    }
    lastOnGround = onGround;
}

void drawVelPlot() noexcept {

    GameData::Lock lock;
    const auto& local = GameData::local();

    if (movementConfig.velocityWindow.pos2 != ImVec2{}) {
        ImGui::SetNextWindowPos(movementConfig.velocityWindow.pos2);
        movementConfig.velocityWindow.pos2 = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;

    float vel = localPlayer->velocity().length2D();

    //wykres
    static float values[90] = {};
    static int values_offset = 0;
    static double refresh_time = 0.0;
    if (refresh_time == 0.0)
        refresh_time = ImGui::GetTime();
    while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
    {
        static float phase = 0.0f;
        values[values_offset] = vel;
        values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
        phase += 0.10f * values_offset;
        refresh_time += 1.0f / 60.0f;
    }

    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(
        movementConfig.velocityWindow.color.color[0],
        movementConfig.velocityWindow.color.color[1],
        movementConfig.velocityWindow.color.color[2],
        movementConfig.velocityWindow.color.color[3]));

    ImGui::Begin("Velocity PlotLines", nullptr, windowFlags);
    if (localPlayer->isAlive()) {
        ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, "", 0.f, 350.f, ImVec2(0, 80.0f));
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::End();
}

void Movement::showVelocity(ImDrawList* drawList) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (movementConfig.velocityWindow.enabled)
        drawVelWindow(drawList);

    if(movementConfig.velocityWindow.plotWin)
        drawVelPlot();

}

std::string currentForwardKey = "";
std::string currentBackKey = "";
std::string currentRightKey = "";
std::string currentLeftKey = "";
int currentButtons = 0;

void Movement::gatherDataOnTick(UserCmd* cmd) noexcept
{
    currentButtons = cmd->buttons;
}

void Movement::handleKeyEvent(int keynum, const char* currentBinding) noexcept
{
    if (!currentBinding || keynum <= 0 || keynum >= ARRAYSIZE(ButtonCodes))
        return;

    const auto buttonName = ButtonCodes[keynum];

    switch (fnv::hash(currentBinding))
    {
    case fnv::hash("+forward"):
        currentForwardKey = std::string(buttonName);
        break;
    case fnv::hash("+back"):
        currentBackKey = std::string(buttonName);
        break;
    case fnv::hash("+moveright"):
        currentRightKey = std::string(buttonName);
        break;
    case fnv::hash("+moveleft"):
        currentLeftKey = std::string(buttonName);
        break;
    default:
        break;
    }
}

void Movement::drawKeyStrokes(ImDrawList* drawList) noexcept {

    if (!movementConfig.strokesWindow.enabled) return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    int screenSizeX, screenSizeY;
    interfaces->engine->getScreenSize(screenSizeX, screenSizeY);
    const float Ypos = static_cast<float>(screenSizeY) * movementConfig.strokesWindow.position;

    std::string keys[3][2];
    if (movementConfig.strokesWindow.showKeyTiles)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                keys[i][j] = "_";
            }
        }
    }

    if (currentButtons & UserCmd::IN_DUCK)
        keys[0][0] = "C";
    if (currentButtons & UserCmd::IN_FORWARD)
        keys[1][0] = currentForwardKey;
    if (currentButtons & UserCmd::IN_JUMP)
        keys[2][0] = "J";
    if (currentButtons & UserCmd::IN_MOVELEFT)
        keys[0][1] = currentLeftKey;
    if (currentButtons & UserCmd::IN_BACK)
        keys[1][1] = currentBackKey;
    if (currentButtons & UserCmd::IN_MOVERIGHT)
        keys[2][1] = currentRightKey;

    const float positions[3] =
    {
       -35.0f, 0.0f, 35.0f
    };

    ImGui::PushFont(gui->getTahoma28Font());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (keys[i][j] == "")
                continue;

            const auto size = ImGui::CalcTextSize(keys[i][j].c_str());
            drawList->AddText(ImVec2{ (static_cast<float>(screenSizeX) / 2 - size.x / 2 + positions[i]) + 1, (Ypos + (j * 25)) + 1 }, Helpers::calculateColor(Color4{ 0.0f, 0.0f, 0.0f, movementConfig.strokesWindow.color.color[3] }), keys[i][j].c_str());
            drawList->AddText(ImVec2{ static_cast<float>(screenSizeX) / 2 - size.x / 2 + positions[i], Ypos + (j * 25) }, Helpers::calculateColor(movementConfig.strokesWindow.color), keys[i][j].c_str());
        }
    }

    ImGui::PopFont();
}

void Movement::showMouseMove() noexcept {
    if (!movementConfig.mouseWindow.enabled.enabled) return;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;

    if (movementConfig.mouseWindow.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    if (movementConfig.mouseWindow.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;

    if (movementConfig.mouseWindow.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(movementConfig.mouseWindow.pos);
        movementConfig.mouseWindow.pos = {};
    }

    ImGui::SetNextWindowSize({ 128.f, 128.f });
    ImGui::Begin("Mouse Movement", nullptr, windowFlags);

    ImGuiIO& io = ImGui::GetIO();
    if (!&io) {
        ImGui::End();
        return;
    }

    ImVec2 mouseDelta = { io.MouseDelta.x, io.MouseDelta.y };
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 center = {ImGui::GetWindowPos().x + 64.f , ImGui::GetWindowPos().y + 64.f };
    const auto color = Helpers::calculateColor(movementConfig.mouseWindow.enabled);

    drawList->AddCircleFilled(center, 4, color, 32);
    drawList->AddLine(center, center + (mouseDelta / 2.f), color, 2);
    drawList->AddCircle(center + (mouseDelta / 2.f), 6, color, 32);

    ImGui::End();

}

void Movement::Checkpoints() noexcept {

    if (!movementConfig.checkpoints.enabled) return;
    if (!localPlayer || !localPlayer->isAlive()) return;
    const auto gameRules = *memory->gameRules;

    if (!gameRules) return;

    if (gameRules->isValveDS()){
        Helpers::CheatChatNotification(false, "This option is disabled for Valve Servers!!!");
        movementConfig.checkpoints.enabled = false;
        return;
    }
    if (!interfaces->cvar->findVar("sv_cheats")->getInt()) {
        Helpers::CheatChatNotification(false, "This feature doesnt work, unless the server has sv_cheats set to 1!!!");
        movementConfig.checkpoints.enabled = false;
        return;
    }

    static Vector pPos = {};
    static Vector pAng = {};
    if (movementConfig.checkpoints.save.isPressed()) {
        pPos = localPlayer->getAbsOrigin();
        pAng = interfaces->engine->getViewAngles();
        Helpers::CheatChatNotification(false, "Position Saved");
    }
    if (movementConfig.checkpoints.load.isPressed() && pPos.notNull() && pAng.notNull()) {
        
        interfaces->engine->clientCmdUnrestricted(
            std::string("setpos ").append(std::to_string(pPos.x)).append(" ").append(std::to_string(pPos.y)).append(" ").append(std::to_string(pPos.z)).append("; setang ")
            .append(std::to_string(pAng.x)).append(" ").append(std::to_string(pAng.y)).append(" ").append(std::to_string(pAng.z)).c_str());
        Helpers::CheatChatNotification(false, "Position Loaded");
    }


    if (movementConfig.checkpoints.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(movementConfig.checkpoints.pos);
        movementConfig.checkpoints.pos = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;

    if (movementConfig.checkpoints.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    if (movementConfig.checkpoints.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Checkpoints", nullptr, windowFlags);
    ImGui::PushID("load"); ImGui::Text("Load Position"); ImGui::SameLine(); ImGui::hotkey("", movementConfig.checkpoints.load); ImGui::PopID();
    ImGui::PushID("save"); ImGui::Text("Save Position"); ImGui::SameLine(); ImGui::hotkey("", movementConfig.checkpoints.save); ImGui::PopID();
    ImGui::End();
}

void Movement::updateInput() noexcept
{

}

static bool windowOpen = false;

void Movement::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Movement")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Movement");
        ImGui::SetWindowPos("Movement", { 100.0f, 100.0f });
    }
}

void Movement::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        ImGui::Begin("Movement", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    ImGui::Checkbox("Bunny-hop", &movementConfig.bunnyHop);
    ImGui::SameLine();
    ImGui::PushID(0);
    ImGui::SetNextItemWidth(100);
    ImGui::SliderInt("", &movementConfig.bhophitchance, 0, 100, "Hitchance: %d%");
    ImGui::PopID();
    ImGui::PushID("Strafe");
    ImGui::Checkbox("Null Strafe", &movementConfig.nulls);
    ImGui::Checkbox("Auto Strafe", &movementConfig.autoStrafe);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::Combo("", &movementConfig.strafeMode, "Legit\0Rage\0");
    ImGui::PopID();
#if defined(_DEBUG)
    ImGui::Checkbox("Strafe Optimizer", &movementConfig.strafeOptimizer);
#endif
    ImGui::PushID("Edge Jump");
    ImGui::Checkbox("Edge Jump", &movementConfig.edgejump);
    ImGui::SameLine();
    ImGui::hotkey("", movementConfig.edgejumpkey);
    ImGui::PopID();


    ImGui::PushID("Ladder Jump");
    ImGui::Checkbox("Ladder Jump", &movementConfig.ladderlj);
    ImGui::SameLine();
    ImGui::hotkey("", movementConfig.ladderljKey);
    ImGui::PopID();

    ImGui::PushID("EdgeBug");
    ImGui::Checkbox("Edge Bug", &movementConfig.edgeBug.enabled);
    ImGui::SameLine();
    if (ImGui::Button("...")) ImGui::OpenPopup("ebp");
    if (ImGui::BeginPopup("ebp")) {
        ImGui::hotkey("", movementConfig.edgeBug.edgeBugKey);
        ImGui::Checkbox("Edge Bug Detect", &movementConfig.edgeBug.detect);
            if (movementConfig.edgeBug.detect) {
                ImGui::Checkbox("Play Sound", &movementConfig.edgeBug.playSound);
                ImGui::SetNextItemWidth(100.f);
                ImGui::Combo("", &movementConfig.edgeBug.sound, "Beep\0Arena Switch\0Bell\0Counter\0Overwatch\0Money Collect\0");
                ImGui::Checkbox("Print text", &movementConfig.edgeBug.print);
                ImGui::Checkbox("Healshot Effect", &movementConfig.edgeBug.screeneffect);
                ImGui::SetNextItemWidth(100.f);
                ImGui::SliderInt("Prediction Amnt", &movementConfig.edgeBug.predAmnt, 0, 128, "%d ticks");
                
            }
            ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Head Bounce");
    ImGui::Checkbox("Head Bounce", &movementConfig.headBounce);
    ImGui::SameLine();
    ImGui::hotkey("", movementConfig.headBounceKey);
    ImGui::PopID();

    ImGui::Checkbox("Auto Align", &movementConfig.autoAlign);
    ImGui::Checkbox("Auto Pixel Surf", &movementConfig.px.enabled);
    ImGui::SameLine();
    ImGui::PushID("Auto pixel Surf Key");
    ImGui::hotkey("", movementConfig.px.autoPixelSurfKey);
    ImGui::PopID();
    if (movementConfig.px.enabled) {
        ImGui::SetNextItemWidth(100.f);
        ImGui::SliderInt("Pred Amnt", &movementConfig.px.autoPixelSurfPredAmnt, 2, 4, "%d ticks");
    }

    ImGui::PushID("Jump Bug");
    ImGui::Checkbox("Jump Bug", &movementConfig.jumpbug);
    ImGui::SameLine();
    ImGui::hotkey("", movementConfig.jumpbugKey);
    ImGui::PopID();

    ImGui::Checkbox("Mini Jump", &movementConfig.mj.enabled);
    ImGui::SameLine();
    ImGui::PushID("Mini Jump Key");
    ImGui::hotkey("", movementConfig.mj.miniJumpKey);
    ImGui::PopID();
    ImGui::Checkbox("Fast Stop", &movementConfig.fastStop);
    ImGui::Checkbox("Infinite Duck", &movementConfig.fastDuck);
    ImGui::Checkbox("Duck-Jump", &movementConfig.duckjump);

    ImGui::Checkbox("Keystrokes", &movementConfig.strokesWindow.enabled);
    ImGui::SameLine();

    ImGui::PushID("Keyboard display");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SliderFloat("Position", &movementConfig.strokesWindow.position, 0.0f, 1.0f);
        ImGui::Checkbox("Show key tiles", &movementConfig.strokesWindow.showKeyTiles);
        ImGuiCustom::colorPicker("Color", movementConfig.strokesWindow.color);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Show Velocity", &movementConfig.velocityWindow.enabled);
    ImGui::SameLine();
    ImGui::PushID("Draw velocity");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SliderFloat("Position", &movementConfig.velocityWindow.position, 0.0f, 1.0f);
        ImGui::SliderFloat("Alpha", &movementConfig.velocityWindow.alpha, 0.0f, 1.0f);
        ImGuiCustom::colorPicker("Force color", movementConfig.velocityWindow.color.color.data(), nullptr, &movementConfig.velocityWindow.color.rainbow, &movementConfig.velocityWindow.color.rainbowSpeed, &movementConfig.velocityWindow.color.enabled);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Show Velocity Plot", &movementConfig.velocityWindow.plotWin);
    ImGui::SameLine();
    ImGui::PushID("Vel Button");
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("");
    }
    if (ImGui::BeginPopup("")) {
        ImGuiCustom::colorPicker("Plot Color", movementConfig.velocityWindow.color);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    //ImGuiCustom::colorPicker("Mouse Movement", movementConfig.mouseWindow.enabled);
    //ImGui::SameLine();
    //ImGui::PushID("mouse");
    //if (ImGui::Button("...")) {
    //    ImGui::OpenPopup("");
    //}
    //if (ImGui::BeginPopup("")) {
    //    ImGui::Checkbox("No Title Bar", &movementConfig.mouseWindow.noTitleBar);
    //    ImGui::Checkbox("No Background", &movementConfig.mouseWindow.noBackground);
    //    ImGui::EndPopup();
    //}
    //ImGui::PopID();
    ImGui::hotkey("Fake Backwards", movementConfig.backwards);

    ImGui::PushID("Jump stats");
    ImGui::Checkbox("Jump stats", &movementConfig.jumpStats.enabled);
    ImGui::SameLine();

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Show fails", &movementConfig.jumpStats.showFails);
        ImGui::Checkbox("Show color on fails", &movementConfig.jumpStats.showColorOnFail);
        ImGui::Checkbox("Simplify naming", &movementConfig.jumpStats.simplifyNaming);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Checkpoints", &movementConfig.checkpoints.enabled);
    ImGui::PushID("Checkpoints Button");
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("");
    }
    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &movementConfig.checkpoints.noTitleBar);
        ImGui::Checkbox("No Background", &movementConfig.checkpoints.noBackground);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    if (!contentOnly)
        ImGui::End();
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const MovementConfig::Checkpoints& o, const MovementConfig::Checkpoints& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);
    WRITE("Save Key", save);
    WRITE("Load Key", load);

    if (const auto window = ImGui::FindWindowByName("Checkpoints")) {
        j["Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MovementConfig::EdgeBug& o, const MovementConfig::EdgeBug& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Detect", detect);
    WRITE("Play Sound", playSound);
    WRITE("Screen Effect", screeneffect);
    WRITE("Print Message", print);
    WRITE("Key", edgeBugKey);
    WRITE("Prediction", predAmnt);
}

static void to_json(json& j, const MovementConfig::PixelSurf& o, const MovementConfig::PixelSurf& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Keybind", autoPixelSurfKey);
    WRITE("Pred Amnt", autoPixelSurfPredAmnt);
}

static void to_json(json& j, const MovementConfig::MiniJump& o, const MovementConfig::MiniJump& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Keybind", miniJumpKey);
}

static void to_json(json& j, const MovementConfig::Strokes& o, const MovementConfig::Strokes& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Position", position);
    WRITE("Show key Tiles", showKeyTiles);
    WRITE("Color", color);
}

static void to_json(json& j, const MovementConfig::JumpStats& o, const MovementConfig::JumpStats& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Show fails", showFails);
    WRITE("Show color on fail", showColorOnFail);
    WRITE("Simplify naming", simplifyNaming);
}

static void to_json(json& j, const MovementConfig::VelocityWindow& o, const MovementConfig::VelocityWindow& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Position", position);
    WRITE("Alpha", alpha);
    WRITE("Plot Window", plotWin);
    WRITE("Plot Color", color);

    if (const auto window = ImGui::FindWindowByName("Velocity PlotLines")) {
        j["Plot Pos"] = window->Pos;
    }
}

static void to_json(json& j, const MovementConfig::Mouse& o, const MovementConfig::Mouse& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);
    WRITE("No Background", noBackground);

    if (const auto window = ImGui::FindWindowByName("Mouse Movement")) {
        j["Pos"] = window->Pos;
    }
}

static void from_json(const json& j, ImVec2& v)
{
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, MovementConfig::JumpStats& js)
{
    read(j, "Enabled", js.enabled);
    read(j, "Show fails", js.showFails);
    read(j, "Show color on fail", js.showColorOnFail);
    read(j, "Simplify naming", js.simplifyNaming);
}

static void from_json(const json& j, MovementConfig::Checkpoints& c)
{
    read(j, "Enabled", c.enabled);
    read(j, "No Title Bar", c.noTitleBar);
    read(j, "No Background", c.noBackground);
    read(j, "Save Key", c.save);
    read(j, "Load Key", c.load);
    read<value_t::object>(j, "Pos", c.pos);
}

static void from_json(const json& j, MovementConfig::Strokes& s)
{
    read(j, "Enabled", s.enabled);
    read(j, "Position", s.position);
    read(j, "Show key Tiles", s.showKeyTiles);
    read<value_t::object>(j, "Color", s.color);
}

static void from_json(const json& j, MovementConfig::VelocityWindow& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Position", o.position);
    read(j, "Alpha", o.alpha);
    read(j, "Plot Window", o.plotWin);
    read<value_t::object>(j, "Plot color", o.color);
    read<value_t::object>(j, "Plot Pos", o.pos2);
}

static void from_json(const json& j, MovementConfig::Mouse& s)
{
    read<value_t::object>(j, "Enabled", s.enabled);
    read(j, "No Background", s.noBackground);
    read(j, "No Title Bar", s.noTitleBar);
    read<value_t::object>(j, "Pos", s.pos);
}

static void from_json(const json& j, MovementConfig::EdgeBug& eb)
{
    read(j, "Enabled", eb.enabled);
    read(j, "Detect", eb.detect);
    read(j, "Play Sound", eb.playSound);
    read(j, "Screen Effect", eb.screeneffect);
    read(j, "Print Message", eb.print);
    read(j, "Key", eb.edgeBugKey);
    read(j, "Prediction", eb.predAmnt);

}

static void from_json(const json& j, MovementConfig::PixelSurf& px)
{
    read(j, "Enabled", px.enabled);
    read(j, "Keybind", px.autoPixelSurfKey);
    read(j, "Pred Amnt", px.autoPixelSurfPredAmnt);
}

static void from_json(const json& j, MovementConfig::MiniJump& mj)
{
    read(j, "Enabled", mj.enabled);
    read(j, "Keybind", mj.miniJumpKey);
}

static void from_json(const json& j, MovementConfig& move)
{
    read(j, "Auto strafe", move.autoStrafe);
    read(j, "Strafe Mode", move.strafeMode);
    read(j, "Strafe Optimizer", move.strafeOptimizer);
    read(j, "Bunny hop", move.bunnyHop);
    read(j, "Ladder Jump", move.ladderlj);
    read(j, "Ladder Jump Key", move.ladderljKey);
    read(j, "Jump Bug", move.jumpbug);
    read(j, "Jump Bug Key", move.jumpbugKey);
    read(j, "Edge Jump", move.edgejump);
    read(j, "Edge Jump Key", move.edgejumpkey);
    read(j, "Duck In-Air", move.duckjump);
    read(j, "Fast Duck", move.fastDuck);
    read(j, "Fast Stop", move.fastStop);
    read<value_t::object>(j, "Jump stats", move.jumpStats);
    read(j, "Fake Backwards", move.backwards);
    read<value_t::object>(j, "Velocity Window", move.velocityWindow);
    read<value_t::object>(j, "Mouse Movement", move.mouseWindow);
    read<value_t::object>(j, "Strokes", move.strokesWindow);
    read<value_t::object>(j, "Edge Bug", move.edgeBug);
    read<value_t::object>(j, "Checkpoints", move.checkpoints);
    read<value_t::object>(j, "Mini Jump", move.mj);
    read<value_t::object>(j, "Pixel Surf", move.px);
}

static void to_json(json& j, const MovementConfig& o)
{
    const MovementConfig dummy;

    WRITE("Auto strafe", autoStrafe);
    WRITE("Strafe Mode", strafeMode);
    WRITE("Strafe Optimizer", strafeOptimizer);
    WRITE("Bunny hop", bunnyHop);
    WRITE("Ladder Jump", ladderlj);
    WRITE("Ladder Jump Key", ladderljKey);
    WRITE("Edge Bug", edgeBug);
    WRITE("Jump Bug", jumpbug);
    WRITE("Jump Bug Key", jumpbugKey);
    WRITE("Edge Jump", edgejump);
    WRITE("Edge Jump Key", edgejumpkey);
    WRITE("Duck In-Air", duckjump);
    WRITE("Fast Duck", fastDuck);
    WRITE("Fast Stop", fastStop);
    WRITE("Jump stats", jumpStats);
    WRITE("Fake Backwards", backwards);
    WRITE("Velocity Window", velocityWindow);
    WRITE("Mouse Movement", mouseWindow);
    WRITE("Strokes", strokesWindow);
    WRITE("Checkpoints", checkpoints);
    WRITE("Mini Jump", mj);
    WRITE("Pixel Surf", px);
}

json Movement::toJson() noexcept
{
    json j;
    to_json(j, movementConfig);
    return j;
}

void Movement::fromJson(const json& j) noexcept
{
    from_json(j, movementConfig);
}

void Movement::resetConfig() noexcept
{
    movementConfig = {};
}