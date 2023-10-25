#include "Walkbot.h"

#include "../SDK/Entity.h"
#include "../SDK/GameEvent.h"
#include "../SDK/Vector.h"
#include "../SDK/EngineTrace.h"
#include <HelpersAimbot.h>
#include "EnginePrediction.h"

struct WalkbotConfig {

    bool enabled{ false };

    int botState = 0;
    int botStatePrev = 0;

    int roundCounter = 0;


    //walkbot vars
    bool shouldwalk{ false };

    //common walkbot vars
    int walkbotType{ 0 };
    int nodeRadius{ 33 };
    std::vector<int> waypointWalkType;
    float dropdownDmg{ 0.f };
    bool improvedPathfinding{ false };

    //walkbot type 0 (dynamic node pathfinding)
    std::vector<Vector> nodes;//node positions
    std::vector<bool> nodesType;    //true if a node is open,false if closed
    std::vector<int>nodesParents;   //index of the parent node
    std::vector<int>walkType;       //same as return of collisionCheck()
    std::vector<float> fcost;       //total cost of node (dist to localplayer+dist to destination)
    int currentNode{ 0 };
    bool pathFound{ false };
    std::vector<Trace>tracez;
    Trace middleTrace;
    Vector checkOrigin{ 0,0,0 };
    Vector tempFloorPos{ 0,0,0 };
    Vector finalDestination{ 0.f,0.f,0.f };
    std::vector<Vector> waypoints;
    std::vector<Vector> waypointParentOffset;
    int curWayPoint{ -2 };
    bool shouldGoTowardsPing{ false };
    bool traceToParentIntersects{ false };
    std::vector<Vector>parentOffset;
    bool stopWalking{ false };


    //walkbot type 1 (static node pathfinding)
    int targetStaticNode{ 0 };                  //the static node that we should walk towards
    std::vector<Vector> presetNodes;          //list of all static nodes
    std::vector<int> nodeGroup;                     //common/camping spot/bombsite a/bombsite b
    const std::array<std::string, 3> maplist{ "borbot","de_mirage","" };//supported maps
    std::string currentMap{ "idk" };
    float maxDistToCalculate{ 1500.f };             //don't calculate fcost for static node if distance from lPlayer to node is > value
    std::vector<float> staticNodesFcosts;           //999999 if node is further than maxDistToCalculate units away (so that

    Vector playerPingLoc{ 0,0,0 };

    float hitglass{ 0.f };
    float tracerayAngle{ 0.f };

    bool shouldDebug{ false };               //debug drawing and ermmmmmm
    float posDrawSize{ 1.5f };
    bool pathfindingDebug{ false };
    bool drawPathfindingTraces{ false };
    bool circlesOrCost{ false };


    bool autoreload{ false };                    //reloadbot logic
    float lastReload{ 0.0f };                      //I THINK(!) this is the last second when no enemy was seen 
    int reloadAfterXSeconds{ 5 };
    float reloadIfClipPercent{ 0.75f };

    bool aimAtEvents{ true };                    //aimbot things
    int aimreason{ -1 };                           //-1:not aiming at anything, 0:weapon_fire event 1:enemy 2:teammates 3:windows 4:path
    bool isShooterVisible{ false };
    float reactionTime{ 0.135f };
    Vector aimspot{ 0,0,0 };
    float startedAiming{ -1.f };
    std::vector<float>aimtime{ 0.1f };
    Vector localViewAngles{ 0,0,0 };
    float lastTimeSawEnemy{ 0.f };
    bool shouldFire{ false };
    bool aimAtPath{ false };

    std::vector<Entity> enemyEntities;       //list of enemy csgo::Entities that we can aim at (line of sight+fov)


    int enemyToAim{ -1 };
    bool shouldScope{ false };
    int shouldUnscope{ 0 };

    float roundStartTime{ 0.f };
    float buyAfter{ 0.f };
    //communication stuff
    const std::array<std::string, 31> radioTranslate{ "","","go","fallback","sticktog","holdpos","followme", "","roger","negative","cheer","compliment",
                                                     "thanks","","enemyspot","needbackup","takepoint","sectorclear","inposition","coverme","regroup",
                                                     "takingfire","report","", "getout", "enemydown","","","go_a","go_b","needrop" };

    bool shouldReportToTeam{ true };        //report attacker info on death
    bool reportDetailsCallout{ true };
    bool reportDetailsDiedTo{ true };

    bool shouldCompliment{ true };
    float complimentChance{ 1.f };

    std::vector<int>botsList;
    bool isListeningForCommands{ false };
    std::vector<int>botsListeningToMe;



} walkCfg;


