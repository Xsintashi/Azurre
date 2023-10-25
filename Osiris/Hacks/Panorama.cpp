#include "Panorama.h"

#include <Interfaces.h>

#include <SDK/Entity.h>
#include <SDK/LocalPlayer.h>
#include <SDK/Panorama.h>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

struct PanoramaConfig {

	std::string nameTagString = "";

	struct Dance {
		int modelIndex = 0;
		int sequence = 0;
		int speed = 100;
		std::string customModel = "models/player/custom_player/";
	} dance;

	struct MainMenu {
		bool enableNews = true;
		bool enableLeftBar = true;
		bool enableFriendsBar = true;
		bool enablePlayerModel = true;
		std::string customBackgroundVideo = "file://{resources}/videos/anubis.webm";
	} mainMenu;

}pCfg;

struct Panels {
	UIPanel* CSGOHud = NULL; //CSGOHud
	UIPanel* MainMenuContainerPanel = NULL; //MainMenuContainerPanel
	UIPanel* CSGOMainMenu = NULL; //CSGOMainMenu
} panels;

UIPanel*& Panorama::getPanel(CSGOPanel panel) {
	switch (panel) {
	default:
	case 0: return panels.CSGOHud;
	case 1: return panels.MainMenuContainerPanel;
	case 2: return panels.CSGOMainMenu;
	}
}

UIPanel*& Panorama::getPanel(int panel) {
	switch (panel) {
	default:
	case 0: return panels.CSGOHud;
	case 1: return panels.MainMenuContainerPanel;
	case 2: return panels.CSGOMainMenu;
	}
}

typedef IUIEvent* (__cdecl* SetupEvent)(int(***)(void));

std::unordered_map<std::string, SetupEvent> panoramaEventConstructors;

IUIEvent* Panorama::createPanoramaEvent(const std::string& eventName, bool cacheOnly) {

	static auto bytesToPattern = [](byte* bytes, DWORD size) {
		std::stringstream pattern;
		pattern << std::hex << std::setfill('0');
		for (size_t i = 0; i < size; i++) {
			const int32_t current_byte = bytes[i];
			pattern << std::setw(2) << current_byte;
			if (i != size - 1)
				pattern << " ";
		}
		return pattern.str();
	};

	//try to find the event in cache first
	auto cacheEntry = panoramaEventConstructors.find(eventName);
	if (cacheEntry != panoramaEventConstructors.end()) {
		if (!cacheEntry->second) //if we didn't find it last time assume we won't ever find it
			return 0;
		if (cacheOnly)
			return 0;
		return cacheEntry->second(0);
	}

	// First of, we're going to find the event name in client.dll, so let's convert the name to an ida style pattern
	std::string pattern = bytesToPattern((byte*)eventName.data(), eventName.size());
	uintptr_t string_in_memory = MemoryN::findPatternA("client.dll", pattern.c_str());
	if (!string_in_memory) {
		panoramaEventConstructors.insert(std::make_pair(eventName, (SetupEvent)0));
		return 0;
	}

	// Now, we're going to find the first reference to the string in client.dll
	byte bytes[4];
	memcpy(bytes, &string_in_memory, 4);
	pattern = bytesToPattern(bytes, 4);
	uintptr_t string_reference = MemoryN::findPatternA("client.dll", pattern);
	if (!string_reference) {
		panoramaEventConstructors.insert(std::make_pair(eventName, (SetupEvent)0));
		return 0;
	}

	// The first function we find after the reference is the setup function of the event. To find it, we can search for the bytes "C7 45 DC"
	// from the string reference.
	uintptr_t setup_fn_ref_addr = MemoryN::findPatternS((void*)string_reference, 999, "C7 45 DC");
	if (!setup_fn_ref_addr) {
		panoramaEventConstructors.insert(std::make_pair(eventName, (SetupEvent)0));
		return 0;
	}

	// To get the actual setup function address, we add 3 bytes (because of the C7 45 DC), and then dereference.
	SetupEvent setup_fn_addr = *(SetupEvent*)(setup_fn_ref_addr + 3);

	//add to cache
	panoramaEventConstructors.insert(std::make_pair(eventName, setup_fn_addr));

	if (cacheOnly)
		return 0;

	// The only thing left to do is call the setup function. It will return the event that the function dispatch_event accepts.
	return setup_fn_addr(0);
}

