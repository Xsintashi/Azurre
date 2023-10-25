#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "imguiCustom.h"

#include "GUI.h"
#include "Config.h"
#include "ConfigStructs.h"
#include "Helpers.h"
#include "Hooks.h"
#include "Memory.h"
#include "Interfaces.h"

#include "DiscordSDK/RPC.h"

#include "InventoryChanger/InventoryChanger.h"

#include "SDK/ClientMode.h"
#include "SDK/Entity.h"
#include "SDK/GameUI.h"
#include "SDK/InputSystem.h"
#include "SDK/PlayerResource.h"

#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Chams.h"
#include "Hacks/Clantag.h"
#include "Hacks/Extra.h"
#include "Hacks/FakeLag.h"
#include "Hacks/Glow.h"
#include "Hacks/GrenadeHelper.h"
#include "Hacks/Grief.h"
#include "Hacks/Lobby.h"
#include "Hacks/Logger.h"
#include "Hacks/Misc.h"
#include "Hacks/Movement.h"
#include "Hacks/MovementRecorder.h"
#include "Hacks/Panorama.h"
#include "Hacks/ProfileChanger.h"
#include "Hacks/Sound.h"
#include "Hacks/StreamProofESP.h"
#include "Hacks/Visuals.h"
#include "Hacks/Walkbot.h"
#include "Hacks/Xsin.h"

//constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

#pragma warning (disable : 4244)

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
    auto file = Helpers::loadBinaryFile(path);
    if (!Helpers::decodeVFONT(file))
        return nullptr;

    ImFontConfig cfg;
    cfg.FontData = file.data();
    cfg.FontDataSize = file.size();
    cfg.FontDataOwnedByAtlas = false;
    cfg.MergeMode = merge;
    cfg.GlyphRanges = glyphRanges;
    cfg.SizePixels = size;

    return ImGui::GetIO().Fonts->AddFont(&cfg);
}

GUI::GUI() noexcept
{
    ImGui::StyleColorsAzurre();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg;
    cfg.SizePixels = 15.0f;

#ifdef _WIN32
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);

        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "trebuc.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        //fonts.normal15px = addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", 15.0f, Helpers::getFontGlyphRanges(), false);
        if (!fonts.normal15px)
            io.Fonts->AddFontDefault(&cfg);

        fonts.tahoma28 = io.Fonts->AddFontFromFileTTF((path / "tahomabd.ttf").string().c_str(), 28.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.tahoma28)
            io.Fonts->AddFontDefault(&cfg);

        fonts.tahoma34 = io.Fonts->AddFontFromFileTTF((path / "trebuc.ttf").string().c_str(), 34.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.tahoma34)
            io.Fonts->AddFontDefault(&cfg);

        fonts.tahoma20 = io.Fonts->AddFontFromFileTTF((path / "trebuc.ttf").string().c_str(), 20.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.tahoma20)
            io.Fonts->AddFontDefault(&cfg);

        addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
        addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
        constexpr auto unicodeFontSize = 16.0f;
        fonts.unicodeFont = addFontFromVFONT("csgo/panorama/fonts/notosans-bold.vfont", unicodeFontSize, Helpers::getFontGlyphRanges(), false);

        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{
            0x2605, 0x2605, // ★
            0
        };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }
#else
    fonts.normal15px = addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", 15.0f, Helpers::getFontGlyphRanges(), false);
#endif
    if (!fonts.normal15px)
        io.Fonts->AddFontDefault(&cfg);
    if (!fonts.tahoma34)
        io.Fonts->AddFontDefault(&cfg);
    if (!fonts.tahoma28)
        io.Fonts->AddFontDefault(&cfg);
    if (!fonts.tahoma20)
        io.Fonts->AddFontDefault(&cfg);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
}

ImFont* GUI::getTahoma28Font() const noexcept
{
    return fonts.tahoma28;
}

ImFont* GUI::getUnicodeFont() const noexcept
{
    return fonts.unicodeFont;
}

// Background effect

void setupPoints(std::vector<std::pair<ImVec2, ImVec2>>& n) {
    ImVec2 screenSize(ImGui::GetIO().DisplaySize);
    for (auto& p : n)
        p.second = p.first = ImVec2(rand() % (int)screenSize.x, rand() % (int)screenSize.y);
}

float length(ImVec2 x) { return x.x * x.x + x.y * x.y; }

void darkBck(ImDrawList* d, ImVec2 b) {
    d->AddRectFilled({ 0.f, 0.f }, b, Helpers::calculateColor(Color4{0.f, 0.f, 0.f, config->style.bgEffect.darkness}), 0.f);
}

void GUI::baner(ImDrawList* d, ImVec2 b) { //640x128 //320x64 
    d->AddImage(GameData::getBaner(), { (b.x / 2.f) - 320.f , 64.f }, { (b.x / 2.f) + 320.f, 192.f });
}

void animehehe(ImDrawList* d, ImVec2 b) {
    d->AddImage(GameData::getAnime(), { 0.f, b.y - 320.f }, { 256.f, b.y });
}

void GUI::debugpic(ImDrawList* d, ImVec2 b) {
    d->AddImage(GameData::getDebug(), { b.x - 205.f, b.y - 256.f }, { b.x, b.y });
}