void Walkbot::aimAtEvent() noexcept {
    if (!walkCfg.enabled)
        return;

    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    if (!interfaces->engine->isInGame())
        return;

    if (walkCfg.aimreason == 3 && walkCfg.startedAiming < 1.f) {
        walkCfg.startedAiming = memory->globalVars->realtime;
    }
    const auto activeWeapon = localPlayer->getActiveWeapon();

    if (walkCfg.aimreason == 1) {

        if (activeWeapon->getWeaponType() == WeaponType::Knife || activeWeapon->getWeaponType() == WeaponType::C4)
            interfaces->engine->clientCmdUnrestricted("slot1");

        if (!localPlayer->isScoped() && activeWeapon->getWeaponType() == WeaponType::SniperRifle)
            walkCfg.shouldScope = true;

    }
    if (walkCfg.startedAiming == -1.f || walkCfg.startedAiming + (walkCfg.aimreason == 0 || walkCfg.aimreason == 1 ? walkCfg.reactionTime : 0.f) > memory->globalVars->realtime)
        return;
    if (walkCfg.aimreason == 0 && !walkCfg.isShooterVisible)
        return;
    const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

    Vector relang = AimHelpers::calculateRelativeAngle(localPlayer->getEyePosition(), walkCfg.aimspot, walkCfg.localViewAngles);

    // Gradually adjust the view angles towards the aimspot over time
    float elapsedTime = memory->globalVars->realtime - walkCfg.startedAiming - (walkCfg.aimreason == 3 ? 0.28f : walkCfg.aimtime[0]) + walkCfg.reactionTime;
    float adjustmentSpeed = elapsedTime / 2;
    float sinAdjustment = sin(adjustmentSpeed);
    float archAdjustment = sinAdjustment * sinAdjustment;
    interfaces->engine->setViewAngles({
      walkCfg.localViewAngles.x + relang.x * archAdjustment,
      walkCfg.localViewAngles.y + relang.y * archAdjustment,
      0.f
        });

    // If the view is close enough to the aimspot, stop aiming
    if (abs(relang.x) < 0.3f && abs(relang.y) < 0.3f) {
        walkCfg.startedAiming = -1.f;
        if (walkCfg.aimreason == 3 || (walkCfg.aimreason == 1 && walkCfg.enemyEntities.size() > 0)) {
            walkCfg.shouldFire = true;
        }
        walkCfg.aimreason = -1;


        return;
    }

    if ((abs(relang.x) < 0.3f && abs(relang.y) < 0.3f) || walkCfg.enemyEntities.size() < 1) {
        walkCfg.shouldFire = false;
    }
}

//populates a vector of enemy entities if any enemy is in sight (line of sight+fov check)
void Walkbot::enemiesRadar() noexcept {
    if (!walkCfg.enabled)
        return;
    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (!interfaces->engine->isInGame() || !interfaces->engine->isConnected())
        return;

    walkCfg.enemyEntities.clear();
    Trace trace; // allocate memory before loop to get fps $$$
    for (int i = 1; i <= interfaces->entityList->getHighestEntityIndex(); i++) {
        const auto entity = interfaces->entityList->getEntity(i);
        if (entity == nullptr || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get()))
            continue;
        if (fabs(AimHelpers::calculateRelativeAngle(localPlayer->getEyePosition(), entity->getEyePosition(), localPlayer->eyeAngles()).y) > 80)
            continue;

        interfaces->engineTrace->traceRay({ localPlayer->getEyePosition(),{entity->getEyePosition().x,entity->getEyePosition().y,entity->getEyePosition().z + 20.f} }, MASK_OPAQUE, localPlayer.get(), trace);
        if (trace.contents != 0)
            continue;

        walkCfg.enemyEntities.push_back(*entity);
    }
}

void Walkbot::handleLocatedEnemies() noexcept {
    if (!localPlayer || !localPlayer->isAlive())
        return;
    if (!interfaces->engine->isInGame())
        return;
    if (walkCfg.aimreason == 1)
        return;
    if (walkCfg.enemyEntities.size() < 1) {
        walkCfg.stopWalking = false;
        return;
    }
    walkCfg.stopWalking = true;
    if (walkCfg.enemyToAim > walkCfg.enemyEntities.size() - 1)
        walkCfg.enemyToAim = fmod(std::rand(), float(walkCfg.enemyEntities.size()));
    walkCfg.aimreason = 1;
    std::vector<int> bones = { 8, 8, 8, 4, 3, 7, 6, 5 };//head is defined three times so that it gets picked as the aim spot more often than other bones 
    walkCfg.aimspot = walkCfg.enemyEntities[walkCfg.enemyToAim].getBonePosition(bones[fmod(std::rand(), float(bones.size()))]);
    walkCfg.startedAiming = memory->globalVars->realtime;

    walkCfg.botState = 2;
}

//proper pathfinding
void Walkbot::findBreakable(UserCmd* cmd) noexcept {
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    if (!walkCfg.enabled)
        return;
    if (!interfaces->engine->isInGame())
        return;

    Trace traceXp; //front,back,right,left
    walkCfg.localViewAngles = cmd->viewangles;
    Vector viewangleshit{ localPlayer->getEyePosition().x + cos(Helpers::deg2rad(walkCfg.localViewAngles.y + 19.9f * walkCfg.tracerayAngle)) * 60,
                                localPlayer->getEyePosition().y + sin(Helpers::deg2rad(walkCfg.localViewAngles.y + 19.9f * walkCfg.tracerayAngle)) * 60,
                                localPlayer->getEyePosition().z };
    const Vector lPlayerEyes = localPlayer->getEyePosition();
    interfaces->engineTrace->traceRay({ lPlayerEyes, viewangleshit }, CONTENTS_WINDOW, localPlayer.get(), traceXp);
    if (walkCfg.tracerayAngle < 9)
        walkCfg.tracerayAngle++;
    else walkCfg.tracerayAngle = -9;
    if (traceXp.fraction < 1.0f && walkCfg.startedAiming == -1.f) {
        walkCfg.aimspot = viewangleshit;
        walkCfg.isShooterVisible = true;
        walkCfg.aimreason = 3;
    }
    if (walkCfg.shouldFire) {
        cmd->buttons |= UserCmd::IN_ATTACK;
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon->getWeaponType() == WeaponType::Pistol)
            walkCfg.shouldFire = false;
    }
}

