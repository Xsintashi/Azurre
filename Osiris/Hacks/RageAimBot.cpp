#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"

#include "../HelpersAimbot.h"
#include "Animations.h"
#include "Backtrack.h"
#include "Extra.h"
#include "RageAimBot.h"
#include "EnginePrediction.h"
#include "Resolver.h"

#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Utils.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/ModelInfo.h"

void RageAimbot::autoZeus(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive() || localPlayer->nextAttack() > memory->globalVars->serverTime())
        return;

    return;

    if (!config->zeusbot.enable) return;

    if (!*memory->gameRules || (*memory->gameRules)->freezePeriod())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip() || activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    static auto lastTime = 0.0f;
    static auto lastContact = 0.0f;

    const auto now = memory->globalVars->realtime;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    if (activeWeapon->itemDefinitionIndex() != WeaponId::Taser)
        return;

    Vector bestTarget{ };
    Vector bestAngle{ };
    auto localPlayerEyePosition = localPlayer->getEyePosition();

    for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
        auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity())
            continue;

        for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
            auto bonePosition = entity->getBonePosition(bone);

            auto angle = AimHelpers::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles);
            auto fov = std::hypotf(angle.x, angle.y);

            Vector viewAngles{ std::cos(Helpers::deg2rad(cmd->viewangles.x + angle.x)) * std::cos(Helpers::deg2rad(cmd->viewangles.y + angle.y)) * weaponData->range,
                std::cos(Helpers::deg2rad(cmd->viewangles.x + angle.x)) * std::sin(Helpers::deg2rad(cmd->viewangles.y + angle.y)) * weaponData->range,
                -std::sin(Helpers::deg2rad(cmd->viewangles.x + angle.x)) * weaponData->range };

            if (!entity->isVisible(bonePosition) && !AimHelpers::handleTaserPenetration(cmd, viewAngles, bonePosition))
                continue;
            else {

                Trace trace;
                interfaces->engineTrace->traceRay({ localPlayer->getEyePosition(), localPlayer->getEyePosition() + viewAngles }, 0x46004009, localPlayer.get(), trace);
                if (trace.entity && trace.entity->getClientClass()->classId == ClassId::CSPlayer && trace.entity->isOtherEnemy(localPlayer.get()) && !trace.entity->gunGameImmunity())
                {
                    float damage = (weaponData->damage * std::pow(weaponData->rangeModifier, trace.fraction * weaponData->range / 510.0f));

                    if (damage >= (true ? trace.entity->health() : 100)) {
                        bestTarget = bonePosition;
                        bestAngle = angle;
                    }
                }
            }
        }
        if (bestTarget.notNull())
        {
            if (!AimHelpers::hitChanceWeapon(localPlayer.get(), entity, activeWeapon, bestAngle, cmd, config->zeusbot.hitchance))
            {
                bestTarget = Vector{ };
                continue;
            }
            break;
        }
    }

    if (bestTarget.notNull()) {
        static Vector lastAngles{ cmd->viewangles };
        static int lastCommand{ };

        if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && config->zeusbot.silent)
            cmd->viewangles = lastAngles;

        auto angle = AimHelpers::calculateRelativeAngle(localPlayer->getEyePosition(), bestTarget, cmd->viewangles);
        bool clamped{ false };

        if (fabs(angle.x) > Extra::maxAngleDelta() || fabs(angle.y) > Extra::maxAngleDelta()) {
            angle.x = std::clamp(angle.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
            angle.y = std::clamp(angle.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
            clamped = true;
        }

        cmd->viewangles += angle;
        if (!config->zeusbot.silent)
            interfaces->engine->setViewAngles(cmd->viewangles);

        const Vector viewAngles{ std::cos(Helpers::deg2rad(cmd->viewangles.x)) * std::cos(Helpers::deg2rad(cmd->viewangles.y)) * weaponData->range,
                 std::cos(Helpers::deg2rad(cmd->viewangles.x)) * std::sin(Helpers::deg2rad(cmd->viewangles.y)) * weaponData->range,
                -std::sin(Helpers::deg2rad(cmd->viewangles.x)) * weaponData->range };
        Trace trace;
        interfaces->engineTrace->traceRay({ localPlayer->getEyePosition(), localPlayer->getEyePosition() + viewAngles }, 0x46004009, localPlayer.get(), trace);
        if (trace.entity && trace.entity->getClientClass()->classId == ClassId::CSPlayer && trace.entity->isOtherEnemy(localPlayer.get()) && !trace.entity->gunGameImmunity())
        {
            float damage = (weaponData->damage * std::pow(weaponData->rangeModifier, trace.fraction * weaponData->range / 500.0f));

            if (damage >= (true ? trace.entity->health() : 100)) {
                cmd->buttons |= UserCmd::IN_ATTACK;
                lastContact = now;
            }
        }

        if (clamped)
            cmd->buttons &= ~UserCmd::IN_ATTACK;

        if (clamped) lastAngles = cmd->viewangles;
        else lastAngles = Vector{ };

        lastCommand = cmd->commandNumber;
    }
}