UIPanel* Panorama::tryGetPanels(CSGOPanel panel) noexcept{

	if (
		panels.CSGOHud &&
		panels.CSGOMainMenu &&
		panels.MainMenuContainerPanel
		) return NULL;

	UIPanel* tempPanel = NULL;
	tempPanel = interfaces->panoramaUIEngine->accessUIEngine()->GetLastDispatchedEventTargetPanel();
	if (interfaces->panoramaUIEngine->accessUIEngine()->IsValidPanelPointer(tempPanel)) {
		do {
			if (!strcmp(tempPanel->GetID(), "CSGOHud") && panel == CSGOPanel::CSGOHud) {
				Helpers::printfColored(Helpers::printfColors::Light_Green, std::string("[!] Found CSGOHud Panel").c_str());
				return tempPanel;
			}
			if (!strcmp(tempPanel->GetID(), "CSGOMainMenu") && panel == CSGOPanel::CSGOMainMenu) {
				Helpers::printfColored(Helpers::printfColors::Light_Green, std::string("[!] Found CSGOMainMenu Panel").c_str());
				return tempPanel;
			}
			if (!strcmp(tempPanel->GetID(), "MainMenuContainerPanel") && panel == CSGOPanel::MainMenuContainerPanel){
				Helpers::printfColored(Helpers::printfColors::Light_Green, std::string("[!] Found MainMenuContainer Panel").c_str());
				return tempPanel;
			}
		} while ((tempPanel = tempPanel->GetParent()) != NULL && interfaces->panoramaUIEngine->accessUIEngine()->IsValidPanelPointer(tempPanel));
	}
	else tempPanel = NULL;

	return NULL;
}

void Panorama::executeScript(UIPanel* panel,const char* script, const char* pathXML) noexcept{

	if (!panel) return;

	interfaces->panoramaUIEngine->accessUIEngine()->RunScript(panel, script, pathXML, 8, 10, false, false);

}