int fallDamageCheck(Vector pos) noexcept {
    if (!localPlayer)
        return -1;
    if (!localPlayer->isAlive())
        return -1;
    //-0.000128943x^{2}+0.341019x-65.806 <- calculates fall damage kinda well 

    Trace trace;
    float fallHeight, fallDamage;

    interfaces->engineTrace->traceRay({ {pos.x,pos.y,pos.z + 40.f},{pos.x,pos.y,pos.z - 1000.f} }, MASK_PLAYERSOLID, localPlayer.get(), trace);
    fallHeight = pos.distTo(trace.endpos);
    walkCfg.tempFloorPos = { trace.endpos };
    fallDamage = -0.000128943f * pow(fallHeight, 2.f) + 0.341019f * (fallHeight - 40.f) - 65.806f;


    return int(fallDamage);
}

//return 0 if there's no way to get to desired position, 1 if you can walk to get to pos,
//2 if you can get to pos by jumping,3 if you can get to pos by crouching, 4 if dropping
//down will cause you great pain (fall damage is over x% of health left)
int collisionCheck(Vector pos, Vector parentpos) noexcept {

    if (!localPlayer)
        return -1;
    //if (!walkCfg.shouldwalk)
    //    return;

    
    if (!interfaces->engine->isInGame())
        return -1;
    walkCfg.tracez.clear();

    if (fallDamageCheck(pos) > localPlayer->health() * walkCfg.dropdownDmg)
        return 4;

    pos.z = walkCfg.tempFloorPos.z + 18.f;      //set node pos to floor height

    walkCfg.checkOrigin = { pos.x, pos.y, pos.z };//{ pos.x,pos.y,pos.z + 18.f };
    //H for horizontal, V for vertical, D for diagonal
    Trace traceHBottom1, traceHBottom2, traceHBottomD1, traceHBottomD2, traceHMiddle1, traceHMiddle2, traceHMiddleD1, traceHMiddleD2, traceHTop1, traceHTop2, traceHTopD1, traceHTopD2, traceToParent;

    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z} }, MASK_PLAYERSOLID, localPlayer.get(), traceHBottom1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z},{walkCfg.checkOrigin.x,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z} }, MASK_PLAYERSOLID, localPlayer.get(), traceHBottom2);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z} }, MASK_PLAYERSOLID, localPlayer.get(), traceHBottomD1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z} }, MASK_PLAYERSOLID, localPlayer.get(), traceHBottomD2);

    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z + 38.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z + 38.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHMiddle1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x,  walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f},{walkCfg.checkOrigin.x,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHMiddle2);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHMiddleD1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 38.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHMiddleD2);

    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z + 42.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y,walkCfg.checkOrigin.z + 42.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHTop1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x,  walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f},{walkCfg.checkOrigin.x,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHTop2);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHTopD1);
    interfaces->engineTrace->traceRay({ {walkCfg.checkOrigin.x + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f},{walkCfg.checkOrigin.x - walkCfg.nodeRadius / 2,walkCfg.checkOrigin.y + walkCfg.nodeRadius / 2,walkCfg.checkOrigin.z + 42.f} }, MASK_PLAYERSOLID, localPlayer.get(), traceHTopD2);

    interfaces->engineTrace->traceRay({ walkCfg.checkOrigin,parentpos }, MASK_PLAYERSOLID, localPlayer.get(), traceToParent);
    if (traceToParent.contents != 0)
        walkCfg.traceToParentIntersects = true;
    else walkCfg.traceToParentIntersects = false;
    const bool walkable{ (traceHBottom1.contents == 0 && traceHBottom2.contents == 0 && traceHBottomD1.contents == 0 && traceHBottomD2.contents == 0) },
        crouchable{ (traceHMiddle1.contents == 0 && traceHMiddle2.contents == 0 && traceHMiddleD1.contents == 0 && traceHMiddleD2.contents == 0) },
        jumpable{ (traceHTop1.contents == 0 && traceHTop2.contents == 0 && traceHTopD1.contents == 0 && traceHTopD2.contents == 0) },
        forcejumpable{ parentpos.z + 36.f < pos.z };

    //    const bool walkable{ (traceHBottom1.fraction == 1.0f && traceHBottom2.fraction == 1.0f && traceHBottomD1.fraction == 1.0f && traceHBottomD2.fraction == 1.0f) }, 
    //    crouchable{ (traceHMiddle1.fraction == 1.0f && traceHMiddle2.fraction == 1.0f && traceHMiddleD1.fraction == 1.0f && traceHMiddleD2.fraction == 1.0f) },
    //    jumpable{ (traceHTop1.fraction == 1.0f && traceHTop2.fraction == 1.0f && traceHTopD1.fraction == 1.0f && traceHTopD2.fraction == 1.0f) },

    if (forcejumpable)              //jump
        return 2;
    else if (walkable && crouchable && jumpable) //just walk
        return 1;
    else if (walkable && crouchable && !jumpable)    //crouch
        return 3;
    else return 0;
}

