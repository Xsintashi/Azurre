#include <algorithm>
#include <array>

#include "LegitAimbot.h"
#include "Backtrack.h"
#include <HelpersAimbot.h>

#include "../imguiCustom.h"
#include "../ConfigStructs.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/Cvar.h"
#include "../SDK/ConVar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/Constants/FrameStage.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/UserCmd.h"
#include "Animations.h"
#include "Tickbase.h"

static std::deque<Backtrack::incomingSequence> sequences;

struct BacktrackConfig {
    bool ignoreSmoke = false,
        recoilBasedFov = false,
        allchams = false,
        disablewhileDead;
    int timeLimit = 0;
    int fakeLatencyAmount = 0;
} backtrackConfig;

struct Cvars {
    ConVar* updateRate;
    ConVar* maxUpdateRate;
    ConVar* interp;
    ConVar* interpRatio;
    ConVar* minInterpRatio;
    ConVar* maxInterpRatio;
    ConVar* maxUnlag;
};

static Cvars cvars;

bool Backtrack::DisableFakeLagWhileBeingDead() noexcept {
    return backtrackConfig.disablewhileDead;
}

int Backtrack::TimeLimitRecive() noexcept {
    return backtrackConfig.timeLimit;
}

int Backtrack::fakeLatencyAmountRecive() noexcept {
    return backtrackConfig.fakeLatencyAmount;
}

bool Backtrack::DrawAllTick() noexcept {
    return backtrackConfig.allchams;
}

float Backtrack::getLerp() noexcept
{
    auto ratio = std::clamp(cvars.interpRatio->getFloat(), cvars.minInterpRatio->getFloat(), cvars.maxInterpRatio->getFloat());
    return (std::max)(cvars.interp->getFloat(), (ratio / ((cvars.maxUpdateRate) ? cvars.maxUpdateRate->getFloat() : cvars.updateRate->getFloat())));
}

void Backtrack::run(UserCmd* cmd) noexcept
{
    if (!backtrackConfig.timeLimit)
        return;

    if (!(cmd->buttons & UserCmd::IN_ATTACK))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    auto localPlayerEyePosition = localPlayer->getEyePosition();

    auto bestFov{ 255.f };
    int bestTargetIndex{ };
    int bestRecord{ };

    const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

    for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
        auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
            || !entity->isOtherEnemy(localPlayer.get()))
            continue;

        const auto player = Animations::getPlayer(i);
        if (!player.gotMatrix)
            continue;

        if (player.backtrackRecords.empty() || (!backtrackConfig.ignoreSmoke && memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), entity->getAbsOrigin(), 1)))
            continue;

        for (int j = static_cast<int>(player.backtrackRecords.size() - 1U); j >= 0; j--)
        {
            if (Backtrack::valid(player.backtrackRecords.at(j).simulationTime))
            {
                for (auto position : player.backtrackRecords.at(j).positions) {
                    auto angle = AimHelpers::calculateRelativeAngle(localPlayerEyePosition, position, cmd->viewangles + aimPunch);
                    auto fov = std::hypotf(angle.x, angle.y);
                    if (fov < bestFov) {
                        bestFov = fov;
                        bestRecord = j;
                        bestTargetIndex = i;
                    }
                }
            }
        }
    }

    const auto player = Animations::getPlayer(bestTargetIndex);
    if (!player.gotMatrix)
        return;


    if (bestRecord) {
        const auto& record = player.backtrackRecords[bestRecord];
        cmd->tickCount = Helpers::timeToTicks(record.simulationTime + getLerp());
    }
}

void Backtrack::addLatencyToNetwork(NetworkChannel* network, float latency) noexcept
{
    for (auto& sequence : sequences)
    {
        if (memory->globalVars->serverTime() - sequence.servertime >= latency)
        {
            network->inReliableState = sequence.inreliablestate;
            network->inSequenceNr = sequence.sequencenr;
            break;
        }
    }
}

void Backtrack::updateIncomingSequences() noexcept
{
    static int lastIncomingSequenceNumber = 0;

    if (!localPlayer)
        return;

    auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return;

    if (network->inSequenceNr != lastIncomingSequenceNumber)
    {
        lastIncomingSequenceNumber = network->inSequenceNr;

        incomingSequence sequence{ };
        sequence.inreliablestate = network->inReliableState;
        sequence.sequencenr = network->inSequenceNr;
        sequence.servertime = memory->globalVars->serverTime();
        sequences.push_front(sequence);
    }

    while (sequences.size() > 2048)
        sequences.pop_back();
}

