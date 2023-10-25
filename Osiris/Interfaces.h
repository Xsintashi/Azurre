#pragma once

#include <optional>
#include <string>
#include <type_traits>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include "SDK/Platform.h"

class BaseFileSystem;
class Client;
class Cvar;
class Engine;
class EngineSound;
class EngineTrace;
class EntityList;
class FileSystem;
class GameEventManager;
class GameMovement;
class GameUI;
class InputSystem;
class Localize;
class MaterialSystem;
class MDLCache;
class ModelInfo;
class ModelRender;
class NetworkStringTableContainer;
class Panel;
class PanoramaUIEngine;
class PhysicsSurfaceProps;
class Prediction;
class RenderView;
class Surface;
class SoundEmitter;
class StudioRender;
class Server;
class DebugOverlay;
class VEffects;
class Effects;
class MatchFramework;

class Interfaces {
public:
#define GAME_INTERFACE(type, name, moduleName, version) \
type* name = reinterpret_cast<type*>(find(moduleName, version));
    
    GAME_INTERFACE(BaseFileSystem, baseFileSystem, FILESYSTEM_DLL, "VBaseFileSystem011")
    GAME_INTERFACE(Client, client, CLIENT_DLL, "VClient018")
    GAME_INTERFACE(Cvar, cvar, VSTDLIB_DLL, "VEngineCvar007")
    GAME_INTERFACE(Engine, engine, ENGINE_DLL, "VEngineClient014")
    GAME_INTERFACE(EngineTrace, engineTrace, ENGINE_DLL, "EngineTraceClient004")
    GAME_INTERFACE(EntityList, entityList, CLIENT_DLL, "VClientEntityList003")
    GAME_INTERFACE(FileSystem, fileSystem, FILESYSTEM_DLL, "VFileSystem017")

    GAME_INTERFACE(GameEventManager, gameEventManager, ENGINE_DLL, "GAMEEVENTSMANAGER002")
    GAME_INTERFACE(GameMovement, gameMovement, CLIENT_DLL, "GameMovement001")
    GAME_INTERFACE(GameUI, gameUI, CLIENT_DLL, "GameUI011")
    GAME_INTERFACE(InputSystem, inputSystem, INPUTSYSTEM_DLL, "InputSystemVersion001")
    GAME_INTERFACE(Localize, localize, LOCALIZE_DLL, "Localize_001")
    GAME_INTERFACE(MaterialSystem, materialSystem, MATERIALSYSTEM_DLL, "VMaterialSystem080")
    GAME_INTERFACE(MDLCache, mdlCache, DATACACHE_DLL, "MDLCache004")
    GAME_INTERFACE(ModelInfo, modelInfo, ENGINE_DLL, "VModelInfoClient004")
    GAME_INTERFACE(ModelRender, modelRender, ENGINE_DLL, "VEngineModel016")
    GAME_INTERFACE(NetworkStringTableContainer, networkStringTableContainer, ENGINE_DLL, "VEngineClientStringTable001")
    GAME_INTERFACE(Panel, panel, VGUI2_DLL, "VGUI_Panel009")
    GAME_INTERFACE(PanoramaUIEngine, panoramaUIEngine, PANORAMA_DLL, "PanoramaUIEngine001")
    GAME_INTERFACE(PhysicsSurfaceProps, physicsSurfaceProps, VPHYSICS_DLL, "VPhysicsSurfaceProps001")
    GAME_INTERFACE(Prediction, prediction, CLIENT_DLL, "VClientPrediction001")
    GAME_INTERFACE(RenderView, renderView, ENGINE_DLL, "VEngineRenderView014")
    GAME_INTERFACE(Surface, surface, VGUIMATSURFACE_DLL, "VGUI_Surface031")
    GAME_INTERFACE(EngineSound, sound, ENGINE_DLL, "IEngineSoundClient003")
    GAME_INTERFACE(SoundEmitter, soundEmitter, SOUNDEMITTERSYSTEM_DLL, "VSoundEmitter003")
    GAME_INTERFACE(StudioRender, studioRender, STUDIORENDER_DLL, "VStudioRender026")
    GAME_INTERFACE(Server, server, SERVER_DLL, "ServerGameDLL005")
    GAME_INTERFACE(DebugOverlay, debugOverlay, ENGINE_DLL, "VDebugOverlay004")
    GAME_INTERFACE(VEffects, iveffects, ENGINE_DLL, "VEngineEffects001")
    GAME_INTERFACE(Effects, effects, CLIENT_DLL, "IEffects001")
    GAME_INTERFACE(MatchFramework, matchFramework, MATCHMAKING_DLL, "MATCHFRAMEWORK_001")

#undef GAME_INTERFACE
private:
    static void* find(const char* moduleName, const char* name) noexcept
    {
        if (const auto createInterface = reinterpret_cast<std::add_pointer_t<void* CDECL_(const char* name, int* returnCode)>>(
#ifdef _WIN32
            GetProcAddress(GetModuleHandleA(moduleName), "CreateInterface")
#else
            dlsym(dlopen(moduleName, RTLD_NOLOAD | RTLD_LAZY), "CreateInterface")
#endif
            )) {
            if (void* foundInterface = createInterface(name, nullptr))
                return foundInterface;
        }

#ifdef _WIN32
        MessageBoxA(nullptr, ("Failed to find " + std::string{ name } + " interface!").c_str(), "Azurre", MB_OK | MB_ICONERROR);
#endif
        std::exit(EXIT_FAILURE);
    }
};

inline std::optional<const Interfaces> interfaces;
