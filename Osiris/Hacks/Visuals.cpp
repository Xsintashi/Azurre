#include <array>
#include <cstring>
#include <string_view>
#include <utility>
#include <vector>
#include <iomanip>
#include <deque>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"

#include "../ConfigStructs.h"
#include "../fnv.h"
#include "../GameData.h"
#include "../Helpers.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../imguiCustom.h"

#include "Animations.h"
#include "Backtrack.h"
#include "Visuals.h"
#include "Misc.h"

#include "../SDK/ConVar.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ClientMode.h"
#include "../SDK/Client.h"
#include "../SDK/Cvar.h"
#include "../SDK/DebugOverlay.h"
#include "../SDK/Effects.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/Constants/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/NetworkStringTable.h"
#include "../SDK/ViewRenderBeams.h"
#include "../SDK/VEffects.h"
#include "../SDK/ViewSetup.h"
#include "../SDK/Panel.h"
#include "../SDK/DLight.h"

#include <GUI.h>
#include <pstring.h>
#include <HelpersAimbot.h>

#pragma warning (disable : 4244)

struct BulletTracers : ColorToggle {
    BulletTracers() : ColorToggle{ 1.f, 1.f, 1.f, 1.f } {}
};

struct VisualsConfig {
    bool zoom = false;
    bool freeCam = false;
    bool thirdperson = false;
    bool fullBright = false;
    bool deagleSpinner = false;
    bool killLightningBolt = false;
    bool flashlight = false;
    bool jigglebones = false;
    bool maskChanger = false;
    bool footTrail = false;
    bool dontRenderTeammates = false;
    int thirdpersonDistance = 100;
    int fov = 0;
    int farZ = 2000;
    int flashReduction = 0;
    int skybox = 0;
    int removals = 0;
    int smokeMode = 0;
    int moloMode = 0;
    int hideHud = 0;
    float thirdpersonTransparency = 0.f;
    float brightness = 0.f;
    ColorToggle3 world;
    ColorToggle3 sky;
    ColorToggle3 props;
    ColorToggle console;
    KeyBindToggle zoomKey;
    KeyBindToggle thirdpersonKey;
    KeyBindToggle flashLight;
    ColorToggle damageNumber{ 1.0f, 1.0f, 0.0f, 1.0f };
    ColorToggle molotovColor{ 1.0f, 0.27f, 0.0f, 0.5f };
    ColorToggle smokeHull{ 1.f, 1.f, 1.f, 1.f };
    ColorToggle smokeColor{ .75f, .75f, .75f, 0.5f};
    ColorToggle zeusLaserColor{ 1.f, 1.f, 1.f, 1.f};
    ColorToggle headShotColor{ 1.f, 1.f, 1.f, 1.f};
    ColorToggle crosshairOutlineColor{ 1.f, 1.f, 1.f, 1.f};

    struct MolotovPolygon {
        bool enabled = false;
        Color4 enemy{ 1.0f, 0.0f, 0.0f, 0.3f };
        Color4 ally{ 0.0f, 0.0f, 1.0f, 0.3f };
        Color4 self{ 1.0f, 0.27f, 0.0f, 0.3f };
    } molotovPolygon;

    BulletTracers dlightsColor;
    BulletTracers dlightsColor1;
    BulletTracers dlightsColor2;

    ColorToggle bulletImpacts{ 0.0f, 0.0f, 1.f, 0.5f };
    float bulletImpactsTime = 4.f;

    int screenEffect = 0;
    int hitEffect = 0;
    int killEffect = 0;
    int hitMarker = 0;
    float hitEffectTime = 0.5f;
    float killEffectTime = 0.5f;
    float hitMarkerTime = 0.5f;
    Color3 hitMarkerColor{ 1.f, 1.f, 1.f };

    struct Ragdoll {
        bool enabled =  false,
         disabled = false;
        int gravity = 600,
            force = 0;
        float timescale = 1.f;
    }ragdolls;

    struct Weather
    {
        bool enabled = false;
        int type = 0;
        float length{ 0.1f };
        int windSpeed = 0;
        float width = 0.5f;
        int sideVel = 130;
        float alpha = 0.4f;
    } weather;

    struct CustomScopeCrosshair {
        ColorToggle color;
        int mode = 0;
        float offset = 100.f,
              height = 300.f,
              speed  = 70.f;
    } customScopeCrosshair;

    BulletTracers bulletTracers;
    char playerModel[256]{ };
    char maskModel[256]{ };
    char customSkybox[256]{ };

    struct DLightSettings {
        bool enabled = false,
             local = true,
             allies = true,
             enemies = true;
       float radius = 100.f,
             decay = 20.f;
    }dLight;

    struct BeamBullets {
        float amplitude = 0.f, life = 2.f;
        int textureid = 0;
    };
    BeamBullets beambullets;

    struct Viewmodel
    {
        bool enabled = false;
        int Fov = 0;
        float x = 0.f;;
        float y = 0.f;;
        float z = 0.f;;
        float roll = 0.f;;
    } viewModel;

    struct MotionBlur
    {
        bool enabled = false;
        bool forwardEnabled = false;
        float fallingMin = 10.0f;
        float fallingMax = 20.0f;
        float fallingIntensity = 1.f;
        float rotationIntensity = 1.f;
        float strength = 1.f;
    } motionBlur;

    struct CustomPostProcessing {
        bool enabled = false;
        float worldExposure = 0.0f;
        float modelAmbient = 0.0f;
        float bloomScale = 0.0f;
    } customPostProcessing;

    struct Asus {
        bool enabled = false;
        float props = 1.f;
        float walls = 1.f;
    }asus;

    struct Fog{
        float FogStart = 0.f, FogDens = 0.001f, FogEnd = 0;
        ColorToggle3 color;
    } fog;

    struct ColorCorrection {
        bool enabled = false;
        float blue = 0.0f;
        float red = 0.0f;
        float mono = 0.0f;
        float saturation = 0.0f;
        float ghost = 0.0f;
        float green = 0.0f;
        float yellow = 0.0f;
    } colorCorrection;

    struct ShadowsChanger
    {
        bool enabled = false;
        int x = 0;
        int y = 0;
    } shadowsChanger;

    struct NetGraph {
        int font = -1;
        ColorToggle color{ 0.8981f, 0.8981f, 0.6981f, 1.f };
    } netGraph;

    struct HitMatrix {
        float duration = 2.f;
        ColorToggle color { 1.f, 1.f, 1.f, 1.f };
    } hitMatrix;

} vCfg;

char* Visuals::maskChar() noexcept {
    return vCfg.maskModel;
}

ColorToggle Visuals::crosshairOutlineColor() noexcept {
    return vCfg.crosshairOutlineColor;
}

ColorToggle Visuals::consoleColor() noexcept {
    return vCfg.console;
}

float Visuals::getScopePlayerTransparency() noexcept {
    return vCfg.thirdpersonTransparency;
}

int Visuals::netGraphFont() noexcept {
    return vCfg.netGraph.font;
}

ColorToggle Visuals::netGraphEnabled() noexcept {
    return vCfg.netGraph.color;
}

void Visuals::updateInput() noexcept
{
    vCfg.thirdpersonKey.handleToggle();
    vCfg.zoomKey.handleToggle();
    vCfg.flashLight.handleToggle();
}

static void from_json(const json& j, VisualsConfig::ColorCorrection& c)
{
    read(j, "Enabled", c.enabled);
    read(j, "Blue", c.blue);
    read(j, "Red", c.red);
    read(j, "Mono", c.mono);
    read(j, "Saturation", c.saturation);
    read(j, "Ghost", c.ghost);
    read(j, "Green", c.green);
    read(j, "Yellow", c.yellow);
}

static void from_json(const json& j, VisualsConfig::CustomPostProcessing& cpp)
{
    read(j, "Enabled", cpp.enabled);
    read(j, "World Exposure", cpp.worldExposure);
    read(j, "Model Ambient", cpp.modelAmbient);
    read(j, "Bloom Scale", cpp.bloomScale);

}

static void from_json(const json& j, VisualsConfig::DLightSettings& dl) {
   read(j, "Enabled", dl.enabled);
   read(j, "Allies", dl.allies);
   read(j, "Enemies", dl.enemies);
   read(j, "Radius", dl.radius);
   read(j, "Decay", dl.decay);
}

static void from_json(const json& j, VisualsConfig::MotionBlur& mb)
{
    read(j, "Enabled", mb.enabled);
    read(j, "Forward", mb.forwardEnabled);
    read(j, "Falling min", mb.fallingMin);
    read(j, "Falling max", mb.fallingMax);
    read(j, "Falling intensity", mb.fallingIntensity);
    read(j, "Rotation intensity", mb.rotationIntensity);
    read(j, "Strength", mb.strength);
}

static void from_json(const json& j, VisualsConfig::NetGraph& ng) {
    read(j, "Font", ng.font);
    read<value_t::object>(j, "Color", ng.color);
}

static void from_json(const json& j, VisualsConfig::HitMatrix& m) {
    read(j, "Time", m.duration);
    read<value_t::object>(j, "Color", m.color);
}

static void from_json(const json& j, VisualsConfig::ShadowsChanger& sw)
{
    read(j, "Enabled", sw.enabled);
    read(j, "X", sw.x);
    read(j, "Y", sw.y);
}

static void from_json(const json& j, VisualsConfig::Fog& sw)
{
    read<value_t::object>(j, "Color", sw.color);
    read(j, "Start", sw.FogStart);
    read(j, "End", sw.FogEnd);
    read(j, "Density", sw.FogDens);
}

static void from_json(const json& j, VisualsConfig::Viewmodel& vxyz)
{
    read(j, "Enabled", vxyz.enabled);
    read(j, "Fov", vxyz.Fov);
    read(j, "X", vxyz.x);
    read(j, "Y", vxyz.y);
    read(j, "Z", vxyz.z);
    read(j, "Roll", vxyz.roll);
}

static void from_json(const json& j, VisualsConfig::Asus& s)
{
    read(j, "Enabled", s.enabled);
    read(j, "Walls", s.walls);
    read(j, "Props", s.props);
}

static void from_json(const json& j, VisualsConfig::Ragdoll& s)
{
    read(j, "Enabled", s.enabled);
    read(j, "Disable", s.disabled);
    read(j, "Timescale", s.timescale);
    read(j, "Gravity", s.gravity);
    read(j, "Force", s.force);
}

static void from_json(const json& j, VisualsConfig::CustomScopeCrosshair& s)
{
    read<value_t::object>(j, "Enabled", s.color);
    read(j, "Mode", s.mode);
    read(j, "Offset", s.offset);
    read(j, "Height", s.height);
    read(j, "Speed", s.speed);
}

static void from_json(const json& j, VisualsConfig::MolotovPolygon& m)
{
    read(j, "Enabled", m.enabled);
    read<value_t::object>(j, "Allies", m.ally);
    read<value_t::object>(j, "Enemy", m.enemy);
    read<value_t::object>(j, "Self", m.self);
}

static void from_json(const json& j, VisualsConfig::Weather& r)
{
    read(j, "Enabled", r.enabled);
    read(j, "Type", r.type);
    read(j, "Length", r.length);
    read(j, "Wind speed", r.windSpeed);
    read(j, "Width", r.width);
    read(j, "Side velocity", r.sideVel);
    read(j, "Alpha", r.alpha);
}

static void from_json(const json& j, BulletTracers& o)
{
    from_json(j, static_cast<ColorToggle&>(o));
}

