#pragma once

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../Memory.h"
#include "../ConfigStructs.h"
#include "../SDK/GlobalVars.h"

struct ImDrawList;
namespace Notifications {
    void draw(ImDrawList* drawList);
    void addNotification(Color4 color, const char* fmt, ...);
}