constexpr const char* fortniteSequencesGUI[] {
	"None",
	"Fonzie Pistol",
	"Bring It On",
	"Thumbs Down",
	"Thumbs Up",
	"Celebration Loop",
	"Blow Kiss",
	"Calculated",
	"Confused",
	"Chug",
	"Cry",
	"Dusting Off Hands",
	"Dust Off Shoulders",
	"Facepalm",
	"Fishing",
	"Flex",
	"Golfclap",
	"Hand Signals",
	"Heel Click",
	"Hotstuff",
	"IBreakYou",
	"IHeartYou",
	"Kung",
	"Laugh",
	"Luchador",
	"Make It Rain",
	"Not Today",
	"[RPS] Paper",
	"[RPS] Rock",
	"[RPS] Scissor",
	"Salt",
	"Salute",
	"Smooth Drive",
	"Snap",
	"StageBow",
	"Wave2",
	"Yeet",
	"Dance Moves",
	"Zippy Dance",
	"Electro Shuffle",
	"Aerobic Champ",
	"Bendy",
	"Band Of The Fort",
	"Capoeira",
	"Charleston",
	"Chicken",
	"No Bones",
	"Shoot",
	"Swipe It",
	"Disco 1",
	"Disco 2",
	"Worm",
	"Loser",
	"Breakdance",
	"Pump",
	"Ride The Pony",
	"Dab",
	"Fancy Feet",
	"Floss Dance",
	"Flippn Sexy",
	"Fresh",
	"Groove Jam",
	"Guitar",
	"Hiphop",
	"Korean Eagle",
	"Kpop",
	"Living Large",
	"Maracas",
	"Pop Lock",
	"Pop Rock",
	"Robot Dance",
	"T-Rex",
	"Techno Zombie",
	"Twist",
	"Wiggle",
	"You're Awesome"
};
constexpr const char* fortniteSequences[]{
	"",
	"Emote_Fonzie_Pistol",
	"Emote_Bring_It_On",
	"Emote_ThumbsDown",
	"Emote_ThumbsUp",
	"Emote_Celebration_Loop",
	"Emote_BlowKiss",
	"Emote_Calculated",
	"Emote_Confused",
	"Emote_Chug",
	"Emote_Cry",
	"Emote_DustingOffHands",
	"Emote_DustOffShoulders",
	"Emote_Facepalm",
	"Emote_Fishing",
	"Emote_Flex",
	"Emote_golfclap",
	"Emote_HandSignals",
	"Emote_HeelClick",
	"Emote_Hotstuff",
	"Emote_IBreakYou",
	"Emote_IHeartYou",
	"Emote_Kung-Fu_Salute",
	"Emote_Laugh",
	"Emote_Luchador",
	"Emote_Make_It_Rain",
	"Emote_NotToday",
	"Emote_RockPaperScissor_Paper",
	"Emote_RockPaperScissor_Rock",
	"Emote_RockPaperScissor_Scissor",
	"Emote_Salt",
	"Emote_Salute",
	"Emote_SmoothDrive",
	"Emote_Snap",
	"Emote_StageBow",
	"Emote_Wave2",
	"Emote_Yeet",
	"DanceMoves",
	"Emote_Zippy_Dance",
	"ElectroShuffle",
	"Emote_AerobicChamp",
	"Emote_Bendy",
	"Emote_BandOfTheFort",
	"Emote_Capoeira",
	"Emote_Charleston",
	"Emote_Chicken",
	"Emote_Dance_NoBones",
	"Emote_Dance_Shoot",
	"Emote_Dance_SwipeIt",
	"Emote_Dance_Disco_T3",
	"Emote_DG_Disco",
	"Emote_Dance_Worm",
	"Emote_Dance_Loser",
	"Emote_Dance_Breakdance",
	"Emote_Dance_Pump",
	"Emote_Dance_RideThePony",
	"Emote_Dab",
	"Emote_FancyFeet",
	"Emote_FlossDance",
	"Emote_FlippnSexy",
	"Emote_Fresh",
	"Emote_GrooveJam",
	"Emote_guitar",
	"Emote_Hiphop_01",
	"Emote_KoreanEagle",
	"Emote_Kpop_02",
	"Emote_LivingLarge",
	"Emote_Maracas",
	"Emote_PopLock",
	"Emote_PopRock",
	"Emote_RobotDance",
	"Emote_T-Rex",
	"Emote_TechnoZombie",
	"Emote_Twist",
	"Emote_Wiggle",
	"Emote_Youre_Awesome"
};

