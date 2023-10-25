#pragma once

#include "imgui/imgui.h"

struct Color4;
struct ColorToggle;
struct ColorToggle3;
struct ColorToggleRounding;
struct ColorToggleThickness;
struct ColorToggleThicknessRounding;
class KeyBind;

namespace ImGuiCustom
{
    void drawTriangleFromCenter(ImDrawList* drawList, const ImVec2& pos, unsigned color = 0x88FFFFFF, bool outline = true) noexcept;
    void HotKeyMode(const char* label, KeyBind& key, float samelineOffset = 0.0f, const ImVec2& size = { 100.0f, 0.0f }) noexcept;
    void boolCombo(const char* name, bool& value, const char* items) noexcept;
    void colorPicker(const char* name, float color[3], float* alpha = nullptr, bool* rainbow = nullptr, float* rainbowSpeed = nullptr, bool* enable = nullptr, float* thickness = nullptr, float* rounding = nullptr) noexcept;
    void circularProgressBar(ImDrawList* drawList, ImVec2 pos, float thick, float rotate, float value, ImU32 col, bool inverse = false);
    void colorPicker(const char* name, ColorToggle3& colorConfig) noexcept;
    void colorPicker(const char* name, Color4& colorConfig, bool* enable = nullptr, float* thickness = nullptr) noexcept;
    void colorPicker(const char* name, ColorToggle& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleRounding& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleThickness& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept;
    void arrowButtonDisabled(const char* id, ImGuiDir dir) noexcept;


    template <typename F>
    void multiCombo(const char* name, F& flagValue, const char* items) noexcept
    {
        constexpr auto singleStringGetter = [](void* data, int idx, const char** outText) noexcept
        {
            const char* itemsSeparatedByZeros = (const char*)data;
            int itemsCount = 0;
            const char* p = itemsSeparatedByZeros;
            while (*p)
            {
                if (idx == itemsCount)
                    break;
                p += std::strlen(p) + 1;
                itemsCount++;
            }
            if (!*p)
                return false;
            if (outText)
                *outText = p;
            return true;
        };

        int count = 0;
        const char* p = items;
        while (*p)
        {
            p += std::strlen(p) + 1;
            count++;
        }

        const char* preview = "...";
        if (flagValue == (1 << count) - 1)
            preview = "All";
        else if (!flagValue)
            preview = "None";

        void* data = (void*)items;

        if (ImGui::BeginCombo(name, preview))
        {
            for (int i = 0; i < count; i++)
            {
                bool selected = flagValue & (1 << i);

                const char* item;
                singleStringGetter(data, i, &item);

                ImGui::PushID(i);
                ImGui::Selectable(item, &selected, ImGuiSelectableFlags_DontClosePopups);
                ImGui::PopID();

                if (selected)
                    flagValue |= (1 << i);
                else
                    flagValue &= ~(1 << i);
            }
            ImGui::EndCombo();
        }
    }
}

namespace ImGui
{
    void progressBarFullWidth(float fraction, float height, bool drawText) noexcept;
    void textUnformattedCentered(const char* text) noexcept;
    void hotkey(const char* label, KeyBind& key, float samelineOffset = 0.0f, const ImVec2& size = { 100.0f, 0.0f }) noexcept;

    bool Toggle(const char* label, bool* v);
}
