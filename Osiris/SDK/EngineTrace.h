#pragma once

#include <cstddef>

#include "../Interfaces.h"

#include "Cvar.h"
#include "ConVar.h"
#include "Vector.h"
#include "VirtualMethod.h"
#include "WeaponData.h"

enum Contents_t {
    CONTENTS_EMPTY = 0,
    CONTENTS_SOLID = 0x1,
    CONTENTS_WINDOW = 0x2,
    CONTENTS_AUX = 0x4,
    CONTENTS_GRATE = 0x8,
    CONTENTS_SLIME = 0x10,
    CONTENTS_WATER = 0x20,
    CONTENTS_BLOCKLOS = 0x40,
    CONTENTS_OPAQUE = 0x80,
    CONTENTS_TESTFOGVOLUME = 0x100,
    CONTENTS_UNUSED = 0x200,
    CONTENTS_BLOCKLIGHT = 0x400,
    CONTENTS_TEAM1 = 0x800,
    CONTENTS_TEAM2 = 0x1000,
    CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
    CONTENTS_MOVEABLE = 0x4000,
    CONTENTS_AREAPORTAL = 0x8000,
    CONTENTS_PLAYERCLIP = 0x10000,
    CONTENTS_MONSTERCLIP = 0x20000,
    CONTENTS_CURRENT_0 = 0x40000,
    CONTENTS_CURRENT_90 = 0x80000,
    CONTENTS_CURRENT_180 = 0x100000,
    CONTENTS_CURRENT_270 = 0x200000,
    CONTENTS_CURRENT_UP = 0x400000,
    CONTENTS_CURRENT_DOWN = 0x800000,
    CONTENTS_ORIGIN = 0x1000000,
    CONTENTS_MONSTER = 0x2000000,
    CONTENTS_DEBRIS = 0x4000000,
    CONTENTS_DETAIL = 0x8000000,
    CONTENTS_TRANSLUCENT = 0x10000000,
    CONTENTS_LADDER = 0x20000000,
    CONTENTS_HITBOX = 0x40000000,
};

enum Masks {
    MASK_ALL = 0xFFFFFFFF,
    MASK_SOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
    MASK_PLAYERSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
    MASK_NPCSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
    MASK_NPCFLUID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
    MASK_WATER = CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME,
    MASK_OPAQUE = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE,
    MASK_OPAQUE_AND_NPCS = MASK_OPAQUE | CONTENTS_MONSTER,
    MASK_BLOCKLOS = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS,
    MASK_BLOCKLOS_AND_NPCS = MASK_BLOCKLOS | CONTENTS_MONSTER,
    MASK_VISIBLE = MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE,
    MASK_VISIBLE_AND_NPCS = MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE,
    MASK_SHOT = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE | CONTENTS_HITBOX,
    MASK_SHOT_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_DEBRIS,
    MASK_SHOT_HULL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE,
    MASK_SHOT_PORTAL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER,
    MASK_SOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE,
    MASK_PLAYERSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE,
    MASK_NPCSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
    MASK_NPCWORLDSTATIC = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
    MASK_NPCWORLDSTATIC_FLUID = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP,
    MASK_SPLITAREPORTAL = CONTENTS_WATER | CONTENTS_SLIME,
    MASK_CURRENT = CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN,
    MASK_DEADSOLID = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE,
};

struct Ray {
    Ray(const Vector& src, const Vector& dest)
        : start(src), delta(dest - src), extents(Vector{ }), startOffset(Vector{ }), worldAxisTransform(NULL), isRay(true) {
        isSwept = delta.x || delta.y || delta.z;
    }

    Ray(const Vector& src, const Vector& dest, const Vector& mins, const Vector& maxs)
        : delta(dest - src), extents(maxs - mins), startOffset(maxs + mins), worldAxisTransform(NULL)
    {
        isSwept = delta.x || delta.y || delta.z;
        extents *= 0.5f;
        isRay = (extents.squareLength() < 1e-6);

        startOffset *= 0.5f;
        start = src + startOffset;
        startOffset *= -1.0f;
    }
    Vector start{ };
    float pad{ };
    Vector delta{ };
    float pad1{ };
    Vector startOffset{ };
    float pad2{ };
    Vector extents{ };
    float pad3{ };
    const matrix3x4* worldAxisTransform;
    bool isRay{ };
    bool isSwept{ };
};

class Entity;

struct TraceFilter {
    TraceFilter(const Entity* entity) : skip{ entity } { }
    virtual bool shouldHitEntity(Entity* entity, int) { return entity != skip; }
    virtual int getTraceType() const { return 0; }
    const void* skip;
};

namespace HitGroup {
    enum {
        Invalid = -1,
        Generic,
        Head,
        Chest,
        Stomach,
        LeftArm,
        RightArm,
        LeftLeg,
        RightLeg,
        Gear = 10
    };

    float getDamageMultiplier(int hitGroup, const WeaponInfo* weaponData, bool hasHeavyArmor, int teamNumber) noexcept;
    bool isArmored(int hitGroup, bool helmet, int armorValue, bool hasHeavyArmor) noexcept;
}

struct Trace {
    Vector startpos;
    Vector endpos;
    struct Plane {
        constexpr auto operator!=(const Plane& v) const noexcept
        {
            return normal.x != v.normal.x || normal.y != v.normal.y || normal.z != v.normal.z;
        }
        Vector normal{ };
        float dist{ };
        std::byte type{ };
        std::byte signBits{ };
        std::byte pad[2]{ };
    } plane;
    float fraction;
    int contents;
    unsigned short dispFlags;
    bool allSolid;
    bool startSolid;
    std::byte pad1[4];
    struct Surface {
        const char* name;
        short surfaceProps;
        unsigned short flags;
    } surface;
    int hitgroup;
    std::byte pad2[4];
    Entity* entity;
    int hitbox;

    bool didHit() const
    {
        return fraction < 1.0f || allSolid || startSolid;
    }
};

// #define TRACE_STATS // - enable to see how many rays are cast per frame

#ifdef TRACE_STATS
#include "../Memory.h"
#include "GlobalVars.h"
#endif

class EngineTrace {
public:
    INCONSTRUCTIBLE(EngineTrace)

    VIRTUAL_METHOD(int, getPointContents, 0, (const Vector& absPosition, int contentsMask), (this, std::cref(absPosition), contentsMask, nullptr))
    VIRTUAL_METHOD(void, _traceRay, 5, (const Ray& ray, unsigned int mask, const TraceFilter& filter, Trace& trace), (this, std::cref(ray), mask, std::cref(filter), std::ref(trace)))
    VIRTUAL_METHOD(void, clipRayToEntity, 3, (const Ray& ray, unsigned int mask, Entity* pEnt, Trace& trace), (this, std::cref(ray), mask, pEnt, std::ref(trace)))

    void traceRay(const Ray& ray, unsigned int mask, const TraceFilter& filter, Trace& trace) noexcept
    {
#ifdef TRACE_STATS
        static int tracesThisFrame, lastFrame;

        if (lastFrame != memory->globalVars->framecount) {
            memory->debugMsg("traces: frame - %d | count - %d\n", lastFrame, tracesThisFrame);
            tracesThisFrame = 0;
            lastFrame = memory->globalVars->framecount;
        }

        ++tracesThisFrame;
#endif
        _traceRay(ray, mask, filter, trace);
    }
};
