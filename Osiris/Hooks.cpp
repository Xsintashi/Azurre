#include "Hooks.h"
#include <Hooks/detours.h>

#include "Config.h"
#include "EventListener.h"
#include "GameData.h"
#include "GUI.h"
#include "Interfaces.h"

#include "DiscordSDK/RPC.h"

#pragma region SDK Includes
#include "SDK/ClientClass.h"
#include "SDK/Cvar.h"
#include "SDK/Client.h"
#include "SDK/ClientMode.h"
#include "SDK/ClientState.h"
#include "SDK/Constants/ClassId.h"
#include "SDK/Constants/FrameStage.h"
#include "SDK/Constants/UserMessages.h"
#include "SDK/CStudioHdr.h"
#include "SDK/Engine.h"
#include "SDK/Entity.h"
#include "SDK/EntityList.h"
#include "SDK/GameEvent.h"
#include "SDK/GameMovement.h"
#include "SDK/GameUI.h"
#include "SDK/GlobalVars.h"
#include "SDK/isteamfriends.h"
#include "SDK/Input.h"
#include "SDK/InputSystem.h"
#include "SDK/ItemSchema.h"
#include "SDK/KeyValues.h"
#include "SDK/LocalPlayer.h"
#include "SDK/MaterialSystem.h"
#include "SDK/ModelRender.h"
#include "SDK/NetworkChannel.h"
#include "SDK/ParticleCollection.h"
#include "SDK/Panorama.h"
#include "SDK/Platform.h"
#include "SDK/Prediction.h"
#include "SDK/RenderContext.h"
#include "SDK/SoundInfo.h"
#include "SDK/SoundEmitter.h"
#include "SDK/StudioRender.h"
#include "SDK/SteamAPI.h"
#include "SDK/Surface.h"
#include "SDK/UserCmd.h"
#include "SDK/ViewSetup.h"
#include "SDK/weaponVisualsData.h"
#pragma endregion

#pragma region Hack Includes
#include "Hacks/ProfileChanger/Protobuffs.h"
#include "Hacks/LegitAimbot.h"
#include "Hacks/RageAimbot.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Chams.h"
#include "Hacks/EnginePrediction.h"
#include "Hacks/StreamProofESP.h"
#include "Hacks/Glow.h"
#include "Hacks/GrenadePrediction.h"
#include "Hacks/Misc.h"
#include "Hacks/Sound.h"
#include "Hacks/Triggerbot.h"
#include "Hacks/Visuals.h"
#include "Hacks/Movement.h"
#include "Hacks/Extra.h"
#include "Hacks/Animations.h"
#include "Hacks/FakeLag.h"
#include "Hacks/Grief.h"
#include "Hacks/Tickbase.h"
#include "Hacks/Resolver.h"
#include "Hacks/GrenadeHelper.h"
#include "Hacks/Walkbot.h"
#include "Hacks/MovementRecorder.h"
#include "Hacks/Xsin.h"
#include "Hacks/Panorama.h"
#include "Hacks/Notifications.h"
#include "Hacks/ProfileChanger.h"
#include "Hacks/Radar.h"
#include "Hacks/Lobby.h"
#include "Hacks/Clantag.h"
#include "Hacks/KnifeBot.h"
#pragma endregion

//#define findMDLHook

#if defined(_DEBUG) // ALL BROKEN TO DEBUG
//#define ENABLEFIXANIM
#else
#define DISCORDRICHPRESENSE
//#define ENABLEFIXANIM
#endif

constexpr float pi = std::numbers::pi_v<float>;

#ifdef _WIN32

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT STDCALL wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    [[maybe_unused]] static const auto once = [](HWND window) noexcept {
        printf("[+] Initializing\n");

        Netvars::init();
        EventListener::init();
#if defined(DISCORDRICHPRESENSE)
        Discord::Run();
#endif
        Misc::initHiddenCvars();
        ImGui::CreateContext();
        GameData::createNeededThirdPartyFiles();
        ImGui_ImplWin32_Init(window);
        config.emplace(Config{});
        gui.emplace(GUI{});

        hooks->install();
        printf("[+] Instlling Completed\n");
        std::cout << std::endl << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl << "Console Logs: " << std::endl;
#if defined(_DEBUG)
        const bool loaded = config->load(u8"debug", false, false);
        
#else
        const bool loaded = config->load(u8"default", false, false);
#endif

#if !defined(_DEBUG)
        static std::string welcomeMsg;
        welcomeMsg += "Let's get started!\n";
        welcomeMsg += "To open GUI press Insert key\n";
        welcomeMsg += "on your keyboard.\n\n";
        welcomeMsg += "Configs are stored in \"Documents/azurre/config\" directory.\n\n";

        welcomeMsg += (loaded ? "Default config has been loaded!" : "No default Config to load :(");

        static std::string welcomeName;
        welcomeName += "Welcome ";
        welcomeName += steamIFriends->GetPersonaName();
        welcomeName += " (￣▽￣)";

        interfaces->gameUI->createCommandMsgBox(welcomeName.c_str(), welcomeMsg.c_str());
#endif


        return true;
    }(window);

    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);

    interfaces->inputSystem->enableInput(config->style.blockInput || !interfaces->engine->isInGame() ? !gui->isOpen() : true); // enable moving while gui is open

    return CallWindowProcW(hooks->originalWndProc, window, msg, wParam, lParam);

}

#pragma optimize( "", off)
#pragma runtime_checks("", off)
static bool STDCALL getUnverifiedFileHashes(void* thisPointer, int maxFiles)
{
    if (Misc::shouldEnableSvPureBypass())
        return false;
    return hooks->fileSystem.callOriginal<bool, 101>(thisPointer, maxFiles);
}

static int FASTCALL canLoadThirdPartyFiles(void* thisPointer, void* edx) noexcept
{
    if (Misc::shouldEnableSvPureBypass())
        return 1;
    static auto original = hooks->fileSystem.getOriginal<int, 128>();

    return original(thisPointer);
}

#pragma runtime_checks("", restore)
#pragma optimize("", on)

static HRESULT STDCALL reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    InventoryChanger::clearItemIconTextures();
    GameData::clearTextures();
    return hooks->originalReset(device, params);
}

#endif

