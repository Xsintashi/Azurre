#include "MovementRecorder.h"
#include "../SDK/Vector.h"
#include "../JsonForward.h"

#include "../SDK/UserCmd.h"
#include "../SDK/entity.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Engine.h"
#include "../Interfaces.h"
#include "../HelpersAimbot.h"
#include "../imgui/imgui_stdlib.h"
#include "../GUI.h"
#include "../imguiCustom.h"

#include "../Config.h"
#include <deque>

struct Startpoint
{
	Vector origin;
	std::string title;
};

struct Frame
{
	ImVec2 viewangles{ 0.f,0.f };
	float forwardmove{ 0.f };
	float sidemove{ 0.f };
	float upmove{ 0.f };
	int buttons{ 0 };
	unsigned char impulse{ 0 };
	short mousedx{ 0 };
	short mousedy{ 0 };
};
struct Movement
{
	Startpoint startpoint;
	std::vector<Frame> frames;
	std::deque<Vector> steps;
};

struct MovementRecorderConfig
{
	bool enabled{ false };
	bool lockViewangles{ true };
	bool playReversed{ false };
	KeyBindToggle recordKey;
	KeyBind playKey;
	KeyBind stopKey;
	Color4 groundRing{ 0.26f, 0.35f, 0.8f, 0.602f };
	Color4 groundText{ 1.f,1.f,1.f,1.f };
	Color4 steps{ 1.f,1.f,1.f,.2f };
	float renderDistance{ 400.f };
}mRConfig;

static std::vector<std::string> recordFiles;
static float timeToNextConfigRefresh{ 0.1f };

static std::unordered_map<int, Movement> movementsMap;


static int currentRecord = 0;
static int currentplayback = 0;
static size_t currentframe = 0;

void MovementRecorder::record(UserCmd* cmd) noexcept
{
	if (!mRConfig.enabled || !mRConfig.recordKey.isToggled() || !localPlayer || !localPlayer->isAlive())
		return;

	if (!currentRecord)
	{

		Movement newRecord;
		currentRecord = movementsMap.size() + 1;
		while (movementsMap.find(currentRecord) != movementsMap.end())
		{
			currentRecord++;
		}
		newRecord.startpoint.title = "New record " + std::to_string(currentRecord);
		newRecord.startpoint.origin = localPlayer->getAbsOrigin();
		movementsMap[currentRecord] = newRecord;

	}

	Frame newFrame;
	newFrame.viewangles.x = cmd->viewangles.x;
	newFrame.viewangles.y = cmd->viewangles.y;
	newFrame.forwardmove = cmd->forwardmove;
	newFrame.sidemove = cmd->sidemove;
	newFrame.upmove = cmd->upmove;
	newFrame.buttons = cmd->buttons;
	newFrame.impulse = cmd->impulse;
	newFrame.mousedx = cmd->mousedx;
	newFrame.mousedy = cmd->mousedy;

	movementsMap[currentRecord].frames.push_back(newFrame);
	movementsMap[currentRecord].steps.push_front(localPlayer->getAbsOrigin());
}