constexpr const char* fortniteModelsGUI[]{
	"Special Agent Ava | FBI",
	"Operator | FBI SWAT",
	"Markus Delrow | FBI HRT",
	"Michael Syfers | FBI Sniper",
	"B Squadron Officer | SAS",
	"Seal Team 6 Soldier | NSWC SEAL",
	"Buckshot | NSWC SEAL",
	"Lt. Commander Ricksaw | NSWC SEAL",
	"Third Commando Company | KSK",
	"'Two Times' McCoy | USAF TACP",
	"Dragomir | Sabre",
	"Rezan The Ready | Sabre",
	"'The Doctor' Romanov | Sabre",
	"Maximus | Sabre",
	"Blackwolf | Sabre",
	"The Elite Mr. Muhlik | Elite Crew",
	"Ground Rebel | Elite Crew",
	"Osiris | Elite Crew",
	"Prof. Shahmat | Elite Crew",
	"Enforcer | Phoenix",
	"Slingshot | Phoenix",
	"Soldier | Phoenix",
	"Street Soldier | Phoenix",
	"'Blueberries' Buckshot | NSWC SEAL",
	"'Two Times' McCoy | TACP Cavalry",
	"Rezan the Redshirt | Sabre",
	"Dragomir | Sabre Footsoldier",
	"Cmdr. Mae 'Dead Cold' Jamison | SWAT",
	"001st Lieutenant Farlow | SWAT",
	"John 'Van Healen' Kask | SWAT",
	"Bio-Haz Specialist | SWAT",
	"Sergeant Bombson | SWAT",
	"Chem-Haz Specialist | SWAT",
	"Sir Bloody Miami Darryl | The Professionals",
	"Sir Bloody Silent Darryl | The Professionals",
	"Sir Bloody Skullhead Darryl | The Professionals",
	"Sir Bloody Darryl Royale | The Professionals",
	"Sir Bloody Loudmouth Darryl | The Professionals",
	"Safecracker Voltzmann | The Professionals",
	"Little Kev | The Professionals",
	"Number K | The Professionals",
	"Getaway Sally | The Professionals",
	"Anarchist",
	"Anarchist (Variant A)",
	"Anarchist (Variant B)",
	"Anarchist (Variant C)",
	"Anarchist (Variant D)",
	"Pirate",
	"Pirate (Variant A)",
	"Pirate (Variant B)",
	"Pirate (Variant C)",
	"Pirate (Variant D)",
	"Professional",
	"Professional (Variant 1)",
	"Professional (Variant 2)",
	"Professional (Variant 3)",
	"Professional (Variant 4)",
	"Separatist",
	"Separatist (Variant A)",
	"Separatist (Variant B)",
	"Separatist (Variant C)",
	"Separatist (Variant D)",
	"GIGN",
	"GIGN (Variant A)",
	"GIGN (Variant B)",
	"GIGN (Variant C)",
	"GIGN (Variant D)",
	"GSG-9",
	"GSG-9 (Variant A)",
	"GSG-9 (Variant B)",
	"GSG-9 (Variant C)",
	"GSG-9 (Variant D)",
	"IDF",
	"IDF (Variant B)",
	"IDF (Variant C)",
	"IDF (Variant D)",
	"IDF (Variant E)",
	"IDF (Variant F)",
	"SWAT",
	"SWAT (Variant A)",
	"SWAT (Variant B)",
	"SWAT (Variant C)",
	"SWAT (Variant D)",
	"SAS (Variant A)",
	"SAS (Variant B)",
	"SAS (Variant C)",
	"SAS (Variant D)",
	"ST6",
	"ST6 (Variant A)",
	"ST6 (Variant B)",
	"ST6 (Variant C)",
	"ST6 (Variant D)",
	"Balkan (Variant E)",
	"Balkan (Variant A)",
	"Balkan (Variant B)",
	"Balkan (Variant C)",
	"Balkan (Variant D)",
	"Jumpsuit (Variant A)",
	"Jumpsuit (Variant B)",
	"Jumpsuit (Variant C)",
	"Phoenix Heavy",
	"Heavy",
	"Leet (Variant A)",
	"Leet (Variant B)",
	"Leet (Variant C)",
	"Leet (Variant D)",
	"Leet (Variant E)",
	"Phoenix",
	"Phoenix (Variant A)",
	"Phoenix (Variant B)",
	"Phoenix (Variant C)",
	"Phoenix (Variant D)",
	"FBI",
	"FBI (Variant A)",
	"FBI (Variant C)",
	"FBI (Variant D)",
	"FBI (Variant E)",
	"Custom"
};
constexpr const char* fortniteModels[]{
	"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
	"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
	"models/player/custom_player/legacy/ctm_st6_variante.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
	"models/player/custom_player/legacy/tm_leet_variantf.mdl",
	"models/player/custom_player/legacy/tm_leet_variantg.mdl",
	"models/player/custom_player/legacy/tm_leet_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_varianti.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianti.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantj.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantl.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantk.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantl.mdl",
	"models/player/custom_player/legacy/ctm_swat_variante.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantf.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantg.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianth.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianti.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantj.mdl",
	"models/player/custom_player/legacy/tm_professional_varf.mdl",
	"models/player/custom_player/legacy/tm_professional_varf1.mdl",
	"models/player/custom_player/legacy/tm_professional_varf2.mdl",
	"models/player/custom_player/legacy/tm_professional_varf3.mdl",
	"models/player/custom_player/legacy/tm_professional_varf4.mdl",
	"models/player/custom_player/legacy/tm_professional_varg.mdl",
	"models/player/custom_player/legacy/tm_professional_varh.mdl",
	"models/player/custom_player/legacy/tm_professional_vari.mdl",
	"models/player/custom_player/legacy/tm_professional_varj.mdl",
	"models/player/custom_player/legacy/tm_anarchist.mdl",
	"models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
	"models/player/custom_player/legacy/tm_anarchist_variantd.mdl",
	"models/player/custom_player/legacy/tm_pirate.mdl",
	"models/player/custom_player/legacy/tm_pirate_varianta.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantb.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantc.mdl",
	"models/player/custom_player/legacy/tm_pirate_variantd.mdl",
	"models/player/custom_player/legacy/tm_professional.mdl",
	"models/player/custom_player/legacy/tm_professional_var1.mdl",
	"models/player/custom_player/legacy/tm_professional_var2.mdl",
	"models/player/custom_player/legacy/tm_professional_var3.mdl",
	"models/player/custom_player/legacy/tm_professional_var4.mdl",
	"models/player/custom_player/legacy/tm_separatist.mdl",
	"models/player/custom_player/legacy/tm_separatist_varianta.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantb.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantc.mdl",
	"models/player/custom_player/legacy/tm_separatist_variantd.mdl",
	"models/player/custom_player/legacy/ctm_gign.mdl",
	"models/player/custom_player/legacy/ctm_gign_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gign_variantd.mdl",
	"models/player/custom_player/legacy/ctm_gsg9.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_varianta.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantb.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantc.mdl",
	"models/player/custom_player/legacy/ctm_gsg9_variantd.mdl",
	"models/player/custom_player/legacy/ctm_idf.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantb.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantc.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantd.mdl",
	"models/player/custom_player/legacy/ctm_idf_variante.mdl",
	"models/player/custom_player/legacy/ctm_idf_variantf.mdl",
	"models/player/custom_player/legacy/ctm_swat.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianta.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantb.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantc.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantd.mdl",
	"models/player/custom_player/legacy/ctm_sas_varianta.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantb.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantc.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantd.mdl",
	"models/player/custom_player/legacy/ctm_st6.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianta.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantb.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantc.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantd.mdl",
	"models/player/custom_player/legacy/tm_balkan_variante.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianta.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantb.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantc.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantd.mdl",
	"models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl",
	"models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl",
	"models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl",
	"models/player/custom_player/legacy/tm_phoenix_heavy.mdl",
	"models/player/custom_player/legacy/ctm_heavy.mdl",
	"models/player/custom_player/legacy/tm_leet_varianta.mdl",
	"models/player/custom_player/legacy/tm_leet_variantb.mdl",
	"models/player/custom_player/legacy/tm_leet_variantc.mdl",
	"models/player/custom_player/legacy/tm_leet_variantd.mdl",
	"models/player/custom_player/legacy/tm_leet_variante.mdl",
	"models/player/custom_player/legacy/tm_phoenix.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianta.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantb.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantc.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantd.mdl",
	"models/player/custom_player/legacy/ctm_fbi.mdl",
	"models/player/custom_player/legacy/ctm_fbi_varianta.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantc.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantd.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variante.mdl",
	""
};