#ifdef _WIN32
static HRESULT STDCALL present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{

    [[maybe_unused]] static bool imguiInit{ ImGui_ImplDX9_Init(device) };

    if (config->loadScheduledFonts())
        ImGui_ImplDX9_DestroyFontsTexture();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
#else
static void swapWindow(SDL_Window * window) noexcept
{
    [[maybe_unused]] static const auto _ = ImGui_ImplSDL2_InitForOpenGL(window, nullptr);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
#endif
    ImGui::NewFrame();

    if (const auto& displaySize = ImGui::GetIO().DisplaySize; displaySize.x > 0.0f && displaySize.y > 0.0f) {
        Walkbot::drawPath();
        Walkbot::drawPathfinding();
        Walkbot::drawPresetNodes();
        NadePrediction::draw();
        StreamProofESP::render();
        Extra::visualizeQuickPeek(ImGui::GetBackgroundDrawList());
        Movement::Checkpoints();
        Movement::drawKeyStrokes(ImGui::GetBackgroundDrawList());
        Movement::showVelocity(ImGui::GetBackgroundDrawList());
        Movement::showMouseMove();
        MovementRecorder::drawStartpoints(ImGui::GetBackgroundDrawList());
        AntiAim::Visualize(ImGui::GetBackgroundDrawList());
        Misc::drawOffscreenEnemies(ImGui::GetBackgroundDrawList());
        Misc::visualizeAccuracy(ImGui::GetBackgroundDrawList());
        Misc::drawAimbotFov(ImGui::GetBackgroundDrawList());
        Misc::guiVoteRevealer();
        Misc::purchaseList();
        Misc::drawBombTimer();
        Misc::hurtIndicator();
        Misc::indicators();
        Misc::spectatorList();
        Misc::colorPaletteWindow();
        Misc::watermark();
        Misc::showBinds();
        Misc::drawPlayerList();
        Misc::drawSmokeTimer(ImGui::GetBackgroundDrawList());
        Misc::drawMolotovTimer(ImGui::GetBackgroundDrawList());
        Visuals::customScopeCrosshair(ImGui::GetBackgroundDrawList());
        Visuals::damageNumber(ImGui::GetBackgroundDrawList());
        Visuals::shadowChanger();
        Visuals::fullBright();
        Visuals::noHUDBlur();
        Visuals::hitMarker(nullptr, ImGui::GetBackgroundDrawList());
        Visuals::drawMolotovPolygon(ImGui::GetBackgroundDrawList());
        Visuals::drawSmokeHull(ImGui::GetBackgroundDrawList());
        Nade::Run(ImGui::GetBackgroundDrawList());
        Xsin::debugBox(ImGui::GetBackgroundDrawList());
        AntiAim::lines(ImGui::GetBackgroundDrawList());
        Xsin::drawDebugWindow();
        Notifications::draw(ImGui::GetBackgroundDrawList());
#if defined(DISCORDRICHPRESENSE)
        Discord::Update(); // i know -_- ;_;
#endif
        Glow::changeGlowThickness();
        RageAimbot::updateInput();
        LegitAimbot::updateInput();
        Visuals::updateInput();
        StreamProofESP::updateInput();
        Misc::updateInput();
        Movement::updateInput();
        Triggerbot::updateInput();
        AntiAim::updateInput();
        Chams::updateInput();
        Glow::updateInput();
        Extra::updateInput();
        Xsin::javaScriptConsole();
#if defined(_DEBUG)
        Xsin::backgroundDebugDraw(ImGui::GetBackgroundDrawList());
#endif
//#if defined(_DEBUG)
//        if (Radar::should_update_radar)
//            Radar::OnMapLoad(interfaces->engine->getLevelName());
//        else
//            Radar::render();
//#endif
        gui->handleToggle();

        if (gui->isOpen())
            gui->render();

		if(config->style.banner && (gui->isOpen() || Misc::isInChat() || interfaces->engine->isConsoleVisible()))
            gui->baner(ImGui::GetBackgroundDrawList(), displaySize);

#if defined(_DEBUG)
      //gui->debugpic(ImGui::GetBackgroundDrawList(), displaySize);
#endif
    }

    ImGui::EndFrame();
    ImGui::Render();

#ifdef _WIN32
    if (device->BeginScene() == D3D_OK) {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        device->EndScene();
    }
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

    GameData::clearUnusedAvatars();
    InventoryChanger::clearUnusedItemIconTextures();
#ifdef _WIN32
    return hooks->originalPresent(device, src, dest, windowOverride, dirtyRegion);
#else
    hooks->swapWindow(window);
#endif
}

static void __fastcall packetStart(void* thisPointer, void* edx, int incomingSequence, int outgoingAcknowledged) noexcept
{
    Animations::packetStart();

    return hooks->clientState.callOriginal<void, 5>(incomingSequence, outgoingAcknowledged);
}

static void __fastcall processPacket(void* thisPointer, void* edx, void* packet, bool header) noexcept
{
    static auto original = hooks->clientState.getOriginal<void>(packet, header);
    if (!memory->clientState->netChannel)
        return original(thisPointer, packet, header);

    original(thisPointer, packet, header);

    for (auto it{ memory->clientState->pEvents }; it != nullptr; it = it->m_next) {
        if (!it->class_id)
            continue;

        // set all delays to instant.
        it->fire_delay = 0.f;
    }

    interfaces->engine->fireEvents();
}

static void FASTCALL SettingsChatText(void* thisPointer, void* edx, const char* text)
{
    static auto original = hooks->SFUIHook.getOriginal<void>(text);
    
    //    "SFUI_Settings_Chat_ButtonLabel" "Send"
    //    "SFUI_Settings_Chat_Say" "Say to all"
    //    "SFUI_Settings_Chat_SayTeam" "Say to team"
    //    "SFUI_Settings_Chat_SayParty" "Say to party"
    //    "SFUI_Settings_Chat_EnableGotv" "GOTV Chat"
    //    "SFUI_Settings_Chat_NotePartyChat" "[PARTY]"
    //    "SFUI_Settings_Chat_NotePartyInvited" "invited"

    switch (fnv::hash(text))
    {
    case fnv::hashConst("#SFUI_Settings_Chat_Say"):
        return original(thisPointer, "Trashtalk here");

    case fnv::hashConst("#SFUI_Settings_Chat_SayTeam"):
    case fnv::hashConst("#SFUI_Settings_Chat_SayParty"):
        return original(thisPointer, "Info here");
    }

    original(thisPointer, text);
}

static void FASTCALL particleCollectionSimulateHook(ParticleCollection* thisPointer, void* edx) {

    static auto original = hooks->particleCollectionSimulate.getOriginal<bool>();

    original(thisPointer);
    if (!interfaces->engine->isConnected())
        return;

    ParticleCollection* rootCollection = thisPointer;
    while (rootCollection->parent)
        rootCollection = rootCollection->parent;

    const char* rootName = rootCollection->def.object->name.buffer;

    //printf(std::string(rootName).append("\n").c_str()); //prints existing particles

    switch (fnv::hash(rootName)) {
        case fnv::hash("molotov_groundfire"):
        case fnv::hash("molotov_groundfire_00MEDIUM"):
        case fnv::hash("molotov_groundfire_00HIGH"):
        case fnv::hash("molotov_groundfire_fallback"):
        case fnv::hash("molotov_groundfire_fallback2"):
        case fnv::hash("molotov_explosion"):
        case fnv::hash("explosion_molotov_air"):
        case fnv::hash("extinguish_fire"):
        case fnv::hash("weapon_molotov_held"):
        case fnv::hash("weapon_molotov_fp"):
        case fnv::hash("weapon_molotov_thrown"):
        case fnv::hash("incgrenade_thrown_trail"):
            switch (fnv::hash(thisPointer->def.object->name.buffer))
            {
            case fnv::hash("explosion_molotov_air_smoke"):
            case fnv::hash("molotov_smoking_ground_child01"):
            case fnv::hash("molotov_smoking_ground_child02"):
            case fnv::hash("molotov_smoking_ground_child02_cheapo"):
            case fnv::hash("molotov_smoking_ground_child03"):
            case fnv::hash("molotov_smoking_ground_child03_cheapo"):
            case fnv::hash("molotov_smoke_screen"):
                break;
            default:
                if (Visuals::colorMolotov().enabled) {

                    for (int i = 0; i < thisPointer->activeParticles; i++) {
                        float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                        if (Visuals::colorMolotov().rainbow) {
                            color[0] = std::sin(Visuals::colorMolotov().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
                            color[4] = std::sin(Visuals::colorMolotov().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f;
                            color[8] = std::sin(Visuals::colorMolotov().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f;
                        }
                        else {
                            color[0] = Visuals::colorMolotov().color[0];
                            color[4] = Visuals::colorMolotov().color[1];
                            color[8] = Visuals::colorMolotov().color[2];
                        }
                        float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                        *alpha = Visuals::colorMolotov().color[3];
                    }

                }
                break;
            }
        break;
        case fnv::hash("explosion_smokegrenade_fallback"): {
            if (Visuals::colorSmoke().enabled) {

                for (int i = 0; i < thisPointer->activeParticles; i++) {
                    float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                    if (Visuals::colorSmoke().rainbow) {
                        color[0] = std::sin(Visuals::colorSmoke().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
                        color[4] = std::sin(Visuals::colorSmoke().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f;
                        color[8] = std::sin(Visuals::colorSmoke().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f;
                    }
                    else {
                        color[0] = Visuals::colorSmoke().color[0];
                        color[4] = Visuals::colorSmoke().color[1];
                        color[8] = Visuals::colorSmoke().color[2];
                    }
                    float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                    *alpha = Visuals::colorSmoke().color[3];
                }

            }
        }
        break;
        case fnv::hash("weapon_tracers_taser_fallback2"): 
        case fnv::hash("weapon_muzzle_flash_taser_fallback2"): {
            if (Visuals::colorZeusLaser().enabled) {

                for (int i = 0; i < thisPointer->activeParticles; i++) {
                    float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                    if (Visuals::colorZeusLaser().rainbow) {
                        color[0] = std::sin(Visuals::colorZeusLaser().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
                        color[4] = std::sin(Visuals::colorZeusLaser().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f;
                        color[8] = std::sin(Visuals::colorZeusLaser().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f;
                    }
                    else {
                        color[0] = Visuals::colorZeusLaser().color[0];
                        color[4] = Visuals::colorZeusLaser().color[1];
                        color[8] = Visuals::colorZeusLaser().color[2];
                    }
                    float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                    *alpha = Visuals::colorZeusLaser().color[3];
                }

            }
        }
        break;
        case fnv::hash("impact_helmet_headshot"): {
            if (Visuals::colorHeadShot().enabled) {

                for (int i = 0; i < thisPointer->activeParticles; i++) {
                    float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                    if (Visuals::colorHeadShot().rainbow) {
                        color[0] = std::sin(Visuals::colorHeadShot().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
                        color[4] = std::sin(Visuals::colorHeadShot().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f;
                        color[8] = std::sin(Visuals::colorHeadShot().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f;
                    }
                    else {
                        color[0] = Visuals::colorHeadShot().color[0];
                        color[4] = Visuals::colorHeadShot().color[1];
                        color[8] = Visuals::colorHeadShot().color[2];
                    }
                    float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                    *alpha = Visuals::colorHeadShot().color[3];
                }

            }
        }
        break;
    }
}

static void FASTCALL removeScopeHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->removeScope.getOriginal<void>();

    if (false/*is true when disable rendering scope overall*/)
        return;

    original(thisPointer);
}

static int FASTCALL sendDatagramHook(NetworkChannel* network, void* edx, void* datagram)
{
    static auto original = hooks->sendDatagram.getOriginal<int>(datagram);
    if (Backtrack::fakeLatencyAmountRecive() <= 0 || datagram || !interfaces->engine->isInGame() || (((*memory->gameRules)->freezePeriod() || !localPlayer->isAlive()) && Backtrack::DisableFakeLagWhileBeingDead()))
        return original(network, datagram);

    const int instate = network->inReliableState;
    const int insequencenr = network->inSequenceNr;

    const float delta = std::max(0.f, (Backtrack::fakeLatencyAmountRecive() / 1000.f));

    Backtrack::addLatencyToNetwork(network, delta);

    int result = original(network, datagram);

    network->inReliableState = instate;
    network->inSequenceNr = insequencenr;

    return result;
}

static void FASTCALL calcViewBobHook(void* thisPointer, void* edx, float eyeOrigin) noexcept
{

    static auto original = hooks->calcViewBob.getOriginal<void>(eyeOrigin);

    auto entity = reinterpret_cast<Entity*>(thisPointer);

    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer, eyeOrigin);

    if (Visuals::noBoob())
        return;

    original(thisPointer, eyeOrigin);
}

UserCmd* usrcmd;

static bool STDCALL createMove(float inputSampleTime, UserCmd* cmd, bool& sendPacket) noexcept
{
    static auto previousViewAngles{ cmd->viewangles };
    const auto viewAngles{ cmd->viewangles };
    auto currentViewAngles{ cmd->viewangles };
    const auto currentCmd{ *cmd };
    auto angOldViewPoint{ cmd->viewangles };
    const auto currentPredictedTick{ interfaces->prediction->split->commandsPredicted - 1 };

    if (Tickbase::isShifting()) {
        sendPacket = Tickbase::isFinalTick();
        cmd->buttons &= ~(UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2);
        const float unpredict_velocity = localPlayer->velocity().z;
        //Set all movement to its max value
        if (cmd->sidemove > 5.0f)
            cmd->sidemove = 450.0f;
        else if (cmd->sidemove < -5.0f)
            cmd->sidemove = -450.0f;

        if (cmd->forwardmove > 5.0f)
            cmd->forwardmove = 450.0f;
        else if (cmd->forwardmove < -5.0f)
            cmd->forwardmove = -450.0f;

        //Run all movement related stuff
        Movement::duckWhenJump(cmd);
        Movement::bunnyHop(cmd);
        Movement::removeCrouchCooldown(cmd);

        EnginePrediction::update();
        EnginePrediction::run(cmd);
        Extra::quickPeek(cmd);
        Movement::edgeJump(cmd);
        Movement::ladderJump(cmd);
        Movement::autoStrafe(cmd, currentViewAngles);
        Movement::autoPixelSurf(cmd);
        Movement::autoAlign(cmd);
        Movement::headBounce(cmd);
        Movement::miniJump(cmd);

        if (AntiAim::canRun(cmd))
            AntiAim::run(cmd, previousViewAngles, currentViewAngles, sendPacket);

        //Clamp angles and fix movement
        auto viewAnglesDelta{ cmd->viewangles - previousViewAngles };
        viewAnglesDelta.normalize();
        viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
        viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());

        cmd->viewangles = previousViewAngles + viewAnglesDelta;

        cmd->viewangles.normalize();

        if (Extra::AntiUntrusted()) {
            viewAnglesDelta.normalize();
            viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
            viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());

            cmd->viewangles = previousViewAngles + viewAnglesDelta;

            cmd->viewangles.normalize();
        }

        if ((currentViewAngles != cmd->viewangles
            || cmd->forwardmove != currentCmd.forwardmove
            || cmd->sidemove != currentCmd.sidemove)
            && (cmd->sidemove != 0 || cmd->forwardmove != 0))
        {
            Misc::fixMovement(cmd, currentViewAngles.y);
        }

        if (Extra::AntiUntrusted()) {
            cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.f, 89.f);
            cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.f, 180.f);
            cmd->viewangles.z = std::clamp(cmd->viewangles.z, -50.f, 50.f); //https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/client/in_main.cpp#L747-L756
            //cmd->viewangles.z = std::clamp(cmd->viewangles.z, -90.f, 90.f);
            cmd->forwardmove = std::clamp(cmd->forwardmove, -450.f, 450.f);
            cmd->sidemove = std::clamp(cmd->sidemove, -450.f, 450.f);
            cmd->upmove = std::clamp(cmd->upmove, -320.f, 320.f);
        }


        previousViewAngles = cmd->viewangles;

        if (localPlayer && localPlayer->isAlive())
            memory->restoreEntityToPredictedFrame(0, currentPredictedTick);
        Movement::jumpStats(cmd);

#if defined(ENABLEFIXANIM)
        Animations::update(cmd, sendPacket);
        Animations::fake();
#endif
        return false;
    }
    Movement::gatherDataOnTick(cmd);
    Tickbase::start(cmd);

    memory->globalVars->serverTime(cmd);

    Xsin::rainbowColor();
    Xsin::renderServerHitboxes();
    Clan::update();
    Visuals::particleFootTrail();
    Visuals::removeShadows();
    Misc::disableExtrapolation();
    Misc::fixTabletSignal();
    Misc::outOfAmmoNotification();
    Misc::antiAfkKick(cmd);
    Misc::runReportbot();
    Misc::autoPistol(cmd);
    Misc::autoReload(cmd);
    Misc::revealRanks(cmd);
    Misc::autoDefuse(cmd);
    Misc::fastPlant(cmd);
    Misc::BlockInputWhileOpenGui(cmd);
    Nade::Move(cmd);

    if (!MovementRecorder::isPlayingback())
    {
        Movement::duckWhenJump(cmd);
        Movement::bunnyHop(cmd);
        Movement::removeCrouchCooldown(cmd);
        Movement::fastStop(cmd);

        Extra::airStuck(cmd);
        Extra::slowWalk(cmd);

        Fun::ChatSpammer();
        Fun::stealNames();
        Fun::prepareNamesSpam();
        Fun::blockBot(cmd);

        Movement::prePrediction(cmd);
        const float unpredictedVelocity = localPlayer->velocity().z;
        Backtrack::updateIncomingSequences();

        EnginePrediction::update();
        EnginePrediction::run(cmd);
        NadePrediction::run(cmd);
        LegitAimbot::run(cmd);
        Backtrack::run(cmd);
        RageAimbot::run(cmd);
        RageAimbot::autoZeus(cmd);
        Triggerbot::run(cmd);
        KnifeBot::run(cmd);
        Tickbase::end(cmd);
        Extra::quickPeek(cmd);
        Movement::autoPixelSurf(cmd);
        Movement::miniJump(cmd);
        Movement::edgeBug(cmd, angOldViewPoint);
        AntiAim::backwards(cmd, previousViewAngles, currentViewAngles, sendPacket);
        if (AntiAim::canRun(cmd))
        {
            FakeLag::run(sendPacket);
            AntiAim::run(cmd, previousViewAngles, currentViewAngles, sendPacket);
        }
        Extra::fakeWalk(cmd, sendPacket);
        Extra::fakeDuck(cmd, sendPacket);
        Movement::ladderJump(cmd);
        Movement::autoStrafe(cmd, currentViewAngles);
        Movement::autoAlign(cmd);
        Movement::headBounce(cmd);
        Movement::jumpBug(cmd);
        Movement::edgeJump(cmd);
        Movement::EdgebugDetect(unpredictedVelocity);
        Extra::prepareRevolver(cmd);
        Misc::runFreeCam(cmd, viewAngles);
        Movement::gatherDataOnTick(cmd);
        Misc::fixMouseDelta(cmd);

        MovementRecorder::record(cmd);
}
    Misc::antiTKilling(cmd);
    MovementRecorder::play(cmd);

    auto viewAnglesDelta{ cmd->viewangles - previousViewAngles };

    if (Extra::AntiUntrusted()) {
        viewAnglesDelta.normalize();
        viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -Extra::maxAngleDelta(), Extra::maxAngleDelta());
        viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -Extra::maxAngleDelta(), Extra::maxAngleDelta());

        cmd->viewangles = previousViewAngles + viewAnglesDelta;

        cmd->viewangles.normalize();
    }

    if ((currentViewAngles != cmd->viewangles
        || cmd->forwardmove != currentCmd.forwardmove
        || cmd->sidemove != currentCmd.sidemove)
        && (cmd->sidemove != 0 || cmd->forwardmove != 0))
    {
        Misc::fixMovement(cmd, currentViewAngles.y);
    }
    Misc::moonwalk(cmd);

    if (Extra::AntiUntrusted()) {
        cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.f, 89.f);
        cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.f, 180.f);
        cmd->viewangles.z = std::clamp(cmd->viewangles.z, -90.f, 90.f); //https://i.imgur.com/DyFsy2A.png
        cmd->forwardmove = std::clamp(cmd->forwardmove, -450.f, 450.f);
        cmd->sidemove = std::clamp(cmd->sidemove, -450.f, 450.f);
        cmd->upmove = std::clamp(cmd->upmove, -320.f, 320.f);
    }

    previousViewAngles = cmd->viewangles;

    if (localPlayer && localPlayer->isAlive())
        memory->restoreEntityToPredictedFrame(0, currentPredictedTick);

    Movement::jumpStats(cmd);

#if defined(ENABLEFIXANIM)
    Animations::update(cmd, sendPacket);
    Animations::fake();
#endif

    Xsin::SentPacket(sendPacket);
    Misc::getRoll(cmd);
    usrcmd = cmd;

    return false;
}

static void STDCALL CHLCreateMove(int sequenceNumber, float inputSampleTime, bool active, bool& sendPacket) noexcept
{
    static auto original = hooks->client.getOriginal<void, 22>(sequenceNumber, inputSampleTime, active);

    original(interfaces->client, sequenceNumber, inputSampleTime, active);

    UserCmd* cmd = memory->input->getUserCmd(0, sequenceNumber);
    if (!cmd || !cmd->commandNumber)
        return;

    VerifiedUserCmd* verified = memory->input->getVerifiedUserCmd(sequenceNumber);
    if (!verified)
        return;

    bool cmoveactive = createMove(inputSampleTime, cmd, sendPacket);

    verified->cmd = *cmd;
    verified->crc = cmd->getChecksum();
}

#pragma warning(disable : 4409)
__declspec(naked) void STDCALL createMoveProxy(int sequenceNumber, float inputSampleTime, bool active)
{
    __asm
    {
        PUSH	EBP
        MOV		EBP, ESP
        PUSH	EBX
        LEA		ECX, [ESP]
        PUSH	ECX
        PUSH	active
        PUSH	inputSampleTime
        PUSH	sequenceNumber
        CALL	CHLCreateMove
        POP		EBX
        POP		EBP
        RETN	0xC
    }
}
typedef const void(__thiscall* shutDown)(void*, void*, const char*);
shutDown originalShutdown;
static void FASTCALL shutdownHook(void* thisPointer, void* unk1, void* unk2, const char* reason) noexcept {

    if (Fun::getDisconnect()) {
        return originalShutdown(thisPointer, nullptr, Fun::getDisconnectMessage());
        Helpers::CheatConsoleNotification({ 1.f, 1.f, 1.f, 1.f }, "\n\n\nDisconnected...\n");
    }

    return originalShutdown(thisPointer, nullptr, reason);

}

void shutdownHookWhenPossible() noexcept {

    static bool hookOnce = false;

    if (hookOnce) return;

    auto networkChannel = interfaces->engine->getNetworkChannel();

    if (!networkChannel)
        return;

    if (!interfaces->engine->isConnected()) return;

    DWORD ptrShutdown = *((DWORD*)networkChannel) + 36 * 4;
    DWORD addrShutdown = *(DWORD*)ptrShutdown;
    originalShutdown = (shutDown)DetourFunction((PBYTE)(addrShutdown),(PBYTE)shutdownHook);
    Helpers::printfColored(Helpers::printfColors::Light_Green, std::string("[!] Detoured Shutdown").c_str());
    hookOnce = true;
}

bool STDCALL sendLobbyChatMessage(CSteamID steamIdLobby, const void* pvMsgBody, int cubMsgBody)
{
    typedef bool(__thiscall* SendLobbyChatMessage)(ISteamMatchmaking*, CSteamID, const void*, int);
    static auto original = hooks->steamMatchmakingHook.get_original<SendLobbyChatMessage>(26);

    if (!LobbyClass::Get()->InterpretLobbyMessage(steamIdLobby, pvMsgBody, cubMsgBody))
        return original(steamMatchmaking, steamIdLobby, pvMsgBody, cubMsgBody);

    return true;
}

static void STDCALL doPostScreenEffects(LINUX_ARGS(void* thisPointer,) void* param) noexcept
{
    if (interfaces->engine->isInGame()) {
        Visuals::thirdperson();
        Visuals::ragdolls();
        Visuals::noScopeSway();
        Visuals::hideHUD();
        Visuals::updateBrightness();
        Visuals::remove3dSky();
        Visuals::removeSprites();
        Glow::render();
    }
    hooks->clientMode.callOriginal<void, WIN32_LINUX(44, 45)>(param);
}

static float STDCALL getViewModelFov(LINUX_ARGS(void* thisPointer)) noexcept
{
    float additionalFov = Visuals::viewModelFov();
    if (localPlayer) {
        if (const auto activeWeapon = localPlayer->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::Tablet)
            additionalFov = 0.0f;
    }

    return hooks->clientMode.callOriginal<float, WIN32_LINUX(35, 36)>() + additionalFov;
}

static void STDCALL drawModelExecute(LINUX_ARGS(void* thisPointer,) void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (interfaces->studioRender->isForcedMaterialOverride())
        return hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    if (Visuals::removeHands(info.model->name) || Visuals::removeSleeves(info.model->name) || Visuals::removeWeapons(info.model->name))
        return;

    if (static Chams chams; !chams.render(ctx, state, info, customBoneToWorld))
        hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    interfaces->studioRender->forcedMaterialOverride(nullptr);
}

static bool FASTCALL svCheatsGetBool(void* _this) noexcept
{
    if (std::uintptr_t(_ReturnAddress()) == memory->cameraThink && Visuals::isThirdpersonOn())
        return true;

    return hooks->svCheats.getOriginal<bool, WIN32_LINUX(13, 16)>()(_this);
}

static void __stdcall paintTraverse(unsigned int panel, bool forceRepaint, bool allowForce) noexcept
{
    //if (interfaces->panel->getName(panel) == "MatSystemTopPanel")
    //Xsin::showVelocity();

    hooks->panel.callOriginal<void, 41>(panel, forceRepaint, allowForce);
}

static void STDCALL frameStageNotify(LINUX_ARGS(void* thisPointer,) csgo::FrameStage stage) noexcept
{

    [[maybe_unused]] static auto backtrackInit = (Backtrack::init(), false);
    Animations::init();

    if (interfaces->engine->isConnected() && !interfaces->engine->isInGame())
        Fun::changeName(true, nullptr, 0.0f);

    if (stage == csgo::FrameStage::START) {
        GameData::update();
    }

    if (stage == csgo::FrameStage::RENDER_START) {
        if (!Panorama::getPanel(Panorama::CSGOPanel::CSGOHud))
            Panorama::getPanel(Panorama::CSGOPanel::CSGOHud) = Panorama::tryGetPanels(Panorama::CSGOPanel::CSGOHud);
        if (!Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu))
            Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu) = Panorama::tryGetPanels(Panorama::CSGOPanel::CSGOMainMenu);
        if (!Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel))
            Panorama::getPanel(Panorama::CSGOPanel::MainMenuContainerPanel) = Panorama::tryGetPanels(Panorama::CSGOPanel::MainMenuContainerPanel);
        if (Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu)){
            static bool once = false;

            constexpr const char* script =
            #include "Hacks/PanoramaJs/once.js"
            ;
            if (!once)
                interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), script, "panorama/layout/mainmenu.xml", 8, 10, false, false); once = true;
            Lobby::lobbyRankParty();
            Lobby::retardLobby();
            Lobby::commandSystemScript();
        }
        Misc::overheadInfo(); // makes enemies as allies, causes aimbot to not shoot them
        Visuals::asus();
        // shutdownHookWhenPossible(); //no idea :/ //broken anyway
        Profile::updateInfo();
		Misc::unlockHiddenCvars();
        Misc::forceRelayCluster();
        Misc::preserveKillfeed();
        Misc::updateEventListeners();
        Visuals::updateEventListeners();
        Visuals::doBloomEffects();
        Visuals::RunDLights();
        Visuals::colorConsole();
        Visuals::dontRenderTeammates();
        Lobby::startMatchmaking();
    }
    if (interfaces->engine->isInGame()) {
        Visuals::Flashlight(stage);
        Visuals::skybox(stage);
        Visuals::removeBlur(stage);
        Visuals::removeGrass(stage);
        Visuals::modifySmoke(stage);
        Visuals::modifyMolotov(stage);
        Visuals::disablePostProcessing(stage);
        Visuals::removeVisualRecoil(stage);
        Visuals::applyZoom(stage);
        Visuals::playerModel(stage);
        Visuals::maskChanger(stage);
        Visuals::drawBulletImpacts();
        Misc::fixAnimationLOD(stage);
        Misc::forceCrosshair(stage);
        Misc::recoilCrosshair(stage);
        Misc::oppositeHandKnife(stage);
#if defined(ENABLEFIXANIM)
        //Animations::renderStart(stage);
#endif
        Animations::handlePlayers(stage);

    }
    
    inventory_changer::InventoryChanger::instance().run(stage);
    hooks->client.callOriginal<void, 37>(stage);
    if (interfaces->engine->isInGame())
        EnginePrediction::apply(stage);
}