void Walkbot::drawPathfinding()noexcept {
    if (!walkCfg.enabled)
        return;
    if (!walkCfg.pathfindingDebug || !walkCfg.shouldDebug)
        return;
    if (!localPlayer)
        return;

    if (!interfaces->engine->isInGame())
        return;

    ImDrawList* dlist;
    dlist = ImGui::GetBackgroundDrawList();
    if (walkCfg.nodes.size() > 0) {
        for (uint32_t index = 0; index < walkCfg.nodes.size(); index++) {
            ImVec2 screenNodePos;
            Helpers::worldToScreenPixelAligned(walkCfg.nodes[index], screenNodePos);
            dlist->AddRectFilled({ screenNodePos.x - 13.f,screenNodePos.y - 13.f }, { screenNodePos.x + 13.f,screenNodePos.y + 20.f }, 0xCC333333);
            dlist->AddText({ screenNodePos.x - 12.f,screenNodePos.y - 13.f }, 0xFFFFFFFF, std::to_string(index).c_str());
            dlist->AddText({ screenNodePos.x - 12.f,screenNodePos.y }, 0xFFFFFFFF, std::to_string(walkCfg.nodesParents[index]).c_str());
            const char* walker{ "-" };
            switch (walkCfg.walkType[index]) {
            case 1:walker = "WALK"; break;
            case 2:walker = "JUMP"; break;
            case 3:walker = "CROUCH"; break;
            default:walker = "-"; break;
            }
            dlist->AddText({ screenNodePos.x - 12.f,screenNodePos.y + 13.f }, 0xFFFFFFFF, walker);
            dlist->AddText({ screenNodePos.x - 12, screenNodePos.y + 20.f }, 0xFFFFFFFF, std::to_string(walkCfg.fcost[index]).c_str());
            dlist->AddText(ImVec2(500, 125), 0xFFFFFFFF, std::to_string(std::distance(walkCfg.fcost.begin(), std::min_element(walkCfg.fcost.begin(), walkCfg.fcost.end()))).c_str());
        }
    }
    if (walkCfg.tracez.size() == 0 || !walkCfg.drawPathfindingTraces)
        return;
    else {
        std::vector<ImVec2>traceScreenPosStart, traceScreenPosEnd;
        for (uint32_t index = 0; index < walkCfg.tracez.size(); index++) {
            ImVec2 temp;
            Helpers::worldToScreenPixelAligned(walkCfg.tracez[index].startpos, temp);
            traceScreenPosStart.push_back(temp);
            Helpers::worldToScreenPixelAligned(walkCfg.tracez[index].endpos, temp);
            traceScreenPosEnd.push_back(temp);

            dlist->AddLine(traceScreenPosStart[index], traceScreenPosEnd[index], ((walkCfg.tracez[index].fraction < 1.0f || walkCfg.tracez[0].contents == 1) ? 0xDDFF0000 : 0xDD00FF00));
        }
    }

}

void Walkbot::addNeighborNodes() noexcept {
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    for (int index = 0; index < 8; index++) {
        Vector potentialOpen;
        Vector offset{ 0,0,0 };
        //2 3 4
        //1 d 5
        //0 7 6
        switch (index) {
        case 0:
            offset.x -= walkCfg.nodeRadius;
            offset.y -= walkCfg.nodeRadius;
            break;
        case 1:
            offset.x -= walkCfg.nodeRadius;
            break;
        case 2:
            offset.x -= walkCfg.nodeRadius;
            offset.y += walkCfg.nodeRadius;
            break;
        case 3:
            offset.y += walkCfg.nodeRadius;
            break;
        case 4:
            offset.x += walkCfg.nodeRadius;
            offset.y += walkCfg.nodeRadius;
            break;
        case 5:
            offset.x += walkCfg.nodeRadius;
            break;
        case 6:
            offset.x += walkCfg.nodeRadius;
            offset.y -= walkCfg.nodeRadius;
            break;
        case 7:
            offset.y -= walkCfg.nodeRadius;
            break;
        default:break;
        }
        potentialOpen = walkCfg.nodes[walkCfg.currentNode] + offset;
        if (walkCfg.currentNode != -1)
            walkCfg.fcost[walkCfg.currentNode] = 99999.f;
        if (std::find(walkCfg.nodes.begin(), walkCfg.nodes.end(), potentialOpen) != walkCfg.nodes.end())
            if (walkCfg.nodes[std::distance(walkCfg.nodes.begin(), std::find(walkCfg.nodes.begin(), walkCfg.nodes.end(), potentialOpen))].z < potentialOpen.z + walkCfg.nodeRadius &&
                walkCfg.nodes[std::distance(walkCfg.nodes.begin(), std::find(walkCfg.nodes.begin(), walkCfg.nodes.end(), potentialOpen))].z > potentialOpen.z - walkCfg.nodeRadius)
                continue;
        int collides = collisionCheck(potentialOpen, walkCfg.nodes[walkCfg.currentNode]);
        if (walkCfg.traceToParentIntersects && potentialOpen.z - walkCfg.tempFloorPos.z > 30.f) {
            potentialOpen += offset;
            collides = collisionCheck(potentialOpen, walkCfg.nodes[walkCfg.currentNode]);
            if (collides == 0 || collides == 4)
                potentialOpen -= offset;
        }

        if (collides == 0 || collides == 4)
            continue;
        Vector checkOffset = potentialOpen - offset;
        potentialOpen.z = walkCfg.tempFloorPos.z;

        walkCfg.nodes.push_back(potentialOpen);
        walkCfg.nodesType.push_back(false);
        walkCfg.nodesParents.push_back(walkCfg.currentNode);
        walkCfg.walkType.push_back(collides);
        walkCfg.parentOffset.push_back(offset);

        //unused right now, going to use it for optimization
                                    //uncomment  whenever a better "node-already-exists" detection is added, improves pathfinding 10000x
        float fcost = (/*walkCfg.improvedPathfinding ? walkCfg.nodes.back().distTo(localPlayer->getAbsOrigin()) : 0.f) + */walkCfg.nodes.back().distTo(walkCfg.finalDestination));
        if (collides == 3)
            fcost += walkCfg.nodeRadius;
        ;        walkCfg.fcost.push_back(fcost);

        if (walkCfg.nodes.back().distTo(walkCfg.finalDestination) < walkCfg.nodeRadius - 1.f)
        {
            walkCfg.pathFound = true;
            return;
        }
    }
}