static void from_json(const json& j, VisualsConfig& v)
{
    read(j, "Smoke mode", v.smokeMode);
    read(j, "Molotov mode", v.moloMode);
    read(j, "Zoom", v.zoom);
    read(j, "Zoom key", v.zoomKey);
    read(j, "Thirdperson", v.thirdperson);
    read(j, "Thirdperson key", v.thirdpersonKey);
    read(j, "Thirdperson distance", v.thirdpersonDistance);
    read(j, "FOV", v.fov);
    read(j, "Far Z", v.farZ);
    read(j, "Flash reduction", v.flashReduction);
    read(j, "Nightmode", v.brightness);
    read(j, "Skybox", v.skybox);
    read(j, "Custom skybox", v.customSkybox, sizeof(v.customSkybox));
    read<value_t::object>(j, "World", v.world);
    read<value_t::object>(j, "Sky", v.sky);
    read<value_t::object>(j, "Console", v.console);
    read(j, "Deagle spinner", v.deagleSpinner);
    read(j, "Screen effect", v.screenEffect);
    read(j, "Hit effect", v.hitEffect);
    read(j, "Hit effect time", v.hitEffectTime);
    read(j, "Hit marker", v.hitMarker);
    read(j, "Hit marker time", v.hitMarkerTime);
    read<value_t::object>(j, "Hit marker color", v.hitMarkerColor);
    read<value_t::object>(j, "Color correction", v.colorCorrection);
    read<value_t::object>(j, "Bullet Tracers", v.bulletTracers);
    read<value_t::object>(j, "Molotov Polygon", v.molotovPolygon);
    read<value_t::object>(j, "Smoke Hull", v.smokeHull);
    read<value_t::object>(j, "Beam Bullets", v.beambullets);
    read<value_t::object>(j, "Custom Post Processing", v.customPostProcessing);
    read<value_t::object>(j, "Motion Blur", v.motionBlur);
    read<value_t::object>(j, "Asus", v.asus);
    read<value_t::object>(j, "Ragdolls", v.ragdolls);
    read(j, "Custom Model", v.playerModel, sizeof(v.playerModel));
    read<value_t::object>(j, "Shadows changer", v.shadowsChanger);
    read<value_t::object>(j, "Fog changer", v.fog);
    read<value_t::object>(j, "Viewmodel", v.viewModel);
    read<value_t::object>(j, "DLight", v.dLight);
    read<value_t::object>(j, "Allies Color", v.dlightsColor);
    read<value_t::object>(j, "Self Color", v.dlightsColor1);
    read<value_t::object>(j, "Enemies Color", v.dlightsColor2);
    read(j, "Thunderbolt Kills", v.killLightningBolt);
    read(j, "Removals", v.removals);
    read(j, "Jiggle Bones", v.jigglebones);
    read(j, "Mask", v.maskChanger);
    read(j, "Foot Trail", v.footTrail);
    read(j, "Thirdperson Transparency", v.thirdpersonTransparency);
    read<value_t::object>(j, "Net Graph", v.netGraph);
    read<value_t::object>(j, "Scope Crosshair", v.customScopeCrosshair);
    read<value_t::object>(j, "Damage Number Color", v.damageNumber);
    read<value_t::object>(j, "Smoke Color", v.smokeColor);
    read<value_t::object>(j, "Molotov Color", v.molotovColor);
    read<value_t::object>(j, "Zeus Laser Color", v.zeusLaserColor);
    read<value_t::object>(j, "Headshot Color", v.headShotColor);
    read<value_t::object>(j, "Crosshair Outline Color", v.crosshairOutlineColor);
    read<value_t::object>(j, "Weather", v.weather);
    read<value_t::object>(j, "Bullet Impacts", v.bulletImpacts);
    read<value_t::object>(j, "On Hit Matrix", v.hitMatrix);
    read(j, "Bullet Impacts time", v.bulletImpactsTime);
}

static void from_json(const json& j, VisualsConfig::BeamBullets& b)
{
    read(j, "Power", b.amplitude);
    read(j, "Lifetime", b.life);
    read(j, "Texture", b.textureid);

}

static void to_json(json& j, const VisualsConfig::BeamBullets& o, const VisualsConfig::BeamBullets& dummy = {}) {

    WRITE("Power", amplitude);
    WRITE("Lifetime", life);
    WRITE("Texture", textureid);
}

static void to_json(json& j, const VisualsConfig::Asus& o, const VisualsConfig::Asus& dummy = {}) {

    WRITE("Enabled", enabled);
    WRITE("Walls", walls);
    WRITE("Props", props);
}

static void to_json(json& j, const VisualsConfig::Fog& o, const VisualsConfig::Fog& dummy)
{
    WRITE("Color", color);
    WRITE("Start", FogStart);
    WRITE("End", FogEnd);
    WRITE("Density", FogDens);
}

static void to_json(json& j, const VisualsConfig::HitMatrix& o, const VisualsConfig::HitMatrix& dummy) {
    WRITE("Time", duration);
    WRITE("Color", color);
}

static void to_json(json& j, const VisualsConfig::NetGraph& o, const VisualsConfig::NetGraph& dummy)
{
    WRITE("Font", font);
    WRITE("Color", color);
}


static void to_json(json& j, const VisualsConfig::ShadowsChanger& o, const VisualsConfig::ShadowsChanger& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const VisualsConfig::CustomPostProcessing& o, const VisualsConfig::CustomPostProcessing& dummy = {}) {

    WRITE("Enabled", enabled);
    WRITE("World Exposure", worldExposure);
    WRITE("Model Ambient", modelAmbient);
    WRITE("Bloom Scale", bloomScale);
}

static void to_json(json& j, const VisualsConfig::DLightSettings& o, const VisualsConfig::DLightSettings& dummy) {
    WRITE("Enabled", enabled);
    WRITE("Allies", allies);
    WRITE("Enemies", enemies);
    WRITE("Radius", radius);
    WRITE("Decay", decay);
}
static void to_json(json& j, const VisualsConfig::MotionBlur& o, const VisualsConfig::MotionBlur& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Forward", forwardEnabled);
    WRITE("Falling min", fallingMin);
    WRITE("Falling max", fallingMax);
    WRITE("Falling intensity", fallingIntensity);
    WRITE("Rotation intensity", rotationIntensity);
    WRITE("Strength", strength);
}

static void to_json(json& j, const VisualsConfig::Ragdoll& o, const VisualsConfig::Ragdoll& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Disable", disabled);
    WRITE("Timescale", timescale);
    WRITE("Gravity", gravity);
    WRITE("Force", force);
}
static void to_json(json& j, const VisualsConfig::Viewmodel& o, const VisualsConfig::Viewmodel& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Fov", Fov);
    WRITE("X", x);
    WRITE("Y", y);
    WRITE("Z", z);
    WRITE("Roll", roll);
}

static void to_json(json& j, const VisualsConfig::ColorCorrection& o, const VisualsConfig::ColorCorrection& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Blue", blue);
    WRITE("Red", red);
    WRITE("Mono", mono);
    WRITE("Saturation", saturation);
    WRITE("Ghost", ghost);
    WRITE("Green", green);
    WRITE("Yellow", yellow);
}

static void to_json(json& j, const VisualsConfig::CustomScopeCrosshair& o, const VisualsConfig::CustomScopeCrosshair& dummy)
{

    WRITE("Enabled", color);
    WRITE("Mode", mode);
    WRITE("Offset", offset);
    WRITE("Height", height);
    WRITE("Speed", speed);

}

static void to_json(json& j, const VisualsConfig::Weather& o, const VisualsConfig::Weather& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Type", type);
    WRITE("Length", length);
    WRITE("Wind speed", windSpeed);
    WRITE("Width", width);
    WRITE("Side velocity", sideVel);
    WRITE("Alpha", alpha);
}

static void to_json(json& j, const VisualsConfig::MolotovPolygon& o, const VisualsConfig::MolotovPolygon& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Allies", ally);
    WRITE("Enemy", enemy);
    WRITE("Self", self);
}

static void to_json(json& j, const BulletTracers& o, const BulletTracers& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
}

static void to_json(json& j, const VisualsConfig& o)
{
    const VisualsConfig dummy;

    WRITE("Ragdolls", ragdolls);
    WRITE("Smoke mode", smokeMode);
    WRITE("Molotov mode", moloMode);
    WRITE("Zoom", zoom);
    WRITE("Zoom key", zoomKey);
    WRITE("Thirdperson", thirdperson);
    WRITE("Thirdperson key", thirdpersonKey);
    WRITE("Thirdperson distance", thirdpersonDistance);
    WRITE("FOV", fov);
    WRITE("Far Z", farZ);
    WRITE("Flash reduction", flashReduction);
    WRITE("Nightmode", brightness);
    WRITE("Skybox", skybox);
    if (o.customSkybox[0])
        j["Custom skybox"] = o.customSkybox;
    WRITE("World", world);
    WRITE("Sky", sky);
    WRITE("Console", console);
    WRITE("Asus", asus);
    WRITE("Deagle spinner", deagleSpinner);
    WRITE("Screen effect", screenEffect);
    WRITE("Hit effect", hitEffect);
    WRITE("Hit effect time", hitEffectTime);
    WRITE("Hit marker", hitMarker);
    WRITE("Hit marker time", hitMarkerTime);
    WRITE("Hit marker color", hitMarkerColor);
    WRITE("Color correction", colorCorrection);
    WRITE("Bullet Tracers", bulletTracers);
    WRITE("Molotov Polygon", molotovPolygon);
    WRITE("Smoke Hull", smokeHull);
    WRITE("Beam Bullets", beambullets);
    WRITE("Custom Post Processing", customPostProcessing);
    WRITE("Motion Blur", motionBlur);
    if (o.playerModel[0])
        j["Custom Model"] = o.playerModel;

    WRITE("Viewmodel", viewModel);
    WRITE("Shadows changer", shadowsChanger);
    WRITE("Fog changer", fog);
    WRITE("DLight", dLight);
    WRITE("Allies Color", dlightsColor);
    WRITE("Self Color", dlightsColor1);
    WRITE("Enemies Color", dlightsColor2);
    WRITE("Thunderbolt Kills", killLightningBolt);
    WRITE("Removals", removals);
    WRITE("Jiggle Bones", jigglebones);
    WRITE("Mask", maskChanger);
    WRITE("Foot Trail", footTrail);
    WRITE("Thirdperson Transparency", thirdpersonTransparency);
    WRITE("Net Graph", netGraph);
    WRITE("Scope Crosshair", customScopeCrosshair);
    WRITE("Damage Number Color", damageNumber);
    WRITE("Smoke Color", smokeColor);
    WRITE("Molotov Color", molotovColor);
    WRITE("Zeus Laser Color", zeusLaserColor);
    WRITE("Headshot Color", headShotColor);
    WRITE("Crosshair Outline Color", crosshairOutlineColor);
    WRITE("Weather", weather);
    WRITE("On Hit Matrix", hitMatrix);
    WRITE("Bullet Impacts", bulletImpacts);
    WRITE("Bullet Impacts time", bulletImpactsTime);
}

ColorToggle Visuals::colorMolotov()noexcept {
    return vCfg.molotovColor;
}

ColorToggle Visuals::colorSmoke()noexcept {
    return vCfg.smokeColor;
}

ColorToggle Visuals::colorZeusLaser()noexcept {
    return vCfg.zeusLaserColor;
}

ColorToggle Visuals::colorHeadShot()noexcept {
    return vCfg.headShotColor;
}

bool Visuals::jiggleBones() noexcept {
    return vCfg.jigglebones;
}

bool Visuals::noBlood() noexcept {
    return Helpers::getByteFromBytewise(vCfg.removals, 17);
}

bool Visuals::noBoob() noexcept { // No boobs??? ;_;
    return Helpers::getByteFromBytewise(vCfg.removals, 16);
}

void Visuals::noScopeSway() noexcept {
    static auto convar = interfaces->cvar->findVar("cl_use_new_headbob");
    convar->onChangeCallbacks.size = 0b0;
    convar->setValue(!Helpers::getByteFromBytewise(vCfg.removals, 2));
}

bool Visuals::customScopeCrosshirEnabled() noexcept {
    return vCfg.customScopeCrosshair.color.enabled;
}

bool Visuals::isThirdpersonOn() noexcept
{
    return vCfg.thirdperson;
}

bool Visuals::isZoomOn() noexcept
{
    return vCfg.zoom;
}

bool Visuals::isDeagleSpinnerOn() noexcept
{
    return vCfg.deagleSpinner;
}

bool Visuals::shouldRemoveFog() noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 10);
}

bool Visuals::shouldRemoveScopeOverlay() noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 0);
}

bool Visuals::shouldRemoveSmoke() noexcept
{
    return vCfg.smokeMode;
}

float Visuals::viewModelFov() noexcept
{
    return static_cast<float>(vCfg.viewModel.Fov);
}

float Visuals::fov() noexcept
{
    return static_cast<float>(vCfg.fov);
}

bool Visuals::noWeapons() noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 7);
}

float Visuals::farZ() noexcept
{
    return static_cast<float>(vCfg.farZ);
}

