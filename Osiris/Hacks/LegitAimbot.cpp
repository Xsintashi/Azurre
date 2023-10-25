#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <memory>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"

#include "../GameData.h"

#include "LegitAimbot.h"
#include "../Config.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/UserCmd.h"
#include "../SDK/GameEvent.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/WeaponData.h"
#include "../SDK/Constants/ClassId.h"
#include "../SDK/ClientClass.h"
#include "../SDK/BSPFlags.h"
#include <Hacks/Extra.h>
#include <Hacks/Visuals.h>
#include <HelpersAimbot.h>
#include "Animations.h"

static bool keyPressed = false;
static bool minKeyPressed = false;

bool LegitAimbot::isPressedOrHold() noexcept {
    return keyPressed;
}

void LegitAimbot::updateInput() noexcept
{
    if (config->aimbotKeyMode == 0)
        keyPressed = config->aimbotKey.isDown();
    if (config->aimbotKeyMode == 1 && config->aimbotKey.isPressed())
        keyPressed = !keyPressed;
}

static void setRandomSeed(int seed) noexcept
{
    using randomSeedFn = void(*)(int);
    static auto randomSeed{ reinterpret_cast<randomSeedFn>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed")) };
    randomSeed(seed);
}

float getRandom(float bottom, float top) noexcept
{
    std::random_device rd{};
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> dis(bottom, top);
    return dis(generator);
}

void LegitAimbot::run(UserCmd* cmd) noexcept
{

    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    if (interfaces->engine->isHLTV())
        return;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());

    if (!config->legitaimbot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->legitaimbot[weaponIndex].enabled)
        weaponIndex = 0;

    if (!config->legitaimbot[weaponIndex].betweenShots && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    if (!config->legitaimbot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->aimbotKey.isSet() && !keyPressed)
        return;

    if (activeWeapon->itemDefinitionIndex() == WeaponId::Taser)
        return;

    if (config->legitaimbot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->legitaimbot[weaponIndex].aimlock)) {

        auto bestFov = config->legitaimbot[weaponIndex].fov / 1.411764705882353;
        auto deadZone = config->legitaimbot[weaponIndex].deadzone / 1.411764705882353;
        Vector bestTarget{ };
        const auto localPlayerEyePosition = localPlayer->getEyePosition();

        auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };
        if (config->legitaimbot[weaponIndex].standaloneRCS && (config->legitaimbot[weaponIndex].recoilControlX || config->legitaimbot[weaponIndex].recoilControlY) && aimPunch.notNull())
    {
        static Vector lastAimPunch{ };
        if (localPlayer->shotsFired() > config->legitaimbot[weaponIndex].shotsFired)
        {
            if (cmd->buttons & UserCmd::IN_ATTACK)
            {
                Vector currentPunch = aimPunch;

                currentPunch.x *= config->legitaimbot[weaponIndex].recoilControlY;
                currentPunch.y *= config->legitaimbot[weaponIndex].recoilControlX;

                cmd->viewangles.y -= currentPunch.y;
                cmd->viewangles.x -= currentPunch.x;
                lastAimPunch = Vector{ };
                
            }
        }
        else
        {
            lastAimPunch = Vector{ };
        }
    }
        std::array<bool, Hitboxes::Max> hitbox{ false };

        // Head
        hitbox[Hitboxes::Head] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 0) == 1 << 0;
        // Chest
        hitbox[Hitboxes::UpperChest] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 1) == 1 << 1;
        hitbox[Hitboxes::Thorax] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 1) == 1 << 1;
        hitbox[Hitboxes::LowerChest] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 1) == 1 << 1;
        //Stomach
        hitbox[Hitboxes::Belly] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 2) == 1 << 2;
        hitbox[Hitboxes::Pelvis] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 2) == 1 << 2;
        //Arms
        hitbox[Hitboxes::RightUpperArm] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 3) == 1 << 3;
        hitbox[Hitboxes::RightForearm] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 3) == 1 << 3;
        hitbox[Hitboxes::LeftUpperArm] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 3) == 1 << 3;
        hitbox[Hitboxes::LeftForearm] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 3) == 1 << 3;
        //Legs
        hitbox[Hitboxes::RightCalf] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 4) == 1 << 4;
        hitbox[Hitboxes::RightThigh] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 4) == 1 << 4;
        hitbox[Hitboxes::LeftCalf] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 4) == 1 << 4;
        hitbox[Hitboxes::LeftThigh] = (config->legitaimbot[weaponIndex].hitboxes & 1 << 4) == 1 << 4;

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) && !config->legitaimbot[weaponIndex].friendlyFire || entity->gunGameImmunity())
                continue;

            const Model* model = entity->getModel();
            if (!model)
                continue;

            StudioHdr* hdr = interfaces->modelInfo->getStudioModel(model);
            if (!hdr)
                continue;

            StudioHitboxSet* set = hdr->getHitboxSet(0);
            if (!set)
                continue;

            const auto player = Animations::getPlayer(i);
            if (!player.gotMatrix)
                continue;
            for (size_t j = 0; j < hitbox.size(); j++)
            {
                if (!hitbox[j])
                    continue;

                StudioBbox* hitbox = set->getHitbox(j);
                if (!hitbox)
                    continue;

                for (auto& bonePosition : AimHelpers::multiPoint(entity, player.matrix.data(), hitbox, localPlayerEyePosition, j, 0)) {
                    const auto angle = AimHelpers::calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch);
                    const auto fov = angle.length2D();

                    if (fov > bestFov)
                        continue;

                    if (fov < deadZone)
                        continue;

                    if (!config->legitaimbot[weaponIndex].ignoreSmoke && memory->lineGoesThroughSmoke(localPlayerEyePosition, bonePosition, 1))
                        continue;

                    if (!entity->isVisible(bonePosition) && (config->legitaimbot[weaponIndex].visibleOnly || !AimHelpers::canScan(entity, bonePosition, activeWeapon->getWeaponData(), config->legitaimbot[weaponIndex].killshot ? entity->health() : config->legitaimbot[weaponIndex].minDamage, config->legitaimbot[weaponIndex].friendlyFire)))
                        continue;

                    if (fov < bestFov) {
                        bestFov = fov;
                        bestTarget = bonePosition;
                    }
                }
            }
        }

        static float lastTime = 0.f;
        if (bestTarget.notNull()) {

            if (memory->globalVars->realtime - lastTime <= static_cast<float>(config->legitaimbot[weaponIndex].reactionTime) / 1000.f)
                return;

            static Vector lastAngles{ cmd->viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull())
                cmd->viewangles = lastAngles;

            auto angle = AimHelpers::calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
            bool clamped{ false };

            if (std::abs(angle.x) > Extra::maxAngleDelta() || std::abs(angle.y) > Extra::maxAngleDelta()) {
                angle.x = std::clamp(angle.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
                angle.y = std::clamp(angle.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
                clamped = true;
            }

            angle /= config->legitaimbot[weaponIndex].smooth;
            cmd->viewangles += angle;
            interfaces->engine->setViewAngles(cmd->viewangles);

            if (config->legitaimbot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
                return;

            if (clamped || config->legitaimbot[weaponIndex].smooth > 1.0f) lastAngles = cmd->viewangles;
            else lastAngles = Vector{ };

            lastCommand = cmd->commandNumber;
        }
        else lastTime = memory->globalVars->realtime;
    }
}