static int STDCALL emitSound(LINUX_ARGS(void* thisPointer,) void* filter, int entityIndex, int channel, const char* soundEntry, unsigned int soundEntryHash, const char* sample, float volume, int seed, int soundLevel, int flags, int pitch, const Vector& origin, const Vector& direction, void* utlVecOrigins, bool updatePositions, float soundtime, int speakerentity, void* soundParams) noexcept
{
    if (EnginePrediction::isInPrediction())
        return 0;
    Sound::modulateSound(soundEntry, entityIndex, volume);
    Misc::autoAccept(soundEntry);

    volume = std::clamp(volume, 0.0f, 1.0f);
    return hooks->sound.callOriginal<int, WIN32_LINUX(5, 6)>(filter, entityIndex, channel, soundEntry, soundEntryHash, sample, volume, seed, soundLevel, flags, pitch, std::cref(origin), std::cref(direction), utlVecOrigins, updatePositions, soundtime, speakerentity, soundParams);
}

static bool STDCALL shouldDrawFog(LINUX_ARGS(void* thisPointer)) noexcept
{
#ifdef _WIN32
    if constexpr (std::is_same_v<HookType, MinHook>) {
        if (RETURN_ADDRESS() != memory->shouldDrawFogReturnAddress)
            return hooks->clientMode.callOriginal<bool, 17>();
    }
#endif
    
    return !Visuals::shouldRemoveFog();
}