void Walkbot::openNode(int nodeIndex) noexcept {
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;

    if (nodeIndex != 0 && walkCfg.nodesType[nodeIndex] == false) {
        walkCfg.currentNode = nodeIndex;
        walkCfg.nodesType[nodeIndex] = true;
        Walkbot::addNeighborNodes();
    }
}

void Walkbot::findPath() noexcept {
    if (!localPlayer || !localPlayer->isAlive())
        return;
    if (!(walkCfg.nodes.size() > 0))
        return;
    if (!walkCfg.shouldwalk)
        return;
    if (walkCfg.finalDestination.distTo({ 0.f,0.f,0.f }) == 0.f)
        return;


    if (!walkCfg.pathFound)
        openNode(std::distance(walkCfg.fcost.begin(), std::min_element(walkCfg.fcost.begin(), walkCfg.fcost.end())));
    else {
        if (walkCfg.curWayPoint == -2)
            walkCfg.curWayPoint = (walkCfg.nodes.size() - 1);
        while (walkCfg.curWayPoint != -1) {
            walkCfg.waypoints.push_back(walkCfg.nodes[walkCfg.curWayPoint]);
            walkCfg.waypointWalkType.push_back(walkCfg.walkType[walkCfg.curWayPoint]);
            walkCfg.waypointParentOffset.push_back(walkCfg.parentOffset[walkCfg.curWayPoint]);
            walkCfg.curWayPoint = walkCfg.nodesParents[walkCfg.curWayPoint];
        }
    }
}

void Walkbot::pathfind() noexcept {
    if (!walkCfg.enabled)
        return;
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    
    if (!interfaces->engine->isInGame())
        return;
    //resett
    walkCfg.pathFound = false;
    walkCfg.nodes.clear();
    walkCfg.nodesParents.clear();
    walkCfg.nodesType.clear();
    walkCfg.walkType.clear();
    walkCfg.fcost.clear();
    walkCfg.waypoints.clear();
    walkCfg.currentNode = 0;
    walkCfg.curWayPoint = -2;

    walkCfg.nodes.push_back(localPlayer->getAbsOrigin());
    fallDamageCheck(walkCfg.nodes.back());
    walkCfg.nodes.back().z = walkCfg.tempFloorPos.z;
    walkCfg.nodesType.push_back(true);
    walkCfg.nodesParents.push_back(-1);
    walkCfg.walkType.push_back(1);
    walkCfg.fcost.push_back(999999.f);
    Walkbot::addNeighborNodes();
}