void MovementRecorder::play(UserCmd* cmd) noexcept
{
	if (!mRConfig.enabled || !currentplayback)
		return;

	if (currentframe == movementsMap[currentplayback].frames.size() - 1 || !localPlayer || !localPlayer->isAlive() || mRConfig.stopKey.isPressed())
	{
		currentplayback = currentframe = 0;
		return;
	}
	if (currentframe == 0)
	{

		const float dist = localPlayer->getAbsOrigin().distTo(movementsMap[currentplayback].startpoint.origin);
		const auto current_position = localPlayer->getAbsOrigin();
		auto difference = current_position - movementsMap[currentplayback].startpoint.origin;

		auto velocity = Vector(difference.x * cosf(cmd->viewangles.y / 180.0f * (float)M_PI) + difference.y * sinf(cmd->viewangles.y / 180.0f * (float)M_PI), difference.y * cosf(cmd->viewangles.y / 180.0f * (float)M_PI) - difference.x * sin(cmd->viewangles.y / 180.0f * (float)M_PI), difference.z);

		cmd->forwardmove = -velocity.x * 20.0f;
		cmd->sidemove = velocity.y * 20.0f;
		if (dist > 1.f)
			return;

		Vector startViewAngles = { movementsMap[currentplayback].frames[0].viewangles.x,movementsMap[currentplayback].frames[0].viewangles.y ,0.f };

		auto delta = startViewAngles - cmd->viewangles;
		delta.normalize();
		//delta /= 4.f;
		if (float length = delta.length(); length > 2.f)
		{
			delta *= 2.f / length;

		}
		cmd->viewangles += delta;
		interfaces->engine->setViewAngles(cmd->viewangles);
		if (delta.length2D() >= 0.0001f)
			return;
	}

	if (mRConfig.lockViewangles)
	{
		cmd->viewangles.x = movementsMap[currentplayback].frames[currentframe].viewangles.x;
		cmd->viewangles.y = movementsMap[currentplayback].frames[currentframe].viewangles.y;
		interfaces->engine->setViewAngles(cmd->viewangles);
	}

	cmd->forwardmove = movementsMap[currentplayback].frames[currentframe].forwardmove;
	cmd->sidemove = movementsMap[currentplayback].frames[currentframe].sidemove;
	cmd->upmove = movementsMap[currentplayback].frames[currentframe].upmove;
	cmd->buttons = movementsMap[currentplayback].frames[currentframe].buttons;
	cmd->impulse = movementsMap[currentplayback].frames[currentframe].impulse;
	cmd->mousedx = movementsMap[currentplayback].frames[currentframe].mousedx;
	cmd->mousedy = movementsMap[currentplayback].frames[currentframe].mousedy;
	currentframe++;
}

void MovementRecorder::drawStartpoints(ImDrawList* drawList) noexcept
{
	if (!mRConfig.enabled || !localPlayer || !localPlayer->isAlive())
		return;
	mRConfig.recordKey.handleToggle();

	ImVec2 center = ImGui::GetIO().DisplaySize / 2.f;

	if (currentRecord)
		drawList->AddText(center, IM_COL32(255, 0, 0, 255), "Now recording");

	if (currentplayback)
	{
		std::string indicator{ "Now playing, Frame:" + std::to_string(currentframe) };
		drawList->AddText(center, IM_COL32(127, 74, 240, 255), indicator.c_str());

		for (size_t i = 2; i < movementsMap[currentplayback].steps.size(); i++)
		{

			if (!movementsMap[currentplayback].steps.size()) continue;

			if (ImVec2 cpos; Helpers::worldToScreen(movementsMap[currentplayback].steps[i - 1], cpos))
				if (ImVec2 cpos2; Helpers::worldToScreen(movementsMap[currentplayback].steps[i], cpos2))
					drawList->AddLine({ cpos.x, cpos.y }, { cpos2.x, cpos2.y }, Helpers::calculateColor(mRConfig.steps), 1.f);
		}
	}

	if (currentplayback || mRConfig.stopKey.isPressed() || !interfaces->engine->isInGame())
	{
		currentRecord = 0;
		mRConfig.recordKey.reset();
		return;
	}

	if (movementsMap.empty() || currentplayback || currentRecord)
		return;

	for (const auto& movement : movementsMap)
	{
		const float dist = localPlayer->getAbsOrigin().distTo(movement.second.startpoint.origin);
		if (dist > mRConfig.renderDistance)
			continue;
		Helpers::Draw3DFilledCircleFilled(drawList, movement.second.startpoint.origin, 20.f, Helpers::calculateColor(mRConfig.groundRing));
		ImVec2 screenPoint{ 0.f,0.f };
		if (Helpers::worldToScreen(movement.second.startpoint.origin, screenPoint))
		{
			//std::string title{ movement.second.startpoint.title + " dist:" + std::to_string((int)dist)};

			drawList->AddText(screenPoint, Helpers::calculateColor(mRConfig.groundText), movement.second.startpoint.title.c_str());
		}
		if (dist > 100.f)
			continue;

		if (mRConfig.playKey.isPressed())
		{
			currentplayback = movement.first;
			currentframe = 0;
		}
	}
}

bool MovementRecorder::isPlayingback() noexcept
{
	if (currentRecord)
		return false;
	return currentplayback;
}

