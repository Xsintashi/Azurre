#include "GrenadeHelper.h"
#include <SDK/GameEvent.h>
#include <GameData.h>
#include <iostream>

/*

    This is actually fully made by me

*/

struct GrenadeConfig {
    bool enabled = false, enablesmooth = false, throwinstedofme = false;

    Color4 color{ .77f, .77f, .77f, 1.f };
    Color4 colorEditAdd{ .77f, .77f, .77f, 1.f };
    Color4 colorText{ 1.f, 1.f, 1.f, 1.f };

    Color4 colorHighE{ .79f, .64f, .22f, 1.f };
    Color4 colorSmoke{ .5f, .5f, .5f, 1.f };
    Color4 colorFlash{ .3f, .4f, .29f, 1.f };
    Color4 colorMolot{ .72f, .28f, .23f, 1.f };

    KeyBind throwBind;

    int renderDistance = 500;
    float smoothAngle = 50.f;

    struct WindowGH {
        bool enableGUI = false;
        ImVec2 pos;
        bool noTitleBar = false;
        bool noBackground = false;
    }wgh;
}nadeConfig;

struct NadesStruct {
    /*
    * Modes:
        0 - Stand LL
        1 - Jump LL
        2 - Stand RR
        3 - Jump RR
    * Types:
        0 - Grenade
        1 - Flash
        2 - Smoke
        3 - Molotov
*/
    int mode, type;
    const char* name;
        const char* map;
    Vector pos, aimpos, Angle;
} nadesStructrue;

static int doMode = -1;
static int isIn = -1;

/*
    1 - Manage
    2 - Add
    3 - Edit
*/

static bool windowOpen = false;

std::vector<NadesStruct> nadesVector;

bool Nade::isKeyDown() noexcept {
    return nadeConfig.throwBind.isDown();
}

void printMeToHardcode(int mode, int nade, const char* title, const char* lvlName, Vector origin, Vector lookAt, Vector viewAngles) noexcept {

    std::string sMode;
    std::string sNade;
    switch (mode) {
    default:
        case 0:
            sMode = "Throws::StandLeft";
            break;
        case 1:
            sMode = "Throws::JumpLeft";
            break;
        case 2:
            sMode = "Throws::StandRight";
            break;
        case 3:
            sMode = "Throws::JumpRight";
            break;
    }

    switch (nade) {
    default:
        case 0:
            sNade = "NadesTypes::He";
            break;
        case 1:
            sNade = "NadesTypes::Flash";
            break;
        case 2:
            sNade = "NadesTypes::Smoke";
            break;
        case 3:
            sNade = "NadesTypes::Molotov";
            break;
    }

    std::cout << std::string("{ ").append(sMode).append(", ").append(sNade).append(", \"").append(title).append("\", \"").append(lvlName).append("\", {")
        .append(std::to_string(origin.x).append("f, ")).append(std::to_string(origin.y).append("f, ")).append(std::to_string(origin.z).append("f }, {"))
        .append(std::to_string(lookAt.x).append("f, ")).append(std::to_string(lookAt.y).append("f, ")).append(std::to_string(lookAt.z).append("f }, {"))
        .append(std::to_string(viewAngles.x).append("f, ")).append(std::to_string(viewAngles.y).append("f, ")).append(std::to_string(viewAngles.z).append("f } },\n\n")).c_str();
}

