#pragma once

namespace Radar
{
	inline bool should_update_radar = false;
	bool OnMapLoad(const char* pszMapName) noexcept;

	void render() noexcept;
}