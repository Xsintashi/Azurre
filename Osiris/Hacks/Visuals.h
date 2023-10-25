#pragma once

#include "../JsonForward.h"
#include <array>
#include <ConfigStructs.h>

namespace csgo { enum class FrameStage; }
class GameEvent;
struct ImDrawList;
struct ViewSetup;

namespace Visuals
{
    ColorToggle colorMolotov() noexcept;
    ColorToggle colorSmoke() noexcept;
    ColorToggle colorZeusLaser() noexcept;
    ColorToggle colorHeadShot() noexcept;
    bool jiggleBones() noexcept;
    bool noBlood() noexcept;
    bool noBoob() noexcept;
    bool customScopeCrosshirEnabled() noexcept;
    bool isThirdpersonOn() noexcept;
    bool isZoomOn() noexcept;
    bool isDeagleSpinnerOn() noexcept;
    bool shouldRemoveFog() noexcept;
    bool shouldRemoveScopeOverlay() noexcept;
    bool shouldRemoveSmoke() noexcept;
    bool noWeapons() noexcept;
    bool FogEnabled() noexcept;
    bool removeHands(const char* modelName) noexcept;
    bool removeSleeves(const char* modelName) noexcept;
    bool removeWeapons(const char* modelName) noexcept;
    void customScopeCrosshair(ImDrawList* drawList) noexcept;
    bool noZoom() noexcept;
    void maskChanger(csgo::FrameStage stage) noexcept;
    void noScopeSway() noexcept;
    char* maskChar() noexcept;
    ColorToggle crosshairOutlineColor() noexcept;
    ColorToggle consoleColor() noexcept;
    float getScopePlayerTransparency() noexcept;
    int netGraphFont() noexcept;
    ColorToggle netGraphEnabled() noexcept;
    float viewModelFov() noexcept;
    float fov() noexcept;
    float farZ() noexcept;
    float FogStart() noexcept;
    float FogEnd() noexcept;
    float FogDensity() noexcept;
    ColorToggle3 FogColor() noexcept;
    ColorToggle3 worldColor() noexcept;
    void shadowChanger() noexcept;
    void performColorCorrection() noexcept;
    void ragdolls() noexcept;
    void doBloomEffects() noexcept;
    void motionBlur(ViewSetup* setup) noexcept;
    void particleFootTrail() noexcept;
    void colorConsole() noexcept;
    void modifySmoke(csgo::FrameStage stage) noexcept;
    void modifyMolotov(csgo::FrameStage stage) noexcept;
    void thirdperson() noexcept;
    void removeVisualRecoil(csgo::FrameStage stage) noexcept;
    void removeBlur(csgo::FrameStage stage) noexcept;
    void updateBrightness() noexcept;
    void fullBright() noexcept;
    void noHUDBlur() noexcept;
    void removeGrass(csgo::FrameStage stage) noexcept;
    void remove3dSky() noexcept;
    void removeSprites() noexcept;
    void removeShadows() noexcept;
    void removeWeaponSway() noexcept;
    void viewModelChanger(ViewSetup* setup) noexcept;
    void applyZoom(csgo::FrameStage stage) noexcept;
    void applyScreenEffects() noexcept;
    void hitEffect(GameEvent* event = nullptr) noexcept;
    void damageNumberEvent(GameEvent* event = nullptr, bool reset = false) noexcept;
    void damageNumber(ImDrawList* drawList) noexcept;
    void hitMarker(GameEvent* event, ImDrawList* drawList = nullptr) noexcept;
    void disablePostProcessing(csgo::FrameStage stage) noexcept;
    float reduceFlashEffect() noexcept;
    void playerModel(csgo::FrameStage stage) noexcept;
    void skybox(csgo::FrameStage stage) noexcept;
    void DLights(Entity* entity);
    void RunDLights();
    void killLightning(GameEvent& event) noexcept;
    void drawHitboxMatrix(GameEvent* event = nullptr) noexcept;
    void bulletTracer(GameEvent& event) noexcept;
    void drawMolotovPolygon(ImDrawList* drawList) noexcept;
    void drawSmokeHull(ImDrawList* drawList) noexcept;
    void drawBulletImpacts() noexcept;
    void bulletImpact(GameEvent& event) noexcept;
    void Flashlight(csgo::FrameStage stage) noexcept;
    void dontRenderTeammates() noexcept;
    void asus() noexcept;

    void hideHUD() noexcept;


    inline constexpr std::array skyboxList{ "Default", "cs_baggage_skybox_", "cs_tibet", "embassy", "italy", "jungle", "nukeblank", "office", "sky_cs15_daylight01_hdr", "sky_cs15_daylight02_hdr", "sky_cs15_daylight03_hdr", "sky_cs15_daylight04_hdr", "sky_csgo_cloudy01", "sky_csgo_night_flat", "sky_csgo_night02", "sky_day02_05_hdr", "sky_day02_05", "sky_dust", "sky_l4d_rural02_ldr", "sky_venice", "vertigo_hdr", "vertigo", "vertigoblue_hdr", "vietnam", "sky_lunacy", "sky_hr_aztec", "custom"};

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
