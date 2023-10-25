#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "Config.h"
#include "imguiCustom.h"
#include "ConfigStructs.h"
#include "InputUtil.h"
#include <numbers>

void ImGuiCustom::drawTriangleFromCenter(ImDrawList* drawList, const ImVec2& pos, unsigned color, bool outline) noexcept
{
    const auto l = std::sqrtf(ImLengthSqr(pos));
    if (!l) return;
    const auto posNormalized = pos / l;
    const auto center = ImGui::GetIO().DisplaySize / 2 + pos;

    const ImVec2 trianglePoints[] = {
        center + ImVec2{-0.4f * posNormalized.y, 0.4f * posNormalized.x} *30,
        center + ImVec2{0.4f * posNormalized.y, -0.4f * posNormalized.x} *30,
        center + ImVec2{1.0f * posNormalized.x, 1.0f * posNormalized.y} *30
    };

    drawList->AddConvexPolyFilled(trianglePoints, 3, color);
    if (outline)
        drawList->AddPolyline(trianglePoints, 3, color | IM_COL32_A_MASK, ImDrawFlags_Closed, 1.5f);
}

void ImGuiCustom::HotKeyMode(const char* label, KeyBind& key, float samelineOffset, const ImVec2& size) noexcept
{
    const auto id = ImGui::GetID(label);

    ImGui::PushID(label);

    if (ImGui::GetActiveID() == id)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        ImGui::Button("Press a key...");
        ImGui::PopStyleColor();
        ImGui::GetCurrentContext()->ActiveIdAllowOverlap = true;

        if ((!ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[0]) || key.setToPressedKey())
            ImGui::ClearActiveID();
    }
    else
    {
        if (key.keyMode == KeyMode::Always)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::ButtonEx("On", {}, ImGuiButtonFlags_Disabled);
            ImGui::PopStyleColor();
        }
        else if (key.keyMode == KeyMode::Off)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
            ImGui::ButtonEx("Off", {}, ImGuiButtonFlags_Disabled);
            ImGui::PopStyleColor();
        }
        else if (key.isSet())
        {
            if (ImGui::Button(key.toString(), size))
                ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
        }
        else
        {
            if (ImGui::Button("Bind"))
                ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
        }

        if (ImGui::BeginPopup("##mode", ImGuiWindowFlags_AlwaysUseWindowPadding))
        {
            bool selected = key.keyMode == KeyMode::Off;
            ImGui::Selectable("Off", &selected);
            if (selected)
                key.keyMode = KeyMode::Off;
            
            selected = key.keyMode == KeyMode::Always;
            ImGui::Selectable("Always", &selected);
            if (selected)
                key.keyMode = KeyMode::Always;

            selected = key.keyMode == KeyMode::Hold;
            ImGui::Selectable("Hold", &selected);
            if (selected)
                key.keyMode = KeyMode::Hold;

            selected = key.keyMode == KeyMode::Toggle;
            ImGui::Selectable("Toggle", &selected);
            if (selected)
                key.keyMode = KeyMode::Toggle;

            ImGui::EndPopup();
        }
        else if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Right click for options");

            if (ImGui::GetIO().MouseClicked[1])
                ImGui::OpenPopup("##mode");
        }
    }

    ImGui::SameLine();

    ImGui::AlignTextToFramePadding();
    if (std::strncmp(label, "##", 2))
        ImGui::TextUnformatted(label, std::strstr(label, "##"));
    ImGui::PopID();
}

void ImGuiCustom::boolCombo(const char* name, bool& value, const char* items) noexcept
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

    void* data = (void*)items;

    const char* preview;
    singleStringGetter(data, value, &preview);
    if (ImGui::BeginCombo(name, preview))
    {
        for (int i = 0; i < std::min(count, 2); i++)
        {
            bool selected = i == (int)value;

            const char* item;
            singleStringGetter(data, i, &item);

            ImGui::PushID(i);
            ImGui::Selectable(item, &selected);
            ImGui::PopID();

            if (selected) value = i;
        }
        ImGui::EndCombo();
    }
}