void FX(ImDrawList* d, ImVec2 b)
{
    static std::vector<std::pair<ImVec2, ImVec2>> points(config->style.bgEffect.N);
    static auto once = (setupPoints(points), true);
    float Dist;
    for (auto& p : points) {
        Dist = sqrt(length(p.first - p.second));
        if (Dist > 0) p.first += (p.second - p.first) / Dist;
        if (Dist < 4) p.second = ImVec2(rand() % (int)b.x, rand() % (int)b.y);
    }
    for (int i = 0; i < config->style.bgEffect.N; i++) {
        for (int j = i + 1; j < config->style.bgEffect.N; j++) {
            Dist = length(points[i].first - points[j].first);
            if (Dist < config->style.bgEffect.lineMaxDist) d->AddLine(points[i].first, points[j].first, Helpers::calculateColor(config->style.bgEffect.lineCol.asColor3()), config->style.bgEffect.lineThickness);
        }
    }
}// Background effect

void GUI::render() noexcept
{
    ImGui::GetIO().FontGlobalScale = config->style.scale;

    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    if (config->style.bgEffect.darkness)
        darkBck(ImGui::GetBackgroundDrawList(), screenSize);
    
    if (config->style.bgEffect.lineCol.enabled) 
        FX(ImGui::GetBackgroundDrawList(), screenSize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, getTransparency());

    if (open && toggleAnimationEnd < 1.0f)
        ImGui::SetWindowFocus();

    toggleAnimationEnd += ImGui::GetIO().DeltaTime / animationLength();

    if (config->style.menuStyle) {
        renderMenuBar();
        renderLegitBotGui(false);
        AntiAim::drawLegitGUI(false);
        renderTriggerbotWindow(false);
        Backtrack::drawGUI(false);

        renderRageBotGui(false);
        AntiAim::drawGUI(false);
        FakeLag::drawGUI(false);

        Visuals::drawGUI(false);
        StreamProofESP::drawGUI(false);
        renderChamsWindow();
        Glow::drawGUI(false);
        renderStyleWindow();

        Misc::drawGUI(false);
        Movement::drawGUI(false);
        Sound::drawGUI(false);
        Fun::drawGUI(false);
        Extra::drawGUI(false);
        Nade::drawGUI(false);
        Discord::drawGUI(false);
        Lobby::drawGUI(false);

        inventory_changer::InventoryChanger::instance().drawGUI(false);
        Profile::drawGUI(false);

        renderConfigWindow(false);
    } else {
        renderGuiStyle2();
    }    

    auto& guiSettings = ImGui::GetStyle();
    auto& guiconfig = config->style;

    if (config->style.roundedCorners) {
        guiSettings.WindowRounding = 7.f;
        guiSettings.ChildRounding = 10.0f;
        guiSettings.FrameRounding = 10.0f;
        guiSettings.GrabRounding = 10.0f;
        guiSettings.PopupRounding = 10.0f;
        guiSettings.ScrollbarRounding = 10.0f;
        guiSettings.LogSliderDeadzone = 10.0f;
        guiSettings.TabRounding = 7.0f;
    }
    else {
        guiSettings.WindowRounding = 0.f;
        guiSettings.ChildRounding = 0.0f;
        guiSettings.FrameRounding = 0.0f;
        guiSettings.GrabRounding = 0.0f;
        guiSettings.PopupRounding = 0.0f;
        guiSettings.ScrollbarRounding = 0.0f;
        guiSettings.LogSliderDeadzone = 0.0f;
        guiSettings.TabRounding = 0.0f;
    }

    guiSettings.AntiAliasedLines = guiconfig.antiAliasing;
    guiSettings.AntiAliasedLinesUseTex = guiconfig.antiAliasing;
    guiSettings.AntiAliasedFill = guiconfig.antiAliasing;
    guiSettings.WindowBorderSize = guiconfig.windowBorder ? 1.f : 0.f;
    guiSettings.PopupBorderSize = guiconfig.windowBorder ? 1.f : 0.f;
    guiSettings.FrameBorderSize = guiconfig.frameBorder ? 1.f : 0.f;
    guiSettings.WindowTitleAlign = guiconfig.centerTitle ? ImVec2{0.5f, 0.5f} : ImVec2{ 0.f, 0.5f };

#if defined(_DEBUG)
    if(Xsin::ImGuiDemo())
        ImGui::ShowDemoWindow();
#endif
    ImGui::PopStyleVar();
}

void GUI::updateColors() noexcept
{
    switch (config->style.menuColors) {
    default:
    case 0: ImGui::StyleColorsAzurre(); break;
    case 1: ImGui::StyleColorsGenshi(); break;
    case 2: ImGui::StyleColorsLime(); break;
    case 3: ImGui::StyleColorsRed(); break;
    case 4: ImGui::StyleColorsGold(); break;
    case 5: ImGui::StyleColorsPandora(); break;
    case 6: ImGui::StyleColorsLight(); break;
    case 7: ImGui::StyleColorsDeepDark(); break;
    case 8: ImGui::StyleColorsVisualStudio(); break;
    case 9: ImGui::StyleColorsGoldSrc(); break;
    case 10: ImGui::StyleColorsClassic(); break;
    case 11: ImGui::StyleColorsNeverlose(); break;
    case 12: ImGui::StyleColorsAimware(); break;
    case 13: ImGui::StyleColorsOnetap(); break;
    case 14: break; //Custom
    }
}