bool Visuals::FogEnabled() noexcept {
    return vCfg.fog.color.enabled;
}

float Visuals::FogStart() noexcept {
    return vCfg.fog.FogStart;
}

float Visuals::FogEnd() noexcept {
    return vCfg.fog.FogEnd;
}

float Visuals::FogDensity() noexcept {
    return vCfg.fog.FogDens;
}

ColorToggle3 Visuals::FogColor() noexcept{

    return vCfg.fog.color;
}

ColorToggle3 Visuals::worldColor() noexcept{

    return vCfg.world;
}

void Visuals::dontRenderTeammates() noexcept {
    if (!vCfg.dontRenderTeammates)
        return;

    if (!localPlayer)
        return;

    for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
    {
        auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || entity->isOtherEnemy(localPlayer.get()))
            continue;

        entity->readyToDraw() = false;
    }
}

void Visuals::shadowChanger() noexcept
{
    static auto cl_csm_rot_override = interfaces->cvar->findVar("cl_csm_rot_override");
    static auto cl_csm_max_shadow_dist = interfaces->cvar->findVar("cl_csm_max_shadow_dist");
    static auto cl_csm_rot_x = interfaces->cvar->findVar("cl_csm_rot_x");
    static auto cl_csm_rot_y = interfaces->cvar->findVar("cl_csm_rot_y");

    if (Helpers::getByteFromBytewise(vCfg.removals, 8) || !vCfg.shadowsChanger.enabled)
    {
        cl_csm_rot_override->setValue(0);
        return;
    }

    cl_csm_max_shadow_dist->setValue(800);
    cl_csm_rot_override->setValue(1);
    cl_csm_rot_x->setValue(vCfg.shadowsChanger.x);
    cl_csm_rot_y->setValue(vCfg.shadowsChanger.x);
}

void Visuals::performColorCorrection() noexcept
{
    if (const auto& cfg = vCfg.colorCorrection; cfg.enabled) {
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x49C, 0x908)) = cfg.blue;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4A4, 0x918)) = cfg.red;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4AC, 0x928)) = cfg.mono;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4B4, 0x938)) = cfg.saturation;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4C4, 0x958)) = cfg.ghost;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4CC, 0x968)) = cfg.green;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4D4, 0x978)) = cfg.yellow;
    }
}

void Visuals::ragdolls() noexcept
{
    const auto force = interfaces->cvar->findVar("phys_pushscale");
    const auto disable = interfaces->cvar->findVar("cl_ragdoll_physics_enable");
    const auto speed = interfaces->cvar->findVar("cl_phys_timescale");
    const auto gravity = interfaces->cvar->findVar("cl_ragdoll_gravity");

    disable->setValue(vCfg.ragdolls.enabled ? !vCfg.ragdolls.disabled : 1);
    force->setValue(vCfg.ragdolls.enabled ? vCfg.ragdolls.force : 1);
    speed->setValue(vCfg.ragdolls.enabled ? vCfg.ragdolls.timescale : 1);
    gravity->setValue(vCfg.ragdolls.enabled ? vCfg.ragdolls.gravity : 600);
}

void Visuals::doBloomEffects() noexcept
{
    if (!vCfg.customPostProcessing.enabled) return;

    if (!localPlayer) return;

    for (int i = 0; i < 2048; i++)
    {
        Entity* ent = interfaces->entityList->getEntity(i);

        if (!ent)
            continue;

        std::string clientClass = ent->getClientClass()->networkName;

        if (!clientClass.ends_with("TonemapController"))
            continue;

        bool enabled = vCfg.customPostProcessing.enabled;
        ent->useCustomAutoExposureMax() = enabled;
        ent->useCustomAutoExposureMin() = enabled;
        ent->useCustomBloomScale() = enabled;

        if (!enabled)
            return;

        float worldExposure = vCfg.customPostProcessing.worldExposure;
        ent->customAutoExposureMin() = worldExposure;
        ent->customAutoExposureMax() = worldExposure;

        float bloomScale = vCfg.customPostProcessing.bloomScale;
        ent->customBloomScale() = bloomScale;

        ConVar* modelAmbientMin = interfaces->cvar->findVar("r_modelAmbientMin");
        modelAmbientMin->setValue(vCfg.customPostProcessing.modelAmbient);
    }
}

void Visuals::colorConsole() noexcept
{

    static Material* material[5];
    if (!material[0] || !material[1] || !material[2] || !material[3] || !material[4]) {
        for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
            const auto mat = interfaces->materialSystem->getMaterial(h);

            if (!mat)
                continue;

            if (strstr(mat->getName(), "vgui_white"))
                material[0] = mat;
            else if (strstr(mat->getName(), "800corner1"))
                material[1] = mat;
            else if (strstr(mat->getName(), "800corner2"))
                material[2] = mat;
            else if (strstr(mat->getName(), "800corner3"))
                material[3] = mat;
            else if (strstr(mat->getName(), "800corner4"))
                material[4] = mat;
        }
    }
    else {
        for (unsigned int num = 0; num < 5; num++) {
            if (!vCfg.console.enabled || !interfaces->engine->isConsoleVisible()) {
                material[num]->colorModulate(1.f, 1.f, 1.f);
                material[num]->alphaModulate(1.f);
                continue;
            }

            if (vCfg.console.asColor4().rainbow) {
                material[num]->colorModulate(rainbowColor(vCfg.console.asColor4().rainbowSpeed));
                material[num]->alphaModulate(vCfg.console.asColor4().color[3]);
            }
            else {
                material[num]->colorModulate(vCfg.console.asColor4().color[0], vCfg.console.asColor4().color[1], vCfg.console.asColor4().color[2]);
                material[num]->alphaModulate(vCfg.console.asColor4().color[3]);
            }
        }
    }
}

void Visuals::modifySmoke(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    if (vCfg.smokeMode == 3) {
        *(int*)memory->smokeCount = 0;
        const auto material = interfaces->materialSystem->findMaterial("particle/vistasmokev1/vistasmokev1_smokegrenade");
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == csgo::FrameStage::RENDER_START);
        return;
    }

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == csgo::FrameStage::RENDER_START && vCfg.smokeMode == 1);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == csgo::FrameStage::RENDER_START && vCfg.smokeMode == 2);
    }
}

void Visuals::modifyMolotov(csgo::FrameStage stage) noexcept
{
    constexpr std::array fireMaterials{
        "decals/molotovscorch.vmt",
        "particle/fire_burning_character/fire_env_fire.vmt",
        "particle/fire_burning_character/fire_env_fire_depthblend.vmt",
        "particle/particle_flares/particle_flare_001.vmt",
        "particle/particle_flares/particle_flare_004.vmt",
        "particle/particle_flares/particle_flare_004b_mod_ob.vmt",
        "particle/particle_flares/particle_flare_004b_mod_z.vmt",
        "particle/fire_explosion_1/fire_explosion_1_bright.vmt",
        "particle/fire_explosion_1/fire_explosion_1b.vmt",
        "particle/fire_particle_4/fire_particle_4.vmt",
        "particle/fire_explosion_1/fire_explosion_1_oriented.vmt",
        "particle/vistasmokev1/vistasmokev1_nearcull_nodepth.vmt"
    };

    for (const auto mat : fireMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == csgo::FrameStage::RENDER_START && vCfg.moloMode == 1);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == csgo::FrameStage::RENDER_START && vCfg.moloMode == 2);
    }
}

void Visuals::thirdperson() noexcept
{
    if (!vCfg.thirdperson)
        return;

    if (!localPlayer) return;

    if (localPlayer->isAlive()) {
        memory->input->isCameraInThirdPerson = (vCfg.thirdpersonKey.isToggled() || Misc::FreeCamKey());
        memory->input->cameraOffset.z = static_cast<float>(vCfg.thirdpersonDistance);
    }
    else if (localPlayer->getObserverTarget() && (localPlayer->observerMode() == ObsMode::InEye || localPlayer->observerMode() == ObsMode::Chase)){
        memory->input->isCameraInThirdPerson = false;
        localPlayer->observerMode() = vCfg.thirdpersonKey.isToggled() ? ObsMode::InEye : ObsMode::Chase;
    }
}

void Visuals::removeVisualRecoil(csgo::FrameStage stage) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == csgo::FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (Helpers::getByteFromBytewise(vCfg.removals, 3))
            localPlayer->aimPunchAngle() = Vector{ };

        if (Helpers::getByteFromBytewise(vCfg.removals, 4))
            localPlayer->viewPunchAngle() = Vector{ };

    } else if (stage == csgo::FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeSprites() noexcept
{
    if (!localPlayer) return;

    static auto sky = interfaces->cvar->findVar("r_drawsprites");
    sky->onChangeCallbacks.size = NULL;
    sky->setValue(!Helpers::getByteFromBytewise(vCfg.removals, 11));
}

void Visuals::removeBlur(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == csgo::FrameStage::RENDER_START && Helpers::getByteFromBytewise(vCfg.removals, 14));
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(1.f - (vCfg.brightness / 100));
}

void Visuals::fullBright() noexcept {
    static auto blur = interfaces->cvar->findVar("mat_fullbright");
    blur->setValue(vCfg.fullBright);
}

void Visuals::noHUDBlur() noexcept {
    static auto blur = interfaces->cvar->findVar("@panorama_disable_blur");
    blur->setValue(Helpers::getByteFromBytewise(vCfg.removals, 15));
}

void Visuals::removeGrass(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    constexpr auto getGrassMaterialName = []() noexcept -> const char* {
        switch (fnv::hashRuntime(interfaces->engine->getLevelName())) {
        case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
        case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
        default: return nullptr;
        }
    };

    if (const auto grassMaterialName = getGrassMaterialName())
        interfaces->materialSystem->findMaterial(grassMaterialName)->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == csgo::FrameStage::RENDER_START && Helpers::getByteFromBytewise(vCfg.removals, 11));
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!Helpers::getByteFromBytewise(vCfg.removals, 9));
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!Helpers::getByteFromBytewise(vCfg.removals, 8));
}

void Visuals::removeWeaponSway() noexcept
{
    static auto sway = interfaces->cvar->findVar("cl_wpn_sway_scale");
    sway->setValue(!Helpers::getByteFromBytewise(vCfg.removals, 17));
}

void Visuals::viewModelChanger(ViewSetup* setup) noexcept
{
    if (!localPlayer)
        return;

    constexpr auto setViewmodel = [](Entity* viewModel, const Vector& angles) constexpr noexcept
    {
        if (viewModel)
        {
            Vector forward = Vector::fromAngle(angles);
            Vector up = Vector::fromAngle(angles - Vector{ 90.0f, 0.0f, 0.0f });
            Vector side = forward.cross(up);
            Vector offset = side * vCfg.viewModel.x + forward * vCfg.viewModel.y + up * vCfg.viewModel.z;
            memory->setAbsOrigin(viewModel, viewModel->getRenderOrigin() + offset);
            memory->setAbsAngle(viewModel, angles + Vector{ 0.0f, 0.0f, vCfg.viewModel.roll });
        }
    };

    if (localPlayer->isAlive())
    {
        if (vCfg.viewModel.enabled && !localPlayer->isScoped() && !memory->input->isCameraInThirdPerson)
            setViewmodel(interfaces->entityList->getEntityFromHandle(localPlayer->viewModel()), setup->angles);
    }
    else if (auto observed = localPlayer->getObserverTarget(); observed != nullptr && observed->isAlive() && localPlayer->getObserverMode() == ObsMode::InEye)
    {
            if (vCfg.viewModel.enabled && !observed->isScoped())
            setViewmodel(interfaces->entityList->getEntityFromHandle(observed->viewModel()), setup->angles);
    }
}

bool Visuals::noZoom() noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 1);
}

void Visuals::applyZoom(csgo::FrameStage stage) noexcept
{
    if (!vCfg.zoom) return;
    if (!localPlayer) return;
        if (stage == csgo::FrameStage::RENDER_START && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            if (vCfg.zoomKey.isToggled()) {
                localPlayer->fov() = 40;
                localPlayer->fovStart() = 40;
            }
        }
    
}