static bool minDamageHandle = false;

void RageAimbot::updateInput() noexcept
{
    if (!config->minDamageMode)
        minDamageHandle = config->minDamageOverride.isDown();
    else if (config->minDamageMode && config->minDamageOverride.isPressed())
        minDamageHandle = !minDamageHandle;
}

void runRagebot(UserCmd* cmd, Entity* entity, matrix3x4* matrix, RageAimbot::Enemies target, std::array<bool, Hitboxes::Max> hitbox, Entity* activeWeapon, int weaponIndex, Vector localPlayerEyePosition, Vector aimPunch, int multiPoint, int minDamage, float& damageDiff, Vector& bestAngle, Vector& bestTarget) noexcept
{
    const auto& cfg = config->rageaimbot;

    damageDiff = FLT_MAX;

    const Model* model = entity->getModel();
    if (!model)
        return;

    StudioHdr* hdr = interfaces->modelInfo->getStudioModel(model);
    if (!hdr)
        return;

    StudioHitboxSet* set = hdr->getHitboxSet(0);
    if (!set)
        return;

    for (size_t i = 0; i < hitbox.size(); i++)
    {
        if (!hitbox[i])
            continue;

        StudioBbox* hitbox = set->getHitbox(i);
        if (!hitbox)
            continue;

        for (auto& bonePosition : AimHelpers::multiPoint(entity, matrix, hitbox, localPlayerEyePosition, i, multiPoint))
        {
            const auto angle{ AimHelpers::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch) };
            const auto fov{ angle.length2D() };
            if (fov > cfg[weaponIndex].rfov)
                continue;

            float damage = AimHelpers::getScanDamage(entity, bonePosition, activeWeapon->getWeaponData(), minDamage, cfg[weaponIndex].rfriendlyFire);
            damage = std::clamp(damage, 0.0f, (float)entity->maxHealth());
            if (damage <= 0.f)
                continue;

            if (!entity->isVisible(bonePosition) && (cfg[weaponIndex].rvisibleOnly || !damage))
                continue;

            if (cfg[weaponIndex].rautoScope && activeWeapon->isSniperRifle() && !localPlayer->isScoped() && !activeWeapon->zoomLevel() && localPlayer->flags() & 1 && !(cmd->buttons & UserCmd::IN_JUMP))
                cmd->buttons |= UserCmd::IN_ZOOM;

            if (cfg[weaponIndex].rscopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
                return;

            if (config->rageaimbot[weaponIndex].rforceAccuracy && localPlayer->isOnGround() && !(cmd->buttons & UserCmd::IN_JUMP)) {
                const auto velocity = EnginePrediction::getVelocity();
                const auto speed = velocity.length2D();
                if (speed >= 15.0f)
                {
                    Vector direction = velocity.toAngle();
                    direction.y = cmd->viewangles.y - direction.y;

                    const auto negatedDirection = Vector::fromAngle(direction) * -speed;
                    cmd->forwardmove = negatedDirection.x;
                    cmd->sidemove = negatedDirection.y;
                }
            }

            if (std::fabsf((float)target.health - damage) <= damageDiff)
            {
                bestAngle = angle;
                damageDiff = std::fabsf((float)target.health - damage);
                bestTarget = bonePosition;
            }
        }
    }

    if (bestTarget.notNull())
    {
        if (!AimHelpers::hitChance(localPlayer.get(), entity, set, matrix, activeWeapon, bestAngle, cmd, cfg[weaponIndex].rhitchance))
        {
            bestTarget = Vector{ };
            bestAngle = Vector{ };
            damageDiff = FLT_MAX;
        }
    }
}

