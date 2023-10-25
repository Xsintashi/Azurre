#pragma once
#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../ProtobufReader.h"

#include "../GUI.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"

#include "../imguiCustom.h"
#include <Config.h>

struct Debug {
	bool enable = false;

	Vector serverAngles = {0.f, 0.f, 0.f}; //What clients see
	Vector fakeAngles = { 0.f, 0.f, 0.f }; //What server seees
	Vector engineAngles = { 0.f, 0.f, 0.f }; //What you see

	Entity* targetedEntity;

	int minDamage;
	float damage;

	struct CustomKnifeColors {
		bool enabled = false;
		Color3 color1{1.f, 1.f, 1.f};
		Color3 color2{1.f, 1.f, 1.f};
		Color3 color3{1.f, 1.f, 1.f};
		Color3 color4{1.f, 1.f, 1.f};

	}customKnife;

};

inline Debug debug;

namespace Xsin {
	bool ImGuiDemo() noexcept;
	void debugBox(ImDrawList* drawList) noexcept;
	void backgroundDebugDraw(ImDrawList* dw);
	std::string getKnifeModel() noexcept;
	bool getDebug1();
	void rainbowColor() noexcept;
	bool SentPacket(bool& sendPacket) noexcept;
	void printDebug(const char* x);
	void javaScriptConsole();
	void renderServerHitboxes() noexcept;
	void drawDebugWindow();
	void drawGUI(bool contentOnly) noexcept;

}