void modifygetPanel() noexcept {
	if (!Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel)) return;
	if (localPlayer) return;

	std::string script;

	script += "var panelNews = $('#JsNewsContainer');";
	script += "var panelNavBarLeft = $('#MainMenuNavBarLeft');";
	script += "var panelNavBarRight = $('#MainMenuFriendsAndParty');";
	script += "var panelVanity = $('#JsMainmenu_Vanity');";
	script += "if (panelNews)";
	script += std::string("	panelNews.visible = ") + std::to_string(pCfg.mainMenu.enableNews).append(";");
	script += "if (panelNavBarLeft)";
	script += std::string("	panelNavBarLeft.visible = ") + std::to_string(pCfg.mainMenu.enableLeftBar).append(";");
	script += "if (panelNavBarRight)";
	script += std::string("	panelNavBarRight.visible = ") + std::to_string(pCfg.mainMenu.enableFriendsBar).append(";");
	script += "if (panelVanity)";
	script += std::string("	panelVanity.visible = ") + std::to_string(pCfg.mainMenu.enablePlayerModel).append(";");

	interfaces->panoramaUIEngine->accessUIEngine()->RunScript(panels.MainMenuContainerPanel, script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
}

void updateBackgroundVideo() noexcept {

std::string script = "var azurreMovie = \"" + pCfg.mainMenu.customBackgroundVideo + "\";\n" +
#include "PanoramaJs/mainMenuVideo.js"
;

interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel), script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
}