void RageAimbot::run(UserCmd* cmd) noexcept
{
    const auto& cfg = config->rageaimbot;

    if (!config->globalEnabledRagebot)
        return;

    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
    if (!cfg[weaponIndex].renabled)
        weaponIndex = weaponClass;

    if (!cfg[weaponIndex].renabled)
        weaponIndex = 0;

    if (!cfg[weaponIndex].rbetweenShots && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    if (!(cfg[weaponIndex].renabled && (cmd->buttons & UserCmd::IN_ATTACK || cfg[weaponIndex].rautoShot || cfg[weaponIndex].raimlock)))
        return;

    float damageDiff = FLT_MAX;
    Vector bestTarget{ };
    Vector bestAngle{ };
    int bestIndex{ -1 };
    float bestSimulationTime = 0;
    const auto localPlayerEyePosition = localPlayer->getEyePosition();
    const auto aimPunch = localPlayer->getAimPunch();

    std::array<bool, Hitboxes::Max> hitbox{ false };

    // Head
    hitbox[Hitboxes::Head] = (cfg[weaponIndex].rhitbox & 1 << 0) == 1 << 0;
    // Chest
    hitbox[Hitboxes::UpperChest] = (cfg[weaponIndex].rhitbox & 1 << 1) == 1 << 1;
    hitbox[Hitboxes::Thorax] = (cfg[weaponIndex].rhitbox & 1 << 1) == 1 << 1;
    hitbox[Hitboxes::LowerChest] = (cfg[weaponIndex].rhitbox & 1 << 1) == 1 << 1;
    //Stomach
    hitbox[Hitboxes::Belly] = (cfg[weaponIndex].rhitbox & 1 << 2) == 1 << 2;
    hitbox[Hitboxes::Pelvis] = (cfg[weaponIndex].rhitbox & 1 << 2) == 1 << 2;
    //Arms
    hitbox[Hitboxes::RightUpperArm] = (cfg[weaponIndex].rhitbox & 1 << 3) == 1 << 3;
    hitbox[Hitboxes::RightForearm] = (cfg[weaponIndex].rhitbox & 1 << 3) == 1 << 3;
    hitbox[Hitboxes::LeftUpperArm] = (cfg[weaponIndex].rhitbox & 1 << 3) == 1 << 3;
    hitbox[Hitboxes::LeftForearm] = (cfg[weaponIndex].rhitbox & 1 << 3) == 1 << 3;
    //Legs
    hitbox[Hitboxes::RightCalf] = (cfg[weaponIndex].rhitbox & 1 << 4) == 1 << 4;
    hitbox[Hitboxes::RightThigh] = (cfg[weaponIndex].rhitbox & 1 << 4) == 1 << 4;
    hitbox[Hitboxes::LeftCalf] = (cfg[weaponIndex].rhitbox & 1 << 4) == 1 << 4;
    hitbox[Hitboxes::LeftThigh] = (cfg[weaponIndex].rhitbox & 1 << 4) == 1 << 4;


    std::vector<RageAimbot::Enemies> enemies;
    const auto localPlayerOrigin{ localPlayer->getAbsOrigin() };
    for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
        const auto player = Animations::getPlayer(i);
        if (!player.gotMatrix)
            continue;

        const auto entity{ interfaces->entityList->getEntity(i) };
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
            || !entity->isOtherEnemy(localPlayer.get()) && !cfg[weaponIndex].rfriendlyFire || entity->gunGameImmunity())
            continue;

        const auto angle{ AimHelpers::calculateRelativeAngle(localPlayerEyePosition, player.matrix[8].origin(), cmd->viewangles + aimPunch) };
        const auto origin{ entity->getAbsOrigin() };
        const auto fov{ angle.length2D() }; //fov
        const auto health{ entity->health() }; //health
        const auto distance{ localPlayerOrigin.distTo(origin) }; //distance
        enemies.emplace_back(i, health, distance, fov);
    }

    if (enemies.empty())
        return;

    switch (cfg[weaponIndex].rpriority)
    {
    case 0:
        std::sort(enemies.begin(), enemies.end(), healthSort);
        break;
    case 1:
        std::sort(enemies.begin(), enemies.end(), distanceSort);
        break;
    case 2:
        std::sort(enemies.begin(), enemies.end(), fovSort);
        break;
    default:
        break;
    }

    static auto frameRate = 1.0f;
    frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;

    auto multiPoint = cfg[weaponIndex].rmultiPoint;
    //if (cfg[weaponIndex].disableMultipointIfLowFPS && static_cast<int>(1 / frameRate) <= 1 / memory->globalVars->intervalPerTick)
    //    multiPoint = 0;

    for (const auto& target : enemies)
    {
        auto entity{ interfaces->entityList->getEntity(target.id) };
        auto player = Animations::getPlayer(target.id);
        int minDamage = std::clamp(std::clamp(minDamageHandle ? cfg[weaponIndex].roverridedMinDamage : cfg[weaponIndex].rminDamage, 0, target.health), 0, activeWeapon->getWeaponData()->damage);

        matrix3x4* backupBoneCache = entity->getBoneCache().memory;
        Vector backupMins = entity->getCollideable()->obbMins();
        Vector backupMaxs = entity->getCollideable()->obbMaxs();
        Vector backupOrigin = entity->getAbsOrigin();
        Vector backupAbsAngle = entity->getAbsAngle();

        for (int cycle = 0; cycle < 2; cycle++)
        {
            float currentSimulationTime = -1.0f;

            if (Backtrack::TimeLimitRecive())
            {
                const auto records = Animations::getBacktrackRecords(entity->index());
                if (!records || records->empty())
                    continue;

                int bestTick = -1;
                if (cycle == 0)
                {
                    for (size_t i = 0; i < records->size(); i++)
                    {
                        if (Backtrack::valid(records->at(i).simulationTime))
                        {
                            bestTick = static_cast<int>(i);
                            break;
                        }
                    }
                }
                else
                {
                    for (int i = static_cast<int>(records->size() - 1U); i >= 0; i--)
                    {
                        if (Backtrack::valid(records->at(i).simulationTime))
                        {
                            bestTick = i;
                            break;
                        }
                    }
                }

                if (bestTick <= -1)
                    continue;

                memcpy(entity->getBoneCache().memory, records->at(bestTick).matrix, std::clamp(entity->getBoneCache().size, 0, MAXSTUDIOBONES) * sizeof(matrix3x4));
                memory->setAbsOrigin(entity, records->at(bestTick).origin);
                memory->setAbsAngle(entity, Vector{ 0.f, records->at(bestTick).absAngle.y, 0.f });
                memory->setCollisionBounds(entity->getCollideable(), records->at(bestTick).mins, records->at(bestTick).maxs);

                currentSimulationTime = records->at(bestTick).simulationTime;
            }
            else
            {
                //We skip backtrack
                if (cycle == 1)
                    continue;

                memcpy(entity->getBoneCache().memory, player.matrix.data(), std::clamp(entity->getBoneCache().size, 0, MAXSTUDIOBONES) * sizeof(matrix3x4));
                memory->setAbsOrigin(entity, player.origin);
                memory->setAbsAngle(entity, Vector{ 0.f, player.absAngle.y, 0.f });
                memory->setCollisionBounds(entity->getCollideable(), player.mins, player.maxs);

                currentSimulationTime = player.simulationTime;
            }

            runRagebot(cmd, entity, entity->getBoneCache().memory, target, hitbox, activeWeapon, weaponIndex, localPlayerEyePosition, aimPunch, multiPoint, minDamage, damageDiff, bestAngle, bestTarget);
            Helpers::resetMatrix(entity, backupBoneCache, backupOrigin, backupAbsAngle, backupMins, backupMaxs);
            if (bestTarget.notNull())
            {
                bestSimulationTime = currentSimulationTime;
                bestIndex = target.id;
                break;
            }
        }
        if (bestTarget.notNull())
            break;
    }

    if (bestTarget.notNull())
    {
        static Vector lastAngles{ cmd->viewangles };
        static int lastCommand{ };

        if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && cfg[weaponIndex].rsilent)
            cmd->viewangles = lastAngles;

        auto angle = AimHelpers::calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
        bool clamped{ false };

        if (fabs(angle.x) > Extra::maxAngleDelta() || fabs(angle.y) > Extra::maxAngleDelta()) {
            angle.x = std::clamp(angle.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
            angle.y = std::clamp(angle.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
            clamped = true;
        }

        if (activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime())
        {
            cmd->viewangles += angle;
            if (!cfg[weaponIndex].rsilent)
                interfaces->engine->setViewAngles(cmd->viewangles);

            if (cfg[weaponIndex].rautoShot && !clamped)
                cmd->buttons |= UserCmd::IN_ATTACK;
        }

        if (clamped)
            cmd->buttons &= ~UserCmd::IN_ATTACK;

        if (cmd->buttons & UserCmd::IN_ATTACK)
        {
            cmd->tickCount = Helpers::timeToTicks(bestSimulationTime + Backtrack::getLerp());
            Resolver::saveRecord(bestIndex, bestSimulationTime);
        }

        if (clamped) lastAngles = cmd->viewangles;
        else lastAngles = Vector{ };

        lastCommand = cmd->commandNumber;
    }
}