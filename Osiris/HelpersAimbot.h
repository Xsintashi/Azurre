#pragma once

#include "../SDK/Entity.h"
#include "../SDK/Vector.h"
#include "../SDK/EngineTrace.h"

struct Vector;
struct UserCmd;
struct SurfaceData;
struct StudioBbox;
struct StudioHitboxSet;

namespace AimHelpers {
	Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept;
	bool handleTaserPenetration(UserCmd* cmd, Vector& angle, Vector& target) noexcept;
	bool hitChance(Entity* localPlayer, Entity* entity, StudioHitboxSet* set, const matrix3x4 matrix[MAXSTUDIOBONES], Entity* activeWeapon, const Vector& destination, const UserCmd* cmd, const int hitChance) noexcept;
	bool hitChanceWeapon(Entity* localPlayer, Entity* entity, Entity* weaponData, const Vector& destination, const UserCmd* cmd, const int hitChance) noexcept;
	void calculateArmorDamage(float armorRatio, int armorValue, bool hasHeavyArmor, float& damage) noexcept;
	bool canScan(Entity* entity, const Vector& destination, const WeaponInfo* weaponData, int minDamage, bool allowFriendlyFire) noexcept;
	float getScanDamage(Entity* entity, const Vector& destination, const WeaponInfo* weaponData, int minDamage, bool allowFriendlyFire) noexcept;
	bool hitboxIntersection(const matrix3x4 matrix[MAXSTUDIOBONES], int iHitbox, StudioHitboxSet* set, const Vector& start, const Vector& end) noexcept;
	std::vector<Vector> multiPoint(Entity* entity, const matrix3x4 matrix[MAXSTUDIOBONES], StudioBbox* hitbox, Vector localEyePos, int _hitbox, int _multiPoint);
}