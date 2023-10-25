#pragma once
#include <fstream>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../SDK/Entity.h"
#include "../Interfaces.h"
#include "../SDK/Engine.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/EntityList.h"
#include "../Helpers.h"

#include "Radar.h"

#include <memory>
//#define STBI_NO_FAILURE_STRINGS
/*
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
#endif
*/
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

bool m_bLoaded = false;
//IDirect3DTexture9* m_pMapTexture = nullptr;
//LPDIRECT3DTEXTURE9 m_pMapTexture = nullptr;
//IDirect3DTexture9 * m_pMapTexture;
ImTextureID m_pMapTexture;

Vector m_vMapOrigin;
float m_flMapScale;
Vector bomb_siteA;
Vector bomb_siteB;

int m_iTexture;

std::string parseString(const std::string& szBefore, const std::string& szSource) noexcept
{
	if (!szBefore.empty() && !szSource.empty() && (szSource.find(szBefore) != std::string::npos))
	{
		std::string t = strstr(szSource.c_str(), szBefore.c_str()); //-V522
		t.erase(0, szBefore.length());
		size_t firstLoc = t.find('\"', 0);
		size_t secondLoc = t.find('\"', firstLoc + 1);
		t = t.substr(firstLoc + 1, secondLoc - 3);
		return t;
	}
	else
		return "";
}