void ImGuiCustom::colorPicker(const char* name, float color[3], float* alpha, bool* rainbow, float* rainbowSpeed, bool* enable, float* thickness, float* rounding) noexcept
{
    ImGui::PushID(name);
    if (enable) {
        ImGui::Checkbox("##check", enable);
        ImGui::SameLine(0.0f, 5.0f);
    }
    bool openPopup = ImGui::ColorButton("##btn", ImVec4{ color[0], color[1], color[2], alpha ? *alpha : 1.0f }, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview);
    if (ImGui::BeginDragDropTarget()) {
        if (alpha) {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F)) {
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
                *alpha = 1.0f;
            }
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 4, color);
        } else {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine(0.0f, 5.0f);
    ImGui::TextUnformatted(name);

    if (openPopup)
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        if (alpha) {
            float col[]{ color[0], color[1], color[2], *alpha }; 
            ImGui::ColorPicker4("##picker", col, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar);
            color[0] = col[0];
            color[1] = col[1];
            color[2] = col[2];
            *alpha = col[3];
        } else {
            ImGui::ColorPicker3("##picker", color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview);
        }

        if (rainbow || rainbowSpeed || thickness || rounding) {
            ImGui::SameLine();
            if (ImGui::BeginChild("##child", { 150.0f, 0.0f })) {
                if (rainbow)
                    ImGui::Toggle("Rainbow", rainbow);
                ImGui::PushItemWidth(85.0f);
                if (rainbowSpeed)
                    ImGui::InputFloat("Speed", rainbowSpeed, 0.01f, 0.15f, "%.2f");

                if (rounding || thickness)
                    ImGui::Separator();

                if (rounding) {
                    ImGui::InputFloat("Rounding", rounding, 0.1f, 0.0f, "%.1f");
                    *rounding = (std::max)(*rounding, 0.0f);
                }

                if (thickness) {
                    ImGui::InputFloat("Thickness", thickness, 0.1f, 0.0f, "%.1f");
                    *thickness = (std::max)(*thickness, 1.0f);
                }

                ImGui::PopItemWidth();
            }
            ImGui::EndChild();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void ImGuiCustom::circularProgressBar(ImDrawList* drawList, ImVec2 pos, float thick, float rotate, float value, ImU32 col, bool inverse) {

    constexpr float pi = std::numbers::pi_v<float>;

    for (float i = rotate; i < rotate + value; i++) {
        auto rad = i * pi / 180;

        if (!inverse)
            drawList->AddLine({ pos.x + cos(rad) * thick, pos.y + sin(rad) * thick }, { pos.x + cos(rad) * thick / 2, pos.y + sin(rad) * thick / 2 }, col);
        else
            drawList->AddLine({ pos.x - sin(rad) * thick, pos.y - cos(rad) * thick }, { pos.x - sin(rad) * thick / 2, pos.y - cos(rad) * thick / 2 }, col);
    }

}

void ImGuiCustom::colorPicker(const char* name, ColorToggle3& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor3().color.data(), nullptr, &colorConfig.asColor3().rainbow, &colorConfig.asColor3().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPicker(const char* name, Color4& colorConfig, bool* enable, float* thickness) noexcept
{
    colorPicker(name, colorConfig.color.data(), &colorConfig.color[3], &colorConfig.rainbow, &colorConfig.rainbowSpeed, enable, thickness);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggle& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleRounding& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, nullptr, &colorConfig.rounding);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleThickness& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColorToggle().asColor4().color.data(), &colorConfig.asColorToggle().asColor4().color[3], &colorConfig.asColorToggle().asColor4().rainbow, &colorConfig.asColorToggle().asColor4().rainbowSpeed, &colorConfig.asColorToggle().enabled, &colorConfig.thickness);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, &colorConfig.thickness, &colorConfig.rounding);
}

void ImGuiCustom::arrowButtonDisabled(const char* id, ImGuiDir dir) noexcept
{
    float sz = ImGui::GetFrameHeight();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::ArrowButtonEx(id, dir, ImVec2{ sz, sz }, ImGuiButtonFlags_Disabled);
    ImGui::PopStyleVar();
}

void ImGui::progressBarFullWidth(float fraction, float height, bool drawText) noexcept
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2{ -1, 0 }, CalcItemWidth(), height + style.FramePadding.y * 2.0f);
    ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    fraction = ImSaturate(fraction);
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));

    if (fraction == 0.0f)
        return;

    const ImVec2 p0{ ImLerp(bb.Min.x, bb.Max.x, 0.0f), bb.Min.y };
    const ImVec2 p1{ ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y };

    const float x0 = ImMax(p0.x, bb.Min.x);
    const float x1 = ImMin(p1.x, bb.Max.x);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << (fraction * 100.f) << "%";

    window->DrawList->PathLineTo({ x0, p1.y });
    window->DrawList->PathLineTo({ x0, p0.y });
    window->DrawList->PathLineTo({ x1, p0.y });
    window->DrawList->PathLineTo({ x1, p1.y });
    window->DrawList->PathFillConvex(GetColorU32(ImGuiCol_PlotHistogram));

    if(drawText)
        window->DrawList->AddText({ x1 , p0.y - height }, GetColorU32(ImGuiCol_Text), ss.str().c_str());
}

void ImGui::textUnformattedCentered(const char* text) noexcept
{
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) / 2.0f);
    ImGui::TextUnformatted(text);
}

void ImGui::hotkey(const char* label, KeyBind& key, float samelineOffset, const ImVec2& size) noexcept
{
    const auto id = GetID(label);
    PushID(label);

    TextUnformatted(label);
    SameLine(samelineOffset);

    if (GetActiveID() == id) {
        PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
        Button("Press a key...", size);
        PopStyleColor();

        GetCurrentContext()->ActiveIdAllowOverlap = true;
        if ((!IsItemHovered() && GetIO().MouseClicked[0]) || key.setToPressedKey())
            ClearActiveID();
    } else if (Button(key.toString(), size)) {
        SetActiveID(id, GetCurrentWindow());
    }

    PopID();
}

bool ImGui::Toggle(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float height = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;

    float width = height * 2.f;
    float radius = height * 0.50f;

    const ImRect total_bb(pos, pos + ImVec2(width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    float last_active_id_timer = g.LastActiveIdTimer;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
        g.LastActiveIdTimer = 0.f;
    }

    if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed)
        g.LastActiveIdTimer = last_active_id_timer;

    float t = *v ? 1.0f : 0.0f;

    if (g.LastActiveId == id)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / 0.16f);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg = GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    const ImRect frame_bb(pos, pos + ImVec2(width, height));

    RenderFrame(frame_bb.Min, frame_bb.Max, col_bg, true, config->style.roundedCorners ? (height * 0.5f) : 0);
    RenderNavHighlight(total_bb, id);

    ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y);
    RenderText(label_pos, label);
    //if(config->style.roundedCorners)
    window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius), radius - 1.5f, ImGui::GetColorU32(ImGuiCol_CheckMark), 36);
    //else
    //window->DrawList->AddRectFilled({ pos.x + 20 , pos.y + 2}, { pos.x + 40, pos.y + 18 },  ImGui::GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
    return pressed;
}