void Walkbot::drawPath() noexcept {
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    if (!walkCfg.circlesOrCost)
        return;
    ImDrawList* dlist;
    ImDrawList* dlistfg;
    dlist = ImGui::GetBackgroundDrawList();
    dlistfg = ImGui::GetForegroundDrawList();
    for (uint32_t index = 0; index < walkCfg.nodes.size(); index++) {
        ImVec2 screenPosTL, screenPosTR, screenPosBL, screenPosBR, screenPosMID;
        Helpers::worldToScreenPixelAligned({ walkCfg.nodes[index].x,       walkCfg.nodes[index].y,       walkCfg.nodes[index].z + 13.f }, screenPosMID);
        Helpers::worldToScreenPixelAligned({ walkCfg.nodes[index].x - walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].y + walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].z }, screenPosTL);
        Helpers::worldToScreenPixelAligned({ walkCfg.nodes[index].x + walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].y + walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].z }, screenPosTR);
        Helpers::worldToScreenPixelAligned({ walkCfg.nodes[index].x - walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].y - walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].z }, screenPosBL);
        Helpers::worldToScreenPixelAligned({ walkCfg.nodes[index].x + walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].y - walkCfg.nodeRadius / 2.f,walkCfg.nodes[index].z }, screenPosBR);

        if (screenPosMID.x < 5 || screenPosTL.x < 5 || screenPosTR.x < 5 || screenPosBL.x < 5 || screenPosBR.x < 5 ||
            screenPosMID.y < 5 || screenPosTL.y < 5 || screenPosTR.y < 5 || screenPosBL.y < 5 || screenPosBR.y < 5)
            continue;
        if (walkCfg.nodesType[index]) {
            dlistfg->AddTriangleFilled(screenPosTL, screenPosTR, screenPosMID, 0x9944AA44);
            dlistfg->AddTriangleFilled(screenPosTL, screenPosBL, screenPosMID, 0x9944AA44);
            dlistfg->AddTriangleFilled(screenPosBL, screenPosBR, screenPosMID, 0x9944AA44);
            dlistfg->AddTriangleFilled(screenPosBR, screenPosTR, screenPosMID, 0x9944AA44);
            if (walkCfg.nodes[index].distTo(localPlayer->getEyePosition()) < 400.f) {
                dlistfg->AddLine(screenPosTR, screenPosBR, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosTL, screenPosBL, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosBL, screenPosMID, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosBR, screenPosMID, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosBR, screenPosBL, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosTL, screenPosMID, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosTR, screenPosMID, 0x99000000, 1.f);
                dlistfg->AddLine(screenPosTR, screenPosTL, 0x99000000, 1.f);
            }
        }
        else if (!walkCfg.nodesType[index]) {
            dlist->AddTriangleFilled(screenPosTL, screenPosTR, screenPosMID, 0x99AA4444);
            dlist->AddTriangleFilled(screenPosTL, screenPosBL, screenPosMID, 0x99AA4444);
            dlist->AddTriangleFilled(screenPosBL, screenPosBR, screenPosMID, 0x99AA4444);
            dlist->AddTriangleFilled(screenPosBR, screenPosTR, screenPosMID, 0x99AA4444);
            if (walkCfg.nodes[index].distTo(localPlayer->getEyePosition()) < 400.f) {
                dlist->AddLine(screenPosTR, screenPosBR, 0x99000000, 1.f);
                dlist->AddLine(screenPosTL, screenPosBL, 0x99000000, 1.f);
                dlist->AddLine(screenPosBL, screenPosMID, 0x99000000, 1.f);
                dlist->AddLine(screenPosBR, screenPosMID, 0x99000000, 1.f);
                dlist->AddLine(screenPosBR, screenPosBL, 0x99000000, 1.f);
                dlist->AddLine(screenPosTL, screenPosMID, 0x99000000, 1.f);
                dlist->AddLine(screenPosTR, screenPosMID, 0x99000000, 1.f);
                dlist->AddLine(screenPosTR, screenPosTL, 0x99000000, 1.f);
            }
        }

    }
    std::vector<ImVec2> points;
    if (walkCfg.waypoints.size() > 0) {
        for (uint32_t index = 0; index < walkCfg.waypoints.size(); index++) {
            points.push_back({ 0.f,0.f });
            Helpers::worldToScreenPixelAligned(walkCfg.waypoints[index], points.back());
        }
        for (uint32_t index = 0; index < points.size(); index++) {
            if (index != points.size() - 1)
                dlistfg->AddLine(points[index], points[index + 1], 0xFF8888FF, 3.f);
        }

    }
    ImVec2 screenPosFinal;
    Helpers::worldToScreenPixelAligned(walkCfg.finalDestination, screenPosFinal);
    dlist->AddCircleFilled(screenPosFinal, 15.f, 0xFFAA4444, 8);
}


void Walkbot::reload(UserCmd* cmd) noexcept {
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    if (!walkCfg.enabled || !walkCfg.autoreload)
        return;
    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (activeWeapon == nullptr)
        return;
    if (activeWeapon->getWeaponType() == WeaponType::Knife || activeWeapon->getWeaponType() == WeaponType::C4 || activeWeapon->getWeaponType() == WeaponType::Grenade || activeWeapon->getWeaponType() == WeaponType::Fists || activeWeapon->getWeaponType() == WeaponType::Melee || activeWeapon->getWeaponType() == WeaponType::Unknown || !activeWeapon->isWeapon())
        return;
    if (activeWeapon->getWeaponData()->maxClip * walkCfg.reloadIfClipPercent < activeWeapon->clip())
        return;

    if (memory->globalVars->realtime - walkCfg.lastReload < 5)
        return;

    for (int i = 1; i <= interfaces->entityList->getHighestEntityIndex(); i++) {
        const auto entity = interfaces->entityList->getEntity(i);
        if (entity == nullptr || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
            || !entity->isOtherEnemy(localPlayer.get()) || entity->gunGameImmunity())
            continue;

        if (entity->isVisible(entity->getEyePosition())) {
            walkCfg.lastTimeSawEnemy = memory->globalVars->realtime;
        }
    }
    if (memory->globalVars->realtime - walkCfg.lastTimeSawEnemy > walkCfg.reloadAfterXSeconds) {
        cmd->buttons |= UserCmd::IN_RELOAD;
        walkCfg.lastReload = memory->globalVars->realtime;
    }
    if (EnginePrediction::getFlags() & 13 && memory->globalVars->realtime == walkCfg.lastReload) { interfaces->engine->clientCmdUnrestricted("coverme"); };

}