static bool STDCALL shouldDrawViewModel(LINUX_ARGS(void* thisPointer)) noexcept
{
    if (Visuals::isZoomOn() && localPlayer && localPlayer->fov() < 45 && localPlayer->fovStart() < 45)
        return false;
    return hooks->clientMode.callOriginal<bool, WIN32_LINUX(27, 28)>();
}

static void STDCALL lockCursor() noexcept
{
    if (gui->isOpen())
        return interfaces->surface->unlockCursor();
    return hooks->surface.callOriginal<void, 67>();
}

/*
*
*   Tickbase
* 
*/

static void __cdecl clSendMoveHook() noexcept
{
    int nextCommandNr = memory->clientState->lastOutgoingCommand + memory->clientState->chokedCommands + 1;
    int chokedCommands = memory->clientState->chokedCommands;

    byte data[4000 /* MAX_CMD_BUFFER */];
    clMsgMove moveMsg;

    moveMsg.dataOut.startWriting(data, sizeof(data));

    const int backupCommands = 2;
    const int newCommands = std::max(chokedCommands + 1, 0);

    moveMsg.setNumBackupCommands(backupCommands);
    moveMsg.setNumNewCommands(newCommands);

    const int numCmds = newCommands + backupCommands;
    int from = -1;
    bool ok = true;

    for (int to = nextCommandNr - numCmds + 1; to <= nextCommandNr; ++to) {

        bool isnewcmd = to >= (nextCommandNr - newCommands + 1);

        ok = ok && interfaces->client->writeUsercmdDeltaToBuffer(0, &moveMsg.dataOut, from, to, isnewcmd);
        from = to;
    }

    if (ok) {

        moveMsg.setData(moveMsg.dataOut.getData(), moveMsg.dataOut.getNumBytesWritten());

        memory->clientState->netChannel->sendNetMsg(reinterpret_cast<void*>(&moveMsg));
    }
}

static void FASTCALL physicsSimulateHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->physicsSimulate.getOriginal<void>();

    const auto entity = reinterpret_cast<Entity*>(thisPointer);
    if (!localPlayer || !localPlayer->isAlive() || entity != localPlayer.get())
        return original(thisPointer);

    const int simulationTick = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(thisPointer) + 0x2AC);
    if (simulationTick == memory->globalVars->tickCount)
        return;

    CommandContext* commandContext = localPlayer->getCommandContext();
    if (!commandContext || !commandContext->needsProcessing)
        return;

    static int lastCommandNumber = -1;
    static bool once = false;

    if (Tickbase::pausedTicks() && !once)
    {
        lastCommandNumber = commandContext->commandNumber;
        once = true;
    }

    if (lastCommandNumber != commandContext->commandNumber)
    {
        lastCommandNumber = commandContext->commandNumber;
        once = false;
        Tickbase::pausedTicks() = 0;
    }

    localPlayer->tickBase() = Tickbase::getCorrectTickbase(commandContext->commandNumber);

    original(thisPointer);

    // save netvar data
    EnginePrediction::store();
}

static void writeUsercmd(bufferWrite* buffer, UserCmd* toCmd, UserCmd* fromCmd) noexcept
{
    const auto writeCmd = memory->writeUsercmd;
    __asm
    {
        mov     ecx, buffer
        mov     edx, toCmd
        push    fromCmd
        call    writeCmd
        add     esp, 4
    }
}

static bool FASTCALL writeUsercmdDeltaToBuffer(void* thisPointer, void* edx, int slot, bufferWrite* buffer, int from, int to, bool newCmd) noexcept
{
    static auto original = hooks->client.getOriginal<bool, 24, int, bufferWrite*, int, int, bool>(slot, buffer, from, to, newCmd);

    if (Tickbase::getTickshift() <= 0 || Extra::isTeleportEnabled())
        return original(thisPointer, slot, buffer, from, to, newCmd);

    const int extraCommands = Tickbase::getTickshift();
    Tickbase::resetTickshift();

    int* backupCommandsPointer = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x30);
    int* newCommandsPointer = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x2C);

    const int newCommands = *newCommandsPointer;
    const int nextCommand = memory->clientState->chokedCommands + memory->clientState->lastOutgoingCommand + 1;

    *backupCommandsPointer = 0;

    for (to = nextCommand - newCommands + 1; to <= nextCommand; to++)
    {
        if (!original(thisPointer, slot, buffer, from, to, true))
            return false;

        from = to;
    }

    *newCommandsPointer = newCommands + extraCommands;

    UserCmd* cmd = memory->input->getUserCmd(slot, from);
    if (!cmd)
        return true;

    UserCmd toCmd = *cmd;
    UserCmd fromCmd = *cmd;

    toCmd.commandNumber++;
    if (!Extra::isTeleportEnabled())
        toCmd.tickCount += 200;

    for (int i = 0; i < extraCommands; i++)
    {
        writeUsercmd(buffer, &toCmd, &fromCmd);

        toCmd.tickCount++;
        toCmd.commandNumber++;

        fromCmd.tickCount = toCmd.tickCount - 1;
        fromCmd.commandNumber = toCmd.commandNumber - 1;
    }

    return true;
}

static void __cdecl clMoveHook(float frameTime, bool isFinalTick) noexcept
{
    using clMoveFn = void(__cdecl*)(float, bool);
    static auto original = (clMoveFn)hooks->clMove.getDetour();

    if (!Tickbase::canRun())
        return;

    original(frameTime, isFinalTick);

    if (!Tickbase::getTickshift() || !Extra::isTeleportEnabled())
        return;

    int remainToShift = 0;
    int tickShift = Tickbase::getTickshift();

    Tickbase::isShifting() = true;

    for (int shiftAmount = 0; shiftAmount < tickShift; shiftAmount++)
    {
        Tickbase::isFinalTick() = (tickShift - shiftAmount) == 1;
        original(frameTime, isFinalTick);
    }
    Tickbase::isShifting() = false;

    Tickbase::resetTickshift();
}

/*
*
*   Tickbase
* 
*/

