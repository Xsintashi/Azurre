#pragma once

#include "../JsonForward.h"

#include "../Interfaces.h"
#include "../Memory.h"

namespace csgo { enum class FrameStage; }
enum class GameMode;
class GameEvent;
struct ImDrawList;
struct UserCmd;
struct Vector;
struct ViewSetup;
struct Color4;

namespace Misc
{
    bool FreeCamEnabled() noexcept;
    bool shouldRevealMoney() noexcept;
    bool FreeCamKey() noexcept;
    bool shouldRevealSuspect() noexcept;
    bool shouldDisableModelOcclusion() noexcept;
    bool shouldFixBoneMatrix() noexcept;
    bool isRadarHackOn() noexcept;
    bool isMenuKeyPressed() noexcept;
    float aspectRatio() noexcept;
    void colorPaletteWindow() noexcept;
    void FlashGame(int idx);
    void outOfAmmoNotification() noexcept;
    void drawSmokeTimerEvent(GameEvent* event) noexcept;
    void drawMolotovTimerEvent(GameEvent* event) noexcept;
    void drawSmokeTimer(ImDrawList* drawList) noexcept;
    void drawMolotovTimer(ImDrawList* drawList) noexcept;
    Color4 GetNadePredictLineColor() noexcept;
    Color4 GetNadePredictCircleColor() noexcept;
    Color4 GetNadePredictDmgColor() noexcept;
    bool AdBlockEnabled() noexcept;
    bool isRadarSquared() noexcept;
    bool isInChat() noexcept;
    bool unlockInventory() noexcept;
    bool shouldEnableSvPureBypass() noexcept;
    bool nadePrediction() noexcept;
    bool nadePredictionDmg() noexcept;
    void spectatorList() noexcept;
    void forceCrosshair(csgo::FrameStage stage) noexcept;
    void recoilCrosshair(csgo::FrameStage stage) noexcept;
    void watermark() noexcept;
    void showBinds() noexcept;
    void antiTKilling(UserCmd* cmd) noexcept;
    void GrenadeAnimationCancel(GameEvent& event) noexcept;
    void autoDefuseEvents(GameEvent& event);
    void autoDefuse(UserCmd* cmd) noexcept;
    void isBombBeingPlanted(GameEvent& event) noexcept;
    void isBombPlantingAborted(GameEvent& event) noexcept;
    void fastPlant(UserCmd*) noexcept;
    void initHiddenCvars() noexcept;
    void unlockHiddenCvars() noexcept;
    void drawBombTimer() noexcept;
    void hurtIndicator() noexcept;
    void drawBombTimer() noexcept;
    void drawPlayerList() noexcept;
    void fixTabletSignal() noexcept;
    void killMessage(GameEvent& event) noexcept;
    void DeadMessage(GameEvent& event) noexcept;
    void fixMovement(UserCmd* cmd, float yaw) noexcept;
    void antiAfkKick(UserCmd* cmd) noexcept;
    void fixMouseDelta(UserCmd* cmd) noexcept;
    void fixAnimationLOD(csgo::FrameStage stage) noexcept;
    void autoPistol(UserCmd* cmd) noexcept;
    void autoReload(UserCmd* cmd) noexcept;
    void revealRanks(UserCmd* cmd) noexcept;
    void moonwalk(UserCmd* cmd) noexcept;
    void playHitSound(GameEvent& event) noexcept;
    void killSound(GameEvent& event) noexcept;
    void BuyBot(GameEvent* event = nullptr) noexcept;
    void BlockInputWhileOpenGui(UserCmd* cmd) noexcept;
    void footstepESP(GameEvent& event);
    void purchaseList(GameEvent* event = nullptr) noexcept;
    void oppositeHandKnife(csgo::FrameStage stage) noexcept;
    void disableExtrapolation() noexcept;
    void forceRelayCluster() noexcept;
    void runReportbot() noexcept;
    void resetReportbot() noexcept;
    void preserveKillfeed(bool roundStart = false) noexcept;
    void killfeedChanger(GameEvent& event) noexcept;
    void customMVPSound(GameEvent& event) noexcept;
    void voteRevealer(GameEvent& event) noexcept;
    void onVoteStart(const void* data, int size) noexcept;
    void onVotePass() noexcept;
    void onVoteFailed() noexcept;
    void guiVoteRevealer() noexcept;
    void drawOffscreenEnemies(ImDrawList* drawList) noexcept;
    void faceitSimulator(GameEvent* event, int eventID) noexcept;
    void autoAccept(const char* soundEntry) noexcept;
    void overheadInfo() noexcept;
    void getRoll(UserCmd* cmd) noexcept;
    void indicators() noexcept;
    void runFreeCam(UserCmd* cmd, Vector viewAngles) noexcept;
    void freeCam(ViewSetup* setup) noexcept;
    void drawAimbotFov(ImDrawList* drawList) noexcept;
    void visualizeAccuracy(ImDrawList* drawList) noexcept;

    GameMode getGameMode() noexcept;

    void updateEventListeners(bool forceRemove = false) noexcept;
    void updateInput() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