void Walkbot::gotoBotzPos(UserCmd* cmd) noexcept {

    if (walkCfg.shouldScope) {
        cmd->buttons |= UserCmd::IN_ATTACK2;
        walkCfg.shouldScope = false;
    }
    if (walkCfg.shouldUnscope > 0) {
        cmd->buttons |= UserCmd::IN_ATTACK2;
        walkCfg.shouldUnscope--;
    }
    if (!walkCfg.enabled || !walkCfg.shouldwalk || !walkCfg.pathFound || walkCfg.stopWalking)
        return;
    if (localPlayer->isScoped())
        walkCfg.shouldUnscope = 2;
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    if (walkCfg.nodes.size() < 1)
        return;

    
    Vector relAngle;
    Vector worldpos;
    worldpos = (walkCfg.waypoints.size() > 0 ? walkCfg.waypoints.back() : localPlayer->getAbsOrigin());
    relAngle = AimHelpers::calculateRelativeAngle(localPlayer->getEyePosition(), worldpos, localPlayer->eyeAngles());
    //waypoint approximation will consider the point reached if the player is
    //within x units from the point i.e. the point is at x 1035 and approx is
    //15, the player will stop moving at 1035±15 and consider the point reached
    if (walkCfg.waypoints.size() > 0) {
        if ((localPlayer->getAbsOrigin().x - walkCfg.nodeRadius - 1.f < walkCfg.waypoints.back().x && walkCfg.waypoints.back().x < localPlayer->getAbsOrigin().x + walkCfg.nodeRadius - 1.f) &&
            (localPlayer->getAbsOrigin().y - walkCfg.nodeRadius - 1.f < walkCfg.waypoints.back().y && walkCfg.waypoints.back().y < localPlayer->getAbsOrigin().y + walkCfg.nodeRadius - 1.f) &&
            (localPlayer->getAbsOrigin().z - 50.f < walkCfg.waypoints.back().z && walkCfg.waypoints.back().z < localPlayer->getAbsOrigin().z + 50.f)) {
            walkCfg.waypoints.pop_back();
            walkCfg.waypointWalkType.pop_back();
        }
        else {
            if (walkCfg.aimAtPath) {
                walkCfg.startedAiming = memory->globalVars->realtime - walkCfg.reactionTime;
                if (walkCfg.aimreason == -1)
                    walkCfg.aimreason = 4;
                walkCfg.aimspot = { walkCfg.waypoints.back().x,walkCfg.waypoints.back().y,walkCfg.waypoints.back().z + 62.f };
            }
            //not in position yet, move closer/do other stuff while you're walking(todo)
            if (walkCfg.waypointWalkType.back() > 1) {
                cmd->buttons |= UserCmd::IN_DUCK;
            }
            if (walkCfg.waypointWalkType.back() == 2)
                cmd->buttons |= UserCmd::IN_JUMP;
            cmd->forwardmove = 250 * cos(Helpers::deg2rad(relAngle.y));
            cmd->sidemove = 250 * sin(Helpers::deg2rad(relAngle.y)) * -1;
        }
    }




}



//static nodes pathfinding


void Walkbot::addNewNode(Vector pingPos) noexcept {
    if (walkCfg.walkbotType != 1)
        return;
    if (!localPlayer)
        return;
    if (!localPlayer->isAlive())
        return;
    
    if (!interfaces->engine->isInGame())
        return;
    const auto activeWeapon = localPlayer->getActiveWeapon();
    int nodeType{ 0 };
    switch (activeWeapon->getWeaponData()->price) {
    case 50:
        interfaces->engine->clientCmdUnrestricted("say holding a decoy rn");
        nodeType = 0;   //common
        break;
    case 200:
        interfaces->engine->clientCmdUnrestricted("say holding a flash grenade rn");
        nodeType = 1;   //bombsite A
        break;
    case 300:
        interfaces->engine->clientCmdUnrestricted("say holding a smoke/he grenade rn");
        nodeType = 2;   //bombsite B
        break;
    case 400:
    case 600:
        interfaces->engine->clientCmdUnrestricted("say holding a firebomb rn");
        nodeType = 3;   //camping spot
        break;
    default:interfaces->engine->clientCmdUnrestricted(std::to_string(activeWeapon->getWeaponData()->price).c_str());
    }
    walkCfg.presetNodes.push_back(pingPos);
    walkCfg.nodeGroup.push_back(nodeType);
}