bool MovementRecorder::shouldLockViewAngles() noexcept
{
	return false;
	//return mRConfig.lockViewangles * isPlayingback();
}

static void from_json(const json& j, const char* key, ImVec2& i)
{
	if (!j.contains(key))
		return;
	read<value_t::number_float>(j[key], "x", i.x);
	read<value_t::number_float>(j[key], "y", i.y);
}

static void from_json(const json& j, const char* key, Vector& i)
{
	if (!j.contains(key))
		return;
	read<value_t::number_float>(j[key], "x", i.x);
	read<value_t::number_float>(j[key], "y", i.y);
	read<value_t::number_float>(j[key], "z", i.z);
}

static void from_json(const json& j, Vector& i)
{
	read<value_t::number_float>(j, "x", i.x);
	read<value_t::number_float>(j, "y", i.y);
	read<value_t::number_float>(j, "z", i.z);
}

static void from_json(const json& j, const char* key, Startpoint& i)
{
	if (!j.contains(key))
		return;

	from_json(j[key], "origin", i.origin);
	read<value_t::string>(j[key], "title", i.title);
}

static void from_json(const json& j, Frame& i)
{
	from_json(j, "viewangles", i.viewangles);
	read<value_t::number_float>(j, "forwardmove", i.forwardmove);
	read<value_t::number_float>(j, "sidemove", i.sidemove);
	read<value_t::number_float>(j, "upmove", i.upmove);
	read(j, "buttons", i.buttons);
	//read<value_t::number_unsigned>(j, "impulse", i.impulse);
	read<value_t::number_float>(j, "mousedx", i.mousedx);
	read<value_t::number_float>(j, "mousedy", i.mousedy);
}

static void from_json(const json& j, const char* key, std::vector<Frame>& i)
{
	if (!j.contains(key))
		return;

	for (const auto& item : j[key])
	{
		Frame frame;
		from_json(item, frame);
		i.push_back(frame);
	}
}

static void from_json(const json& j, const char* key, std::deque<Vector>& i)
{
	if (!j.contains(key))
		return;

	for (const auto& item : j[key])
	{
		Vector v;
		from_json(item, v);
		i.push_front(v);
	}
}

static void from_json(const json& j, Movement& i)
{
	from_json(j, "startpoint", i.startpoint);
	from_json(j, "frames", i.frames);
	from_json(j, "steps", i.steps);

}

static void loadData(const char* filename, bool incremental = false) noexcept
{
	json j;
	if (std::ifstream in{ config->path / ".." / "movement" / filename }; in.good()) {
		j = json::parse(in, nullptr, false, true);
		if (j.is_discarded())
			return;
	}
	else {
		return;
	}

	if (!incremental)
		movementsMap.clear();

	size_t iterator = 1; // not zero, because zero will disable playback
	if (incremental)
		iterator = movementsMap.size() + 1;
	for (const auto& item : j)
	{
		Movement movement;
		//read<value_t::object>(item, item.key().c_str(), movement);
		from_json(item, movement);
		if (incremental)
		{
			while (movementsMap.find(iterator) != movementsMap.end())
			{
				iterator++;
			}
		}
		movementsMap[iterator] = movement;
		iterator++;
	}
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
	to_json(j["x"], o.x, {});
	to_json(j["y"], o.y, {});
}

static void to_json(json& j, const Vector& o, const Vector& dummy = {})
{
	to_json(j["x"], o.x, {});
	to_json(j["y"], o.y, {});
	to_json(j["z"], o.z, {});
}

static void to_json(json& j, const Startpoint& o, const Startpoint& dummy = {})
{
	to_json(j["origin"], o.origin, {});
	to_json(j["title"], o.title, {});

}

static void to_json(json& j, const Frame& o, const Frame& dummy = {})
{
	to_json(j["viewangles"], o.viewangles, {});
	to_json(j["forwardmove"], o.forwardmove, {});
	to_json(j["sidemove"], o.sidemove, {});
	to_json(j["upmove"], o.upmove, {});
	to_json(j["buttons"], o.buttons, {});
	to_json(j["impulse"], o.impulse, {});
	to_json(j["mousedx"], o.mousedx, {});
	to_json(j["mousedy"], o.mousedy, {});
}

