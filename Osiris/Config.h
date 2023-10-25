#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>

#include "ConfigStructs.h"
#include "InputUtil.h"

struct ImFont;

class Config {
public:
    Config() noexcept;
    bool load(std::size_t, bool incremental, bool clip) noexcept;
    bool load(const char8_t* name, bool incremental, bool clip) noexcept;
    void save(std::size_t, bool clip) const noexcept;
    void add(const char8_t*) noexcept;
    void remove(std::size_t) noexcept;
    void rename(std::size_t, std::u8string_view newName) noexcept;
    void reset() noexcept;
    void listConfigs() noexcept;
    void createConfigDir() const noexcept;
    void openConfigDir() const noexcept;

    constexpr auto& getConfigs() noexcept
    {
        return configs;
    }

    struct LegitAimbot {
        bool enabled{ false };
        bool aimlock{ false };
        bool friendlyFire{ false };
        bool visibleOnly{ true };
        bool scopedOnly{ true };
        bool ignoreFlash{ false };
        bool ignoreSmoke{ false };
        bool autoShot{ false };
        float fov{ 0.0f };
        float deadzone{ 0.0f };
        float reactionTime{ 0.0f };
        float smooth{ 1.0f };
        int hitboxes{ 0 };
        int minDamage{ 0 };

        bool standaloneRCS{ false };
        bool randomRCS{ true };
        float recoilControlX{ 0.0f };
        float recoilControlY{ 0.0f };
        int shotsFired{ 0 };

        bool killshot{ false };
        bool betweenShots{ true };
    };
    std::array<LegitAimbot, 40> legitaimbot;
    bool globalEnabledRagebot = false, resolver = false;
    ColorToggle legitbotFov{ 1.0f, 1.0f, 1.0f, 0.25f };
    ColorToggle legitbotDeadzone{ 1.0f, 1.0f, 1.0f, 0.25f };
    KeyBind aimbotKey;
    KeyBind visibleOnlyOverride;
    KeyBind minDamageOverride;
    int minDamageMode = 0;
    KeyBind forceOnShot;
    KeyBind forceBaim;
    int aimbotKeyMode{ 0 };
    struct RageAimbot {
        bool renabled{ false };
        bool raimlock{ false };
        bool rsilent{ false };
        bool rfriendlyFire{ false };
        bool rvisibleOnly{ true };
        bool rautoShot{ false };
        bool rscopedOnly{ false };
        bool rautoScope{ false };
        float rfov{ 0.0f };
        int rhitbox{ 1 };
        int rbaimHitbox{ 0 };
        int rmultiPoint{ 0 };
        int rminDamage{ 1 };
        int rhitchance{ 1 };
        int roverridedMinDamage{ 1 };
        float rinaccurancy{ 0 };
        bool rbetweenShots{ false };
        bool rforceAccuracy{ false };
        bool rautoDuck{ false };
        int rpriority{ 0 };
    };
    std::array<RageAimbot, 40> rageaimbot;

    struct ZeusBot {
        bool enable{ false };
        bool silent{ false };
        int autoZeusMaxPenDist{ 0 };
        int hitchance{ 0 };
    }zeusbot;

    struct Triggerbot {
        bool enabled = false;
        bool friendlyFire = false;
        bool scopedOnly = true;
        bool ignoreFlash = false;
        bool ignoreSmoke = false;
        bool killshot = false;
        int hitgroup = 0;
        int shotDelay = 0;
        int minDamage = 1;
        float burstTime = 0.0f;
    };
    std::array<Triggerbot, 40> triggerbot;
    KeyBind triggerbotHoldKey;

    struct Chams {
        struct Material : Color4 {
            bool enabled = false;
            bool healthBased = false;
            bool blinking = false;
            bool wireframe = false;
            bool cover = false;
            bool ignorez = false;
            int material = 0;
            std::array<std::string, 40> customWeapon;
            std::array<std::string, 40> customMaterial;
        };
        std::array<Material, 8> materials;
    };
    int activeWeaponChams = 0;
    int customChams = 0;
    std::unordered_map<std::string, Chams> chams;
    KeyBindToggle chamsToggleKey;
    KeyBind chamsHoldKey;

    struct StreamProofESP {
        KeyBindToggle toggleKey;
        KeyBind holdKey;

        std::unordered_map<std::string, Player> allies;
        std::unordered_map<std::string, Player> enemies;
        std::unordered_map<std::string, Weapon> weapons;
        std::unordered_map<std::string, Projectile> projectiles;
        std::unordered_map<std::string, Shared> lootCrates;
        std::unordered_map<std::string, Shared> otherEntities;
    } streamProofESP;

    struct Font {
        ImFont* tiny;
        ImFont* medium;
        ImFont* big;
    };
   
    struct Style {
        int menuStyle{ 0 };
        int menuColors{ 0 };
        int prefixColor{ 10 };
        bool roundedCorners = true, 
            antiAliasing = true, windowBorder = false, centerTitle = false, frameBorder = false, blockInput = true, escCloseMenu = false;
        float scale = 1.f;

        struct BackgroundEffect {
            ColorToggle3 lineCol;
            float darkness = 0.f;
            int N = 1000;
            int lineMaxDist = 1000;
            float lineThickness = 2.0f;
        }bgEffect;
        bool banner = false;

    } style;

    struct SkinChangerOptions {
        bool rareAnimationKnives = false;
    }skinConfig;

    struct ProfileChanger {
        bool enabled{ false };
        bool enabledRanks{ false };
        bool enabledCommends{ false };
        bool enabledBans{ false };
        bool enabledStats{ false };
        bool enabledPrime{ false };
        bool hasCommunicationAbuseMute{ false };
        int accountStatus = 0;
        int colorTeam = 0;
        int  friendly = 0;
        int  teach = 0;
        int  leader = 0;
        int  rank = 0;
        int  wmrank = 0;
        int  dzrank = 0;
        int  wins = 10;
        int  wmwins = 10;
        int  dzwins = 10;
        int  level = 1;
        int  exp = 0;
        int  banType = 0;
        int  banTime = 0;
        int  kills = 0;
        int  assists = 0;
        int  deaths = 0;
        int  mvp = 0;
        int  score = 0;
    }profilechanger;

    void scheduleFontLoad(const std::string& name) noexcept;
    bool loadScheduledFonts() noexcept;
    const auto& getSystemFonts() const noexcept { return systemFonts; }
    const auto& getFonts() const noexcept { return fonts; }
    std::filesystem::path path;
private:
    std::vector<std::string> scheduledFonts{ "Default" };
    std::vector<std::string> systemFonts{ "Default" };
    std::unordered_map<std::string, Font> fonts;
    std::vector<std::u8string> configs;
};

inline std::optional<Config> config;