#ifdef _WIN32
#undef xor
#define DRAW_SCREEN_EFFECT(material) \
{ \
    const auto drawFunction = memory->drawScreenEffectMaterial; \
    int w, h; \
    interfaces->engine->getScreenSize(w, h); \
    __asm { \
        __asm push h \
        __asm push w \
        __asm push 0 \
        __asm xor edx, edx \
        __asm mov ecx, material \
        __asm call drawFunction \
        __asm add esp, 12 \
    } \
}

#else
#define DRAW_SCREEN_EFFECT(material) \
{ \
    int w, h; \
    interfaces->engine->getScreenSize(w, h); \
    reinterpret_cast<void(*)(Material*, int, int, int, int)>(memory->drawScreenEffectMaterial)(material, 0, 0, w, h); \
}
#endif

void Visuals::applyScreenEffects() noexcept
{
    if (!vCfg.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects{
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (vCfg.screenEffect <= 2 || static_cast<std::size_t>(vCfg.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[vCfg.screenEffect - 2];
    }());

    if (vCfg.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (vCfg.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (vCfg.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept
{
    if (vCfg.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + vCfg.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[]{
                "effects/dronecam",
                "effects/underwater_overlay",
                "effects/healthboost",
                "effects/dangerzone_screen"
                };

                if (vCfg.hitEffect <= 2)
                    return effects[0];
                return effects[vCfg.hitEffect - 2];
            };

           
            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (vCfg.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (vCfg.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (vCfg.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

struct DamageNumber {
    int hitbox;
    int damage;
    bool initialized;
    float eraseTime;
    float lastUpdate;
    Entity* entity;
    Vector pos;
};

std::vector<DamageNumber> damageIndicator;

void Visuals::damageNumberEvent(GameEvent* event, bool reset) noexcept {

    if (!event) return;

    if (reset) {
        damageIndicator.clear();
        return;
    }

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("attacker")));

    if (!userID) return;

    if (!attacker) return;

    if (localPlayer->getUserId() != attacker->getUserId() && localPlayer->getUserId() != userID->getUserId()) return;


    DamageNumber dmgIndicator;
    dmgIndicator.damage = event->getInt("dmg_health");
    dmgIndicator.entity = userID;
    dmgIndicator.eraseTime = memory->globalVars->currenttime + 3.f;
    dmgIndicator.initialized = false;
    dmgIndicator.hitbox = event->getInt("hitgroup");

    damageIndicator.push_back(dmgIndicator);

}

void Visuals::damageNumber(ImDrawList* drawList) noexcept{

    if (!vCfg.damageNumber.enabled)
        return;

    if (!localPlayer) return;

    float currentTime = memory->globalVars->currenttime;

    auto color = Helpers::calculateColor(vCfg.damageNumber.asColor4());

    for (unsigned int i = 0; i < damageIndicator.size(); ++i) {
        if (damageIndicator[i].eraseTime < currentTime) {
            damageIndicator.erase(damageIndicator.begin() + i);
            continue;
        }

        if (!damageIndicator[i].initialized) {
            damageIndicator[i].pos = damageIndicator[i].entity->getBonePosition(damageIndicator[i].hitbox);
            damageIndicator[i].initialized = true;
        }

        if (currentTime - damageIndicator[i].lastUpdate > 0.0001f) {
            damageIndicator[i].pos.z -= (0.1f * (currentTime - damageIndicator[i].eraseTime));
            damageIndicator[i].lastUpdate = currentTime;
        }

        ImVec2 ScreenPosition;
        if (Helpers::worldToScreen(damageIndicator[i].pos, ScreenPosition))
            drawList->AddText(ScreenPosition, color, std::to_string(damageIndicator[i].damage).c_str());
    }
}

void Visuals::hitMarker(GameEvent* event, ImDrawList* drawList) noexcept
{
    if (vCfg.hitMarker == 0)
        return;

    static float lastHitTime = 0.0f;

    if (event) {
        if (localPlayer && event->getInt("attacker") == localPlayer->getUserId())
            lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + vCfg.hitMarkerTime < memory->globalVars->realtime)
        return;

    const auto& mid = ImGui::GetIO().DisplaySize / 2.0f;

    switch (vCfg.hitMarker) {
    case 1:{
        auto color = Helpers::calculateColor(
            static_cast<int>(vCfg.hitMarkerColor.color[0] * 255.f),
            static_cast<int>(vCfg.hitMarkerColor.color[1] * 255.f),
            static_cast<int>(vCfg.hitMarkerColor.color[2] * 255.f),
            255);
            drawList->AddLine({ mid.x - 10, mid.y - 10 }, { mid.x - 4, mid.y - 4 }, color);
            drawList->AddLine({ mid.x + 10.5f, mid.y - 10.5f }, { mid.x + 4.5f, mid.y - 4.5f }, color);
            drawList->AddLine({ mid.x + 10.5f, mid.y + 10.5f }, { mid.x + 4.5f, mid.y + 4.5f }, color);
            drawList->AddLine({ mid.x - 10, mid.y + 10 }, { mid.x - 4, mid.y + 4 }, color);
            break;
        }
    case 2: {
            auto color = Helpers::calculateColor(
            static_cast<int>(vCfg.hitMarkerColor.color[0] * 255.f),
            static_cast<int>(vCfg.hitMarkerColor.color[1] * 255.f),
            static_cast<int>(vCfg.hitMarkerColor.color[2] * 255.f),
            static_cast<int>(Helpers::lerp(fabsf(lastHitTime + vCfg.hitMarkerTime - memory->globalVars->realtime) / vCfg.hitMarkerTime + FLT_EPSILON, 0.0f, 255.0f)));
            drawList->AddLine({ mid.x - 10, mid.y - 10 }, { mid.x - 4, mid.y - 4 }, color);
            drawList->AddLine({ mid.x + 10.5f, mid.y - 10.5f }, { mid.x + 4.5f, mid.y - 4.5f }, color);
            drawList->AddLine({ mid.x + 10.5f, mid.y + 10.5f }, { mid.x + 4.5f, mid.y + 4.5f }, color);
            drawList->AddLine({ mid.x - 10, mid.y + 10 }, { mid.x - 4, mid.y + 4 }, color);
            break;
        }
    }
}

void Visuals::disablePostProcessing(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == csgo::FrameStage::RENDER_START && Helpers::getByteFromBytewise(vCfg.removals, 13);
}

float Visuals::reduceFlashEffect() noexcept
{
    return vCfg.flashReduction;
}

bool Visuals::removeHands(const char* modelName) noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 5) && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 6) && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept
{
    return Helpers::getByteFromBytewise(vCfg.removals, 7) && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::customScopeCrosshair(ImDrawList* drawList) noexcept {

    if (!vCfg.customScopeCrosshair.color.enabled) return;

    if (!localPlayer) return;

    const auto weapon = localPlayer->getActiveWeapon();

    if (!weapon || !weapon->isSniperRifle()) return;

    const auto io = ImGui::GetIO();

    float
        displayWidth = io.DisplaySize.x,
        displayHeight = io.DisplaySize.y,
        x = displayWidth / 2.f,
        y = displayHeight / 2.f,
        offset = vCfg.customScopeCrosshair.offset,
        height = vCfg.customScopeCrosshair.height;

    const float scopeLineCVarSize = interfaces->cvar->findVar("cl_crosshair_sniper_width")->getFloat();

    const auto color = Helpers::calculateColor(vCfg.customScopeCrosshair.color.asColor4());
    const static auto emptyColor = Helpers::calculateColor(Color4{ 0.f, 0.f, 0.f, 0.f });

    const float frequency = (vCfg.customScopeCrosshair.speed / 10.f) * 3.f;
    static float opacity;

    // the increment / decrement per frame.
    float step = frequency * memory->globalVars->frametime;

    // if open		-> increment
    // if closed	-> decrement
    localPlayer->isScoped() ? opacity += step : opacity -= step;

    // clamp the opacity.
    opacity = std::clamp(opacity, 0.f, 1.f);

    // Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
    if (scopeLineCVarSize > 1.f) {
        x -= (scopeLineCVarSize / 2.f);
        y -= (scopeLineCVarSize / 2.f);
    }

    // draw our lines.
    if (opacity && vCfg.customScopeCrosshair.mode == 1) {

        static float x, y, w, h;

        x = displayWidth / 2.f + offset + height - (height * opacity);
        y = displayHeight / 2.f;
        w = height * opacity;
        h = 1.f;

        drawList->AddRectFilledMultiColor({ x , y }, { x + w, y + h }, color, emptyColor, emptyColor, color); //right

        x = displayWidth / 2.f - height - offset;
        drawList->AddRectFilledMultiColor({ x , y }, { x + w, y + h }, emptyColor, color, color, emptyColor); //left

        x = displayWidth / 2.f;
        y = displayHeight / 2.f + offset + height - (height * opacity);
        w = 1.f;
        h = height * opacity;
        drawList->AddRectFilledMultiColor({ x , y }, { x + w, y + h }, color, color, emptyColor, emptyColor); //bottom

        y = displayHeight / 2 - height - offset;
        drawList->AddRectFilledMultiColor({ x , y }, { x + w, y + h }, emptyColor, emptyColor, color, color); //top
    }
    else if (localPlayer->isScoped() && vCfg.customScopeCrosshair.mode == 0) {
        drawList->AddRectFilled({ 0.f, y }, { displayWidth, y + scopeLineCVarSize }, color);
        drawList->AddRectFilled({ x, 0.f }, { x + scopeLineCVarSize, displayHeight}, color);
    }
}

void Visuals::maskChanger(csgo::FrameStage stage) noexcept {

    if (stage != csgo::FrameStage::NET_UPDATE_POSTDATAUPDATE_START && stage != csgo::FrameStage::RENDER_END)
        return;

    if (!localPlayer) return;

    if (!localPlayer->isAlive()) return;

    //auto isValidModel = [](std::string name) noexcept -> bool
    //{
    //    if (name.empty() || name.front() == ' ' || name.back() == ' ' || !name.ends_with(".mdl"))
    //        return false;

    //    if (!name.starts_with("models") && !name.starts_with("/models") && !name.starts_with("\\models"))
    //        return false;

    //    //Check if file exists within directory
    //    std::string path = interfaces->engine->getGameDirectory();
    //    if (vCfg.playerModel[0] != '\\' && vCfg.playerModel[0] != '/')
    //        path += "/";
    //    path += vCfg.playerModel;

    //    struct stat buf;
    //    if (stat(path.c_str(), &buf) != -1)
    //        return true;

    //    return false;
    //};

    //const bool is = isValidModel(static_cast<std::string>(vCfg.playerModel));

    //if (is) {
    //    if (stage == csgo::FrameStage::NET_UPDATE_POSTDATAUPDATE_START) {
    //        if (const auto modelprecache = interfaces->networkStringTableContainer->findTable("modelprecache")) {
    //            const auto index = modelprecache->addString(false, vCfg.maskModel);
    //            if (index == -1)
    //                return;
    //        }
    //    }
    //}

    if (vCfg.maskChanger)
        localPlayer->addonBits() |= 0x10000 | 0x00800;
    else
        localPlayer->addonBits() &= ~0x10000 | 0x00800;


    /*  'models/player/holiday/facemasks/facemask_dallas.mdl',
        'models/player/holiday/facemasks/facemask_battlemask.mdl',
        'models/player/holiday/facemasks/evil_clown.mdl',
        'models/player/holiday/facemasks/facemask_anaglyph.mdl',
        'models/player/holiday/facemasks/facemask_boar.mdl',
        'models/player/holiday/facemasks/facemask_bunny.mdl',
        'models/player/holiday/facemasks/facemask_bunny_gold.mdl',
        'models/player/holiday/facemasks/facemask_chains.mdl',
        'models/player/holiday/facemasks/facemask_chicken.mdl',
        'models/player/holiday/facemasks/facemask_devil_plastic.mdl',
        'models/player/holiday/facemasks/facemask_hoxton.mdl',
        'models/player/holiday/facemasks/facemask_pumpkin.mdl',
        'models/player/holiday/facemasks/facemask_samurai.mdl',
        'models/player/holiday/facemasks/facemask_sheep_bloody.mdl',
        'models/player/holiday/facemasks/facemask_sheep_gold.mdl',
        'models/player/holiday/facemasks/facemask_sheep_model.mdl',
        'models/player/holiday/facemasks/facemask_skull.mdl',
        'models/player/holiday/facemasks/facemask_template.mdl',
        'models/player/holiday/facemasks/facemask_wolf.mdl',
        'models/player/holiday/facemasks/porcelain_doll.mdl'
    */
}

void Visuals::playerModel(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::NET_UPDATE_POSTDATAUPDATE_START && stage != csgo::FrameStage::RENDER_END)
        return;

    static int originalIdx = 0;

    if (!localPlayer) {
        originalIdx = 0;
        return;
    }

    auto isValidModel = [](std::string name) noexcept -> bool
    {
        if (name.empty() || name.front() == ' ' || name.back() == ' ' || !name.ends_with(".mdl"))
            return false;

        if (!name.starts_with("models") && !name.starts_with("/models") && !name.starts_with("\\models"))
            return false;

        //Check if file exists within directory
        std::string path = interfaces->engine->getGameDirectory();
        if (vCfg.playerModel[0] != '\\' && vCfg.playerModel[0] != '/')
            path += "/";
        path += vCfg.playerModel;

        struct stat buf;
        if (stat(path.c_str(), &buf) != -1)
            return true;

        return false;
    };

    const bool custom = isValidModel(static_cast<std::string>(vCfg.playerModel));

    if (custom) {
        if (stage == csgo::FrameStage::NET_UPDATE_POSTDATAUPDATE_START) {
            originalIdx = localPlayer->modelIndex();
            if (const auto modelprecache = interfaces->networkStringTableContainer->findTable("modelprecache")) {
                const auto index = modelprecache->addString(false, vCfg.playerModel);
                if (index == -1)
                    return;

                const auto viewmodelArmConfig = memory->getPlayerViewmodelArmConfigForPlayerModel(vCfg.playerModel);
                modelprecache->addString(false, viewmodelArmConfig[2]);
                modelprecache->addString(false, viewmodelArmConfig[3]);
            }
        }

        const auto idx = stage == csgo::FrameStage::RENDER_END && originalIdx ? originalIdx : interfaces->modelInfo->getModelIndex(vCfg.playerModel);

        localPlayer->setModelIndex(idx);

        if (const auto ragdoll = interfaces->entityList->getEntityFromHandle(localPlayer->ragdoll()))
            ragdoll->setModelIndex(idx);
    }
}

void Visuals::asus() noexcept{

    if (!localPlayer) return;

    static auto drawSpecificStaticProp = interfaces->cvar->findVar("r_drawspecificstaticprop");
    drawSpecificStaticProp->setValue(0);

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat || !mat->isPrecached())
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (textureGroup.starts_with("World")) {
            mat->alphaModulate(vCfg.asus.walls);
        }
        else if (textureGroup.starts_with("StaticProp")) {
                mat->alphaModulate(vCfg.asus.props);
        }
    }
}

void Visuals::skybox(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    if (stage == csgo::FrameStage::RENDER_START && vCfg.skybox > 0 && static_cast<std::size_t>(vCfg.skybox) < skyboxList.size() - 1) {
        memory->loadSky(skyboxList[vCfg.skybox]);
    }
    else if (vCfg.skybox == 26 && stage == csgo::FrameStage::RENDER_START) {
        memory->loadSky(vCfg.customSkybox);
    }
    else{
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}

void Visuals::DLights(Entity* entity) {

    auto cvarDynamic = interfaces->cvar->findVar("r_dynamic");
    static auto cvarDynamicTemp = cvarDynamic->getInt();
    
    if (!vCfg.dLight.enabled) {
        if(cvarDynamic->getInt() != cvarDynamicTemp)
            cvarDynamic->setValue(cvarDynamicTemp);
        return;
    }

    if (!localPlayer) return;

    if (!interfaces->engine->isInGame() && !interfaces->engine->isConnected()) return;

    if (!entity->isAlive()) return;

    if (cvarDynamic->getInt() == 0)
        cvarDynamic->setValue(1);

    if (localPlayer->isAlive()) {
        if (!vCfg.dLight.local) return;
        DLight* dlight = interfaces->iveffects->clAllocDLight(localPlayer->index());
        dlight->color.r = 255.0f * vCfg.dlightsColor1.asColor4().color[0];
        dlight->color.g = 255.0f * vCfg.dlightsColor1.asColor4().color[1];
        dlight->color.b = 255.0f * vCfg.dlightsColor1.asColor4().color[2];
        dlight->color.exponent = 10.f;
        dlight->direction = localPlayer->getRenderOrigin();
        dlight->origin = localPlayer->getRenderOrigin();
        dlight->radius = vCfg.dLight.radius / 2.f;
        dlight->dieTime = memory->globalVars->currenttime + 0.05f;
        dlight->decay = vCfg.dLight.decay;
        dlight->key = localPlayer->index();
        dlight->flags = 0;

        DLight* elight = interfaces->iveffects->clAllocELight(localPlayer->index());
        elight->color.r = 255.0f * vCfg.dlightsColor1.asColor4().color[0];
        elight->color.g = 255.0f * vCfg.dlightsColor1.asColor4().color[1];
        elight->color.b = 255.0f * vCfg.dlightsColor1.asColor4().color[2];
        elight->color.exponent = 10.f;
        elight->direction = localPlayer->getEyePosition();
        elight->origin = localPlayer->getEyePosition();
        elight->radius = vCfg.dLight.radius;
        elight->dieTime = memory->globalVars->currenttime + 0.05f;
        elight->decay = vCfg.dLight.decay;
        elight->key = localPlayer->index();
        elight->flags = 0;
    }else if (entity->getTeamNumber() == localPlayer->getTeamNumber()) { //TEAMMATES
        if (!vCfg.dLight.allies) return;
        DLight* dlight = interfaces->iveffects->clAllocDLight(entity->index());
        dlight->color.r = 255.0f * vCfg.dlightsColor.asColor4().color[0];
        dlight->color.g = 255.0f * vCfg.dlightsColor.asColor4().color[1];
        dlight->color.b = 255.0f * vCfg.dlightsColor.asColor4().color[2];
        dlight->color.exponent = 10.f;
        dlight->direction = entity->getRenderOrigin();
        dlight->origin = entity->getRenderOrigin();
        dlight->radius = vCfg.dLight.radius / 2.f;
        dlight->dieTime = memory->globalVars->currenttime + 0.05f;
        dlight->decay = vCfg.dLight.decay;
        dlight->key = entity->index();
        dlight->flags = 0;

        DLight* elight = interfaces->iveffects->clAllocELight(entity->index());
        elight->color.r = 255.0f * vCfg.dlightsColor.asColor4().color[0];
        elight->color.g = 255.0f * vCfg.dlightsColor.asColor4().color[1];
        elight->color.b = 255.0f * vCfg.dlightsColor.asColor4().color[2];
        elight->color.exponent = 10.f;
        elight->direction = entity->getEyePosition();
        elight->origin = entity->getEyePosition();
        elight->radius = vCfg.dLight.radius;
        elight->dieTime = memory->globalVars->currenttime + 0.05f;
        elight->decay = vCfg.dLight.decay;
        elight->key = entity->index();
        elight->flags = 0;

    }else { //ENEMY

        if (!vCfg.dLight.enemies) return;

        DLight* dlight = interfaces->iveffects->clAllocDLight(entity->index());

        dlight->color.r = 255.0f * vCfg.dlightsColor2.asColor4().color[0];
        dlight->color.g = 255.0f * vCfg.dlightsColor2.asColor4().color[1];
        dlight->color.b = 255.0f * vCfg.dlightsColor2.asColor4().color[2];
        dlight->color.exponent = 10.f;
        dlight->direction = entity->getRenderOrigin();
        dlight->origin = entity->getRenderOrigin();
        dlight->radius = vCfg.dLight.radius / 2.f;
        dlight->dieTime = memory->globalVars->currenttime + 0.05f;
        dlight->decay = vCfg.dLight.decay;
        dlight->key = entity->index();
        dlight->flags = 0;

        DLight* elight = interfaces->iveffects->clAllocELight(entity->index());
        elight->color.r = 255.0f * vCfg.dlightsColor2.asColor4().color[0];
        elight->color.g = 255.0f * vCfg.dlightsColor2.asColor4().color[1];
        elight->color.b = 255.0f * vCfg.dlightsColor2.asColor4().color[2];
        elight->color.exponent = 10.f;
        elight->direction = entity->getEyePosition();
        elight->origin = entity->getEyePosition();
        elight->radius = vCfg.dLight.radius;
        elight->dieTime = memory->globalVars->currenttime + 0.05f;
        elight->decay = vCfg.dLight.decay;
        elight->key = entity->index();
        elight->flags = 0;
    }
}

void Visuals::RunDLights()
{
    if (!vCfg.dLight.enabled) return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    for (int i = 1; i <= memory->globalVars->maxClients; i++) {
        auto entity = reinterpret_cast<Entity*>(interfaces->entityList->getEntity(i));
        if (!entity || !entity->isAlive())
            continue;
            Visuals::DLights(entity);
    }
}

void Visuals::killLightning(GameEvent& event) noexcept
{
    auto cvarDynamic = interfaces->cvar->findVar("r_dynamic");
    static auto cvarDynamicTemp = cvarDynamic->getInt();

    if (!vCfg.killLightningBolt) {
        if (cvarDynamic->getInt() != cvarDynamicTemp)
            cvarDynamic->setValue(cvarDynamicTemp);
        return;
    }

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (cvarDynamic->getInt() == 0)
        cvarDynamic->setValue(1);

    const auto userid = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("attacker")));

    if (!attacker) return;
    if (!userid) return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    DLight* dlight = interfaces->iveffects->clAllocDLight(userid->index());
    dlight->color.r = 255;
    dlight->color.g = 255;
    dlight->color.b = 255;
    dlight->color.exponent = 10.f;
    dlight->direction = userid->getRenderOrigin();
    dlight->origin = userid->getRenderOrigin();
    dlight->radius = 50.f;
    dlight->dieTime = memory->globalVars->currenttime + 0.05f;
    dlight->decay = 20.f;
    dlight->key = userid->index();

    DLight* elight = interfaces->iveffects->clAllocELight(userid->index());
    elight->color.r = 255;
    elight->color.g = 255;
    elight->color.b = 255;
    elight->color.exponent = 10.f;
    elight->direction = userid->getEyePosition();
    elight->origin = userid->getEyePosition();
    elight->radius = 100.f;
    elight->dieTime = memory->globalVars->currenttime + 0.05f;
    elight->decay = 20.f;
    elight->key = userid->index();

    BeamInfo beamInfo;
    beamInfo.start.x = userid->getAbsOrigin().x;
    beamInfo.start.y = userid->getAbsOrigin().y;
    beamInfo.start.z = userid->getAbsOrigin().z + 1000.f;
    beamInfo.end = userid->getAbsOrigin();
    beamInfo.modelIndex = -1;
    beamInfo.red = 255.0f;
    beamInfo.green = 255.0f;
    beamInfo.blue = 255.0f;
    beamInfo.brightness = 255.0f;
    beamInfo.type = 0;
    beamInfo.life = 0.0f;
    beamInfo.amplitude = 4;
    beamInfo.segments = -1;
    beamInfo.renderable = true;
    beamInfo.speed = 0.2f;
    beamInfo.modelName = "sprites/physbeam.vmt";
    beamInfo.startFrame = 0;
    beamInfo.frameRate = 0.0f;
    beamInfo.width = 2.0f;
    beamInfo.endWidth = 2.0f;
    beamInfo.flags = 0x40;
    beamInfo.fadeLength = 20.0f;

    if (const auto beam = memory->viewRenderBeams->createBeamPoints(beamInfo)) {
        constexpr auto FBEAM_FOREVER = 0x4000;
        beam->flags &= ~FBEAM_FOREVER;
        beam->die = memory->globalVars->currenttime + .5f;
    }
}

void Visuals::drawHitboxMatrix(GameEvent* event) noexcept {
    
    if (!vCfg.hitMatrix.color.enabled) return;

    if (vCfg.hitMatrix.duration < 0.f) return;

    if (!event) return;

    if (!localPlayer) return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("attacker")));

    if (!userID) return;

    if (!attacker) return;

    if (localPlayer->getUserId() != attacker->getUserId() || localPlayer->getUserId() == userID->getUserId()) return;

    StudioHdr* hdr = interfaces->modelInfo->getStudioModel(userID->getModel());
    StudioHitboxSet* set = hdr->getHitboxSet(0);

    auto records = Animations::getBacktrackRecords(userID->index());
    const matrix3x4* matrix = userID->getBoneCache().memory;
    auto bestFov{ 255.f };

    if (records && !records->empty()) {
        for (int i = static_cast<int>(records->size() - 1); i >= 0; i--)
        {
            if (Backtrack::valid(records->at(i).simulationTime))
            {
                for (auto &position : records->at(i).positions) {
                    auto angle = AimHelpers::calculateRelativeAngle(localPlayer->getEyePosition(), position, interfaces->engine->getViewAngles());
                    auto fov = std::hypotf(angle.x, angle.y);
                    if (fov < bestFov) {
                        bestFov = fov;
                        matrix = records->at(i).matrix;
                    }
                }
            }
        }

    }

    const int r = static_cast<int>(vCfg.hitMatrix.color.color[0] * 255.f);
    const int g = static_cast<int>(vCfg.hitMatrix.color.color[1] * 255.f);
    const int b = static_cast<int>(vCfg.hitMatrix.color.color[2] * 255.f);
    const int a = static_cast<int>(vCfg.hitMatrix.color.color[3] * 255.f);
    const float d = vCfg.hitMatrix.duration;

    for (int i = 0; i < set->numHitboxes; i++) {
        StudioBbox* hitbox = set->getHitbox(i);

        if (!hitbox)
            continue;

        Vector vMin = hitbox->bbMin.transform(matrix[hitbox->bone]);
        Vector vMax = hitbox->bbMax.transform(matrix[hitbox->bone]);
        float size = hitbox->capsuleRadius;

        interfaces->debugOverlay->capsuleOverlay(vMin, vMax, size <= 0 ? 3.f : hitbox->capsuleRadius, r, g, b, a, d, 0, 1);
    }
}

constexpr const char* beamtextures[] = {
    "sprites/blueglow1.vmt",
    "sprites/bubble.vmt",
    "sprites/glow01.vmt",
    "sprites/physbeam.vmt",
    "sprites/purpleglow1.vmt",
    "sprites/purplelaser1.vmt",
    "sprites/radio.vmt",
    "sprites/white.vmt"
};

void Visuals::bulletTracer(GameEvent& event) noexcept
{
    if (!vCfg.bulletTracers.enabled)
        return;

    if (!localPlayer)
        return;

    if (event.getInt("userid") != localPlayer->getUserId())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    BeamInfo beamInfo;

    if (!localPlayer->shouldDraw()) {
        const auto viewModel = interfaces->entityList->getEntityFromHandle(localPlayer->viewModel());
        if (!viewModel)
            return;

        if (!viewModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex1stPerson(viewModel), beamInfo.start))
            return;
    } else {
        const auto worldModel = interfaces->entityList->getEntityFromHandle(activeWeapon->weaponWorldModel());
        if (!worldModel)
            return;

        if (!worldModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex3rdPerson(), beamInfo.start))
            return;
    }

    beamInfo.end.x = event.getFloat("x");
    beamInfo.end.y = event.getFloat("y");
    beamInfo.end.z = event.getFloat("z");

    beamInfo.modelName = beamtextures[vCfg.beambullets.textureid];
    beamInfo.modelIndex = -1;
    beamInfo.haloName = nullptr;
    beamInfo.haloIndex = -1;

    beamInfo.red = 255.0f * vCfg.bulletTracers.asColor4().color[0];
    beamInfo.green = 255.0f * vCfg.bulletTracers.asColor4().color[1];
    beamInfo.blue = 255.0f * vCfg.bulletTracers.asColor4().color[2];
    beamInfo.brightness = 255.0f * vCfg.bulletTracers.asColor4().color[3];

    beamInfo.type = TE_BEAMPOINTS;
    beamInfo.amplitude = vCfg.beambullets.amplitude;
    beamInfo.segments = -1;
    beamInfo.renderable = true;
    beamInfo.speed = 0.0f;
    beamInfo.startFrame = 0;
    beamInfo.frameRate = 0.0f;
    beamInfo.width = 2.0f;
    beamInfo.endWidth = 2.0f;
    beamInfo.fadeLength = 20.0f;

    auto beam = memory->viewRenderBeams->createBeamPoints(beamInfo);

    if (beam) {
        beam->flags = FBEAM_FADEOUT | FBEAM_HALOBEAM;
        beam->die = memory->globalVars->currenttime + vCfg.beambullets.life;
    }
}

