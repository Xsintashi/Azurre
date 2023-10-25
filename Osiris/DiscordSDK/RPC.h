#pragma once
#include "include/discord_register.h"
#include "include/discord_rpc.h"
#include "../JsonForward.h"
#include "../imgui/imgui.h"
#include "../imguiCustom.h"

#include <string>
#include <array>
#include <charconv>
#include <iostream>
#include <string_view>
#include <system_error>

#include <Hacks/AntiAim.h>

#include "../SDK/Cvar.h"
#include "../SDK/ConVar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/PlayerResource.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../GameData.h"
#include "../Config.h"


namespace Discord {

	void Run();
	void Update();
	void customRichPresenseTest(const char* state, const char* details, int64_t endTimestamp, const char* smallImageKey, const char* largeImageKey, const char* largeImageText);
	void Shutdown();
	void menuBarItem() noexcept;
	void drawGUI(bool contentOnly);

	// Config
	json toJson() noexcept;
	void fromJson(const json& j) noexcept;
	void resetConfig() noexcept;
}