void fortniteDance() noexcept{

	// https://raw.githubusercontent.com/stqcky/AimwareScripts/master/Main%20Menu%20Model%20Changer%20%26%20Fortnite%20Dances/MainMenuModelChanger.lua

	if (!Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel)) return;
	if (localPlayer) return;

	std::string script;

	auto isValidModel = [](std::string name) noexcept -> bool
	{
		if (name.empty() || name.front() == ' ' || name.back() == ' ' || !name.ends_with(".mdl"))
			return false;

		if (!name.starts_with("models") && !name.starts_with("/models") && !name.starts_with("\\models"))
			return false;

		//Check if file exists within directory
		std::string path = interfaces->engine->getGameDirectory();
		if (pCfg.dance.customModel[0] != '\\' && pCfg.dance.customModel[0] != '/')
			path += "/";
		path += pCfg.dance.customModel;

		struct stat buf;
		if (stat(path.c_str(), &buf) != -1)
			return true;

		return false;
	};

	if (pCfg.dance.modelIndex == 117 && !isValidModel(pCfg.dance.customModel))
		return;

	if (pCfg.dance.sequence == 0) {
		script += "$.DispatchEvent('HideContentPanel');";
		script += "var vanityPanel = $('#JsMainmenu_Vanity');";
		script += "if (vanityPanel)";
		script += "{";
		script += "vanityPanel.visible = true;";
		script += "vanityPanel.SetScene('azurre/menuPanorama/default.res','";
		if (pCfg.dance.modelIndex == 117)
			script += pCfg.dance.customModel;
		else
			script += fortniteModels[pCfg.dance.modelIndex];
		script += "' , false);";
		script += "vanityPanel.SetSceneModel('";
		if (pCfg.dance.modelIndex == 117)
			script += pCfg.dance.customModel;
		else
			script += fortniteModels[pCfg.dance.modelIndex];
		script += "');";
		script += "vanityPanel.SetPlaybackRateMultiplier(";
		script += "1";
		script += ",";
		script += "1";
		script += "); ";
		script += "}";

		interfaces->panoramaUIEngine->accessUIEngine()->RunScript(panels.MainMenuContainerPanel, script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
		return;
	}

	script += "$.DispatchEvent('HideContentPanel');";
	script += "var vanityPanel = $('#JsMainmenu_Vanity');";
	script += "if (vanityPanel)";
	script += "{";
	script += "vanityPanel.visible = true;";
	script += "vanityPanel.SetScene('azurre/menuPanorama/forniteDances.res','";
	if (pCfg.dance.modelIndex == 117)
		script += pCfg.dance.customModel;
	else
		script += fortniteModels[pCfg.dance.modelIndex];
	script += "' , false);";
	script += "vanityPanel.PlaySequence('";
	script += fortniteSequences[pCfg.dance.sequence];
	script += "', true);";
	script += "vanityPanel.SetPlaybackRateMultiplier(";
	script += std::to_string(static_cast<float>(pCfg.dance.speed) / 100.f);
	script += ",";
	script += std::to_string(static_cast<float>(pCfg.dance.speed) / 100.f);
	script += "); ";
	script += "}";

	interfaces->panoramaUIEngine->accessUIEngine()->RunScript(panels.MainMenuContainerPanel, script.c_str(), "panorama/layout/mainmenu.xml", 8, 10, false, false);
}


static bool windowOpen = false;

void Panorama::menuBarItem() noexcept
{
	if (ImGui::MenuItem("Panorama")) {
		windowOpen = true;
		ImGui::SetWindowFocus("Panorama");
		ImGui::SetWindowPos("Panorama", { 100.0f, 100.0f });
	}
}