// Used to sort Vectors in ccw order about a pivot.
static float ccw(const Vector& a, const Vector& b, const Vector& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

struct ccwSorter {
    const Vector& pivot;

    ccwSorter(const Vector& inPivot) : pivot(inPivot) { }

    bool operator()(const Vector& a, const Vector& b) {
        return ccw(pivot, a, b) < 0;
    }
};

static bool isLeftOf(const Vector& a, const Vector& b) {
    return (a.x < b.x || (a.x == b.x && a.y < b.y));
}

static std::vector<Vector> gift_wrapping(std::vector<Vector> v) {
    std::vector<Vector> hull;

    // There must be at least 3 points
    if (v.size() < 3)
        return hull;

    // Move the leftmost Vector to the beginning of our vector.
    // It will be the first Vector in our convext hull.
    std::swap(v[0], *min_element(v.begin(), v.end(), isLeftOf));

    // Repeatedly find the first ccw Vector from our last hull Vector
    // and put it at the front of our array. 
    // Stop when we see our first Vector again.
    do {
        hull.push_back(v[0]);
        std::swap(v[0], *min_element(v.begin() + 1, v.end(), ccwSorter(v[0])));
    } while (v[0].x != hull[0].x && v[0].y != hull[0].y);

    return hull;
}


void Visuals::drawMolotovPolygon(ImDrawList* drawList) noexcept
{
    if (!vCfg.molotovPolygon.enabled)
        return;

    ImColor enemy = Helpers::calculateColor(vCfg.molotovPolygon.enemy);
    ImColor ally = Helpers::calculateColor(vCfg.molotovPolygon.ally);
    ImColor self = Helpers::calculateColor(vCfg.molotovPolygon.self);

    constexpr float pi = std::numbers::pi_v<float>;

    GameData::Lock lock;

    /* add the inferno position with largest possible inferno width so it's showing accurate radius. */
    auto flameCircumference = [](std::vector<Vector> points)
    {
        std::vector<Vector> new_points;

        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto& pos = points[i];

            for (int j = 0; j <= 3; j++)
            {
                float p = j * (360.0f / 4.0f) * (pi / 200.0f);
                new_points.emplace_back(pos + Vector(std::cos(p) * 60.f, std::sin(p) * 60.f, 0.f));
            }
        }

        return new_points;
    };

    for (const auto& molotov : GameData::infernos())
    {
        const auto color = !molotov.owner || molotov.owner->isOtherEnemy(localPlayer.get()) ? enemy:
            molotov.owner->index() != localPlayer->index() ? ally : self;

        /* we only wanted to draw the points on the edge, use giftwrap algorithm. */
        std::vector<Vector> giftWrapped = gift_wrapping(flameCircumference(molotov.points));

        /* transforms world position to screen position. */
        std::vector<ImVec2> points;

        for (size_t i = 0; i < giftWrapped.size(); ++i)
        {
            const auto& pos = giftWrapped[i];

            auto screen_pos = ImVec2();
            if (!Helpers::worldToScreen(pos, screen_pos))
                continue;

            points.emplace_back(ImVec2(screen_pos.x, screen_pos.y));
        }

        drawList->AddConvexPolyFilled(points.data(), points.size(), color);
    }
}

