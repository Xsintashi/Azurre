#pragma once

#include <optional>

struct ImFont;
struct pngTXT;

class GUI {
public:
    GUI() noexcept;
    ImFont* getUnicodeFont() const noexcept;
    ImFont* getTahoma28Font() const noexcept;
    void baner(ImDrawList* d, ImVec2 b);
    void debugpic(ImDrawList* d, ImVec2 b);
    void render() noexcept;
    void handleToggle() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
    void updateColors() noexcept;

    struct {
        ImFont* normal15px = nullptr;
        ImFont* unicodeFont = nullptr;
        ImFont* tahoma34 = nullptr;
        ImFont* tahoma28 = nullptr;
        ImFont* tahoma20 = nullptr;
    } fonts;

    float getTransparency() const noexcept {
        return std::clamp(open ? toggleAnimationEnd : 1.0f - toggleAnimationEnd, 0.0f, 1.0f);
    }

private:

    enum SECTION {
        LEGIT = 0,
        RAGE,
        VISUALS,
        MISC,
        CHANGER
    };

    enum WINDOW {
        LEGITBOT,
        LEGIT_ANTIAIM,
        TRIGGERBOT,
        BACKTRACK,
        RAGEBOT,
        ANTIAIM,
        FAKE_LAG,
        VISUALS_MAIN,
        CHAMS,
        ESP,
        GLOW,
        GUINTERFACE,
        MISC_MAIN,
        MOVEMENT,
        MOVEMENT_RECORDER,
        SOUND,
        GRIEF,
        EXTRA,
        GRENADE_HELPER,
        DISCORD_RP,
        WALKBOT,
        LOBBY,
        PANORMAMA,
        INVENTORY,
        PROFILE,
        DEBUG
    };

    bool open = false;
    void renderMenuBar() noexcept;
    void renderLegitBotGui(bool contentOnly = false);
    void renderRageBotGui(bool contentOnly);
    void renderTriggerbotWindow(bool contentOnly = false) noexcept;
    void renderChamsWindow(bool contentOnly = false) noexcept;
    void renderStyleWindow(bool contentOnly = false) noexcept;
    void renderConfigWindow(bool contentOnly = false) noexcept;
    void renderGuiStyle2() noexcept;

    inline constexpr float animationLength() { return 0.25f; }
    float toggleAnimationEnd = 0.0f;

    struct {
        bool
            legit = false,
            rage = false,
            triggerbot = false,
            chams = false,
            profileChanger = false,
            sound = false,
            style = false,
            config = false,
            about = false;
    } window;

    float timeToNextConfigRefresh = 0.1f;
};

inline std::optional<GUI> gui;
