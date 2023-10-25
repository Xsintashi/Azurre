#pragma once

#include <cstddef>
#include <memory>

#ifdef _WIN32
#include <x86RetSpoof.h>
#endif

#include "../Memory.h"
#include "../SDK/Platform.h"

class MinHook {
public:
    void init(void* base) noexcept;
    void detour(uintptr_t base, void* fun) noexcept;
    void restore() noexcept {}
    void hookAt(std::size_t index, void* fun) noexcept;
    void enable(std::size_t index) noexcept;

    template<typename T, std::size_t Idx = 0, typename ...Args>
    constexpr auto getOriginal(Args... args) const noexcept
    {
        if (!Idx)
            return reinterpret_cast<T(__thiscall*)(void*, Args...)>(original);
        return reinterpret_cast<T(__thiscall*)(void*, Args...)>(originals[Idx]);
    }

    template<typename T, std::size_t Idx = 0, typename ...Args>
    constexpr auto callOriginal(Args... args) const noexcept
    {
        return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(base), originals[Idx], memory->jmpEbxGadgetInClient, args...);
    }

    auto getDetour() noexcept
    {
        return original;
    }

private:
    void* base;
    std::unique_ptr<uintptr_t[]> originals;
    uintptr_t original;
};