void Visuals::drawSmokeHull(ImDrawList* drawList) noexcept
{
    if (!vCfg.smokeHull.enabled)
        return;

    const auto color = Helpers::calculateColor(vCfg.smokeHull.asColor4());
 

    GameData::Lock lock;

    static const auto smokeCircumference = []
    {
        std::array<Vector, 72> points;
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            constexpr auto smokeRadius = 150.0f; // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/cstrike15/Effects/inferno.cpp#L90
            points[i] = Vector{ smokeRadius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
                smokeRadius * std::sin(Helpers::deg2rad(i * (360.0f / points.size()))),
                0.0f };
        }
        return points;
    }();

    for (const auto& smoke : GameData::smokes())
    {
        std::array<ImVec2, smokeCircumference.size()> screenPoints;
        std::size_t count = 0;

        for (const auto& point : smokeCircumference)
        {
            if (Helpers::worldToScreen(smoke.origin + point, screenPoints[count]))
                ++count;
        }

        if (count < 1)
            continue;

        std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

        constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c)
        {
            return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
        };
        std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });

        drawList->AddConvexPolyFilled(screenPoints.data(), count, color);

    }
}

//Why 2 functions when you can do it in 1?, because it causes a crash doing it in 1 function

static std::deque<Vector> positions;

void Visuals::drawBulletImpacts() noexcept
{
    if (!vCfg.bulletImpacts.enabled)
        return;

    if (!localPlayer)
        return;

    if (!interfaces->debugOverlay)
        return;

    const int r = static_cast<int>(vCfg.bulletImpacts.color[0] * 255.f);
    const int g = static_cast<int>(vCfg.bulletImpacts.color[1] * 255.f);
    const int b = static_cast<int>(vCfg.bulletImpacts.color[2] * 255.f);
    const int a = static_cast<int>(vCfg.bulletImpacts.color[3] * 255.f);

    for (int i = 0; i < static_cast<int>(positions.size()); i++)
    {
        if (!positions.at(i).notNull())
            continue;
        interfaces->debugOverlay->boxOverlay(positions.at(i), Vector{ -2.0f, -2.0f, -2.0f }, Vector{ 2.0f, 2.0f, 2.0f }, Vector{ 0.0f, 0.0f, 0.0f }, r, g, b, a, vCfg.bulletImpactsTime);
    }
    positions.clear();
}

void Visuals::bulletImpact(GameEvent& event) noexcept
{
    if (!vCfg.bulletImpacts.enabled)
        return;

    if (!localPlayer)
        return;

    if (event.getInt("userid") != localPlayer->getUserId())
        return;

    Vector endPos = Vector{ event.getFloat("x"), event.getFloat("y"), event.getFloat("z") };
    positions.push_front(endPos);
}

void Visuals::Flashlight(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::RENDER_START && stage != csgo::FrameStage::RENDER_END)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (vCfg.flashLight.isToggled() && stage == csgo::FrameStage::RENDER_START)
        localPlayer->effectFlags() |= EffectFlag::Flashlight;
    else
        localPlayer->effectFlags() &= ~EffectFlag::Flashlight;
}

void Visuals::hideHUD() noexcept {
    if (!localPlayer) return;

    //ALL\0Health\0Misc\0Chat\0Crosshair\0Radar\0Scoreboard

    if (Helpers::getByteFromBytewise(vCfg.hideHud, 0))
        localPlayer->hideHUD() |= HideHudType::ALL;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 1))
        localPlayer->hideHUD() |= HideHudType::HEALTH;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 2))
        localPlayer->hideHUD() |= HideHudType::MISCSTATUS;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 3))
        localPlayer->hideHUD() |= HideHudType::CHAT;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 4))
        localPlayer->hideHUD() |= HideHudType::CROSSHAIR;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 5))
        localPlayer->hideHUD() |= HideHudType::RADAR;
    if (Helpers::getByteFromBytewise(vCfg.hideHud, 6))
        localPlayer->hideHUD() |= HideHudType::MINISCOREBOARD;
}

struct MotionBlurHistory
{
    MotionBlurHistory() noexcept
    {
        lastTimeUpdate = 0.0f;
        previousPitch = 0.0f;
        previousYaw = 0.0f;
        previousPositon = Vector{ 0.0f, 0.0f, 0.0f };
        noRotationalMotionBlurUntil = 0.0f;
    }

    float lastTimeUpdate;
    float previousPitch;
    float previousYaw;
    Vector previousPositon;
    float noRotationalMotionBlurUntil;
};