static void to_json(json& j, const std::vector<Frame>& o, const std::vector<Frame>& dummy = {})
{
	if (o.empty())
		return;

	int iterator = 0;
	for (const auto& i : o)
	{
		to_json(j[iterator], o[iterator], {});
		iterator++;
	}
}

static void to_json(json& j, const std::deque<Vector>& o, const std::deque<Vector>& dummy = {})
{
	if (o.empty())
		return;

	int iterator = 0;
	for (const auto& i : o)
	{
		to_json(j[iterator], o[iterator], {});
		iterator++;
	}
}

static void to_json(json& j, const Movement& o, const Movement& dummy = {})
{
	to_json(j["startpoint"], o.startpoint, {});
	to_json(j["frames"], o.frames, {});
	to_json(j["steps"], o.steps, {});

}

static void saveData(const char* filename) noexcept
{
	json j;
	if (movementsMap.empty())
		return;

	int iterator = 0;
	for (const auto& movement : movementsMap)
	{
		to_json(j[iterator], movement.second, {});
		iterator++;
	}
	std::error_code ec; std::filesystem::create_directories(config->path / ".." / "movement", ec);
	if (std::ofstream out{ config->path / ".." / "movement" / filename }; out.good())
		out << j;
}

static void framesEdit(std::vector<Frame>* frames) noexcept
{
	ImGui::BeginChildFrame(ImGui::GetID("frameEdit"), { -1,48.f }, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	unsigned int iterator = 1;
	for (auto& frame : *frames)
	{
		ImGui::PushID(iterator + 1);
		if (ImGui::Button(std::to_string(iterator - 1).c_str(), { 0,-1 }))//ICON_FA_CIRCLE_NOTCH
			ImGui::OpenPopup("itemEdit");

		if (ImGui::BeginPopup("itemEdit"))
		{
			ImGui::InputFloat("view angle x", &frame.viewangles.x);
			ImGui::InputFloat("view angle y", &frame.viewangles.y);
			ImGui::InputFloat("forwardmove", &frame.forwardmove);
			ImGui::InputFloat("sidemove", &frame.sidemove);
			ImGui::InputFloat("upmove", &frame.upmove);
			int mousedx = (int)frame.mousedx;
			ImGui::InputInt("mousedx", &mousedx);
			frame.mousedx = (short)mousedx;
			int mousedy = (int)frame.mousedy;
			ImGui::InputInt("mousedx", &mousedy);
			frame.mousedy = (short)mousedy;
			ImGui::InputInt("button", &frame.buttons);
			ImGui::LabelText("impulse", std::to_string(frame.impulse).c_str());

			ImGui::EndPopup();
		}
		ImGui::PopID();
		ImGui::SameLine();
		iterator++;
	}
	ImGui::EndChildFrame();
}

static bool windowOpen = false;

void MovementRecorder::drawGUI(bool contentOnly) noexcept
{
	//disable recorder or player if menu is opened
	if (mRConfig.recordKey.isToggled()) mRConfig.recordKey.reset();
	if (currentRecord) currentRecord = 0;
	if (currentplayback) currentplayback = 0;
	if (currentframe) currentframe = 0;
	static bool incremental_load = false;

	if (!contentOnly) {
		if (!windowOpen)
			return;
		ImGui::SetNextWindowSize({ 320.0f, 0.0f });
		ImGui::Begin("Movement", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	}

	ImGui::Checkbox("Enable", &mRConfig.enabled);
	ImGui::SetNextItemWidth(200.f);
	ImGui::InputFloat("View distance", &mRConfig.renderDistance, 10.f, 50.f, "%.2f");
	ImGui::hotkey("Play key", mRConfig.playKey, 200.f);
	ImGui::hotkey("Stop key", mRConfig.stopKey, 200.f);
	ImGui::hotkey("Record key", mRConfig.recordKey, 200.f);
	ImGui::Checkbox("Lock view", &mRConfig.lockViewangles);
	ImGuiCustom::colorPicker("Ground circle", mRConfig.groundRing);
	ImGuiCustom::colorPicker("Ground text", mRConfig.groundText);
	ImGuiCustom::colorPicker("Movement trail", mRConfig.steps);

	if (ImGui::Button(" Load"))
		ImGui::OpenPopup("load config");

	if (ImGui::BeginPopup("load config"))
	{
		ImGui::Checkbox("Incremental load", &incremental_load);

		if (ImGui::BeginChildFrame('L', { 200.f,200.f }))
		{
			for (const auto& configFile : recordFiles)
			{
				if (ImGui::Selectable(("  " + configFile).c_str()))
				{
					loadData(configFile.c_str(), incremental_load);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndChildFrame();
		}
		ImGui::EndPopup();
	}

	timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;

	if (timeToNextConfigRefresh <= 0.0f) {
		recordFiles.clear();

		std::error_code ec;
		std::transform(std::filesystem::directory_iterator{ config->path / ".." / "movement", ec },
			std::filesystem::directory_iterator{ },
			std::back_inserter(recordFiles),
			[](const auto& entry) { return entry.path().filename().string(); });
		timeToNextConfigRefresh = 0.1f;
	}

	ImGui::SameLine();

	static std::string savefilename;

	if (ImGui::Button(" Save"))
	{
		savefilename.clear();
		if (interfaces->engine->isInGame())
			savefilename = interfaces->engine->getLevelName();
		ImGui::OpenPopup(" Save");
	}

	if (ImGui::BeginPopupModal(" Save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::LabelText("", " File name:");
		ImGui::InputText("", &savefilename);
		if (ImGui::Button(" Save"))
		{
			saveData(savefilename.c_str());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(" Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();

	if (ImGui::Button(" Clear"))
		movementsMap.clear();

	ImGui::BeginChildFrame('c', { 200.f,-1 });
	int id = 10;
	for (auto& movement : movementsMap)
	{

		ImGui::BeginChildFrame(id, { -1,ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().FramePadding.y });

		ImGui::LabelText("", movement.second.startpoint.title.c_str());

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 35.f * 2.f);

		if (ImGui::Button("Del", { 35.f,0 })){
			movementsMap.erase(movement.first);
			ImGui::End();
		}
			
		ImGui::SameLine();

		if (ImGui::Button("Edit", { 35.f,0 }))
			ImGui::OpenPopup("Edit");

		if (ImGui::BeginPopupModal("Edit", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::LabelText("", "Display title");
			ImGui::InputText("", &movement.second.startpoint.title);

			//std::string count = "frame count: " + std::to_string(movement.second.frames.size());
			//ImGui::LabelText("", count.c_str());
			//framesEdit(&movement.second.frames);

			if (ImGui::Button(" Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::EndChildFrame();
		id++;
	}
	ImGui::EndChildFrame();

	if (!contentOnly)
		ImGui::End();
}

static void to_json(json& j, const MovementRecorderConfig& o, const MovementRecorderConfig& dummy = {})
{
	WRITE("enabled", enabled);
	WRITE("lock viewangles", lockViewangles);
	WRITE("render distance", renderDistance);
	WRITE("play reversed", playReversed);
	WRITE("ring color", groundRing);
	WRITE("text color", groundText);
	WRITE("steps color", steps);
	WRITE("record", recordKey);
	WRITE("play", playKey);
	WRITE("stop", stopKey);
}

json MovementRecorder::toJson() noexcept
{
	json j;
	to_json(j, mRConfig);
	return j;
}
static void from_json(const json& j, MovementRecorderConfig& i)
{
	read(j, "enabled", i.enabled);
	read(j, "lock viewangles", i.lockViewangles);
	read(j, "render distance", i.renderDistance);
	read(j, "play reversed", i.playReversed);
	read<value_t::object>(j, "ring color", i.groundRing);
	read<value_t::object>(j, "text color", i.groundText);
	read<value_t::object>(j, "steps color", i.steps);
	read(j, "record", i.recordKey);
	read(j, "play", i.playKey);
	read(j, "stop", i.stopKey);
}

void MovementRecorder::fromJson(const json& j) noexcept
{
	from_json(j, mRConfig);
}

void MovementRecorder::resetConfig() noexcept
{
	mRConfig = {};
}