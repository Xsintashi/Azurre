#include "KeyValues.h"
#include "../Memory.h"

KeyValues* KeyValues::fromString(const char* name, const char* value) noexcept
{
#ifdef _WIN32
    const auto keyValuesFromString = memory->keyValuesFromString;
    KeyValues* keyValues;
    __asm {
        push 0
        mov edx, value
        mov ecx, name
        call keyValuesFromString
        add esp, 4
        mov keyValues, eax
    }
    return keyValues;
#else
    return reinterpret_cast<KeyValues*(*)(const char*, const char*, const char**)>(memory->keyValuesFromString)(name, value, nullptr);
#endif
}

KeyValues* KeyValues::findKey(const char* keyName, bool create) noexcept
{
    return memory->keyValuesFindKey(this, keyName, create);
}

void KeyValues::setString(const char* keyName, const char* value) noexcept
{
    if (const auto key = findKey(keyName, true))
        memory->keyValuesSetString(key, value);
}

const char* KeyValues::getString(const char* keyName, const char* defaultValue)
{
    typedef const char* (__thiscall* OriginalFn)(PVOID, const char*, const char*);
    return ((OriginalFn)memory->keyValuesGetString)(this, keyName, defaultValue);
}