void Walkbot::drawPresetNodes() noexcept {
    if (!walkCfg.circlesOrCost)
        return;
    if (walkCfg.walkbotType != 1)
        return;
    if (!localPlayer)
        return;
    

    if (!interfaces->engine->isInGame())
        return;


    ImDrawList* dlist;
    dlist = ImGui::GetBackgroundDrawList();
    for (uint32_t index = 0; index < walkCfg.presetNodes.size(); index++) {
        ImVec2 screenPosTL, screenPosTR, screenPosBL, screenPosBR, screenPosMID;
        ImU32 color;
        std::string nodetype;
        switch (walkCfg.nodeGroup[index]) {
        case 0:nodetype = "Common"; color = 0x99BBBBBB; break;
        case 1:nodetype = "Bombsite A"; color = 0x99FFA500; break;
        case 2:nodetype = "Bombiste B"; color = 0x99EBD934; break;
        case 3:nodetype = "Camping\n spot"; color = 0x99A534EB; break;
        }
        Helpers::worldToScreenPixelAligned({ walkCfg.presetNodes[index].x,walkCfg.presetNodes[index].y,walkCfg.presetNodes[index].z + 13.f }, screenPosMID);
        Helpers::worldToScreenPixelAligned({ walkCfg.presetNodes[index].x - 16.f,walkCfg.presetNodes[index].y + 16.f,walkCfg.presetNodes[index].z }, screenPosTL);
        Helpers::worldToScreenPixelAligned({ walkCfg.presetNodes[index].x + 16.f,walkCfg.presetNodes[index].y + 16.f,walkCfg.presetNodes[index].z }, screenPosTR);
        Helpers::worldToScreenPixelAligned({ walkCfg.presetNodes[index].x - 16.f,walkCfg.presetNodes[index].y - 16.f,walkCfg.presetNodes[index].z }, screenPosBL);
        Helpers::worldToScreenPixelAligned({ walkCfg.presetNodes[index].x + 16.f,walkCfg.presetNodes[index].y - 16.f,walkCfg.presetNodes[index].z }, screenPosBR);
        if (screenPosMID.x < 1 || screenPosTL.x < 1 || screenPosTR.x < 1 || screenPosBL.x < 1 || screenPosBL.x < 1)
            continue;

        dlist->AddTriangleFilled(screenPosTL, screenPosTR, screenPosMID, color);
        dlist->AddTriangleFilled(screenPosTL, screenPosBL, screenPosMID, color);
        dlist->AddTriangleFilled(screenPosBL, screenPosBR, screenPosMID, color);
        dlist->AddTriangleFilled(screenPosBR, screenPosTR, screenPosMID, color);
        dlist->AddLine(screenPosTR, screenPosBR, 0xCC000000, 2.f);
        dlist->AddLine(screenPosTL, screenPosBL, 0xCC000000, 2.f);
        dlist->AddLine(screenPosBL, screenPosMID, 0xCC000000, 2.f);
        dlist->AddLine(screenPosBR, screenPosMID, 0xCC000000, 2.f);
        dlist->AddLine(screenPosBR, screenPosBL, 0xCC000000, 2.f);
        dlist->AddLine(screenPosTL, screenPosMID, 0xCC000000, 2.f);
        dlist->AddLine(screenPosTR, screenPosMID, 0xCC000000, 2.f);
        dlist->AddLine(screenPosTR, screenPosTL, 0xCC000000, 2.f);

        //if(index=walkCfg.targetStaticNode)
          //  dlist->AddText(screenPosMID, 0xFFFFFF00, "TARGET");

        if (screenPosMID.x > ImGui::GetIO().DisplaySize.x / 2.f - 40.f && screenPosMID.x<ImGui::GetIO().DisplaySize.x / 2.f + 40.f && screenPosMID.y>ImGui::GetIO().DisplaySize.y / 2.f - 40.f && screenPosMID.y < ImGui::GetIO().DisplaySize.y / 2.f + 40.f) {
            dlist->AddText(ImVec2(screenPosMID.x + 1.f, screenPosMID.y + 1.f), 0xFF000000, nodetype.c_str());
            dlist->AddText(screenPosMID, 0xFFFFFFFF, nodetype.c_str());
            if (GetKeyState(VK_DELETE) & 0x8000) {
                walkCfg.presetNodes.erase(walkCfg.presetNodes.begin() + index);
                walkCfg.nodeGroup.erase(walkCfg.nodeGroup.begin() + index);
            }
        }
    }

}

void Walkbot::event(GameEvent& event) noexcept {
    walkCfg.playerPingLoc = { event.getFloat("x"), event.getFloat("y"), event.getFloat("z") };
    walkCfg.finalDestination = walkCfg.playerPingLoc;
    pathfind();
}

static bool windowOpen = false;

void Walkbot::drawGUI(bool contentOnly) noexcept {

    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        ImGui::Begin("Walkbot", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    ImGui::Checkbox("Enabled", &walkCfg.enabled);
    if (walkCfg.enabled) {
    ImGui::Checkbox("Should walk towards pos", &walkCfg.shouldwalk);

    //ImGui::Checkbox(translate.walkImprovedPfind[miscConfig.language].c_str(), &walkCfg.improvedPathfinding);
    //if (ImGui::IsItemHovered()) {
    //    ImGui::BeginTooltip();
    //    ImGui::Text(translate.walkImprovedPWarn[miscConfig.language].c_str());
    //    ImGui::EndTooltip();
    //}

    ImGui::Checkbox("Go towards teammate\'s pings", &walkCfg.shouldGoTowardsPing);
    ImGui::Checkbox("Aim at path", &walkCfg.aimAtPath);
    ImGui::Separator();

    ImGui::PushItemWidth(200.f);
    ImGui::SliderFloat("Max fall damage percent", &walkCfg.dropdownDmg, 0.0f, 0.99f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt("Node spacing", &walkCfg.nodeRadius, 2, 150, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::Checkbox("Draw nodes", &walkCfg.circlesOrCost);
    ImGui::Checkbox("Debug features", &walkCfg.shouldDebug);
    if (walkCfg.shouldDebug) {
        ImGui::Text("|");
        ImGui::Checkbox("Draw debug information", &walkCfg.pathfindingDebug);
        ImGui::Checkbox("Draw collision check traces", &walkCfg.drawPathfindingTraces);
        if (ImGui::Button("Force reset local pos"))
            pathfind();
    }
    ImGui::Separator();
    
    }

    if (!contentOnly)
        ImGui::End();
}

void Walkbot::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Walkbot")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Walkbot");
        ImGui::SetWindowPos("Walkbot", { 100.0f, 100.0f });
    }
}