bool Backtrack::valid(float simtime) noexcept
{
    const auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return false;

    const auto deadTime = static_cast<int>(memory->globalVars->serverTime() - cvars.maxUnlag->getFloat());
    if (simtime < deadTime)
        return false;

    const auto extraTickbaseDelta = Tickbase::canShift(Tickbase::getTargetTickShift()) ? Helpers::ticksToTime(Tickbase::getTargetTickShift()) : 0.0f;
    const auto delta = std::clamp(network->getLatency(0) + network->getLatency(1) + getLerp(), 0.f, cvars.maxUnlag->getFloat()) - (memory->globalVars->serverTime() - extraTickbaseDelta - simtime);
    return std::abs(delta) <= 0.2f;
}

void Backtrack::init() noexcept
{
    cvars.updateRate = interfaces->cvar->findVar("cl_updaterate");
    cvars.maxUpdateRate = interfaces->cvar->findVar("sv_maxupdaterate");
    cvars.interp = interfaces->cvar->findVar("cl_interp");
    cvars.interpRatio = interfaces->cvar->findVar("cl_interp_ratio");
    cvars.minInterpRatio = interfaces->cvar->findVar("sv_client_min_interp_ratio");
    cvars.maxInterpRatio = interfaces->cvar->findVar("sv_client_max_interp_ratio");
    cvars.maxUnlag = interfaces->cvar->findVar("sv_maxunlag");
}

float Backtrack::getMaxUnlag() noexcept
{
    return cvars.maxUnlag->getFloat();
}

static bool backtrackWindowOpen = false;

void Backtrack::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Backtrack")) {
        backtrackWindowOpen = true;
        ImGui::SetWindowFocus("Backtrack");
        ImGui::SetWindowPos("Backtrack", { 100.0f, 100.0f });
    }
}

void Backtrack::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!backtrackWindowOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Backtrack", &backtrackWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::Checkbox("Ignore smoke", &backtrackConfig.ignoreSmoke);
    ImGui::Checkbox("Recoil based fov", &backtrackConfig.recoilBasedFov);
    ImGui::Checkbox("Draw All Ticks", &backtrackConfig.allchams);
    ImGui::Checkbox("Disable on Dead & Freeze Time", &backtrackConfig.disablewhileDead);
    ImGui::PushItemWidth(220.0f);
#if defined(_DEBUG)
    ImGui::SliderInt("##tl", &backtrackConfig.timeLimit, 0, static_cast<int>(cvars.maxUnlag->getFloat() * 1000), "Time Limit: %dms");
    ImGui::SliderInt("##pl", &backtrackConfig.fakeLatencyAmount, 0, static_cast<int>(cvars.maxUnlag->getFloat() * 1000), "Fake Ping: %dms");
#else
    ImGui::SliderInt("##tl", &backtrackConfig.timeLimit, 0, 200, "Time Limit: %dms");
    ImGui::SliderInt("##pl", &backtrackConfig.fakeLatencyAmount, 0, 200, "Fake Ping: %dms");
#endif
    ImGui::PopItemWidth();

    if (!contentOnly)
        ImGui::End();
}

static void to_json(json& j, const BacktrackConfig& o, const BacktrackConfig& dummy = {})
{
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Recoil based fov", recoilBasedFov);
    WRITE("Draw All Ticks", allchams);
    WRITE("Time limit", timeLimit);
    WRITE("Fake Ping", fakeLatencyAmount);
    WRITE("Disable Fake Ping while Dead", disablewhileDead);
}

json Backtrack::toJson() noexcept
{
    json j;
    to_json(j, backtrackConfig);
    return j;
}

static void from_json(const json& j, BacktrackConfig& b)
{
    read(j, "Ignore smoke", b.ignoreSmoke);
    read(j, "Recoil based fov", b.recoilBasedFov);
    read(j, "Draw All Ticks", b.allchams);
    read(j, "Time limit", b.timeLimit);
    read(j, "Fake Ping", b.fakeLatencyAmount);
    read(j, "Disable Fake Ping while Dead", b.disablewhileDead);
}

void Backtrack::fromJson(const json& j) noexcept
{
    from_json(j, backtrackConfig);
}

void Backtrack::resetConfig() noexcept
{
    backtrackConfig = {};
}
