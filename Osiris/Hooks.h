#pragma once

#include <memory>
#include <type_traits>
#include <optional>

#include "Hooks/MinHook.h"
#include "Hooks/VmtHook.h"
#include "Hooks/VmtSwap.h"
#include "Hooks/vfunc_hook.hpp"

#include <charconv>
#include <functional>
#include <string>

#include "imgui/imgui.h"

#include "Memory.h"
#include "InventoryChanger/InventoryChanger.h"

#ifdef _WIN32
#include <d3d9.h>
#include <intrin.h>
#include <Windows.h>
#include <Psapi.h>

#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "MinHook/MinHook.h"

#elif __linux__
struct SDL_Window;
union SDL_Event;
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#endif

class matrix3x4;
class ClientState;
struct ModelRenderInfo;
struct SoundInfo;

#ifdef _WIN32
// Easily switch hooking method for all hooks, choose between MinHook/VmtHook/VmtSwap
using HookType = MinHook;
#else
using HookType = VmtSwap;
#endif

class Hooks {
public:
#ifdef _WIN32
    Hooks(HMODULE moduleHandle) noexcept;

    WNDPROC originalWndProc;
    std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)> originalPresent;
    std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)> originalReset;
#else
    Hooks() noexcept;

    std::add_pointer_t<int(SDL_Event*)> pollEvent;
    std::add_pointer_t<void(SDL_Window*)> swapWindow;
#endif

    void install() noexcept;
    void uninstall() noexcept;
    void callOriginalDrawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept;

    std::add_pointer_t<int FASTCALL(SoundInfo&)> originalDispatchSound;
    std::add_pointer_t<int FASTCALL(void*)> debugBreak;

    HookType bspQuery;
    HookType client;
    HookType clientMode;
    HookType engine;
    HookType inventory;
    HookType inventoryManager;
    HookType modelRender;
    HookType panoramaMarshallHelper;
    HookType sound;
    HookType prediction;
    vfunc_hook gameCoordinator;
    HookType computeShadowDepthTextures;
    HookType surface;
    HookType viewRender;
    HookType svCheats;
    HookType fileSystem;
    HookType panel;
    HookType gameMovement;
    HookType clientState;
    MinHook postDataUpdate;
    MinHook networkChannel;
    MinHook isDepthOfFieldEnabled;
    MinHook sendDatagram;
    MinHook performScreenOverlay;
    MinHook calcViewBob;
    MinHook SFUIHook;
    MinHook particleCollectionSimulate;
    MinHook halloweenMask;
    MinHook netGraph;
    MinHook physicsSimulate;
    MinHook standardBlendingRules;
    MinHook doExtraBoneProcessing;
    MinHook buildTransformations;
    MinHook modifyEyePosition;
    MinHook eyeAngles;
    MinHook setupBones;
    MinHook updateFlashBangEffect;
    MinHook visualsData;
    MinHook shouldSkipAnimationFrame;
    MinHook updateClientSideAnimation;
    MinHook calculateView;
    MinHook checkForSequenceChange;
    MinHook setupVelocity;
    MinHook setupMovement;
    MinHook setupAliveloop;
    MinHook updateState;
    MinHook resetState;
    MinHook setText;
    MinHook removeScope;
    MinHook removeScopeSway;
    MinHook getColorModulation;
    MinHook traceFilterForHeadCollision;
    MinHook getClientModelRenderable;
    vfunc_hook mdlCache;
    MinHook shutdown;
#ifdef _WIN32
    HookType keyValuesSystem;
#endif

    MinHook clSendMove;
    MinHook clMove;

    MinHook newFunctionClientDLL;
    MinHook newFunctionEngineDLL;
    MinHook newFunctionStudioRenderDLL;
    MinHook newFunctionMaterialSystemDLL;
    vfunc_hook steamMatchmakingHook;

private:
#ifdef _WIN32
    HMODULE moduleHandle;
    HWND window;
#endif
};

inline std::optional<Hooks> hooks;