void Visuals::motionBlur(ViewSetup* setup) noexcept
{
    if (!localPlayer) return;

    if (!vCfg.motionBlur.enabled)
        return;
    static MotionBlurHistory history;
    static float motionBlurValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (setup)
    {
        const float timeElapsed = memory->globalVars->realtime - history.lastTimeUpdate;

        const auto viewangles = setup->angles;

        const float currentPitch = Helpers::normalizeYaw(viewangles.x);
        const float currentYaw = Helpers::normalizeYaw(viewangles.y);

        Vector currentSideVector;
        Vector currentForwardVector;
        Vector currentUpVector;
        Vector::fromAngleAll(setup->angles, &currentForwardVector, &currentSideVector, &currentUpVector);

        Vector currentPosition = setup->origin;
        Vector positionChange = history.previousPositon - currentPosition;

        if ((positionChange.length() > 30.0f) && (timeElapsed >= 0.5f))
        {
            motionBlurValues[0] = 0.0f;
            motionBlurValues[1] = 0.0f;
            motionBlurValues[2] = 0.0f;
            motionBlurValues[3] = 0.0f;
        }
        else if (timeElapsed > (1.0f / 15.0f))
        {
            motionBlurValues[0] = 0.0f;
            motionBlurValues[1] = 0.0f;
            motionBlurValues[2] = 0.0f;
            motionBlurValues[3] = 0.0f;
        }
        else if (positionChange.length() > 50.0f)
        {
            history.noRotationalMotionBlurUntil = memory->globalVars->realtime + 1.0f;
        }
        else
        {
            const float horizontalFov = setup->fov;
            const float verticalFov = (setup->aspectRatio <= 0.0f) ? (setup->fov) : (setup->fov / setup->aspectRatio);
            const float viewdotMotion = currentForwardVector.dotProduct(positionChange);

            if (vCfg.motionBlur.forwardEnabled)
                motionBlurValues[2] = viewdotMotion;

            const float sidedotMotion = currentSideVector.dotProduct(positionChange);
            float yawdiffOriginal = history.previousYaw - currentYaw;
            if (((history.previousYaw - currentYaw > 180.0f) || (history.previousYaw - currentYaw < -180.0f)) &&
                ((history.previousYaw + currentYaw > -180.0f) && (history.previousYaw + currentYaw < 180.0f)))
                yawdiffOriginal = history.previousYaw + currentYaw;

            float yawdiffAdjusted = yawdiffOriginal + (sidedotMotion / 3.0f);

            if (yawdiffOriginal < 0.0f)
                yawdiffAdjusted = std::clamp(yawdiffAdjusted, yawdiffOriginal, 0.0f);
            else
                yawdiffAdjusted = std::clamp(yawdiffAdjusted, 0.0f, yawdiffOriginal);

            const float undampenedYaw = yawdiffAdjusted / horizontalFov;
            motionBlurValues[0] = undampenedYaw * (1.0f - (fabsf(currentPitch) / 90.0f));

            const float pitchCompensateMask = 1.0f - ((1.0f - fabsf(currentForwardVector[2])) * (1.0f - fabsf(currentForwardVector[2])));
            const float pitchdiffOriginal = history.previousPitch - currentPitch;
            float pitchdiffAdjusted = pitchdiffOriginal;

            if (currentPitch > 0.0f)
                pitchdiffAdjusted = pitchdiffOriginal - ((viewdotMotion / 2.0f) * pitchCompensateMask);
            else
                pitchdiffAdjusted = pitchdiffOriginal + ((viewdotMotion / 2.0f) * pitchCompensateMask);


            if (pitchdiffOriginal < 0.0f)
                pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, pitchdiffOriginal, 0.0f);
            else
                pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, 0.0f, pitchdiffOriginal);

            motionBlurValues[1] = pitchdiffAdjusted / verticalFov;
            motionBlurValues[3] = undampenedYaw;
            motionBlurValues[3] *= (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f);

            if (timeElapsed > 0.0f)
                motionBlurValues[2] /= timeElapsed * 30.0f;
            else
                motionBlurValues[2] = 0.0f;

            motionBlurValues[2] = std::clamp((fabsf(motionBlurValues[2]) - vCfg.motionBlur.fallingMin) / (vCfg.motionBlur.fallingMax - vCfg.motionBlur.fallingMin), 0.0f, 1.0f) * (motionBlurValues[2] >= 0.0f ? 1.0f : -1.0f);
            motionBlurValues[2] /= 30.0f;
            motionBlurValues[0] *= vCfg.motionBlur.rotationIntensity * .15f * vCfg.motionBlur.strength;
            motionBlurValues[1] *= vCfg.motionBlur.rotationIntensity * .15f * vCfg.motionBlur.strength;
            motionBlurValues[2] *= vCfg.motionBlur.rotationIntensity * .15f * vCfg.motionBlur.strength;
            motionBlurValues[3] *= vCfg.motionBlur.fallingIntensity * .15f * vCfg.motionBlur.strength;

        }

        if (memory->globalVars->realtime < history.noRotationalMotionBlurUntil)
        {
            motionBlurValues[0] = 0.0f;
            motionBlurValues[1] = 0.0f;
            motionBlurValues[3] = 0.0f;
        }
        else
        {
            history.noRotationalMotionBlurUntil = 0.0f;
        }
        history.previousPositon = currentPosition;

        history.previousPitch = currentPitch;
        history.previousYaw = currentYaw;
        history.lastTimeUpdate = memory->globalVars->realtime;
        return;
    }

    const auto material = interfaces->materialSystem->findMaterial("dev/motion_blur", "RenderTargets", false);
    if (!material)
        return;

    const auto MotionBlurInternal = material->findVar("$MotionBlurInternal", nullptr, false);

    MotionBlurInternal->setVecComponentValue(motionBlurValues[0], 0);
    MotionBlurInternal->setVecComponentValue(motionBlurValues[1], 1);
    MotionBlurInternal->setVecComponentValue(motionBlurValues[2], 2);
    MotionBlurInternal->setVecComponentValue(motionBlurValues[3], 3);

    const auto MotionBlurViewPortInternal = material->findVar("$MotionBlurViewportInternal", nullptr, false);

    MotionBlurViewPortInternal->setVecComponentValue(0.0f, 0);
    MotionBlurViewPortInternal->setVecComponentValue(0.0f, 1);
    MotionBlurViewPortInternal->setVecComponentValue(1.0f, 2);
    MotionBlurViewPortInternal->setVecComponentValue(1.0f, 3);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::particleFootTrail() noexcept {

    if (!vCfg.footTrail) return;

    if (!localPlayer) return;

    if (localPlayer->isAlive())
        interfaces->effects->EnergySplash(localPlayer->getAbsOrigin(), { 0.f, 0.f, 0.f }, true);

    /*static Vector lastpos;

    BeamInfo beamInfo;
    beamInfo.type = TE_BEAMPOINTS;
    beamInfo.modelName = "sprites/purplelaser1.vmt";
    beamInfo.modelIndex = interfaces->modelInfo->getModelIndex("sprites/purplelaser1.vmt");
    beamInfo.haloScale = 0;
    beamInfo.life = 1.f;
    beamInfo.width = 2;
    beamInfo.endWidth = 2;
    beamInfo.fadeLength = 0;
    beamInfo.amplitude = 0;
    beamInfo.red = 255;
    beamInfo.green = 255;
    beamInfo.blue = 255;
    beamInfo.brightness = 255;
    beamInfo.speed = 0;
    beamInfo.startFrame = 0;
    beamInfo.frameRate = 0;
    beamInfo.segments = 2;
    beamInfo.flags = FBEAM_ONLYNOISEONCE;
    beamInfo.start = localPlayer->origin();
    beamInfo.end = lastpos;

    if (const auto beam = memory->viewRenderBeams->createBeamRingPoints(beamInfo)){
        constexpr auto FBEAM_FOREVER = 0x4000;
        beam->flags &= ~FBEAM_FOREVER;
        beam->die = memory->globalVars->currenttime + .5f;
        memory->viewRenderBeams->drawBeam(beam);
    }
    lastpos = localPlayer->origin();*/
}

void Visuals::updateEventListeners(bool forceRemove) noexcept
{
    class ImpactEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) {
            bulletTracer(*event);
            bulletImpact(*event);
        }
    };

    static ImpactEventListener listener;
    static bool listenerRegistered = false;

    if ((vCfg.bulletImpacts.enabled || vCfg.bulletTracers.enabled) && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "bullet_impact");
        listenerRegistered = true;
    }
    else if (((!vCfg.bulletImpacts.enabled && !vCfg.bulletTracers.enabled) || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

static bool windowOpen = false;

void Visuals::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Visuals")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Visuals");
        ImGui::SetWindowPos("Visuals", { 100.0f, 100.0f });
    }
}

