#pragma once

class UIPanel;
class IUIEvent;

namespace Panorama {

	enum CSGOPanel {
		CSGOHud = 0,
		MainMenuContainerPanel,
		CSGOMainMenu
	};

	UIPanel*& getPanel(CSGOPanel panel);
	UIPanel*& getPanel(int panel);
	IUIEvent* createPanoramaEvent(const std::string& eventName, bool cacheOnly = false);
	UIPanel* tryGetPanels(CSGOPanel panel) noexcept;

	void executeScript(UIPanel* panel, const char* script, const char* pathXML = "panorama/layout/mainmenu.xml") noexcept;

	void menuBarItem() noexcept;
	void drawGUI(bool contentOnly) noexcept;
}