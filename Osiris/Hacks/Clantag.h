#pragma once

#include "../JsonForward.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ConfigStructs.h"

namespace Clan {
	void update(bool reset = false, bool update = false) noexcept;
	void drawGUI();


	json toJson() noexcept;
	void fromJson(const json& j) noexcept;
	void resetConfig() noexcept;
};

namespace ClanTagStealer {
	void update() noexcept;
};