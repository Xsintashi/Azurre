#include "RPC.h"
#include "../SDK/isteamfriends.h"
#include "../SDK/isteamuser.h"

struct DiscordConfig {
	bool enable = true;
	bool showMap = true;
	int	detailsMode = 0;
	int stateMode = 0;
}discordConfig;


void Discord::Run() {
	DiscordEventHandlers Handler;
	memset(&Handler, 0, sizeof(Handler));
	Discord_Initialize("DISCORD_BOT_TOKEN_OR_SMTH", &Handler, 1, NULL);

	Update();
}

const char* getMapName() {
	if (!discordConfig.showMap || !localPlayer) return
#if defined(_DEBUG)
		"debug";
#else 
	"g512";
#endif
	switch (fnv::hashRuntime(interfaces->engine->getLevelName())){
		default: return "mp_community";
		case fnv::hash("cs_agency"): return "cs_agency";
		case fnv::hash("cs_assault"): return "cs_assault";
		case fnv::hash("cs_italy"): return "cs_italy";
		case fnv::hash("cs_militia"): return "cs_militia";
		case fnv::hash("cs_office"): return "cs_office";
		case fnv::hash("de_ancient"): return "de_ancient";
		case fnv::hash("de_cache"): return "de_cache";
		case fnv::hash("de_canals"): return "de_canals";
		case fnv::hash("de_cbble"): return "de_cbble";
		case fnv::hash("de_dust2"): return "de_dust2";
		case fnv::hash("de_inferno"): return "de_inferno";
		case fnv::hash("de_lake"): return "de_lake";
		case fnv::hash("de_mirage"): return "de_mirage";
		case fnv::hash("de_nuke"): return "de_nuke";
		case fnv::hash("de_overpass"): return "de_overpass";
		case fnv::hash("de_shortdust"): return "de_shortdust";
		case fnv::hash("de_shortnuke"): return "de_shortnuke";
		case fnv::hash("de_train"): return "de_train";
		case fnv::hash("de_vertigo"): return "de_vertigo";
	}
}

const char* getMapName2() {
	if (!discordConfig.showMap || !localPlayer) return "azurre";
	return interfaces->engine->getLevelName();

}

const char* getGamemode() {

	if (!localPlayer) return "";

	auto game_mode = interfaces->cvar->findVar("game_mode")->getInt();
	auto game_type = interfaces->cvar->findVar("game_type")->getInt();

	switch (game_type)
	{
	default: return "Custom"; break;
		case 0: 
			switch (game_mode)
			{
				default: case 0: return "Casual"; break;
				case 1: return "Competitive"; break;
				case 2: return "Wingman"; break;
				case 3: return "Weapons Expert"; break;
			}
			break;
		case 1: 
			switch (game_mode)
			{
			default: case 0: return "Arms Race"; break;
			case 1: return "Demolition"; break;
			case 2: return "Deathmatch"; break;
			}
			break;
		case 2: return "Training"; break;
		case 4:
			switch (game_mode)
				{
				default: case 0: return "Guardian"; break;
				case 1: return "Co-op Strike"; break;
				}
			  break;
		case 6: return "Danger Zone"; break;
			break;
	}

}

const char* getIn() {

	if (!localPlayer) return "In Lobby";
	return "In Game";
}

const char* getPlayStyle() {
	if (!localPlayer) return "";

	if (!config->legitaimbot[0].enabled && !config->globalEnabledRagebot) return "Playing: Closet";

	if (config->legitaimbot[0].enabled && !AntiAim::isEnabled()) return "Playing: Legit";

	if (!config->legitaimbot[0].enabled && config->globalEnabledRagebot && !AntiAim::isEnabled()) return "Playing: Semi-Rage";

	if (!config->legitaimbot[0].enabled && config->globalEnabledRagebot && AntiAim::isEnabled()) return "Playing: Rage";

	return "Playing: Closet";
}

const char* getResource(int i) {

	if (!localPlayer) return "";

	auto playerResource = *memory->playerResource;

	switch (i)
	{
	default: 
	case 0: return playerResource->getClan(localPlayer->index());
	case 1: return Helpers::ConvertRankFromNumber(false, playerResource->competitiveRanking()[localPlayer->index()]);
	}

	return "";
}