bool Radar::OnMapLoad(const char* pszMapName) noexcept
{

	m_bLoaded = false;

	if (strlen(pszMapName) == 0) return false;

	static const char* mapName;
	mapName = std::string("csgo\\resource\\overviews\\").append(pszMapName).append("_radar.dds").c_str();

	if (mapName == "\0") return false;

	std::ifstream file;
	file.open(mapName);

	if (!file.is_open())
		return false;

	file.close();
	//D3DXCreateTextureFromFileA(pDevice, szPath, &m_pMapTexture);
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load("csgo\\resource\\overviews\\de_mirage_radar.dds", &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;
	m_pMapTexture = ImGui_CreateTextureRGBA(image_width, image_height, image_data);
	
	std::string mapNameFata;
	mapNameFata = std::string("csgo\\resource\\overviews\\%s.txt", pszMapName);
	std::ifstream ifs(mapNameFata.c_str());

	if (ifs.bad())
		return false;

	std::string szInfo((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	if (szInfo.empty())
		return false;

	m_vMapOrigin.x = std::stof(parseString(("\"pos_x\""), szInfo));
	m_vMapOrigin.y = std::stof(parseString(("\"pos_y\""), szInfo));
	m_flMapScale = std::stof(parseString(("\"scale\""), szInfo));

	m_bLoaded = true;
	Radar::should_update_radar = false;
	return true;
}

void Radar::render() noexcept
{
	// check if texture is loaded and ingame bools
	bool trueRadar = interfaces->engine->isInGame() && interfaces->engine->isConnected();
	if (!trueRadar) {
		Radar::should_update_radar = true;
		return;
	}
		

	static auto open = true;
	static auto set_once = false;
	if (!set_once)
	{
		ImGui::SetNextWindowSize({ 256.f, 256.f }, ImGuiCond_Once);
		set_once = true;
	}

	ImGui::Begin(("##radar"), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::GetWindowSize().x != ImGui::GetWindowSize().y) //-V550
		ImGui::SetWindowSize(ImGui::GetWindowSize().x > ImGui::GetWindowSize().y ? ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().x) : ImVec2(ImGui::GetWindowSize().y, ImGui::GetWindowSize().y));


	// Vars
	float x = ImGui::GetWindowPos().x;
	float y = ImGui::GetWindowPos().y;
	float size = ImGui::GetWindowSize().x;
	float center_size = (size / 2);



	// Scale map
	float new_size = size;
	float size_diff = new_size - size;
	size = new_size;
	x -= size_diff / 2;
	y -= size_diff / 2;


	ImGui::Image(m_pMapTexture, ImVec2(x + size, y + size));
	// Prepare imgui shit

	ImVec2 uv_a(0, 0), uv_c(1, 1), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
	ImVec2 a(x, y), c(x + size, y + size), b(c.x, a.y), d(a.x, c.y);



	// Limit radar bounds
	ImGui::GetWindowDrawList()->PushClipRect(ImVec2(x, y), ImVec2(x + size, y + size), false);
	//Render background first
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), ImColor(0, 0, 0));

	// Render
	ImGui::GetWindowDrawList()->PushTextureID(m_pMapTexture);
	ImGui::GetWindowDrawList()->PrimReserve(6, 4);
	ImGui::GetWindowDrawList()->PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, 0xFFFFFFFF);
	ImGui::GetWindowDrawList()->PopTextureID();
	// Remove limit
	ImGui::GetWindowDrawList()->PopClipRect();
	


	if (!localPlayer) {
		Radar::should_update_radar = true;
		ImGui::End();
	}

	if (localPlayer->isAlive())
	{
		Vector world_pos = localPlayer->getAbsOrigin();
		ImVec2 map_pos;
		Helpers::worldToScreen(world_pos, map_pos);

		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(map_pos.x, map_pos.y), 5, ImColor(30, 255, 30), 25);
	}

	for (int i = 1; i <= interfaces->engine->getMaxClients(); i++)
	{
		auto entity = interfaces->entityList->getEntity(i);

		if (!entity || entity->isDormant() || entity == localPlayer.get() || !entity->isAlive())
			continue;

		//auto team_num = entity->isOtherEnemy(entity);
		//bool bIsEnemy = localPlayer->team() != team_num;
		bool bIsEnemy = localPlayer->isOtherEnemy(entity);

		Vector EntityPos;
		ImVec2 EntityPosScreen;

		EntityPos = entity->getAbsOrigin();
		Helpers::worldToScreen(EntityPos, EntityPosScreen);

		//ImColor clr = bIsEnemy ? ImColor(255, 50, 50) : (team_num == 3 ? ImColor(95, 185, 245) : ImColor(245, 205, 65));
		ImColor clr = bIsEnemy ? ImColor(255, 50, 50) : ImColor(245, 205, 65);
		//ImColor clr_dormant = bIsEnemy ? ImColor(255, 50, 50, (int)(playeresp::get().esp_alpha_fade[i] * 255.0f)) : (team_num == 2 ? ImColor(95, 185, 245) : ImColor(245, 205, 65));

		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(EntityPos.x, EntityPos.y), 5, clr, 30);

		/*if (g_cfg.radar.render_health)
		{
			auto hp = math::clamp((float)e->m_iHealth(), 0.0f, 100.0f) / 100.0f;

			if (hp <= 0.0f)
				continue;

			auto height = math::clamp((float)e->m_iHealth() / 20.0f, 2.0f, 5.0f);

			ImColor clr = ImColor(int((1 - hp) * 255), int(hp * 255), 0, 255);
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(EntityPos.x - 4, EntityPos.y - height), ImVec2(EntityPos.x - 2, EntityPos.y + height), clr);
		}*/
	}
	/*
	if (draw_plantA && !bomb_siteA.IsZero())
	{
		Vector plant_pos_A;
		plant_pos_A.x = bomb_siteA.x * ImGui::GetWindowSize().x;
		plant_pos_A.x += ImGui::GetWindowPos().x;
		plant_pos_A.y = bomb_siteA.y * ImGui::GetWindowSize().y;
		plant_pos_A.y += ImGui::GetWindowPos().y;

		ImGui::GetWindowDrawList()->AddText(ImVec2(plant_pos_A.x, plant_pos_A.y), ImColor(225, 225, 25), "A");
	}

	if (draw_plantB && !bomb_siteB.IsZero())
	{
		Vector plant_pos_B;
		plant_pos_B.x = bomb_siteB.x * ImGui::GetWindowSize().x;
		plant_pos_B.x += ImGui::GetWindowPos().x;
		plant_pos_B.y = bomb_siteB.y * ImGui::GetWindowSize().y;
		plant_pos_B.y += ImGui::GetWindowPos().y;

		ImGui::GetWindowDrawList()->AddText(ImVec2(plant_pos_B.x, plant_pos_B.y), ImColor(225, 225, 25), "B");
	}*/


	ImGui::End();

}