void Visuals::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 680.0f, 0.0f });
        ImGui::Begin("Visuals", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 288.0f);
    ImGui::PushItemWidth(120.f);
    if (!Helpers::getByteFromBytewise(vCfg.removals, 13)) {
        ImGui::Checkbox("Custom post-processing", &vCfg.customPostProcessing.enabled);
        ImGui::SameLine();

        if (ImGui::Button("EdIt"))
            ImGui::OpenPopup("##pppopup");

        if (ImGui::BeginPopup("##pppopup"))
        {
            ImGui::SliderFloat("##worldExposure", &vCfg.customPostProcessing.worldExposure, 0.0f, 100.f, "World exposure: %.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("##modelAmbient", &vCfg.customPostProcessing.modelAmbient, 0.0f, 100.f, "Model Ambient: %.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("##bloomScale", &vCfg.customPostProcessing.bloomScale, 0.0f, 100.f, "Bloom scale: %.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::EndPopup();
        };

        ImGui::Checkbox("Color correction", &vCfg.colorCorrection.enabled);

        ImGui::SameLine();

        if (bool ccPopup = ImGui::Button("Edit"))
            ImGui::OpenPopup("##popup");

        if (ImGui::BeginPopup("##popup")) {
            ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &vCfg.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &vCfg.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &vCfg.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &vCfg.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &vCfg.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &vCfg.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &vCfg.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f"); ImGui::SameLine();
            ImGui::EndPopup();
        }
    };
    ImGuiCustom::colorPicker("Fog controller", vCfg.fog.color);
    ImGui::SameLine();

    ImGui::PushID("Fog changer");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::PushItemWidth(100.f);
        ImGui::SliderFloat("##start", &vCfg.fog.FogStart, 0.f, 5000.f, "Start: %.0f");
        ImGui::SliderFloat("##end", &vCfg.fog.FogEnd, 0.f, 5000.f, "End: %.0f");
        ImGui::SliderFloat("##density", &vCfg.fog.FogDens, 0.001f, 1.f, "Density: %.3f");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Shadow changer", &vCfg.shadowsChanger.enabled);
    ImGui::SameLine();

    ImGui::PushID("Shadow changer");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::PushItemWidth(100.f);
        ImGui::SliderInt("##xShadow", &vCfg.shadowsChanger.x, 0, 360, "X: %d");
        ImGui::SliderInt("##yShadow", &vCfg.shadowsChanger.y, 0, 360, "Y: %d");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Motion Blur", &vCfg.motionBlur.enabled);
    ImGui::SameLine();

    ImGui::PushID("Motion Blur");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {

        ImGui::Checkbox("Forward enabled", &vCfg.motionBlur.forwardEnabled);

        ImGui::PushItemWidth(150.0f);
        ImGui::SliderFloat("##fallingMin", &vCfg.motionBlur.fallingMin, 0.0f, 50.0f, "Falling min: %.2f");
        ImGui::SliderFloat("##fallingMax", &vCfg.motionBlur.fallingMax, 0.0f, 50.0f, "Falling max: %.2f");
        ImGui::SliderFloat("##fallingIntesity", &vCfg.motionBlur.fallingIntensity, 0.0f, 8.0f, "Falling intensity: %.2f");
        ImGui::SliderFloat("##rotationIntensity", &vCfg.motionBlur.rotationIntensity, 0.0f, 8.0f, "Rotation intensity: %.2f");
        ImGui::SliderFloat("##strength", &vCfg.motionBlur.strength, 0.0f, 8.0f, "Strength: %.2f");
        ImGui::PopItemWidth();

        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("ragdolls");
    ImGui::Checkbox("Ragdolls", &vCfg.ragdolls.enabled);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::PushItemWidth(100.0f);
        ImGui::Checkbox("Disable", &vCfg.ragdolls.disabled);
        ImGui::SliderInt("##gravity", &vCfg.ragdolls.gravity, -1000, 1000, "Gravity: %d");
        ImGui::SliderInt("##force", &vCfg.ragdolls.force, -100, 1000, "Force: %d");
        ImGui::SliderFloat("##timescale", &vCfg.ragdolls.timescale, 0.001f, 8.000f, "Timescale: %.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::PopItemWidth();

        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PushID(1);
    ImGui::Checkbox("Viewmodel", &vCfg.viewModel.enabled);
    ImGui::SameLine();
    if (ImGui::Button("...##visuals_5"))
        ImGui::OpenPopup("popup_viewModel");

    if (ImGui::BeginPopup("popup_viewModel"))
    {
        ImGui::PushItemWidth(100.f);
        ImGui::SliderFloat("##vm_x", &vCfg.viewModel.x, -20.00f, 20.00f, "X: %.2f");
        ImGui::SliderFloat("##vm_y", &vCfg.viewModel.y, -20.00f, 20.00f, "Y: %.2f");
        ImGui::SliderFloat("##vm_z", &vCfg.viewModel.z, -20.00f, 20.00f, "Z: %.2f");
        ImGui::SliderFloat("##vm_r", &vCfg.viewModel.roll, -90.00f, 90.00f, "Roll: %.2f");
        ImGui::SliderInt("##vm_f", &vCfg.viewModel.Fov, -60, 60, "FOV: %d");
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGuiCustom::multiCombo("HUD Removals", vCfg.hideHud, "ALL\0Health\0Misc\0Chat\0Crosshair\0Radar\0Scoreboard\0");
    ImGuiCustom::multiCombo("Removals", vCfg.removals, "Scope Overlay\0Scope Zoom\0Scope Sway\0Aim Punch\0View Punch\0Hands\0Sleeves\0Weapons\0Shadows\0Skybox 3D\0Fog\0Sprites\0Grass\0Post-Processing\0Blur\0Panorama Blur\0View Bob\0Weapon Sway\0");
    ImGui::Combo("Smoke Type", &vCfg.smokeMode, "Normal\0Disable\0Wireframe\0Decreased\0");
    ImGui::Combo("Molotov Type", &vCfg.moloMode, "Normal\0Disable\0Wireframe\0");
    ImGui::PushID("playermodel");
    ImGui::InputText("Custom Player Model", vCfg.playerModel, sizeof(vCfg.playerModel));
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("File must be in csgo/models/ directory and must start with models/...");
    ImGui::PopID();
    ImGui::Combo("Skybox", &vCfg.skybox, Visuals::skyboxList.data(), Visuals::skyboxList.size());
    if (vCfg.skybox == 26) {
        ImGui::InputText("Skybox filename", vCfg.customSkybox, sizeof(vCfg.customSkybox));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Skybox files must be put in csgo/materials/skybox/ directory");
    }
    ImGui::Checkbox("Mask", &vCfg.maskChanger);
    ImGui::Checkbox("Don't Render Teammates", &vCfg.dontRenderTeammates);
    ImGui::Checkbox("Disable Jiggle Bones", &vCfg.jigglebones);
    ImGui::Checkbox("Foot Trail", &vCfg.footTrail);
    ImGui::PushID("ThunderK");
    ImGui::Checkbox("Thunderbolt Kills", &vCfg.killLightningBolt);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Strikes player on kill");
    ImGui::PopID();
    ImGui::PushID("Asus");
    ImGui::Checkbox("AsusWalls", &vCfg.asus.enabled);
    if (vCfg.asus.enabled) {
        ImGui::SliderFloat("##walls", &vCfg.asus.walls, 0.f, 1.f, "Walls %.2f");
        ImGui::SliderFloat("##props", &vCfg.asus.props, 0.f, 1.f, "Props %.2f");
    }
    ImGui::PopID();
    ImGui::Checkbox("FullBright", &vCfg.fullBright);
    ImGui::Checkbox("Deagle spinner", &vCfg.deagleSpinner);
    ImGuiCustom::colorPicker("Bullet Impacts", vCfg.bulletImpacts.color.data(), &vCfg.bulletImpacts.color[3], nullptr, nullptr, &vCfg.bulletImpacts.enabled);
    ImGui::SliderFloat("Bullet Impacts time", &vCfg.bulletImpactsTime, 0.1f, 5.0f, "%.2fs");
    ImGui::Checkbox("Bullet Tracer", &vCfg.bulletTracers.enabled);
    ImGui::SameLine();
    ImGui::PushID("Beam");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("beamms");

    if (ImGui::BeginPopup("beamms"))
    {
        ImGuiCustom::colorPicker("Color", vCfg.bulletTracers.asColor4().color.data(), &vCfg.bulletTracers.asColor4().color[3], nullptr, nullptr);
        ImGui::PushID("beamtype");
        ImGui::SetNextItemWidth(135);
        ImGui::Combo("Texture", &vCfg.beambullets.textureid, beamtextures, ARRAYSIZE(beamtextures));
        ImGui::SliderFloat("##power", &vCfg.beambullets.amplitude, 0.f, 5.f, "Power: %.f");
        ImGui::SliderFloat("##life", &vCfg.beambullets.life, 1.f, 60.f, "Life Time: %.f");
        ImGui::EndPopup();
    }

    ImGui::PopID();

    ImGui::Checkbox("DLights", &vCfg.dLight.enabled);
    ImGui::SameLine();
    ImGui::PushID("delights");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("li");

    if (ImGui::BeginPopup("li"))
    {
        ImGuiCustom::colorPicker("Color Allies", vCfg.dlightsColor.asColor4().color.data(), &vCfg.dlightsColor.asColor4().color[3], nullptr, nullptr);
        ImGui::SameLine();
        ImGui::Checkbox("Allies", &vCfg.dLight.allies);
        ImGuiCustom::colorPicker("Color Self", vCfg.dlightsColor1.asColor4().color.data(), &vCfg.dlightsColor1.asColor4().color[3], nullptr, nullptr);
        ImGui::SameLine();
        ImGui::Checkbox("Self", &vCfg.dLight.local);
        ImGuiCustom::colorPicker("Color Ememies", vCfg.dlightsColor2.asColor4().color.data(), &vCfg.dlightsColor2.asColor4().color[3], nullptr, nullptr);
        ImGui::SameLine();
        ImGui::Checkbox("Ememies", &vCfg.dLight.enemies);

        ImGui::SliderFloat("##radius", &vCfg.dLight.radius, 10.0f, 500.0f, "Radius: %.2f");
        ImGui::SliderFloat("##decay", &vCfg.dLight.decay, 10.0f, 200.0f, "Decay: %.2f");
        ImGui::EndPopup();
    }

    ImGui::PopID();

    ImGui::Checkbox("Molotov Polygon", &vCfg.molotovPolygon.enabled);
    ImGui::SameLine();
    ImGui::PushID("molotovPolygon");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGuiCustom::colorPicker("Allies", vCfg.molotovPolygon.ally.color.data(), &vCfg.molotovPolygon.ally.color[3], nullptr, nullptr);
        ImGuiCustom::colorPicker("Enemies", vCfg.molotovPolygon.enemy.color.data(), &vCfg.molotovPolygon.enemy.color[3], nullptr, nullptr);
        ImGuiCustom::colorPicker("Self", vCfg.molotovPolygon.self.color.data(), &vCfg.molotovPolygon.self.color[3], nullptr, nullptr);
        ImGui::EndPopup();
    }

    ImGui::PopID();
    ImGuiCustom::colorPicker("Smoke Hull", vCfg.smokeHull);
    ImGui::NextColumn(); /////////////////////////////////NEW COLUMN
    ImGui::hotkey("Flashlight", vCfg.flashLight);
    ImGui::Checkbox("Zoom", &vCfg.zoom);
    ImGui::SameLine();
    ImGui::PushID("Zoom Key");
    ImGui::hotkey("", vCfg.zoomKey);
    ImGui::PopID();
    ImGui::Checkbox("Thirdperson", &vCfg.thirdperson);
    ImGui::SameLine();
    ImGui::PushID("Thirdperson Key");
    ImGui::hotkey("", vCfg.thirdpersonKey);
    ImGui::PopID();
    ImGui::PushID(0);
    ImGui::SliderInt("", &vCfg.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
    ImGui::PopID();
    ImGui::PushID("fov");
    ImGui::SliderInt("", &vCfg.fov, -60, 60, "FOV: %d");
    ImGui::PopID();
    ImGui::PushID(3);
    ImGui::SliderInt("", &vCfg.farZ, 10, 2000, "Far Z: %d");
    ImGui::PopID();
    ImGui::PushID(4);
    ImGui::SliderInt("", &vCfg.flashReduction, 0, 100, "Flash reduction: %d%%");
    ImGui::PopID();
    ImGui::PushID(5);
    ImGui::SliderFloat("", &vCfg.brightness, 0.0f, 100.0f, "Nightmode: %.2f%");
    ImGui::PopID();
    ImGui::PushID("scopetrans");
    ImGui::SliderFloat("", &vCfg.thirdpersonTransparency, 0.0f, 100.0f, "Scope Transparency: %2.f%");
    ImGui::PopID();
    ImGui::Combo("Screen effect", &vCfg.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(80.f);
    ImGui::Combo("", &vCfg.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::SameLine();
    ImGui::PushID("HE");
    ImGui::SliderFloat("Hit Effect", &vCfg.hitEffectTime, 0.1f, 1.5f, "%.2fs");
    ImGui::Combo("", &vCfg.hitMarker, "None\0Cross\0Cross Fading\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID("HM");
    ImGui::SliderFloat("", &vCfg.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
    ImGui::PopID();
    ImGui::PushID("coloraro");
    ImGui::SameLine();
    ImGuiCustom::colorPicker("Hit Marker", vCfg.hitMarkerColor.color.data());
    ImGui::PopID();
    ImGui::PopItemWidth();

    ImGui::PushID("matrixHitbox");
    ImGuiCustom::colorPicker("Hitbox matrix", vCfg.hitMatrix.color.color.data(), &vCfg.hitMatrix.color.color[3], nullptr, nullptr, &vCfg.hitMatrix.color.enabled);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::PushItemWidth(100.f);
        ImGui::SliderFloat("##time", &vCfg.hitMatrix.duration, 0.1f, 60.f, "Time: %.2fs", ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("netgraph");
    ImGuiCustom::colorPicker("Net Graph", vCfg.netGraph.color);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::PushItemWidth(100.f);
        ImGui::SliderInt("##font", &vCfg.netGraph.font, -1, 297, "Font: %i", ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("skeet scope");
    ImGuiCustom::colorPicker("Scope Crosshair", vCfg.customScopeCrosshair.color);
    ImGui::SameLine();
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup(""))
    {
        ImGui::Combo("Mode", &vCfg.customScopeCrosshair.mode, "Static\0Custom");

        ImGui::SliderFloat("##height", &vCfg.customScopeCrosshair.height, 1.0f, 500.0f, "Height: %.2f");
        ImGui::SliderFloat("##offset", &vCfg.customScopeCrosshair.offset, -500.0f, 500.0f, "Offset: %.2f");
        ImGui::SliderFloat("##speed", &vCfg.customScopeCrosshair.speed, 1.0f, 100.0f, "Speed: %.2f");
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::PushID("Modulate");
    ImGuiCustom::colorPicker("Damage Number", vCfg.damageNumber);
    ImGuiCustom::colorPicker("World Color", vCfg.world);
    ImGuiCustom::colorPicker("Prop Color", vCfg.props);
    ImGuiCustom::colorPicker("Sky Color", vCfg.sky);
    ImGuiCustom::colorPicker("VGUI Color", vCfg.console);
    ImGuiCustom::colorPicker("Molotov Color", vCfg.molotovColor);
    ImGuiCustom::colorPicker("Smoke Color", vCfg.smokeColor);
    ImGuiCustom::colorPicker("Zeus Laser Color", vCfg.zeusLaserColor);
    ImGuiCustom::colorPicker("Headshot Color", vCfg.headShotColor);
    ImGuiCustom::colorPicker("Crosshair Outline Color", vCfg.crosshairOutlineColor);
    ImGui::PopID();
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

json Visuals::toJson() noexcept
{
    json j;
    to_json(j, vCfg);
    return j;
}

void Visuals::fromJson(const json& j) noexcept
{
    from_json(j, vCfg);
}

void Visuals::resetConfig() noexcept
{
    vCfg = {};
}