static void STDCALL setDrawColor(LINUX_ARGS(void* thisPointer,) int r, int g, int b, int a) noexcept
{
    constexpr float pi = std::numbers::pi_v<float>;

    if (Visuals::shouldRemoveScopeOverlay() && (RETURN_ADDRESS() == memory->scopeDust || RETURN_ADDRESS() == memory->scopeArc))
        a = 0;

    if (Visuals::customScopeCrosshirEnabled() && (RETURN_ADDRESS() == memory->scopeBlurry || RETURN_ADDRESS() == memory->scopeLines))
        a = 0;

    if (Visuals::crosshairOutlineColor().enabled && RETURN_ADDRESS() == memory->crosshairOutline) {
        if (Visuals::crosshairOutlineColor().rainbow) {
            r = static_cast<int>(std::sin(Visuals::crosshairOutlineColor().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f);
            g = static_cast<int>(std::sin(Visuals::crosshairOutlineColor().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f);
            b = static_cast<int>(std::sin(Visuals::crosshairOutlineColor().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f);
        }
        else {
            r = static_cast<int>(Visuals::crosshairOutlineColor().color[0] * 255);
            g = static_cast<int>(Visuals::crosshairOutlineColor().color[1] * 255);
            b = static_cast<int>(Visuals::crosshairOutlineColor().color[2] * 255);
        }
        a = static_cast<int>(Visuals::crosshairOutlineColor().color[3] * 255);
    }
    hooks->surface.callOriginal<void, WIN32_LINUX(15, 14)>(r, g, b, a);
}

static void FASTCALL getColorModulationHook(void* thisPointer, void* edx, float* r, float* g, float* b) noexcept
{

    static auto original = hooks->getColorModulation.getOriginal<void>(r, g, b);

    original(thisPointer, r, g, b);

    if (!Visuals::worldColor().enabled)
        return;

    auto drawSpecificStaticProp = interfaces->cvar->findVar("r_drawspecificstaticprop");
    drawSpecificStaticProp->setValue(0);

    if (drawSpecificStaticProp) return;

    const auto material = reinterpret_cast<Material*>(thisPointer);
    if (!material)
        return;

    const std::string_view textureGroup = material->getTextureGroupName();
    if (!textureGroup.starts_with("World") && !textureGroup.starts_with("StaticProp"))
        return;

    const auto isProp = textureGroup.starts_with("StaticProp");
    if (Visuals::worldColor().rainbow)
    {
        const auto [colorR, colorG, colorB] { rainbowColor(Visuals::worldColor().rainbowSpeed) };
        *r *= colorR;
        *g *= colorG;
        *b *= colorB;
    }
    else
    {
        *r *= Visuals::worldColor().color.at(0);
        *g *= Visuals::worldColor().color.at(1);
        *b *= Visuals::worldColor().color.at(2);
    }

    isProp ? *r *= 0.5f : *r *= 0.23f;
    isProp ? *g *= 0.5f : *g *= 0.23f;
    isProp ? *b *= 0.5f : *b *= 0.23f;
}

static void __fastcall processMovement(void* thisPointer, void* edx, Entity* player, MoveData* moveData) noexcept
{
    static auto original = hooks->gameMovement.getOriginal<void, 1>(player, moveData);
    moveData->gameCodeMovedPlayer = false;
    return original(thisPointer, player, moveData);
}

static void FASTCALL setTextHook(void* thisPointer, void* edx, const char* text) noexcept
{
    static auto original = hooks->setText.getOriginal<void>(edx, text);

    if (!localPlayer || !localPlayer->getActiveWeapon())
        return original(thisPointer, edx, text);

    auto weapon = localPlayer->getActiveWeapon();

    static float timerLmao = 0.f;
    if (weapon->itemDefinitionIndex() == WeaponId::C4 && usrcmd->buttons & UserCmd::IN_ATTACK) {

        if (timerLmao == 0.f)
            timerLmao = memory->globalVars->currenttime + 0.5f;

        float flTransition = memory->globalVars->currenttime - timerLmao;

        std::string text = "Azurre";

        // transition time can be made to scale with text length but idc
        std::string thisText;
        for (size_t i = 0; i < sizeof(text); i++)
            if (flTransition >= (i * 0.18f))
                thisText += text[i];
            else
                break;

        return original(thisPointer, edx, thisText.c_str());
    }
    else
        timerLmao = 0.f;

    return original(thisPointer, edx, text);
}

static bool FASTCALL traceFilterForHeadCollisionHook(void* thisPointer, void* edx, Entity* player, unsigned int traceParams) noexcept
{

    static auto original = hooks->traceFilterForHeadCollision.getOriginal<bool>(player, traceParams);

    if (!localPlayer || !localPlayer->isAlive())
        return original(thisPointer, player, traceParams);

    if (!player || !player->isPlayer() || player == localPlayer.get())
        return original(thisPointer, player, traceParams);

    if (fabsf(player->getAbsOrigin().z - localPlayer->getAbsOrigin().z) < 10.0f)
        return false;

    return original(thisPointer, player, traceParams);
}

static void* STDCALL getClientModelRenderableHook() noexcept
{
    return nullptr;
}

static void STDCALL overrideView(ViewSetup* setup) noexcept
{
    const auto zoomSensitivity = interfaces->cvar->findVar("zoom_sensitivity_ratio_mouse");
    static auto zoomSensitivityBackUp = zoomSensitivity->getFloat();
    if (localPlayer)
    {
        static bool once = true;
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->isSniperRifle() && localPlayer->isScoped())
        {
            if (Visuals::noZoom())
            {
                if (once)
                {
                    zoomSensitivityBackUp = zoomSensitivity->getFloat();
                    once = false;
                }
                zoomSensitivity->setValue(0.f);
                setup->fov = 90.f;
            }
        }
        else if (!once)
        {
            zoomSensitivity->setValue(zoomSensitivityBackUp);
            once = true;
        }
    }

    if (localPlayer && !localPlayer->isScoped())
        setup->fov += Visuals::fov();

    setup->farZ = Visuals::farZ() * 10;

    if (localPlayer && localPlayer->isAlive() && Extra::isFakeDuckKeyPressed() && localPlayer->isOnGround()) { //fakeduck
        setup->origin.z = localPlayer->getAbsOrigin().z + interfaces->gameMovement->getPlayerViewOffset(false).z;
    }

    Visuals::viewModelChanger(setup);

    hooks->clientMode.callOriginal<void, 18>(setup);

    Misc::freeCam(setup);
    Visuals::motionBlur(setup);

}

struct RenderableInfo {
    Entity* renderable;
    std::byte pad[18];
    uint16_t flags;
    uint16_t flags2;
};

static int STDCALL listLeavesInBox(LINUX_ARGS(void* thisPointer, ) const Vector& mins, const Vector& maxs, unsigned short* list, int listMax) noexcept
{
    if (Misc::shouldDisableModelOcclusion() && RETURN_ADDRESS() == memory->insertIntoTree) {
        if (const auto info = *reinterpret_cast<RenderableInfo**>(FRAME_ADDRESS() + WIN32_LINUX(0x18, 0x10 + 0x948)); info && info->renderable) {
            if (const auto ent = VirtualMethod::call<Entity*, WIN32_LINUX(7, 8)>(info->renderable - sizeof(std::uintptr_t)); ent && ent->isPlayer()) {
                constexpr float maxCoord = 16384.0f;
                constexpr float minCoord = -maxCoord;
                constexpr Vector min{ minCoord, minCoord, minCoord };
                constexpr Vector max{ maxCoord, maxCoord, maxCoord };
                return hooks->bspQuery.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);
            }
        }
    }

    return hooks->bspQuery.callOriginal<int, 6>(std::cref(mins), std::cref(maxs), list, listMax);
}

static int FASTCALL dispatchSound(SoundInfo& soundInfo) noexcept
{
    if (const char* soundName = interfaces->soundEmitter->getSoundName(soundInfo.soundIndex)) {
        Sound::modulateSound(soundName, soundInfo.entityIndex, soundInfo.volume);
        soundInfo.volume = std::clamp(soundInfo.volume, 0.0f, 1.0f);
    }
    return hooks->originalDispatchSound(soundInfo);
}

static void STDCALL render2dEffectsPreHud(LINUX_ARGS(void* thisPointer,) void* viewSetup) noexcept
{
    Visuals::applyScreenEffects();
    Visuals::hitEffect();
    hooks->viewRender.callOriginal<void, WIN32_LINUX(39, 40)>(viewSetup);
}

static const DemoPlaybackParameters* STDCALL getDemoPlaybackParameters(LINUX_ARGS(void* thisPointer)) noexcept
{
    const auto params = hooks->engine.callOriginal<const DemoPlaybackParameters*, WIN32_LINUX(218, 219)>();

    if (params && Misc::shouldRevealSuspect() && RETURN_ADDRESS() != memory->demoFileEndReached) {
        static DemoPlaybackParameters customParams;
        customParams = *params;
        customParams.anonymousPlayerIdentity = false;
        return &customParams;
    }

    return params;
}

//profile changer

static bool isPrimeAccount;

typedef const int(__thiscall* getAccountData)(void*); //use better hook
getAccountData originalGetAccountData;
int FASTCALL getAccountDataHook(void* _this, void* edx) noexcept
{
    const auto original = originalGetAccountData(_this);
    AccountStatus& getAccountStatus = *reinterpret_cast<AccountStatus*>(original + 24); //prime

    if (!&getAccountStatus) return original;

    //isPrimeAccount = getAccountStatus == AccountStatus::Elevated; //is account prime     //Crashed when there is no connection with valve servers
    if(config->profilechanger.enabledRanks)
    getAccountStatus = static_cast<AccountStatus>(config->profilechanger.accountStatus);

    return original;

}

EGCResult FASTCALL hkGCRetrieveMessage(void* ecx, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{

    static auto original = hooks->gameCoordinator.get_original<EGCResult(__thiscall*)(void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)>(2);
    auto status = original(ecx, punMsgType, pubDest, cubDest, pcubMsgSize);

    if (status == k_EGCResultOK)
    {

        void* thisPointer = nullptr;
        __asm mov thisPointer, ebx;
        auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

        uint32_t messageType = *punMsgType & 0x7FFFFFFF;
        write.ReceiveMessage(thisPointer, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
    }
    return status;

}

EGCResult FASTCALL hkGCSendMessage(void* ecx, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
{
        static auto original = hooks->gameCoordinator.get_original<EGCResult(__thiscall*)(void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)>(0);

        if (!original)
            return k_EGCResultOK;

        return original(ecx, unMsgType, const_cast<void*>(pubData), cubData);
}
//profile changer ^^^

static void STDCALL drawPrintText(const wchar_t* text, int textLength, int textType) {

    if (wcsstr(text, L"fps:")) {
        wchar_t buf[128] = L"[azurre]  ";
        wcscat_s(buf, text);
        return hooks->surface.callOriginal<void, 28>(buf, wcslen(buf), textType);
    }

    return hooks->surface.callOriginal<void, 28>(text, textLength, textType);
}

static void FASTCALL netGraphHook(void* ecx, void* edx, int font, int x, int y, int r, int g, int b, int a, const char* fmt, void* argptr) {
    
    static auto original = hooks->netGraph.getOriginal<void>(font, x, y, r, g, b, a, fmt, argptr);

    if (!Visuals::netGraphEnabled().enabled)
        return original(ecx, font, x, y, r, g, b, a, fmt, argptr);

    static float rColor; 
    static float gColor;
    static float bColor;
    static float aColor;

    std::string fmtc = fmt;

    if (_AddressOfReturnAddress() == memory->netGraphFPS)
        fmtc = std::string(fmt).append(" azurre");

    if (Visuals::netGraphEnabled().rainbow) {
        rColor = (std::sin(Visuals::netGraphEnabled().rainbowSpeed * memory->globalVars->realtime) * 0.5f + 0.5f) * 255.f;
        gColor = (std::sin(Visuals::netGraphEnabled().rainbowSpeed * memory->globalVars->realtime + 2 * pi / 3) * 0.5f + 0.5f) * 255.f;
        bColor = (std::sin(Visuals::netGraphEnabled().rainbowSpeed * memory->globalVars->realtime + 4 * pi / 3) * 0.5f + 0.5f) * 255.f;
    }else {
        rColor = Visuals::netGraphEnabled().color[0] * 255.f;
        gColor = Visuals::netGraphEnabled().color[1] * 255.f;
        bColor = Visuals::netGraphEnabled().color[2] * 255.f;
    }

    aColor = Visuals::netGraphEnabled().color[3] * 255.f;

    int customFont = Visuals::netGraphFont();

    if (customFont < 0)
        customFont = font;

        return original(ecx, customFont, x, y,
            static_cast<int>(rColor),
            static_cast<int>(gColor),
            static_cast<int>(bColor),
            static_cast<int>(aColor),
            fmtc.c_str(), argptr);
}

static unsigned short __fastcall findMDL(void* ecx, void* edx, char* filePath) noexcept
{

    static auto original = hooks->mdlCache.get_original<unsigned short(__thiscall*)(void*, char*)>(10);

    if (strstr(filePath, "v_snip_awp.mdl"))
        sprintf(filePath, "models/weaponf/v_animeawp.mdl");

    return original(ecx, filePath);
}

static bool FASTCALL isHltv() noexcept
{
    if (_ReturnAddress() == memory->setupVelocityAddress || _ReturnAddress() == memory->accumulateLayersAddress || (Misc::isRadarSquared() && RETURN_ADDRESS() == memory->radarHLTV))// || _ReturnAddress() == memory->reevauluateAnimLODAddress)
        return true;

    return hooks->engine.callOriginal<bool, 93>();
}

static bool STDCALL isPlayingDemo(LINUX_ARGS(void* thisPointer)) noexcept
{
    if (Misc::shouldRevealMoney() && RETURN_ADDRESS() == memory->demoOrHLTV && *reinterpret_cast<std::uintptr_t*>(FRAME_ADDRESS() + WIN32_LINUX(8, 24)) == memory->money)
        return true;

    return hooks->engine.callOriginal<bool, 82>();
}

static bool STDCALL isConnected() noexcept
{
    if (Misc::unlockInventory() && RETURN_ADDRESS() == memory->unlockInventory)
        return false;

    return hooks->engine.callOriginal<bool, 27>();
}

//

char* FASTCALL getHalloweenMaskHook(void* ecx, void* edx)
{
    return Visuals::maskChar();
}

static void STDCALL updateColorCorrectionWeights(LINUX_ARGS(void* thisPointer)) noexcept
{
    hooks->clientMode.callOriginal<void, WIN32_LINUX(58, 61)>();

    Visuals::performColorCorrection();
    if (Visuals::shouldRemoveScopeOverlay())
        *memory->vignette = 0.0f;
}

static float STDCALL getScreenAspectRatio(LINUX_ARGS(void* thisPointer,) int width, int height) noexcept
{
    if (Misc::aspectRatio() != 0.0f)
        return Misc::aspectRatio();
    return hooks->engine.callOriginal<float, 101>(width, height);
}

static void STDCALL renderSmokeOverlay(LINUX_ARGS(void* thisPointer,) bool update) noexcept
{
    if (Visuals::shouldRemoveSmoke())
        *reinterpret_cast<float*>(std::uintptr_t(memory->viewRender) + WIN32_LINUX(0x588, 0x648)) = 0.0f;
    else
        hooks->viewRender.callOriginal<void, WIN32_LINUX(41, 42)>(update);
}

static void __stdcall onJump(float stamina) noexcept
{
    hooks->gameMovement.callOriginal<void, 32>(stamina);

    static int counter = 0;
    counter++;
    //We have jumpped
    if (counter % 2 == 0 && counter != 0)
    {
        counter = 0;
        Movement::gotJump();
    }

    if (localPlayer && localPlayer->isAlive() && localPlayer->getAnimstate())
        localPlayer->getAnimstate()->doAnimationEvent(PLAYERANIMEVENT_JUMP);
}

static double STDCALL getArgAsNumber(LINUX_ARGS(void* thisPointer,) void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<double, 5>(params, index);
    inventory_changer::InventoryChanger::instance().getArgAsNumberHook(static_cast<int>(result), RETURN_ADDRESS());
    return result;
}

static const char* STDCALL getArgAsString(LINUX_ARGS(void* thisPointer,) void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<const char*, 7>(params, index);

    if (result)
        inventory_changer::InventoryChanger::instance().getArgAsStringHook(result, RETURN_ADDRESS(), params);

    return result;
}

static void STDCALL setResultInt(LINUX_ARGS(void* thisPointer, ) void* params, int result) noexcept
{
    result = inventory_changer::InventoryChanger::instance().setResultIntHook(RETURN_ADDRESS(), params, result);
    hooks->panoramaMarshallHelper.callOriginal<void, WIN32_LINUX(14, 11)>(params, result);
}

static unsigned STDCALL getNumArgs(LINUX_ARGS(void* thisPointer, ) void* params) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<unsigned, 1>(params);
    inventory_changer::InventoryChanger::instance().getNumArgsHook(result, RETURN_ADDRESS(), params);
    return result;
}


static void STDCALL updateInventoryEquippedState(LINUX_ARGS(void* thisPointer, ) CSPlayerInventory* inventory, std::uint64_t itemID, csgo::Team team, int slot, bool swap) noexcept
{
    inventory_changer::InventoryChanger::instance().onItemEquip(team, slot, itemID);
    return hooks->inventoryManager.callOriginal<void, WIN32_LINUX(29, 30)>(inventory, itemID, team, slot, swap);
}

static void STDCALL soUpdated(LINUX_ARGS(void* thisPointer, ) SOID owner, SharedObject* object, int event) noexcept
{
    inventory_changer::InventoryChanger::instance().onSoUpdated(object);
    hooks->inventory.callOriginal<void, 1>(owner, object, event);
}

static bool STDCALL dispatchUserMessage(LINUX_ARGS(void* thisPointer, ) csgo::UserMessageType type, int passthroughFlags, int size, const void* data) noexcept
{
    if (type == csgo::UserMessageType::Text)
        inventory_changer::InventoryChanger::instance().onUserTextMsg(data, size);
    else if (type == csgo::UserMessageType::VoteStart)
        Misc::onVoteStart(data, size);
    else if (type == csgo::UserMessageType::VotePass)
        Misc::onVotePass();
    else if (type == csgo::UserMessageType::VoteFailed)
        Misc::onVoteFailed();
    else if (type == csgo::UserMessageType::SayText2 || type == csgo::UserMessageType::SayText)
        //Fun::repeatSomeoneMessage(data, size);


    if (type == csgo::UserMessageType::Text || type == csgo::UserMessageType::HudMsg || type == csgo::UserMessageType::SayText)
    {
        if (Misc::AdBlockEnabled() && !(*(memory->gameRules))->isValveDS())
            return true;
    }

    return hooks->client.callOriginal<bool, 38>(type, passthroughFlags, size, data);
}

static void FASTCALL performScreenOverlayHook(void* thisPointer, void* edx, int x, int y, int width, int height) noexcept
{
    static auto original = hooks->performScreenOverlay.getOriginal<void>(x, y, width, height);

    if (!Misc::AdBlockEnabled() || (*(memory->gameRules))->isValveDS())
        return original(thisPointer, x, y, width, height);
}

static bool STDCALL isDepthOfFieldEnabledHook() noexcept
{
    Visuals::motionBlur(nullptr);
    Visuals::dontRenderTeammates();
    return false;
}

static char FASTCALL newFunctionClientBypass(void* thisPointer, void* edx, const char* moduleName) noexcept
{
    return 1;
}

static char FASTCALL newFunctionEngineBypass(void* thisPointer, void* edx, const char* moduleName) noexcept
{
    return 1;
}

static char FASTCALL newFunctionStudioRenderBypass(void* thisPointer, void* edx, const char* moduleName) noexcept
{
    return 1;
}

static char FASTCALL newFunctionMaterialSystemBypass(void* thisPointer, void* edx, const char* moduleName) noexcept
{
    return 1;
}

static void FASTCALL updateFlashBangEffectHook(void* thisPointer, void* edx) noexcept
{
    const auto entity = reinterpret_cast<Entity*>(thisPointer);
    const float flashMaxAlpha = 255.0f - Visuals::reduceFlashEffect() * 2.55f;
    if ((entity->flashBangTime() < memory->globalVars->currenttime) || (flashMaxAlpha <= 0.0f))
    {
        // FlashBang is inactive
        entity->flashScreenshotAlpha() = 0.0f;
        entity->flashOverlayAlpha() = 0.0f;
        return;
    }

    static const float FLASH_BUILD_UP_PER_FRAME = 45.0f;
    static const float FLASH_BUILD_UP_DURATION = (255.0f / FLASH_BUILD_UP_PER_FRAME) * (1.0f / 60.0f);

    const float flashTimeElapsed = entity->getFlashTimeElapsed();

    if (entity->flashBuildUp())
    {
        // build up
        entity->flashScreenshotAlpha() = std::clamp((flashTimeElapsed / FLASH_BUILD_UP_DURATION) * flashMaxAlpha, 0.0f, flashMaxAlpha);
        entity->flashOverlayAlpha() = entity->flashScreenshotAlpha();

        if (flashTimeElapsed >= FLASH_BUILD_UP_DURATION)
            entity->flashBuildUp() = false;
    }
    else
    {
        // cool down
        const float flashTimeLeft = entity->flashBangTime() - memory->globalVars->currenttime;
        entity->flashScreenshotAlpha() = (flashMaxAlpha * flashTimeLeft) / entity->flashDuration();
        entity->flashScreenshotAlpha() = std::clamp(entity->flashScreenshotAlpha(), 0.0f, flashMaxAlpha);

        float alphaPercentage = 1.0f;
        const float certainBlindnessTimeThresh = 3.0f; // yes this is a magic number, necessary to match CS/CZ flashbang effectiveness cause the rendering system is completely different.

        if (flashTimeLeft > certainBlindnessTimeThresh)
        {
            // if we still have enough time of blindness left, make sure the player can't see anything yet.
            alphaPercentage = 1.0f;
        }
        else
        {
            // blindness effects shorter than 'certainBlindness`TimeThresh' will start off at less than 255 alpha.
            alphaPercentage = flashTimeLeft / certainBlindnessTimeThresh;

            // reduce alpha level quicker with dx 8 support and higher to compensate
            // for having the burn-in effect.
            alphaPercentage *= alphaPercentage;
        }

        entity->flashOverlayAlpha() = alphaPercentage *= flashMaxAlpha; // scale a [0..1) value to a [0..MaxAlpha] value for the alpha.

        // make sure the alpha is in the range of [0..MaxAlpha]
        entity->flashOverlayAlpha() = std::clamp(entity->flashOverlayAlpha(), 0.0f, flashMaxAlpha);
    }
}

static void __fastcall setVisualsDataTargetHook(void* thisPointer, void* edx, const char* shaderName) {
    static auto original = hooks->visualsData.getOriginal<void>(edx, shaderName);
    original(thisPointer, edx, shaderName);
    if (!interfaces->engine->isInGame())
        return;

    auto data = reinterpret_cast<weaponVisualsData*>((uintptr_t(thisPointer) - 0x4));
    std::string buffer;

    if (!data)
        return;

    if (!debug.customKnife.enabled)
        return;

    for (int i = 0; i <= 260; i++)
        buffer.append(&data->weaponPath[i]);

    if (!strstr(buffer.c_str(), "knife"))
        return;

    data->color1R = debug.customKnife.color1.color[0] * 255;
    data->color1G = debug.customKnife.color1.color[1] * 255;
    data->color1B = debug.customKnife.color1.color[2] * 255;
    data->color2R = debug.customKnife.color2.color[0] * 255;
    data->color2G = debug.customKnife.color2.color[1] * 255;
    data->color2B = debug.customKnife.color2.color[2] * 255;
    data->color3R = debug.customKnife.color3.color[0] * 255;
    data->color3G = debug.customKnife.color3.color[1] * 255;
    data->color3B = debug.customKnife.color3.color[2] * 255;
    data->color4R = debug.customKnife.color4.color[0] * 255;
    data->color4G = debug.customKnife.color4.color[1] * 255;
    data->color4B = debug.customKnife.color4.color[2] * 255;
}

#pragma region Animation Hooks

static bool __fastcall shouldSkipAnimationFrameHook(void* thisPointer, void* edx) noexcept
{
    return false;
}

static void __fastcall calculateViewHook(void* thisPointer, void* edx, float* eyeOrigin, int eyeAngles, int zNear, int zFar, float* fov) noexcept
{
    static auto original = hooks->calculateView.getOriginal<void>(eyeOrigin, eyeAngles, zNear, zFar, fov);

    auto entity = reinterpret_cast<Entity*>(thisPointer);

    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer, eyeOrigin, eyeAngles, zNear, zFar, fov);

    const auto oldUseNewAnimationState = entity->useNewAnimationState();

    entity->useNewAnimationState() = false;

    original(thisPointer, eyeOrigin, eyeAngles, zNear, zFar, fov);

    entity->useNewAnimationState() = oldUseNewAnimationState;
}

static void __fastcall buildTransformationsHook(void* thisPointer, void* edx, CStudioHdr* hdr, void* pos, void* q, matrix3x4* cameraTransform, int boneMask, void* boneComputed) noexcept
{
    static auto original = hooks->buildTransformations.getOriginal<void>(hdr, pos, q, cameraTransform, boneMask, boneComputed);

    const auto entity = reinterpret_cast<Entity*>(thisPointer);
    if (entity && entity->isAlive() && localPlayer && localPlayer.get() == entity && entity->getAnimstate())
    {
        Animations::saveCorrectAngle(entity->index(), Vector{ entity->getAnimstate()->eyePitch, entity->getAnimstate()->eyeYaw, Animations::getLocalAngle()->z });
        entity->collisionChangeTime() = 0.0f;
    }

    const UtlVector<int> backupFlags = hdr->boneFlags;

    for (int i = 0; i < hdr->boneFlags.size; i++)
    {
        if (Visuals::jiggleBones())
            hdr->boneFlags.elements[i] &= ~0x04;
        else
            hdr->boneFlags.elements[i] |= 0x04;
    }

    original(thisPointer, hdr, pos, q, cameraTransform, boneMask, boneComputed);

    hdr->boneFlags = backupFlags;
}

static void __fastcall doExtraBoneProcessingHook(void* thisPointer, void* edx, void* hdr, void* pos, void* q, const matrix3x4& matrix, uint8_t* bone_list, void* context) noexcept
{
    return;
}

static void __fastcall standardBlendingRulesHook(void* thisPointer, void* edx, void* hdr, void* pos, void* q, float currentTime, int boneMask) noexcept
{
    static auto original = hooks->standardBlendingRules.getOriginal<void>(hdr, pos, q, currentTime, boneMask);

    const auto entity = reinterpret_cast<Entity*>(thisPointer);

    entity->getEffects() |= 8;

    original(thisPointer, hdr, pos, q, currentTime, boneMask);

    entity->getEffects() &= ~8;
}

static Vector* __fastcall eyeAnglesHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->eyeAngles.getOriginal<Vector*>();

    const auto entity = reinterpret_cast<Entity*>(thisPointer);
    if (!localPlayer || entity != localPlayer.get())
        return original(thisPointer);

    if (std::uintptr_t(_ReturnAddress()) != memory->eyePositionAndVectors)
        return original(thisPointer);

    if (Animations::buildTransformationsIndex() == -1 || Animations::buildTransformationsIndex() != entity->index())
        return original(thisPointer);

    Animations::buildTransformationsIndex() = -1;

    return Animations::getCorrectAngle();
}

static void __vectorcall updateStateHook(void* thisPointer, void* unknown, float z, float y, float x, void* unknown1) noexcept
{
    using updateStateFn = void(__vectorcall*)(void*, void*, float, float, float, void*);
    static auto original = (updateStateFn)hooks->updateState.getDetour();

    auto animState = reinterpret_cast<AnimState*>(thisPointer);
    if (!animState)
        return;

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity || !entity->getModelPtr())
        return;

    return original(thisPointer, unknown, z, y, x, unknown1);
}