void manageNadesGUI(ImDrawList* drawList) {
    if (isIn < 1) return;
    if (!localPlayer) return;
    if (!GameData::local().exists) return;
    static int mode = 0;
    static int nade = 0;
    static int item_current_idx = -1;
    std::string static title = "";
    std::string static map = "";
    Vector static position = { 0.f, 0.f, 0.f };
    Vector static angle = { 0.f, 0.f, 0.f };
    Vector static lookPoint = { 0.f, 0.f, 0.f };
    ImVec2 vec;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    auto colorc = Helpers::calculateColor(Color4{ 0.f, 0.f, 0.f, 0.f });
    auto colora = Helpers::calculateColor(nadeConfig.colorEditAdd);

    if (static_cast<std::size_t>(item_current_idx) >= nadesVector.size())
        item_current_idx = -1;

    if (!gui->isOpen() && isIn != 2) {
        return;
    }

    GameData::Lock lock;

    ImGui::Begin("Manage Grenade",nullptr,windowFlags);

    switch (isIn) {
    case 1:                                                                     /* Manage Grenades GUI */

        if (ImGui::BeginListBox("##grenade index"))
        {
            for (unsigned int n = 0; n < nadesVector.size(); n++)
            {

                static bool is_selected_for;

                is_selected_for = (item_current_idx == n);
                if (ImGui::Selectable(nadesVector[n].name, is_selected_for))
                    item_current_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected_for)
                    ImGui::SetItemDefaultFocus();

            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Add Grenade")) { isIn = 3; 
        mode = 0;
        nade = 0;
        title = "";    
        } ImGui::SameLine();
        if (!nadesVector.empty() && item_current_idx != -1) {
            if (ImGui::Button("Remove Grenade")) {
                nadesVector.erase(nadesVector.begin() + item_current_idx);
            } ImGui::SameLine();
            if (ImGui::Button("Edit Grenade")) {
               mode = nadesVector[item_current_idx].mode;
               nade = nadesVector[item_current_idx].type;
               title = nadesVector[item_current_idx].name;
               map = nadesVector[item_current_idx].map;
               position = nadesVector[item_current_idx].pos;
               angle = nadesVector[item_current_idx].Angle;
               lookPoint = nadesVector[item_current_idx].aimpos;
               isIn = 2;
            }
            if (ImGui::Button("Back")) { isIn = -1; }

            ImGui::Text("Amount: %d", nadesVector.size());
        }
        if (ImGui::Button("Close")) isIn = -1;
        ImGui::End();
        return;

        break;

    case 2:                                                                     /* Edit Grenade GUI */
        ImGui::PushItemWidth(100.f);
        ImGui::Combo("Mode", &mode, "Stand Left\0Jump Left\0Stand Right\0Jump Right\0");
        ImGui::Combo("Nade", &nade, "Grenade\0Flash\0Smoke\0Molotov\0");
        ImGui::InputText("Title", &title);
        ImGui::InputText("Map", &map);
        ImGui::PopItemWidth();
        ImGui::TextColored(ImVec4{ 1.f, .0f, .0f, 1.f }, "Remeber to shoot to get Angle point!!!");
        ImGui::PushID("posi");
        ImGui::Text("Position: %.1f %.1f %.1f", position.x, position.y, position.z);
        ImGui::SameLine();
        if (ImGui::Button("Correct")) {
            ImGui::OpenPopup("correctPos");
        }
        if (ImGui::BeginPopup("correctPos")) {
            ImGui::PushItemWidth(100.f);
            ImGui::InputFloat("X", &position.x, 0.5f, 2.f);
            ImGui::InputFloat("Y", &position.y, 0.5f, 2.f);
            ImGui::InputFloat("Z", &position.z, 0.5f, 2.f);
            ImGui::PopItemWidth();
            if (ImGui::Button("Get Player Origin")) {
                position = localPlayer->getAbsOrigin();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
        ImGui::PushID("angl");
        ImGui::Text("Angle: %.1f %.1f", angle.x, angle.y);
        ImGui::SameLine();
        if (ImGui::Button("Correct")) {
            ImGui::OpenPopup("correctAngl");
        }
        if (ImGui::BeginPopup("correctAngl")) {
            ImGui::PushItemWidth(100.f);
            ImGui::InputFloat("X", &angle.x, 0.5f, 2.f);
            ImGui::InputFloat("Y", &angle.y, 0.5f, 2.f);
            ImGui::PopItemWidth();
            if (ImGui::Button("Get Player Angle")) {
                angle = interfaces->engine->getViewAngles();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
        ImGui::PushID("corr");
        ImGui::Text("Angle Point: %.1f %.1f %.1f", lookPoint.x, lookPoint.y, lookPoint.z);
        ImGui::PopID();

        if (ImGui::Button("Edit Grenade")) {
            nadesVector.erase(nadesVector.begin() + item_current_idx);
            nadesVector.insert(nadesVector.begin() + item_current_idx,{ mode, nade, title.c_str(), map.c_str(), position, lookPoint, angle});

            isIn = 1;
        }ImGui::SameLine();
        if (ImGui::Button("Back")) {
            isIn = 1;
        }
        ImGui::End();

        if (Helpers::worldToScreen(position, vec)) {
            Helpers::Draw3DFilledCircleFilled(drawList, position, 4.f, colorc);
            const auto centerText = ImGui::CalcTextSize(title.c_str());
            drawList->AddText(ImVec2(vec.x - (centerText.x / 2), vec.y - (centerText.y / 2)), Helpers::calculateColor(nadeConfig.colorText), std::string("\n\n").append(title).c_str());
        }
        if (Helpers::worldToScreen(GameData::local().lookingAt, vec)) {
            drawList->AddCircle(vec, 4, colora, 24);
        }

        return;
    case 3:                                                                     /* Add New Grenade GUI */
        ImGui::PushItemWidth(100.f);
        ImGui::Combo("Mode", &mode, "Stand Left\0Jump Left\0Stand Right\0Jump Right\0");
        ImGui::Combo("Nade", &nade, "High Explosive\0Flash\0Smoke\0Molotov\0");
        ImGui::InputText("Title", &title);
        ImGui::PopItemWidth();
        ImGui::Text("Position: %.1f %.1f %.1f", localPlayer->getAbsOrigin().x, localPlayer->getAbsOrigin().y, localPlayer->getAbsOrigin().z);
        ImGui::Text("Angle: %.1f %.1f ", interfaces->engine->getViewAngles().x, interfaces->engine->getViewAngles().y);
        ImGui::Text("Angle Point: %.1f %.1f %.1f", GameData::local().lookingAt.x, GameData::local().lookingAt.y, GameData::local().lookingAt.z);
#if defined(_DEBUG)
        if (ImGui::Button("Clip")) {
            printMeToHardcode(mode, nade, title.c_str(), interfaces->engine->getLevelName(), localPlayer->getAbsOrigin(), GameData::local().lookingAt, interfaces->engine->getViewAngles());
        }
#endif
        if (ImGui::Button("Add Grenade")) {
            nadesVector.push_back({ mode, nade, title.c_str(), interfaces->engine->getLevelName() , localPlayer->getAbsOrigin(), GameData::local().lookingAt, interfaces->engine->getViewAngles() });
            isIn = 1;
        }ImGui::SameLine();
        if (ImGui::Button("Back")) {
            isIn = 1;
        }
        ImGui::End();

        if (Helpers::worldToScreen(localPlayer->getAbsOrigin(), vec)) {
            Helpers::Draw3DFilledCircleFilled(drawList, position, 4.f, colorc);

            const auto centerText = ImGui::CalcTextSize(title.c_str());
            drawList->AddText(ImVec2(vec.x - (centerText.x / 2), vec.y - (centerText.y / 2)), Helpers::calculateColor(nadeConfig.colorText) , std::string("\n\n").append(title).c_str());

        }
        if (Helpers::worldToScreen(GameData::local().lookingAt, vec)) {
            drawList->AddCircle(vec, 4, colora, 24);
        }

        return;
    }
    ImGui::End();
    //DO NOT ADD NOTHING MORE HERE
}

void Nade::Run(ImDrawList* drawList) noexcept {
    static bool pushNades = true;

    GameData::Lock lock;

    if (pushNades) {
        nadesVector.insert(nadesVector.end(), {

            //examples // de_mirage
            { Throws::JumpLeft, NadesTypes::Smoke, "Market Window", "de_mirage", {-160.f, 888.f, -135.f}, {-749.25f, 497.f, 776.f}, {-50.2f, -146.4f, 0.f}},
            { Throws::JumpLeft, NadesTypes::Smoke, "Market Door", "de_mirage", {-161.0f, 451.1f, -69.9f }, {-315.f, 408.9f, 101.1f}, {-33.7f, -164.6f, 0.f } }, //back alley
            { Throws::JumpLeft, NadesTypes::Smoke, "Short", "de_mirage", { 1359.f, 222.6f, -195.f }, { 1093.5f, 216.135f, 91.25f }, {-39.9f, -178.6f, 0.f } }, //Tspawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Sniper Window", "de_mirage", { 1127.97f, -1208.06f, -108.969f }, { 689.f, -1103.5f, 190.67f }, { -27.424f, 166.682f, 0.f } }, //roof
            { Throws::JumpLeft, NadesTypes::Smoke, "Connector", "de_mirage", {1135.97f, 647.976f, -261.4f }, { 865.f, 424.f, -13.385f }, {-27.6499f, -140.388f, 0.f } }, //Stairs TV
            { Throws::JumpLeft, NadesTypes::Smoke, "Ramp", "de_mirage", {-2026.4f, -2030.f, -299.1f }, { -1440.f, -1900.f, -60.f }, {-16.3f, 12.5f, 0.f } }, //CT Spawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Palace", "de_mirage", {-2031.97f, -1767.93f, -300.43f }, { -1440.f, -1889.5f, 137.f }, {-31.7553f, -11.5643f, 0.f } }, //CT Spawn
            { Throws::StandLeft, NadesTypes::Smoke, "Stairs", "de_mirage", {1148.49f, -1183.97f, -205.572f }, { 1006.2f, -1222.f, -4.8f }, {-42.8804f, -165.021f, 0.f } }, //CT Spawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Palace Alley", "de_mirage", {-1547.98f, -2407.97f, -240.021f}, { -1236.f, -2216.f, -15.f }, {-23.6702f, 31.5518f, 0.f } }, //CT Spawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Jungle", "de_mirage", {1239.51f, -1023.27f, -259.044f }, { 983.5f, -1086.25f, -67.25f }, {-25.8531f, -166.14f, 0.f } }, //T Spawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Ticket", "de_mirage", {1258.18f, -871.97f, -143.97f }, { 1063.f, -1011.f, 20.25f }, {-22.71f, -144.557f, 0.f } }, //T Spawn
            { Throws::JumpLeft, NadesTypes::Smoke, "Top Ticket", "de_mirage", {983.91f, -1032.0f, -256.1f} , { 554.f, -1371.f, 71.f },  {-25.65f, -141.7f,  0.f} }, //T Alley
            { Throws::JumpLeft, NadesTypes::Smoke, "Back Alley", "de_mirage", {-716.6f, -1569.5f, -168.f} , { -717.25f, -1296.f, 110.75f },  {-38.1f, 90.2f,  0.f} },

            //One ways
            { Throws::StandLeft, NadesTypes::Smoke, "Window | One-Way", "de_mirage", {-1248.f, -924.0f, -167.97f }, { -1152.5f, -780.f, -167.f }, {20.f, 56.5f, 0.f } },
            { Throws::StandRight, NadesTypes::Smoke, "Connector | One-Way", "de_mirage", {-807.969f, -1235.97f, -167.969f }, { -719.5f, -1015.f, -55.f }, {-11.68f, 68.20f, 0.f } },
            { Throws::StandLeft, NadesTypes::Smoke, "Stairs | One-Way", "de_mirage", {-496.023f, -1309.03f, -159.969f }, { -506.f, -1291.f, 90.f }, {-83.6357f, 119.024f, 0.f } },
            { Throws::StandLeft, NadesTypes::Molotov, "Sniper Window | One-Way", "de_mirage", {-1196.71f, -856.0f, -168.0f}, { -1134.75f, -456.f, -9.f }, {-13.26f, 81.23f, 0.f } },
            
            //examples // de_inferno
            { Throws::JumpLeft, NadesTypes::Smoke, "CT", "de_inferno", {-79.4487f, 1330.03f, 106.709f }, {-11.3f, 1434.f, 192.1f}, {-9.7f, 56.8f, 0.f}},
            { Throws::StandLeft, NadesTypes::Smoke, "Coffins", "de_inferno", {110.84f, 1569.6f, 132.0f }, {132.8f, 1661.5f, 290.8f}, {-45.0f, 76.5f, 0.f }},
            { Throws::StandLeft, NadesTypes::Molotov, "Quad", "de_inferno", {1000.f, 1862.6f, 150.7f }, {707.1f, 2102.5f, 400.f}, {-26.1f, 140.6f, 0.f }},
            { Throws::JumpLeft, NadesTypes::Smoke, "Pit", "de_inferno", {538.031250f, 699.968750f, 93.837944f }, {1184.727295f, 458.031250f, 307.882751f }, {-12.255835f, -20.514847f, 0.000000f } },
            { Throws::JumpLeft, NadesTypes::Smoke, "Mid", "de_inferno", {2537.715088f, 2120.964355f, 132.031250f }, {2215.117432f, 1727.031250f, 342.322144f }, {-16.028822f, -129.315842f, 0.000000f } },
            { Throws::JumpLeft, NadesTypes::Smoke, "Pit", "de_inferno", {1314.031250f, 829.968750f, 148.635727f }, {1533.968750f, 644.907654f, 327.007965f }, {-21.688307f, -40.078587f, 0.000000f } },

            //examples // de_nuke
            { Throws::StandLeft, NadesTypes::Flash, "Block Door Decoy", "de_nuke", {815.968750f, -1759.968750f, -415.968750f }, {351.477173f, -1200.031250f, -312.878601f }, {-3.076134f, 129.677338f, 0.000000f } },
            { Throws::StandLeft, NadesTypes::Molotov, "Above Hat", "de_nuke", {-14.793541f, -994.199585f, -415.968750f }, {208.540344f, -974.924072f, -112.031250f }, {-46.943497f, 4.937346f, 0.000000f } },
            { Throws::StandLeft, NadesTypes::Molotov, "Hut", "de_nuke", {-31.685936f, -1469.628418f, -44.519428f }, {254.968750f, -1305.675903f, 81.092445f }, {-10.561006f, 29.769655f, 0.000000f } },
            { Throws::StandLeft, NadesTypes::Molotov, "Heaven Stairs | Crouch", "de_nuke", {167.968750f, -1441.864380f, -91.409798f }, {254.968750f, -1368.973145f, 78.067146f }, {-47.405525f, 39.960636f, 0.000000f } },
            { Throws::JumpLeft, NadesTypes::Smoke, "Top Blue Box", "de_nuke", {63.870182f, -1430.713989f, -89.968750f }, {254.968750f, -1444.994507f, 113.754898f }, {-36.086517f, -4.275842f, 0.000000f } },
            { Throws::StandLeft, NadesTypes::Smoke, "Main", "de_nuke", {-181.525284f, -595.923950f, -105.968750f }, {182.793777f, -805.968750f, 152.470398f }, {-24.808571f, -29.966917f, 0.000000f } },
            
            });

        pushNades = false;
    }

    if (!nadeConfig.enabled) return;
    if (!interfaces->engine->isConnected() || !localPlayer) return;
    if (!localPlayer->isAlive()) return;

    manageNadesGUI(ImGui::GetBackgroundDrawList());

    const auto color = Helpers::calculateColor(nadeConfig.color);
    const auto colorText = Helpers::calculateColor(nadeConfig.colorText);

    ImVec2 vec;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (nadeConfig.wgh.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (nadeConfig.wgh.noBackground)
        windowFlags |= ImGuiWindowFlags_NoBackground;

    std::string modeName;

    for (unsigned int idx = 0; idx < nadesVector.size(); idx++) {
        float Distance = localPlayer->getAbsOrigin().distTo(nadesVector[idx].pos);

        if (fnv::hash(nadesVector[idx].map) != fnv::hashRuntime(interfaces->engine->getLevelName())) continue;

        if (Distance > nadeConfig.renderDistance) continue;

        if (Helpers::worldToScreen(nadesVector[idx].pos, vec)) { // all
            if (Distance > 1) {
                auto colorc = Helpers::calculateColor(Color4{ 0.f, 0.f, 0.f, 0.f });

                switch (nadesVector[idx].type) {
                default:
                case 0: colorc = Helpers::calculateColor(nadeConfig.colorHighE); break;
                case 1: colorc = Helpers::calculateColor(nadeConfig.colorFlash); break;
                case 2: colorc = Helpers::calculateColor(nadeConfig.colorSmoke); break;
                case 3: colorc = Helpers::calculateColor(nadeConfig.colorMolot); break;
                }
                Helpers::Draw3DFilledCircleFilled(drawList, nadesVector[idx].pos, 4.f, colorc);
                const auto centerText = ImGui::CalcTextSize(nadesVector[idx].name);
                drawList->AddText(ImVec2(vec.x - (centerText.x / 2), vec.y -(centerText.y / 2)), colorText, std::string("\n\n").append(nadesVector[idx].name).c_str());
            }
            else { // selected
                Helpers::Draw3DFilledCircleFilled(drawList, nadesVector[idx].pos, 4.f, color);
                /*if (nadeConfig.wgh.enableGUI) {
                    switch (nadesVector[idx].mode) {
                    case 0:  modeName = "Standing"; break;
                    case 1:  modeName = "Jump"; break;
                    case 2:  modeName = "Running"; break;
                    case 3:  modeName = "JumpRunning"; break;
                    case 8:  modeName = "Standing | Right Click"; break;
                    default:  modeName = "Micromovement"; break;
                    }
                    ImGui::SetNextWindowBgAlpha(0.3f);
                    ImGui::Begin("Grenade Helper", nullptr, windowFlags);
                    ImGui::Text("To: %s", nadesVector[idx].name);
                    ImGui::Text("Mode: %s", modeName.c_str());
                    ImGui::Text("Grenade: %s", "Smoke");
                    ImGui::End();
                }*/
            }
            if (nadeConfig.wgh.pos != ImVec2{}) {
                ImGui::SetNextWindowPos(nadeConfig.wgh.pos);
                nadeConfig.wgh.pos = {};
            }
        }
        if (Helpers::worldToScreen(nadesVector[idx].aimpos, vec)) {

            float xViewAngles = interfaces->engine->getViewAngles().x - nadesVector[idx].Angle.x;
            float yViewAngles = interfaces->engine->getViewAngles().y - nadesVector[idx].Angle.y;

            //std::cout << "x: " << xViewAngles << std::endl << "y: " << yViewAngles << std::endl;;

            if (Distance < 1) { // Adjusts aim angles
                if (nadeConfig.enablesmooth && (xViewAngles < 10.f && xViewAngles > -10.f) && (yViewAngles < 10.f && yViewAngles > -10.f) && localPlayer->getActiveWeapon()->isGrenade())
                    interfaces->engine->setViewAngles({ interfaces->engine->getViewAngles().x - (xViewAngles / nadeConfig.smoothAngle), interfaces->engine->getViewAngles().y - (yViewAngles / nadeConfig.smoothAngle), 0.f });

                if ((xViewAngles < .25f && xViewAngles > -.25f) && (yViewAngles < .25f && yViewAngles > -.25f) && nadeConfig.throwBind.isDown() || !nadeConfig.throwBind.isSet()) //execute jump
                    doMode = nadesVector[idx].mode;
            }
            if ((xViewAngles < .25f && xViewAngles > -.25) && (yViewAngles < .25 && yViewAngles > -.25) && (Distance < 1)) {
                drawList->AddCircleFilled(vec, 4, color, 24);
            }
            else {
                drawList->AddCircle(vec, 4, color, 24);
            }
        }
    }
    return;
}

void Nade::Move(UserCmd* cmd) noexcept {
    if (doMode == -1) return;

    if (!localPlayer->getActiveWeapon()->isGrenade() || (localPlayer->getActiveWeapon()->isGrenade() && !localPlayer->getActiveWeapon()->pinPulled())) {
        doMode = -1;
        return;
    }

    switch (doMode)
    {
    default: doMode = -1; break;
    case 0: cmd->buttons &= ~(UserCmd::IN_ATTACK ); doMode = -1; break;
    case 1: cmd->buttons |= (UserCmd::IN_JUMP); cmd->buttons &= ~(UserCmd::IN_ATTACK ); doMode = -1; break;
    case 2: cmd->buttons &= ~(UserCmd::IN_ATTACK2); doMode = -1; break;
    case 3: cmd->buttons |= (UserCmd::IN_JUMP); cmd->buttons &= ~(UserCmd::IN_ATTACK2); doMode = -1; break;
    }
}

void Nade::drawGUI(bool contentOnly) noexcept {
    //ImGui::Checkbox("Show Window", &nadeConfig.wgh.enableGUI);
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        ImGui::Begin("Grenade Helper", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    }

    ImGui::BeginChild("Main", ImVec2(304, 128), true);
        ImGui::Checkbox("Enabled", &nadeConfig.enabled);
        ImGui::SameLine();
        ImGui::hotkey("", nadeConfig.throwBind);
        ImGui::Checkbox("Enable Smoothing", &nadeConfig.enablesmooth);

        if (nadeConfig.enablesmooth)
            ImGui::SliderFloat("##SmoothAngle", &nadeConfig.smoothAngle, 1.f, 100.f, "Smooth Angle: %2.f");

        ImGui::SliderInt("##renderdistance", &nadeConfig.renderDistance, 50, 2000, "Render Distance: %d");
    ImGui::EndChild();

    ImGui::BeginChild("Colors", ImVec2(304, 128), true); 
        ImGuiCustom::colorPicker("Main Color", nadeConfig.color, nullptr);
        ImGuiCustom::colorPicker("Text Color", nadeConfig.colorText, nullptr);
        ImGuiCustom::colorPicker("HE Granade Color", nadeConfig.colorHighE, nullptr);
        ImGuiCustom::colorPicker("Flash Grenade Color", nadeConfig.colorFlash, nullptr);
        ImGuiCustom::colorPicker("Smoke Grenade Color", nadeConfig.colorSmoke, nullptr);
        ImGuiCustom::colorPicker("Molotov Color", nadeConfig.colorMolot, nullptr);
    ImGui::EndChild();

    if (localPlayer || nadeConfig.enabled ) {
        if (ImGui::Button("Manage Grenades", { 128, 32 })) {
            isIn = nadeConfig.enabled;
        }
    }  

    if (!contentOnly)
        ImGui::End();
}

void Nade::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Nade Helper")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Grenade Helper");
        ImGui::SetWindowPos("Grenade Helper", { 100.0f, 100.0f });
    }
}

static void from_json(const json& j, ImVec2& v)
{
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, GrenadeConfig& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Enabled Smooth", o.enablesmooth);
    read<value_t::object>(j, "Main Color", o.color);
    read<value_t::object>(j, "Text Color", o.colorText);
    read<value_t::object>(j, "HE Color", o.colorHighE);
    read<value_t::object>(j, "Smoke Color", o.colorSmoke);
    read<value_t::object>(j, "Flash Color", o.colorFlash);
    read<value_t::object>(j, "Molotov Color", o.colorMolot);
    read(j, "Throw KeyBind", o.throwBind);
    read(j, "Smooth Angle Power", o.smoothAngle);
    read(j, "Render Distance", o.renderDistance);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const NadesStruct& p)
{
    j = { 
        {"Mode", p.mode},
        {"Type", p.type},
        {"Name", p.name},
        {"Map", p.map},
        {"Pos", {"X" , p.pos.x},{"Y" , p.pos.y},{"Z" , p.pos.z}},
        {"Aim Point", {"X" , p.aimpos.x},{"Y" , p.aimpos.y},{"Z" , p.aimpos.z}},
        {"Angle", {"X" , p.Angle.x},{"Y" , p.Angle.y}}
    };
}

static void to_json(json& j, const GrenadeConfig& o)
{
    const GrenadeConfig dummy;

    WRITE("Enabled", enabled);
    WRITE("Enabled Smooth", enablesmooth);
    WRITE("Main Color", color);
    WRITE("Text Color", colorText);
    WRITE("HE Color", colorHighE);
    WRITE("Smoke Color", colorSmoke);
    WRITE("Flash Color", colorFlash);
    WRITE("Molotov Color", colorMolot);
    WRITE("Throw KeyBind", throwBind);
    WRITE("Smooth Angle Power", smoothAngle);
    WRITE("Render Distance", renderDistance);

}

json Nade::toJson() noexcept
{
    json j;
    to_json(j, nadeConfig);
    return j;
}

void Nade::fromJson(const json& j) noexcept
{
    from_json(j, nadeConfig);
}

void Nade::resetConfig() noexcept
{
    nadeConfig = {};
}