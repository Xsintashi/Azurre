#pragma once

#include <forward_list>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "SDK/Entity.h"
#include "SDK/matrix3x4.h"
#include "SDK/Vector.h"

#include "Texture.h"

struct LocalPlayerData;

struct PlayerData;
struct ObserverData;
struct WeaponData;
struct EntityData;
struct LootCrateData;
struct ProjectileData;
struct BombData;
struct InfernoData;
struct SmokeData;

struct Matrix4x4;

namespace GameData
{
    void update() noexcept;
    void clearProjectileList() noexcept;
    void clearTextures() noexcept;
    void clearUnusedAvatars() noexcept;

    class Lock {
    public:
        Lock() noexcept : lock{ mutex } {}
    private:
        std::scoped_lock<std::mutex> lock;
        static inline std::mutex mutex;
    };
    
    // Lock-free
    int getNetOutgoingLatency() noexcept;
    ImTextureID getLogo();
    ImTextureID getNade(int type);
    ImTextureID getAnime();
    ImTextureID getBaner();
    ImTextureID getDebug();
    void createNeededThirdPartyFiles();


    // You have to acquire Lock before using these getters
    const Matrix4x4& toScreenMatrix() noexcept;
    const LocalPlayerData& local() noexcept;
    const std::vector<PlayerData>& players() noexcept;
    const PlayerData* playerByHandle(int handle) noexcept;
    const std::vector<ObserverData>& observers() noexcept;
    const std::vector<WeaponData>& weapons() noexcept;
    const std::vector<EntityData>& entities() noexcept;
    const std::vector<LootCrateData>& lootCrates() noexcept;
    const std::forward_list<ProjectileData>& projectiles() noexcept;
    const BombData& plantedC4() noexcept;
    const std::vector<InfernoData>& infernos() noexcept;
    const std::vector<SmokeData>& smokes() noexcept;
}

struct LocalPlayerData {
    void update() noexcept;
    std::string name;
    bool exists = false;
    bool alive = false;
    bool inReload = false;
    bool shooting = false;
    bool noScope = false;
    bool drawingCrosshair;
    float nextWeaponAttack = 0.0f;
    int fov;
    int handle;
    float flashDuration;
    float velocityModifier;
    Vector eyePosition;
    Vector aimPunch;
    Vector origin;
    Vector velocity;
    Vector lookingAt;
    Vector inaccuracy;
};

class Entity;

struct BaseData {
    BaseData(Entity* entity) noexcept;

    float distanceToLocal;
    Vector obbMins, obbMaxs;
    matrix3x4 coordinateFrame;
};

struct EntityData final : BaseData {
    EntityData(Entity* entity) noexcept;
   
    const char* name;
};

struct ProjectileData : BaseData {
    ProjectileData(Entity* projectile) noexcept;

    void update(Entity* projectile) noexcept;

    constexpr auto operator==(int otherHandle) const noexcept
    {
        return handle == otherHandle;
    }

    bool exploded = false;
    bool thrownByLocalPlayer = false;
    bool thrownByEnemy = false;
    float explosionTime = 0.0f;
    int handle;
    const char* name = nullptr;
    std::vector<std::pair<float, Vector>> trajectory;
};

enum class Team;

struct PlayerData : BaseData {
    PlayerData(Entity* entity) noexcept;
    PlayerData(const PlayerData&) = delete;
    PlayerData& operator=(const PlayerData&) = delete;
    PlayerData(PlayerData&&) = default;
    PlayerData& operator=(PlayerData&&) = default;

    void update(Entity* entity) noexcept;
    [[nodiscard]] ImTextureID getAvatarTexture() const noexcept;
    [[nodiscard]] float fadingAlpha() const noexcept;

    bool dormant;
    bool enemy = false;
    bool visible = false;
    bool audible;
    bool spotted;
    bool inViewFrustum;
    bool alive;
    bool immune = false;
    bool hasHelmet;
    bool hasDefuse;
    bool hasBomb;
    bool isScoping;
    bool isFakeDucking;
    float flashDuration;
    float velocity;
    float lastContactTime = 0.0f;
    float maxDesyncAngle;
    float duckAmount;
    float duckSpeed;
    int money;
    int index;
    int health;
    int armor;
    int competitiveWins;
    int competitiveRank;
    int handle;
    int userId;
    csgo::Team team;
    std::uint64_t steamID;
    std::string name;
    std::string clanTag;
    Vector headMins, headMaxs;
    Vector origin;
    Vector lookingAt;
    Vector angle;
    std::string activeWeapon;
    std::vector<std::pair<Vector, Vector>> bones;
};

struct WeaponData : BaseData {
    WeaponData(Entity* entity) noexcept;

    int clip;
    int reserveAmmo;
    const char* group = "All";
    const char* name = "All";
    std::string displayName;
};

struct LootCrateData : BaseData {
    LootCrateData(Entity* entity) noexcept;

    const char* name = nullptr;
};

struct ObserverData {
    ObserverData(Entity* entity, Entity* obs, bool targetIsLocalPlayer) noexcept;

    int playerHandle;
    int targetHandle;
    bool targetIsLocalPlayer;
};

struct BombData {
    void update() noexcept;

    float blowTime;
    float timerLength;
    int defuserHandle;
    int bombHandle;
    float defuseCountDown;
    float defuseLength;
    int bombsite;
};

struct InfernoData {
    InfernoData(Entity* inferno) noexcept;
    Entity* owner;
    std::vector<Vector> points;
};

#define SMOKEGRENADE_LIFETIME 17.5f
#define MOLOTOV_LIFETIME 7.f

struct SmokeData {
    SmokeData(const Vector& origin, int handle) noexcept;

    [[nodiscard]] float fadingAlpha() const noexcept;

    Vector origin;
    float explosionTime;
    int handle;
};