static void __fastcall resetStateHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->resetState.getOriginal<void>();

    auto animState = reinterpret_cast<AnimState*>(thisPointer);

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity)
        return original(thisPointer);

    original(thisPointer);

    animState->lowerBodyRealignTimer = 0.f;
    animState->deployRateLimiting = false;
    animState->jumping = false;
    animState->buttons = 0;
}

static void __fastcall updateClientSideAnimationHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->updateClientSideAnimation.getOriginal<void>();

    auto entity = reinterpret_cast<Entity*>(thisPointer);

    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || interfaces->engine->isHLTV())
        return original(thisPointer);

    if (entity != localPlayer.get())
    {
        if (Animations::isEntityUpdating())
            return original(thisPointer);
        return;
    }
    else if (entity == localPlayer.get())
    {
        if (Animations::isLocalUpdating())
            return original(thisPointer);
        return;
    }
}

static void __fastcall checkForSequenceChangeHook(void* thisPointer, void* edx, void* hdr, int currentSequence, bool forceNewSequence, bool interpolate) noexcept
{
    static auto original = hooks->checkForSequenceChange.getOriginal<void>(hdr, currentSequence, forceNewSequence, interpolate);

    return original(thisPointer, hdr, currentSequence, forceNewSequence, true);
}

static void __fastcall modifyEyePositionHook(void* thisPointer, void* edx, unsigned int* pos) noexcept
{
    static auto original = hooks->modifyEyePosition.getOriginal<void>(pos);

    auto animState = reinterpret_cast<AnimState*>(thisPointer);

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer, pos);

    const int bone = memory->lookUpBone(entity, "head_0");
    if (bone == -1)
        return;

    Vector eyePosition = reinterpret_cast<Vector&>(pos);

    if (animState->landing || animState->animDuckAmount != 0.f || !entity->groundEntity())
    {
        Vector bonePos;
        entity->getBonePos(bone, bonePos);
        bonePos.z += 1.7f;

        if (bonePos.z < eyePosition.z)
        {
            float lerpFraction = Helpers::simpleSplineRemapValClamped(fabsf(eyePosition.z - bonePos.z),
                FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MIN,
                FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MAX,
                0.0f, 1.0f);

            eyePosition.z = Helpers::lerp(lerpFraction, eyePosition.z, bonePos.z);
        }
    }
    pos = reinterpret_cast<unsigned int*>(&eyePosition);
}