void Panorama::drawGUI(bool contentOnly) noexcept
{
	static int tempIndex = 0;
	static int tempDance = 0;
	static int tempSpeed = 100;
	static std::string tempCustom = "models/player/custom_player/";

	if (!contentOnly) {
		if (!windowOpen)
			return;
		ImGui::SetNextWindowSize({ 560.0f, 480.0f });
		ImGui::Begin("Panorama", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	}
	ImGui::PushItemWidth(256.f);

	ImGui::Text("Name Tag Changer");
	ImGui::BeginChild("Nametag Changer", {256.f, 64.f /* use 32.f for one line | 64.f for 2 lines*/}, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::InputText("Name Tag", &pCfg.nameTagString);
	if (ImGui::Button("Apply", {64.f, 24.f})) {
		std::string cmd = ("InventoryAPI.SetNameToolString(`" + pCfg.nameTagString + "`)");
		interfaces->panoramaUIEngine->accessUIEngine()->RunScript(panels.MainMenuContainerPanel, cmd.c_str(), "panorama/layout/base.xml", 8, 10, false, false);
	}
	ImGui::SameLine();
	if (ImGui::Button("How it works?", { 92.f, 24.f })) {
	}
	ImGui::EndChild();

	ImGui::Text("Main Menu Model Changer");
	ImGui::BeginChild("Dancer", { 256.f, 128.f }, true);
	ImGui::Combo("Model", &pCfg.dance.modelIndex, fortniteModelsGUI, ARRAYSIZE(fortniteModelsGUI));

	if(pCfg.dance.modelIndex == 117)
		ImGui::InputText("Custom", &pCfg.dance.customModel);

	ImGui::Combo("Dance", &pCfg.dance.sequence, fortniteSequencesGUI, ARRAYSIZE(fortniteSequencesGUI));
	ImGui::SliderInt("Speed", &pCfg.dance.speed, 25, 400, "%i%");
	if (pCfg.mainMenu.enablePlayerModel &&
		tempIndex != pCfg.dance.modelIndex ||
		tempDance != pCfg.dance.sequence ||
		tempSpeed != pCfg.dance.speed || 
		tempCustom != pCfg.dance.customModel) {
		fortniteDance();
		tempIndex = pCfg.dance.modelIndex;
		tempDance = pCfg.dance.sequence;
		tempSpeed = pCfg.dance.speed;
		tempCustom = pCfg.dance.customModel;
	}
	ImGui::EndChild();

	ImGui::Text("Main Menu");
	ImGui::BeginChild("MainMenu", { 256.f, 144.f }, true);

	static bool toggleNews = true;
	static bool toggleLeft = true;
	static bool toggleRight = true;
	static bool toggleVanity = true;
	static std::string togglecustomBackgroundVideo = "file://{resources}/videos/anubis.webm";

	ImGui::Checkbox("Show News", &pCfg.mainMenu.enableNews);
	ImGui::Checkbox("Show Left Navigation Bar", &pCfg.mainMenu.enableLeftBar);
	ImGui::Checkbox("Show Friends And Party Bar", &pCfg.mainMenu.enableFriendsBar);
	ImGui::Checkbox("Show Player Model", &pCfg.mainMenu.enablePlayerModel);
	ImGui::InputText("Custom Background Video", &pCfg.mainMenu.customBackgroundVideo);
	ImGui::SameLine();
	ImGui::PushID("?0");
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("{resources} leads to \"csgo/panorama/ folder\".\nVideo must be in .webm format!");
	ImGui::PopID();

	if (toggleNews != pCfg.mainMenu.enableNews ||
		toggleLeft != pCfg.mainMenu.enableLeftBar ||
		toggleRight != pCfg.mainMenu.enableFriendsBar ||
		toggleVanity != pCfg.mainMenu.enablePlayerModel) {
		modifygetPanel();
		toggleNews = pCfg.mainMenu.enableNews;
		toggleLeft = pCfg.mainMenu.enableLeftBar;
		toggleRight = pCfg.mainMenu.enableFriendsBar;
		toggleVanity = pCfg.mainMenu.enablePlayerModel;
	}

	if (togglecustomBackgroundVideo != pCfg.mainMenu.customBackgroundVideo) {
		updateBackgroundVideo();
		togglecustomBackgroundVideo = pCfg.mainMenu.customBackgroundVideo;
	}

	ImGui::EndChild();

	if (!contentOnly)
		ImGui::End();
}