void Discord::Update() {

	static float compare = 0;
	static std::string details;
	static std::string state;
	auto playerResource = *memory->playerResource;

	const char* playerName = steamIFriends->GetPersonaName();

	if (compare + 5.f > memory->globalVars->realtime) return;

	if (!discordConfig.enable) {
		Discord_ClearPresence();
		return;
	}

	if (localPlayer){

		if (!playerResource) return;

		switch (discordConfig.detailsMode)
		{
		default:
			details = "";
			break;
		case 0: details = std::string("Nick: ").append(getResource(0)).append(" ").append(playerName); break;
		case 1: details = std::string("Rank: ").append(getResource(1)); break;
		case 2: details = std::string("Gamemode: ").append(getGamemode()); break;
		}

		switch (discordConfig.stateMode) { //In *Game*\0Rank\0Gamemode\0Statistics\0Play Style\0
			default:
			case 0: state = getIn(); break;
			case 1: state = std::string("Rank: ").append(getResource(1)); break;
			case 2: state = std::string("Gamemode: ").append(getGamemode()); break;
			case 3: state = std::string("MVP:").append(std::to_string(playerResource->GetMVPs()[localPlayer->index()])).
				append(" Score:").append(std::to_string(playerResource->GetScore()[localPlayer->index()])).
				append(" HS:").append(std::to_string(localPlayer->statsHeadShotKills())).
				append(" UD:").append(std::to_string(localPlayer->statsUtilityDamage()))
				.c_str(); break;
			case 4: state = getPlayStyle(); break;
		} 
	}
	else {
		details = std::string("Nick: ").append(getResource(0)).append(" ").append(playerName);
		state = "In Main Menu";
	}


	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = state.c_str();
	discordPresence.details = details.c_str();
	discordPresence.endTimestamp = NULL;
	discordPresence.smallImageKey = "azurre";
	discordPresence.largeImageKey = getMapName();
	discordPresence.largeImageText = getMapName2();
	discordPresence.instance = 1;
	Discord_UpdatePresence(&discordPresence);
	compare = memory->globalVars->realtime;

}

void Discord::customRichPresenseTest(
	const char* state,
	const char* details,
	int64_t endTimestamp,
	const char* smallImageKey,
	const char* largeImageKey,
	const char* largeImageText
	) {
	
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = state;
	discordPresence.details = details;
	discordPresence.endTimestamp = endTimestamp;
	discordPresence.smallImageKey = smallImageKey;
	discordPresence.largeImageKey = largeImageKey;
	discordPresence.largeImageText = largeImageText;
	discordPresence.instance = 1;
	Discord_UpdatePresence(&discordPresence);
}

void Discord::Shutdown() {
	Discord_ClearPresence();
	Discord_Shutdown();
}

static bool DiscordMenuOpen = false;

void Discord::menuBarItem() noexcept
{
	if (ImGui::MenuItem("Discord RP")) {
		DiscordMenuOpen = true;
		ImGui::SetWindowFocus("Discord Rich Presence");
		ImGui::SetWindowPos("Discord Rich Presence", { 100.0f, 100.0f });
	}
}

void Discord::drawGUI(bool contentOnly) {
	if (!contentOnly) {
		if (!DiscordMenuOpen)
			return;
		ImGui::SetNextWindowSize({ 0.0f, 0.0f });
		ImGui::Begin("Discord Rich Presence", &DiscordMenuOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	}

	ImGui::Toggle("Enabled", &discordConfig.enable);
	ImGui::Toggle("Show Map", &discordConfig.showMap);
	ImGui::PushItemWidth(100.f);
	ImGui::Combo("Details Mode", &discordConfig.detailsMode, "Nickname\0Rank\0Gamemode\0Statistics\0");
	ImGui::Combo("State Mode", &discordConfig.stateMode, "In *Game*\0Rank\0Gamemode\0Statistics 2\0Play Style\0");
	ImGui::PopItemWidth();


	if (!contentOnly)
		ImGui::End();
}

static void from_json(const json& j, DiscordConfig& dc)
{
	read(j,"Enabled", dc.enable);
	read(j,"Show Maps", dc.showMap);
	read(j,"Details Mode", dc.detailsMode);
	read(j,"State Mode", dc.stateMode);
}
static void to_json(json& j, const DiscordConfig& o)
{
	const DiscordConfig dummy;

		WRITE("Enabled", enable);
		WRITE("Show Maps", showMap);
		WRITE("Details Mode", detailsMode);
		WRITE("State Mode", stateMode);

}

json Discord::toJson() noexcept
{
	json j;
	to_json(j, discordConfig);
	return j;
}

void Discord::fromJson(const json& j) noexcept
{
	from_json(j, discordConfig);
}

void Discord::resetConfig() noexcept
{
	discordConfig = {};
}