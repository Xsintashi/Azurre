#pragma once
struct UserCmd;
struct ImDrawList;

#include "../JsonForward.h"

namespace MovementRecorder
{
	void record(UserCmd*) noexcept;
	void play(UserCmd*) noexcept;
	void drawStartpoints(ImDrawList*) noexcept;
	bool isPlayingback() noexcept;
	bool shouldLockViewAngles() noexcept;
	void drawGUI(bool contentOnly) noexcept;
	json toJson() noexcept;
	void fromJson(const json&) noexcept;
	void resetConfig() noexcept;
}