void GUI::handleToggle() noexcept
{
    if (ImGui::IsKeyPressed(VK_ESCAPE) && config->style.escCloseMenu && open){
        open = false;
        interfaces->inputSystem->resetInputState();
    }

    if (Misc::isMenuKeyPressed()) {
        open = !open;
        if (!open)
            interfaces->inputSystem->resetInputState();

        ImGui::SetWindowFocus("Azurre Window");
#ifndef _WIN32
        ImGui::GetIO().MouseDrawCursor = gui->open;
#endif
        if (toggleAnimationEnd > 0.0f && toggleAnimationEnd < 1.0f)
            toggleAnimationEnd = 1.0f - toggleAnimationEnd;
        else
            toggleAnimationEnd = 0.0f;
    }
}

static void menuBarItem(const char* name, bool& enabled) noexcept
{
    if (ImGui::MenuItem(name)) {
        enabled = true;
        ImGui::SetWindowFocus(name);
        ImGui::SetWindowPos(name, { 100.0f, 100.0f });
    }
}
void GUI::renderLegitBotGui(bool contentOnly) {
    if (!contentOnly) {
        if (!window.legit)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("LegitBot", &window.legit, windowFlags);
    }

    static const char* hitboxes[]{ "Head","Chest","Stomach","Arms","Legs" };
    static bool hitbox[ARRAYSIZE(hitboxes)] = { false, false, false, false, false };
    static std::string previewvalue = "";
    bool once = false;

    ImGui::PushID("LegitBot");
    ImGui::Columns(2, "legitaimbot", false);
    ImGui::PushItemWidth(70.0f);
    ImGui::PushID(2);
    ImGui::Combo("", &config->aimbotKeyMode, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID("Aimbot Key");
    ImGui::hotkey("", config->aimbotKey);
    ImGui::PopID();
    ImGui::PopItemWidth();
    static int currentCategory{ 0 };
    static int currentWeapon{ 0 };
    ImGui::Checkbox("Enabled", &config->legitaimbot[currentWeapon].enabled);
    ImGui::Checkbox("Aimlock", &config->legitaimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Visible only", &config->legitaimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->legitaimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->legitaimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->legitaimbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(100.f);

    for (size_t i = 0; i < ARRAYSIZE(hitbox); i++)
    {
        hitbox[i] = (config->legitaimbot[currentWeapon].hitboxes & 1 << i) == 1 << i;
    }
    if (ImGui::BeginCombo("Hitbox", previewvalue.c_str()))
    {
        previewvalue = "";
        for (size_t i = 0; i < ARRAYSIZE(hitboxes); i++)
        {
            ImGui::Selectable(hitboxes[i], &hitbox[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
        }
        ImGui::EndCombo();
    }
    for (size_t i = 0; i < ARRAYSIZE(hitboxes); i++)
    {
        if (!once)
        {
            previewvalue = "";
            once = true;
        }
        if (hitbox[i])
        {
            previewvalue += previewvalue.size() ? std::string(", ") + hitboxes[i] : hitboxes[i];
            config->legitaimbot[currentWeapon].hitboxes |= 1 << i;
        }
        else
        {
            config->legitaimbot[currentWeapon].hitboxes &= ~(1 << i);
        }
    }

    ImGuiCustom::colorPicker("Aimbot Fov", config->legitbotFov.asColor4(), &config->legitbotFov.enabled);
    ImGuiCustom::colorPicker("Deadzone Fov", config->legitbotDeadzone.asColor4(), &config->legitbotDeadzone.enabled);
    ImGui::NextColumn(); /*                                    NEXT COLUMN                                    */
    ImGui::PushItemWidth(70.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->legitaimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->legitaimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->legitaimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->legitaimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("Fov", &config->legitaimbot[currentWeapon].fov, 0.0f, 180.0f, "%.2f");
    ImGui::SliderFloat("Deadzone", &config->legitaimbot[currentWeapon].deadzone, 0.0f, 90.0f, "%.2f");
    ImGui::SliderFloat("Smooth", &config->legitaimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Reaction Time", &config->legitaimbot[currentWeapon].reactionTime, 0.0f, 100.0f, "%.2f");
    ImGui::SliderInt("Min damage", &config->legitaimbot[currentWeapon].minDamage, 0, 101, "%d");
    config->legitaimbot[currentWeapon].minDamage = std::clamp(config->legitaimbot[currentWeapon].minDamage, 0, 250);
    ImGui::PopItemWidth();
    // rcs turns off with silent on, and comboboxes don't fit here
    // (you can't control recoil you can't see)
    ImGui::Checkbox("Standalone RCS", &config->legitaimbot[currentWeapon].standaloneRCS);
    if (config->legitaimbot[currentWeapon].standaloneRCS) {
        ImGui::SameLine();
        ImGui::Checkbox("Random RCS factor", &config->legitaimbot[currentWeapon].randomRCS);
        ImGui::InputInt("Ignore Shots", &config->legitaimbot[currentWeapon].shotsFired);
        if (config->legitaimbot[currentWeapon].randomRCS) {
            ImGui::SliderFloat("Recoil control X odds", &config->legitaimbot[currentWeapon].recoilControlX, 0.0f, 1.0f, "%.5f");
            ImGui::SliderFloat("Recoil control Y odds", &config->legitaimbot[currentWeapon].recoilControlY, 0.0f, 1.0f, "%.5f");
        }
        else {
            ImGui::SliderFloat("Recoil control X", &config->legitaimbot[currentWeapon].recoilControlX, 0.0f, 1.0f, "%.5f");
            ImGui::SliderFloat("Recoil control Y", &config->legitaimbot[currentWeapon].recoilControlY, 0.0f, 1.0f, "%.5f");
        }
    }
    ImGui::Checkbox("Killshot", &config->legitaimbot[currentWeapon].killshot);
    config->legitaimbot[currentWeapon].shotsFired = std::clamp(config->legitaimbot[currentWeapon].shotsFired, 0, 150);
    ImGui::Checkbox("Between shots", &config->legitaimbot[currentWeapon].betweenShots);
    ImGui::PopID();

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderRageBotGui(bool contentOnly) {
    if (!contentOnly) {
        if (!window.rage)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("RageBot", &window.rage, windowFlags);
    }

    static const char* hitboxes[]{ "Head","Chest","Stomach","Arms","Legs" };
    static bool hitbox[ARRAYSIZE(hitboxes)] = { false, false, false, false, false };
    static std::string previewvalue = "";
    bool once = false;

    ImGui::PushID("Ragebot");
    ImGui::Checkbox("Enabled", &config->globalEnabledRagebot);
    ImGui::Separator();
    ImGui::Columns(2, "ragebot", false);
    static int currentWeapon{ 0 };
    ImGui::Checkbox("Enabled Weapon", &config->rageaimbot[currentWeapon].renabled);
    static int currentCategory{ 0 };
    ImGui::Checkbox("Aimlock", &config->rageaimbot[currentWeapon].raimlock);
    ImGui::Checkbox("Silent", &config->rageaimbot[currentWeapon].rsilent);
    ImGui::Checkbox("Friendly fire", &config->rageaimbot[currentWeapon].rfriendlyFire);
    ImGui::Checkbox("Visible only", &config->rageaimbot[currentWeapon].rvisibleOnly);
    ImGui::Checkbox("Scoped only", &config->rageaimbot[currentWeapon].rscopedOnly);
    ImGui::Checkbox("Auto Scope", &config->rageaimbot[currentWeapon].rautoScope);
    ImGui::Checkbox("Auto Shot", &config->rageaimbot[currentWeapon].rautoShot);
    ImGui::Checkbox("Auto Stop", &config->rageaimbot[currentWeapon].rforceAccuracy);
    ImGui::Checkbox("Auto Duck", &config->rageaimbot[currentWeapon].rautoDuck);
    ImGui::SetNextItemWidth(100.f);

    for (size_t i = 0; i < ARRAYSIZE(hitbox); i++)
    {
        hitbox[i] = (config->rageaimbot[currentWeapon].rhitbox & 1 << i) == 1 << i;
    }
    if (ImGui::BeginCombo("Hitbox", previewvalue.c_str()))
    {
        previewvalue = "";
        for (size_t i = 0; i < ARRAYSIZE(hitboxes); i++)
        {
            ImGui::Selectable(hitboxes[i], &hitbox[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
        }
        ImGui::EndCombo();
    }
    for (size_t i = 0; i < ARRAYSIZE(hitboxes); i++)
    {
        if (!once)
        {
            previewvalue = "";
            once = true;
        }
        if (hitbox[i])
        {
            previewvalue += previewvalue.size() ? std::string(", ") + hitboxes[i] : hitboxes[i];
            config->rageaimbot[currentWeapon].rhitbox |= 1 << i;
        }
        else
        {
            config->rageaimbot[currentWeapon].rhitbox &= ~(1 << i);
        }
    }
    ImGui::SetNextItemWidth(100.f);
    ImGui::Combo("Priority", &config->rageaimbot[currentWeapon].rpriority, "Health\0Distance\0Fov\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(80.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->rageaimbot[idx ? idx : 35].renabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->rageaimbot[idx ? idx + 10 : 36].renabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->rageaimbot[idx ? idx + 16 : 37].renabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };
        ImGui::PushItemWidth(50.0f);
        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->rageaimbot[idx ? idx + 23 : 38].renabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("Fov", &config->rageaimbot[currentWeapon].rfov, 0.f, 360.f, "%.2f");
    ImGui::SliderInt("Hit chance", &config->rageaimbot[currentWeapon].rhitchance, 0, 100, "%d");
    ImGui::SliderInt("Multipoint", &config->rageaimbot[currentWeapon].rmultiPoint, 0, 100, "%d");
    ImGui::InputInt("Min damage", &config->rageaimbot[currentWeapon].rminDamage, 0 ,250);
    ImGui::InputInt("Min Damage Override", &config->rageaimbot[currentWeapon].roverridedMinDamage, 0, 250);
    ImGui::SameLine();
    ImGui::hotkey("", config->minDamageOverride);
    ImGui::SameLine();
    ImGui::PushID("mindmgOverrdie");
    ImGui::SetNextItemWidth(70.f);
    ImGui::Combo("", &config->minDamageMode, "Hold\0Toggle\0");
    ImGui::PopID();
    ImGui::Checkbox("Between shots", &config->rageaimbot[currentWeapon].rbetweenShots);ImGui::hotkey("Scan Override", config->visibleOnlyOverride);
    ImGui::hotkey("On Shot", config->forceOnShot);
    ImGui::hotkey("Force Baim", config->forceBaim);

    ImGui::PopID();
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Legit")){
            menuBarItem("Legitbot", window.legit);
            AntiAim::menuBarItemL();
            menuBarItem("Triggerbot", window.triggerbot);
            Backtrack::menuBarItem();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Rage")) {
            menuBarItem("Ragebot", window.rage);
            AntiAim::menuBarItem();
            FakeLag::menuBarItem();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Visuals")) {
            Visuals::menuBarItem();
            StreamProofESP::menuBarItem();
            menuBarItem("Chams", window.chams);
            Glow::menuBarItem();
            menuBarItem("GUI", window.style);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Miscellaneous")) {
            Misc::menuBarItem();
            Movement::menuBarItem();
            Sound::menuBarItem();
            Fun::menuBarItem();
            Extra::menuBarItem();
            Nade::menuBarItem();
            Discord::menuBarItem();
            Lobby::menuBarItem();
            Panorama::menuBarItem();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Changer")) {
            InventoryChanger::menuBarItem();
            Profile::menuBarItem();
            ImGui::EndMenu();
        }

        ImGui::Separator();
        menuBarItem("Config", window.config);
        ImGui::Separator();
        if (ImGui::MenuItem("Unhook")) {
#if !defined(_DEBUG)
            ImGui::OpenPopup("Unhook");
#else
            hooks->uninstall();
#endif

            if (ImGui::BeginPopupModal("Unhook", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("\nDo you want to unhook a cheat? (-ω-、)\n\n");

                if (ImGui::Button("Yes", ImVec2(120, 0))) { hooks->uninstall(); }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();

                if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
        }
        ImGui::EndMainMenuBar();   
    }
}

void GUI::renderTriggerbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.triggerbot)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Triggerbot", &window.triggerbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:
        currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->triggerbot[currentWeapon].enabled);
    ImGui::hotkey("Hold Key", config->triggerbotHoldKey);
    ImGui::Checkbox("Friendly fire", &config->triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->triggerbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Killshot", &config->triggerbot[currentWeapon].killshot);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->triggerbot[currentWeapon].minDamage);
    config->triggerbot[currentWeapon].minDamage = std::clamp(config->triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::SliderFloat("Burst Time", &config->triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderChamsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.chams)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Chams", &window.chams, windowFlags);
    }

    ImGui::hotkey("Toggle Key", config->chamsToggleKey, 80.0f);
    ImGui::hotkey("Hold Key", config->chamsHoldKey, 80.0f);
    ImGui::Separator();

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);

    static int material = 1;

    if (ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Desync\0Fake-Lag\0Auto-Peek\0Weapons\0Hands\0Backtrack\0Sleeves\0"))
        material = 1;

    ImGui::PopID();

    constexpr std::array categories{ "Allies", "Enemies", "Planting", "Defusing", "Local player", "Desync" , "Fake-Lag" , "Auto-Peek", "Weapons", "Hands", "Backtrack", "Sleeves" };

    ImGui::SameLine();

    auto& chams{ config->chams[categories[currentCategory]].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Text("Layer: %d", material);
    ImGui::SameLine();
    if (material <= 1)
        ImGuiCustom::arrowButtonDisabled("##left", ImGuiDir_Left);
    else if (ImGui::ArrowButton("##left", ImGuiDir_Left))
        --material;
    ImGui::SameLine();
    if (material >= int(config->chams[categories[currentCategory]].materials.size()))
        ImGuiCustom::arrowButtonDisabled("##right", ImGuiDir_Right);
    else if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        ++material;
    ImGui::SetNextItemWidth(120.f);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0Pearlescent\0Metallic\0Smoke Light\0Smoke Dark\0Water\0Bush\0MP3 Detail\0Snowflakes\0Custom\0");
    
    if (currentCategory == 9)
        ImGui::Combo("Weapon", &config->activeWeaponChams, "Glock-18\0P2000\0USP-S\0Dual Berettas\0P250\0Tec-9\0Five-Seven\0CZ-75\0Desert Eagle\0Revolver\0Nova\0XM1014\0Sawed-off\0MAG-7\0M249\0Negev\0Mac-10\0MP9\0MP7\0MP5-SD\0UMP-45\0P90\0PP-Bizon\0Galil AR\0Famas\0AK-47\0M4A4\0M4A1-S\0SSG-08\0SG-553\0AUG\0AWP\0G3SG1\0SCAR-20\0");
    else if (chams.material == 19)
        ImGui::Combo("Custom Slot", &config->customChams, "0\0001\0002\0003\0004\0005\0006\0007\0008\0009\0");


    if (chams.material == 19) {
        if (currentCategory == 9)
            ImGui::InputText("Custom", &chams.customWeapon[config->activeWeaponChams]);
        else
            ImGui::InputText("Custom", &chams.customMaterial[config->customChams]);

        ImGui::SameLine();
        if (ImGui::Button("Update")) {
            if (currentCategory == 9)
                nChams::initializeWeaponMaterials(config->activeWeaponChams, chams.customWeapon[config->activeWeaponChams]);
            else 
                nChams::initializeCustomMaterials(config->customChams, chams.customMaterial[config->customChams]);
        }
    }
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGui::Checkbox("Cover", &chams.cover);
    ImGui::Checkbox("Ignore-Z", &chams.ignorez);
    ImGuiCustom::colorPicker("Color", chams);

    if (!contentOnly) {
        ImGui::End();
    }
}

void GUI::renderStyleWindow(bool contentOnly) noexcept
{
    bool static openCustom = false;
    if (!contentOnly) {
        if (!window.style)
            return;
        ImGui::SetNextWindowSize({ 272.0f, 0.0f });
        ImGui::Begin("GUI", &window.style, windowFlags);
    }

    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu style", &config->style.menuStyle, "Azurre\0Osiris\0"))
        window = { };
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Azurre\0Genshi\0Emerald\0Bloddy Red\0Gold Mine\0Pandora\0Holy Light\0Deep Dark\0Visual Studio\0GoldSrc\0ImGui\0Neverlose\0Aimware\0Onetap\0Custom\0"))
        updateColors();
    

    if (config->style.menuColors == 14) {
        if (ImGui::Button("Customize")) {
            ImGui::OpenPopup("Customization Window");
        }
    }

    if (ImGui::BeginPopup("Customization Window")) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
        ImGui::EndPopup();
    }
 
  //ImGui::SliderFloat("##scale", &config->style.scale, 0.001f, 2.000f, "Global Scale: %2.f");
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Window Border", &config->style.windowBorder);
    ImGui::Checkbox("Frame Border", &config->style.frameBorder);
    ImGui::Checkbox("Rounded Corners", &config->style.roundedCorners);
    ImGui::Checkbox("ESC Key Closes Menu", &config->style.escCloseMenu);
    ImGui::NextColumn();
    ImGui::Checkbox("AntiAliasing", &config->style.antiAliasing);
    ImGui::Checkbox("Center Titles", &config->style.centerTitle);
    ImGui::Checkbox("Block Input", &config->style.blockInput);
    ImGui::Checkbox("Banner", &config->style.banner);
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::PushID("colorpicker hoer");
    ImGuiCustom::colorPicker("Enabled", config->style.bgEffect.lineCol);
    ImGui::SliderInt("##amount", &config->style.bgEffect.N, 10, 1000, "Amount: %i");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Higher numbers may lags in menu!");
    ImGui::PopID();
    ImGui::SliderInt("##distance", &config->style.bgEffect.lineMaxDist, 10, 2000, "Distance: %i");
    ImGui::NextColumn();
    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("##thick", &config->style.bgEffect.lineThickness, .1f, 5.f, "Thickness: %.1f");
    ImGui::SliderFloat("##dar", &config->style.bgEffect.darkness, 0.01f, 1.f, "Darkness: %.3f");
    ImGui::PopItemWidth();
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderConfigWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.config)
            return;
        ImGui::SetNextWindowSize({ 324.0f, 0.0f });
        ImGui::SetWindowPos({ (ImGui::GetIO().DisplaySize.x * 0.75f ) , (ImGui::GetIO().DisplaySize.y * 0.75f) });
        if (!ImGui::Begin("Config", &window.config, windowFlags)) {
            ImGui::End();
            return;
        }
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 180.0f);

    static bool incrementalLoad = false;
    ImGui::Checkbox("Incremental Load", &incrementalLoad);

    ImGui::PushItemWidth(170.0f);

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;

    static std::u8string buffer;

    timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    if (timeToNextConfigRefresh <= 0.0f) {
        config->listConfigs();
        if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
            currentConfig = std::distance(configItems.begin(), it);
        timeToNextConfigRefresh = 0.1f;
    }

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::u8string>*>(data);
        *out_text = (const char*)vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
            buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer);
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Open config directory", { 128.0f, 25.0f }))
            config->openConfigDir();

        if (ImGui::Button("Create config", { 128.0f, 25.0f })) {
            config->add(buffer.c_str());
            Helpers::CheatChatNotification(false, "Config has been Created");
        }

        if (ImGui::Button("Reset config", { 128.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]{ "Whole", "Legit Aimbot", "Rage Aimbot", "Triggerbot", "Backtrack", "Anti aim", "Fake-Lag", "Extra", "Glow", "Chams", "ESP", "Visuals", "Inventory Changer", "Profile Changer", "Sound", "GUI", "Misc", "Movement", "Movement Recorder", "Grief" ,"Discord RP", "Grenade Helper", "Log", "Clan Tag"};
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1) ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                    case 0: config->reset(); updateColors(); Clan::update(true); inventory_changer::InventoryChanger::instance().scheduleHudUpdate(); break;
                    case 1: config->legitaimbot = { }; break;
                    case 2: config->rageaimbot = { }; break;
                    case 3: config->triggerbot = { }; break;
                    case 4: Backtrack::resetConfig(); break;
                    case 5: AntiAim::resetConfig(); break;
                    case 6: FakeLag::resetConfig(); break;
                    case 7: Extra::resetConfig(); break;
                    case 8: Glow::resetConfig(); break;
                    case 9: config->chams = { }; break;
                    case 10: config->streamProofESP = { }; break;
                    case 11: Visuals::resetConfig(); break;
                    case 12: inventory_changer::InventoryChanger::instance().reset(); inventory_changer::InventoryChanger::instance().scheduleHudUpdate(); break;
                    case 13: config->profilechanger = { }; inventory_changer::InventoryChanger::instance().scheduleHudUpdate(); break;
                    case 14: Sound::resetConfig(); break;
                    case 15: config->style = { }; updateColors(); break;
                    case 16: Misc::resetConfig(); Clan::update(true); break;
                    case 17: Movement::resetConfig(); break;
                    case 18: break; //MoveRecorder::resetConfig(); break;
                    case 19: Fun::resetConfig(); break;
                    case 20: Discord::resetConfig(); break;
                    case 21: Nade::resetConfig(); break;
                    case 22: Log::resetConfig(); break;
                    case 23: Clan::resetConfig(); break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 128.0f, 25.0f })) {                
                config->load(currentConfig, incrementalLoad, false);
                updateColors();
                inventory_changer::InventoryChanger::instance().scheduleHudUpdate();
                Clan::update(true);
                Helpers::CheatChatNotification(false, "Config has been Loaded");
            }
            if (ImGui::Button("Save selected", { 128.0f, 25.0f })) {
                config->save(currentConfig, false);
                Helpers::CheatChatNotification(false, "Config has been Saved");
            }

            if (ImGui::Button("Delete selected", { 128.0f, 25.0f })) {
                config->remove(currentConfig);

                if (static_cast<std::size_t>(currentConfig) < configItems.size())
                    buffer = configItems[currentConfig];
                else
                    buffer.clear();
                Helpers::CheatChatNotification(false, "Config has been Deleted");
            }
        }
        ImGui::Columns(1);
        if (ImGui::Button("Import from Clipboard", { 150.0f, 25.0f })) { 
            config->load(currentConfig, incrementalLoad, true);
            updateColors();
            inventory_changer::InventoryChanger::instance().scheduleHudUpdate();
            Clan::update(true);
            Helpers::CheatChatNotification(false, "Loaded Config from Clipboard");
        }
        ImGui::SameLine();
        if (ImGui::Button("Export to Clipboard", { 150.0f, 25.0f })) {
            config->save(currentConfig, true);
            Helpers::CheatChatNotification(false, "Saved Config from Clipboard");
        }
        if (!contentOnly)
            ImGui::End();
}

void GUI::renderGuiStyle2() noexcept
{
    static int listhor = 0;
    static int listvert = 0;

    const auto time = std::time(nullptr);
    const auto localTime = std::localtime(&time);
    char s[11];
    s[0] = '\0';
    snprintf(s, sizeof(s), "%02d:%02d:%02d", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

    ImGui::SetNextWindowSize({ 800.0f, 600.0f });
    ImGui::Begin("Azurre Window", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushID(2137);
    ImGui::PushFont(fonts.tahoma34); ImGui::TextColored(Extra::AntiUntrusted() ? ImVec4{1.f, 1.f, 1.f, 1.f} : ImVec4{1.f, 0.f, 0.f, 1.f}, cheat_name); ImGui::PopFont(); ImGui::SameLine();
    if(ImGui::IsItemHovered())
        ImGui::SetTooltip("Much Love to Daniel from xs9");
    ImGui::PopID();
    ImGui::Dummy({ 34.f, 0.f }); ImGui::SameLine();
    if (ImGui::Button("Legit", ImVec2(96.f, 32.f))) { listhor = SECTION::LEGIT; listvert = WINDOW::LEGITBOT; }
    ImGui::SameLine();

    if (ImGui::Button("Rage", ImVec2(96.f, 32.f))) { listhor = SECTION::RAGE; listvert = WINDOW::RAGEBOT; }
    ImGui::SameLine();

    if (ImGui::Button("Visuals", ImVec2(96.f, 32.f))) { listhor = SECTION::VISUALS; listvert = WINDOW::VISUALS_MAIN; }
    ImGui::SameLine();

    if (ImGui::Button("Misc", ImVec2(96.f, 32.f))) { listhor = SECTION::MISC; listvert = WINDOW::MISC_MAIN; }
    ImGui::SameLine();

    if (ImGui::Button("Changer", ImVec2(96.f, 32.f))) { listhor = SECTION::CHANGER; listvert = WINDOW::INVENTORY; }

    ImGui::SameLine();
    ImGui::Dummy({ 4.f, 0.f }); ImGui::SameLine();
    ImGui::PushFont(fonts.tahoma34); ImGui::Text(s); ImGui::PopFont();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 112.0f);
    switch (listhor) {
    default:
    case 0: // legit
        if (ImGui::Button("LegitBot", ImVec2(96.f, 32.f))) listvert = WINDOW::LEGITBOT;
        if (ImGui::Button("Legit AntiAim", ImVec2(96.f, 32.f))) listvert = WINDOW::LEGIT_ANTIAIM;
        if (ImGui::Button("Triggerbot", ImVec2(96.f, 32.f))) listvert = WINDOW::TRIGGERBOT;
        if (ImGui::Button("Backtrack", ImVec2(96.f, 32.f))) listvert = WINDOW::BACKTRACK;
        break;

    case 1: // rage
        if (ImGui::Button("RageBot", ImVec2(96.f, 32.f))) listvert = WINDOW::RAGEBOT;
        if (ImGui::Button("AntiAim", ImVec2(96.f, 32.f))) listvert = WINDOW::ANTIAIM;
        if (ImGui::Button("Fake-Lag", ImVec2(96.f, 32.f))) listvert = WINDOW::FAKE_LAG;
        break;

    case 2: //visuals
        if (ImGui::Button("Main", ImVec2(96.f, 32.f))) listvert = WINDOW::VISUALS_MAIN;
        if (ImGui::Button("Chams", ImVec2(96.f, 32.f))) listvert = WINDOW::CHAMS;
        if (ImGui::Button("ESP", ImVec2(96.f, 32.f))) listvert = WINDOW::ESP;
        if (ImGui::Button("Glow", ImVec2(96.f, 32.f))) listvert = WINDOW::GLOW;
        if (ImGui::Button("GUI", ImVec2(96.f, 32.f))) listvert = WINDOW::GUINTERFACE;
        break;

    case 3: // misc
        if (ImGui::Button("Main", ImVec2(96.f, 32.f))) listvert = WINDOW::MISC_MAIN;
        if (ImGui::Button("Movement", ImVec2(96.f, 32.f))) listvert = WINDOW::MOVEMENT;
        if (ImGui::Button("Movement Rec", ImVec2(96.f, 32.f))) listvert = WINDOW::MOVEMENT_RECORDER;
        if (ImGui::Button("Sound", ImVec2(96.f, 32.f))) listvert = WINDOW::SOUND;
        if (ImGui::Button("Grief", ImVec2(96.f, 32.f))) listvert = WINDOW::GRIEF;
        if (ImGui::Button("Extra", ImVec2(96.f, 32.f))) listvert = WINDOW::EXTRA;
        if (ImGui::Button("Grenade Helper", ImVec2(96.f, 32.f))) listvert = WINDOW::GRENADE_HELPER;
        if (ImGui::Button("Discord RP", ImVec2(96.f, 32.f))) listvert = WINDOW::DISCORD_RP;
        if (ImGui::Button("Walkbot", ImVec2(96.f, 32.f))) listvert = WINDOW::WALKBOT;
        if (ImGui::Button("Lobby", ImVec2(96.f, 32.f))) listvert = WINDOW::LOBBY;
        if (ImGui::Button("Panorama", ImVec2(96.f, 32.f))) listvert = WINDOW::PANORMAMA;
        break;

    case 4: //inv prof changer
        if (ImGui::Button("Inventory", ImVec2(96.f, 32.f))) listvert = WINDOW::INVENTORY;
        if (ImGui::Button("Profile", ImVec2(96.f, 32.f))) listvert = WINDOW::PROFILE;
        break;
    }
    
    ImGui::SetCursorPos({ 8.f, 486.f });
    ImGui::PushID("Admin button");
    ImGui::PushStyleColor(ImGuiCol_Border, ImGuiCol_WindowBg);
    ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_WindowBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiCol_WindowBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_WindowBg);
    if (ImGui::Button("", ImVec2(96.f, 32.f))) {
        listvert = WINDOW::DEBUG;
    }
    ImGui::PopStyleColor(4);
    ImGui::PopID();
    if (ImGui::Button("Config", ImVec2(96.f, 32.f))) {
        ImGui::OpenPopup("configwindow");
    }

    if (ImGui::BeginPopup("configwindow", ImGuiWindowFlags_NoTitleBar))
    {
        renderConfigWindow(true);
        ImGui::EndPopup();

    }

    if (ImGui::Button("Unhook", ImVec2(96.f, 32.f)))
#if !defined(_DEBUG)
        ImGui::OpenPopup("Unhook?");
#else
        hooks->uninstall();
#endif

    if (ImGui::BeginPopupModal("Unhook?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("\nDo you want to unhook a cheat? (-ω-、)\n\n");

        if (ImGui::Button("Yes", ImVec2(120, 0))) { hooks->uninstall(); }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    ImGui::NextColumn();
    ImGui::BeginChild("column first");
    switch (listvert) {
        default:
        case WINDOW::LEGITBOT:
            renderLegitBotGui(true); break;
        case WINDOW::LEGIT_ANTIAIM:
            AntiAim::drawLegitGUI(true); break;
        case WINDOW::TRIGGERBOT:
            renderTriggerbotWindow(true); break;
        case WINDOW::BACKTRACK:
            Backtrack::drawGUI(true); break;
        case WINDOW::RAGEBOT:
            renderRageBotGui(true); break;
        case WINDOW::ANTIAIM:
            AntiAim::drawGUI(true); break;
        case WINDOW::FAKE_LAG:
            FakeLag::drawGUI(true); break;
        case WINDOW::VISUALS_MAIN:
            Visuals::drawGUI(true); break;
        case WINDOW::CHAMS:
            renderChamsWindow(true); break;
        case WINDOW::ESP:
            StreamProofESP::drawGUI(true); break;
        case WINDOW::GLOW:
            Glow::drawGUI(true); break;
        case WINDOW::GUINTERFACE:
            renderStyleWindow(true); break;
        case WINDOW::MISC_MAIN:
            Misc::drawGUI(true); break;
        case WINDOW::MOVEMENT:
            Movement::drawGUI(true); break;
        case WINDOW::MOVEMENT_RECORDER:
            MovementRecorder::drawGUI(true); break;
        case WINDOW::SOUND:
            Sound::drawGUI(true); break;
        case WINDOW::GRIEF:
            Fun::drawGUI(true); break;
        case WINDOW::EXTRA:
            Extra::drawGUI(true); break;
        case WINDOW::GRENADE_HELPER:
            Nade::drawGUI(true); break;
        case WINDOW::DISCORD_RP:
            Discord::drawGUI(true); break;
        case WINDOW::WALKBOT:
            Walkbot::drawGUI(true); break;
        case WINDOW::PANORMAMA:
            Panorama::drawGUI(true); break;
        case WINDOW::LOBBY:
            Lobby::drawGUI(true); break;
        case WINDOW::INVENTORY:
            inventory_changer::InventoryChanger::instance().drawGUI(true); break;
        case WINDOW::PROFILE:
            Profile::drawGUI(true); break;
        case WINDOW::DEBUG:
            Xsin::drawGUI(true); break;
    }
    ImGui::EndChild();
    ImGui::Columns(1);
    ImGui::End();
}