static void __fastcall setupVelocityHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->setupVelocity.getOriginal<void>();

    auto animState = reinterpret_cast<AnimState*>(thisPointer);

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer);

    if (Animations::isFakeUpdating())
        return original(thisPointer);

    animState->setupVelocity();
}

static void __fastcall setupMovementHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->setupMovement.getOriginal<void>();

    auto animState = reinterpret_cast<AnimState*>(thisPointer);

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer);

    if (Animations::isFakeUpdating())
        return original(thisPointer);

    animState->setupMovement();
}

static void __fastcall setupAliveloopHook(void* thisPointer, void* edx) noexcept
{
    static auto original = hooks->setupAliveloop.getOriginal<void>();

    auto animState = reinterpret_cast<AnimState*>(thisPointer);

    auto entity = reinterpret_cast<Entity*>(animState->player);
    if (!entity || !entity->isAlive() || !entity->isPlayer() || !localPlayer || entity != localPlayer.get())
        return original(thisPointer);

    if (Animations::isFakeUpdating())
        return original(thisPointer);

    animState->setupAliveLoop();
}

static bool __fastcall setupBonesHook(void* thisPointer, void* edx, matrix3x4* boneToWorldOut, int maxBones, int boneMask, float currentTime) noexcept
{
    static auto original = hooks->setupBones.getOriginal<bool>(boneToWorldOut, boneMask, maxBones, currentTime);

    auto entity = reinterpret_cast<Entity*>(reinterpret_cast<uintptr_t>(thisPointer) - 4);

    if (!entity || !localPlayer || localPlayer.get() != entity || interfaces->engine->isHLTV())
        return original(thisPointer, boneToWorldOut, maxBones, boneMask, currentTime);

    if (!memory->input->isCameraInThirdPerson)
    {
        memory->setAbsAngle(localPlayer.get(), Vector{ 0.f, Animations::getFootYaw(), 0.f });
        return original(thisPointer, boneToWorldOut, maxBones, boneMask, currentTime);
    }

    if (Animations::isFakeUpdating())
        return original(thisPointer, boneToWorldOut, maxBones, boneMask, currentTime);

    if (!Animations::isLocalUpdating())
    {
        const auto poseParameters = localPlayer->poseParameters();
        localPlayer->poseParameters() = Animations::getPoseParameters();

        std::array<AnimationLayer, 13> layers;
        std::memcpy(&layers, localPlayer->animOverlays(), sizeof(AnimationLayer) * localPlayer->getAnimationLayersCount());

        std::array<AnimationLayer, 13> layer = Animations::getAnimLayers();
        std::memcpy(localPlayer->animOverlays(), &layer, sizeof(AnimationLayer) * localPlayer->getAnimationLayersCount());

        memory->setAbsAngle(localPlayer.get(), Vector{ 0.f, Animations::getFootYaw(), 0.f });
        original(thisPointer, nullptr, maxBones, boneMask, currentTime);

        localPlayer->poseParameters() = poseParameters;
        std::memcpy(localPlayer->animOverlays(), &layers, sizeof(AnimationLayer) * localPlayer->getAnimationLayersCount());

        if (boneToWorldOut)
        {
            auto renderOrigin = entity->getRenderOrigin();
            auto realMatrix = Animations::getRealMatrix();
            for (auto& i : realMatrix)
            {
                i[0][3] += renderOrigin.x;
                i[1][3] += renderOrigin.y;
                i[2][3] += renderOrigin.z;
            }
            memcpy(boneToWorldOut, realMatrix.data(), sizeof(matrix3x4) * maxBones);
            renderOrigin = entity->getRenderOrigin();
            for (auto& i : realMatrix)
            {
                i[0][3] -= renderOrigin.x;
                i[1][3] -= renderOrigin.y;
                i[2][3] -= renderOrigin.z;
            }
        }
        return true;
    }
    else
        return original(thisPointer, boneToWorldOut, maxBones, boneMask, currentTime);
}

static void __fastcall postDataUpdateHook(void* thisPointer, void* edx, int updateType) noexcept
{
    static auto original = hooks->postDataUpdate.getOriginal<void>(updateType);

    original(thisPointer, updateType);

    Animations::postDataUpdate();
    return;
}


#pragma endregion

#ifdef _WIN32

static void* STDCALL allocKeyValuesMemory(LINUX_ARGS(void* thisPointer, ) int size) noexcept
{
    if (const auto returnAddress = RETURN_ADDRESS(); returnAddress == memory->keyValuesAllocEngine || returnAddress == memory->keyValuesAllocClient)
        return nullptr;
    return hooks->keyValuesSystem.callOriginal<void*, 2>(size);
}

static int __fastcall keyEvent(void* thisPointer, void* edx, int down, int keynum, const char* currentBinding) noexcept
{
    Movement::handleKeyEvent(keynum, currentBinding);
    return hooks->clientMode.callOriginal<int, 20>(down, keynum, currentBinding);
}

void resetAll(int resetType) noexcept
{
    Animations::reset();
    EnginePrediction::reset();
    Glow::clearCustomObjects();
    Tickbase::reset();
}

static void FASTCALL levelShutDown(void* thisPointer) noexcept
{
    static auto original = hooks->client.getOriginal<void, 7>();

    original(thisPointer);
    resetAll(0);
}

Hooks::Hooks(HMODULE moduleHandle) noexcept : moduleHandle{ moduleHandle }
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // interfaces and memory shouldn't be initialized in wndProc because they show MessageBox on error which would cause deadlock
    interfaces.emplace(Interfaces{});
    memory.emplace(Memory{});
    dynamicClassId.emplace(ClassIdManager{}); //Fog Controller

    window = FindWindowW(L"Valve001", nullptr);
    originalWndProc = WNDPROC(SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(&wndProc)));
}

#endif

void Hooks::install() noexcept
{
    printf("[+] Installing\n");
#ifdef _WIN32
    originalPresent = **reinterpret_cast<decltype(originalPresent)**>(memory->present);
    **reinterpret_cast<decltype(present)***>(memory->present) = present;
    originalReset = **reinterpret_cast<decltype(originalReset)**>(memory->reset);
    **reinterpret_cast<decltype(reset)***>(memory->reset) = reset;

    if constexpr (std::is_same_v<HookType, MinHook>)
        MH_Initialize();
#else
    ImGui_ImplOpenGL3_Init();

    swapWindow = *reinterpret_cast<decltype(swapWindow)*>(memory->swapWindow);
    *reinterpret_cast<decltype(::swapWindow)**>(memory->swapWindow) = ::swapWindow;

#endif
    printf("[+] Detouring Modules\n");
    newFunctionClientDLL.detour(memory->newFunctionClientDLL, newFunctionClientBypass);
    newFunctionEngineDLL.detour(memory->newFunctionEngineDLL, newFunctionEngineBypass);
    newFunctionStudioRenderDLL.detour(memory->newFunctionStudioRenderDLL, newFunctionStudioRenderBypass);
    newFunctionMaterialSystemDLL.detour(memory->newFunctionMaterialSystemDLL, newFunctionMaterialSystemBypass);
    sendDatagram.detour(memory->sendDatagram, sendDatagramHook);
    particleCollectionSimulate.detour(memory->particleCollection, particleCollectionSimulateHook);
    performScreenOverlay.detour(memory->performScreenOverlay, performScreenOverlayHook);
    isDepthOfFieldEnabled.detour(memory->isDepthOfFieldEnabled, isDepthOfFieldEnabledHook);
    calcViewBob.detour(memory->calcViewBob, calcViewBobHook);
    SFUIHook.detour(memory->SFUI, SettingsChatText);
    //visualsData.detour(memory->setVisualsDataTarget, setVisualsDataTargetHook);
    updateFlashBangEffect.detour(memory->updateFlashBangEffect, updateFlashBangEffectHook);
    //removeScope.detour(memory->removeScope, removeScopeHook);
    getColorModulation.detour(memory->getColorModulation, getColorModulationHook);
    traceFilterForHeadCollision.detour(memory->traceFilterForHeadCollision, traceFilterForHeadCollisionHook);
    getClientModelRenderable.detour(memory->getClientModelRenderable, getClientModelRenderableHook);
    netGraph.detour(memory->netGraph, netGraphHook);
    //setText.detour(memory->bombText, setTextHook);
    originalGetAccountData = (getAccountData)DetourFunction((PBYTE)(memory->getAccountData), (PBYTE)getAccountDataHook);

    /*Tickbase*/
    physicsSimulate.detour(memory->physicsSimulate, physicsSimulateHook);
    clSendMove.detour(memory->clSendMove, clSendMoveHook);
    clMove.detour(memory->clMove, clMoveHook);
    /*Tickbase*/

#if defined(ENABLEFIXANIM)

    shouldSkipAnimationFrame.detour(memory->shouldSkipAnimationFrame, shouldSkipAnimationFrameHook);
    calculateView.detour(memory->calculateView, calculateViewHook);
    buildTransformations.detour(memory->buildTransformations, buildTransformationsHook);
    doExtraBoneProcessing.detour(memory->doExtraBoneProcessing, doExtraBoneProcessingHook);
    standardBlendingRules.detour(memory->standardBlendingRules, standardBlendingRulesHook);
    eyeAngles.detour(memory->standardBlendingRules, eyeAnglesHook);
    updateState.detour(memory->updateState, updateStateHook);
    resetState.detour(memory->resetState, resetStateHook);
    updateClientSideAnimation.detour(memory->updateClientSideAnimation, updateClientSideAnimationHook);
    checkForSequenceChange.detour(memory->checkForSequenceChange, checkForSequenceChangeHook);
    setupVelocity.detour(memory->setupVelocity, setupVelocityHook);
    setupMovement.detour(memory->setupMovement, setupMovementHook);
    setupAliveloop.detour(memory->setupAliveloop, setupAliveloopHook);
    postDataUpdate.detour(memory->postDataUpdate, postDataUpdateHook);
    setupBones.detour(memory->setupBones, setupBonesHook);
    modifyEyePosition.detour(memory->modifyEyePosition, modifyEyePositionHook);

#endif
    //halloweenMask.detour(memory->getHalloweenMaskModel, getHalloweenMaskHook);

    printf("[+] Hooking bspQuery\n");
    bspQuery.init(interfaces->engine->getBSPTreeQuery());
    bspQuery.hookAt(6, &listLeavesInBox);

    //memory->conColorMsg({ 0, 100 ,255 ,255 }, "[+] Hooking Client\n");
    printf("[+] Hooking Client\n");
    client.init(interfaces->client);
    client.hookAt(7, levelShutDown);
    client.hookAt(22, createMoveProxy);
    client.hookAt(24, writeUsercmdDeltaToBuffer);
    client.hookAt(37, &frameStageNotify);
    client.hookAt(38, &dispatchUserMessage);

    //printf("[+] Hooking Prediction\n");
    //prediction.init(interfaces->prediction);
    //dont fortget to uncomment prediction.restore() at ~1622

    printf("[+] Hooking ClientState\n");
    clientState.init((ClientState*)(uint32_t(memory->clientState) + 0x8));
    clientState.hookAt(5, packetStart);
    clientState.hookAt(39, processPacket);

    printf("[+] Hooking ClientMode\n");
    clientMode.init(memory->clientMode);
    clientMode.hookAt(WIN32_LINUX(17, 18), shouldDrawFog);
    clientMode.hookAt(WIN32_LINUX(18, 19), overrideView);
    clientMode.hookAt(20, keyEvent);
    clientMode.hookAt(WIN32_LINUX(27, 28), shouldDrawViewModel);
    clientMode.hookAt(WIN32_LINUX(35, 36), getViewModelFov);
    clientMode.hookAt(WIN32_LINUX(44, 45), doPostScreenEffects);
    clientMode.hookAt(WIN32_LINUX(58, 61), updateColorCorrectionWeights);

    printf("[+] Hooking Engine\n");
    engine.init(interfaces->engine);
    engine.hookAt(27, &isConnected);
    engine.hookAt(82, &isPlayingDemo);
    engine.hookAt(93, &isHltv);
    engine.hookAt(101, &getScreenAspectRatio);
    engine.hookAt(WIN32_LINUX(218, 219), getDemoPlaybackParameters);

#if defined(findMDLHook)
    printf("[+] Hooking MDLCache\n");
    mdlCache.setup(interfaces->mdlCache);
    mdlCache.hook_index(10, &findMDL);
#endif

    printf("[+] Hooking Key Values System\n");
    keyValuesSystem.init(memory->keyValuesSystem);
    keyValuesSystem.hookAt(2, allocKeyValuesMemory);

    printf("[+] Hooking FileSystem\n");
    fileSystem.init(interfaces->fileSystem);
    fileSystem.hookAt(101, getUnverifiedFileHashes);
    fileSystem.hookAt(128, canLoadThirdPartyFiles);

    printf("[+] Hooking GameMovement\n");
    gameMovement.init(interfaces->gameMovement);
    gameMovement.hookAt(1, processMovement);
    //gameMovement.hookAt(32, onJump);

    printf("[+] Hooking Inventory\n");
    inventory.init(memory->inventoryManager->getLocalInventory());
    inventory.hookAt(1, soUpdated);

    printf("[+] Hooking Inventory Manager\n");
    inventoryManager.init(memory->inventoryManager);
    inventoryManager.hookAt(WIN32_LINUX(29, 30), updateInventoryEquippedState);

    printf("[+] Hooking ModelRenderer\n");
    modelRender.init(interfaces->modelRender);
    modelRender.hookAt(21, drawModelExecute);

    printf("[+] Hooking PanoramaMarshallHelper\n");
    panoramaMarshallHelper.init(memory->panoramaMarshallHelper);
    panoramaMarshallHelper.hookAt(1, getNumArgs);
    panoramaMarshallHelper.hookAt(5, getArgAsNumber);
    panoramaMarshallHelper.hookAt(7, getArgAsString);
    panoramaMarshallHelper.hookAt(WIN32_LINUX(14, 11), setResultInt);

    //printf("[+] Hooking paintTraverse\n");
    //panel.init(interfaces->panel);
    //panel.hookAt(41, paintTraverse);

    printf("[+] Hooking Sound\n");
    sound.init(interfaces->sound);
    sound.hookAt(WIN32_LINUX(5, 6), &emitSound);

    printf("[+] Hooking Surface\n");
    surface.init(interfaces->surface);
    surface.hookAt(WIN32_LINUX(15, 14), &setDrawColor);

    printf("[+] Hooking svCheats\n"); // Dont use it with animations
    svCheats.init(interfaces->cvar->findVar("sv_cheats"));
    svCheats.hookAt(WIN32_LINUX(13, 16), &svCheatsGetBool);

    printf("[+] Hooking ViewRender\n");
    viewRender.init(memory->viewRender);
    viewRender.hookAt(WIN32_LINUX(39, 40), &render2dEffectsPreHud);
    viewRender.hookAt(WIN32_LINUX(41, 42), &renderSmokeOverlay);

    printf("[+] Hooking GameCoordinator\n");
    gameCoordinator.setup(memory->steamGameCoordinator);
    gameCoordinator.hook_index(2, hkGCRetrieveMessage);
    gameCoordinator.hook_index(0, hkGCSendMessage);

    printf("[+] Hooking Steam Matchmaking\n");
    steamMatchmakingHook.setup(steamMatchmaking);
    steamMatchmakingHook.hook_index(26, sendLobbyChatMessage);

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        originalDispatchSound = decltype(originalDispatchSound)(uintptr_t(memory->dispatchSound + 1) + *memory->dispatchSound);
        *memory->dispatchSound = uintptr_t(&dispatchSound) - uintptr_t(memory->dispatchSound + 1);
        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }

    surface.hookAt(28, &drawPrintText);
    surface.hookAt(67, &lockCursor);

    if constexpr (std::is_same_v<HookType, MinHook>)
        MH_EnableHook(MH_ALL_HOOKS);
}

#ifdef _WIN32

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

static DWORD WINAPI unload(HMODULE moduleHandle) noexcept
{
    Sleep(100);

    interfaces->inputSystem->enableInput(true);
    EventListener::remove();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    _CRT_INIT(moduleHandle, DLL_PROCESS_DETACH, nullptr);

    FreeLibraryAndExitThread(moduleHandle, 0);
}

#endif

void Hooks::uninstall() noexcept
{
    printf("[+] Uninstalling\n");
    interfaces->panoramaUIEngine->accessUIEngine()->RunScript(Panorama::getPanel(Panorama::CSGOPanel::CSGOMainMenu), "azurreUnhook();", "panorama/layout/mainmenu.xml", 8, 10, false, false);
    inventory_changer::InventoryChanger::instance().scheduleHudUpdate();
    Misc::updateEventListeners(true);
    Visuals::updateEventListeners(true);
    Clan::update(true);
#if defined(DISCORDRICHPRESENSE)
    Discord::Shutdown();
#endif

#ifdef _WIN32
    if constexpr (std::is_same_v<HookType, MinHook>) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
#endif
    printf("[+] Restoring\n");
    bspQuery.restore();
    client.restore();
    clientState.restore();
    //prediction.restore();
    clientMode.restore();
    engine.restore();
#ifdef _WIN32
    keyValuesSystem.restore();
#endif

    fileSystem.restore();
    gameMovement.restore();
    inventory.restore();
    inventoryManager.restore();
    modelRender.restore();
    panoramaMarshallHelper.restore();
    panel.restore();
    sound.restore();
    surface.restore();
    svCheats.restore();
    viewRender.restore();
    fileSystem.restore();

    gameCoordinator.unhook_all();
    steamMatchmakingHook.unhook_all();

#if defined(findMDLHook)
    mdlCache.unhook_all();
#endif

    DetourRemove(reinterpret_cast<BYTE*>(originalGetAccountData), reinterpret_cast<BYTE*>(getAccountDataHook));

    Netvars::restore();

    Glow::clearCustomObjects();
    inventory_changer::InventoryChanger::instance().reset();

#ifdef _WIN32
    SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(originalWndProc));
    **reinterpret_cast<void***>(memory->present) = originalPresent;
    **reinterpret_cast<void***>(memory->reset) = originalReset;

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        *memory->dispatchSound = uintptr_t(originalDispatchSound) - uintptr_t(memory->dispatchSound + 1);
        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }

    if (HANDLE thread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(unload), moduleHandle, 0, nullptr))
        CloseHandle(thread);
#else
    *reinterpret_cast<decltype(pollEvent)*>(memory->pollEvent) = pollEvent;
    *reinterpret_cast<decltype(swapWindow)*>(memory->swapWindow) = swapWindow;
#endif

    printf("[+] Azurre Has Been Unhooked - Restart Game Anyway!!!\n");

}

void Hooks::callOriginalDrawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);
}

#ifndef _WIN32

static int pollEvent(SDL_Event* event) noexcept
{
    [[maybe_unused]] static const auto once = []() noexcept {
        Netvars::init();
        EventListener::init();

        ImGui::CreateContext();
        config.emplace(Config{});

        gui.emplace(GUI{});

        hooks->install();

        return true;
    }();

    const auto result = hooks->pollEvent(event);

    if (result && ImGui_ImplSDL2_ProcessEvent(event) && gui->isOpen())
        event->type = 0;

    return result;
}

Hooks::Hooks() noexcept
{
    interfaces.emplace(Interfaces{});
    memory.emplace(Memory{});

    pollEvent = *reinterpret_cast<decltype(pollEvent)*>(memory->pollEvent);
    *reinterpret_cast<decltype(::pollEvent)**>(memory->pollEvent) = ::pollEvent;
}

#endif