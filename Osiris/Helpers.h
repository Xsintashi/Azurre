#pragma once

#include <concepts>
#include <cwctype>
#include <mutex>
#include <numbers>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "SDK/WeaponId.h"

struct Color3;
struct Color4;
struct Vector;
#define PI std::numbers::pi_v<float>

class matrix3x4;
class Entity;

namespace Helpers
{
    enum printfColors {
        Black,
        Blue,
        Green,
        Aqua,
        Red,
        Purple,
        Yellow,
        White,
        Gray,
        Light_Blue,
        Light_Green = 0xA,
        Light_Aqua = 0xB,
        Light_Red = 0xC,
        Light_Purple = 0xD,
        Light_Yellow = 0xE,
        Bright_White = 0xF
    };

    Vector approach(Vector target, Vector value, float speed) noexcept;
    float approachValSmooth(float target, float value, float fraction);
    float approach(float target, float value, float speed) noexcept;
    float approachAngle(float target, float value, float speed) noexcept;
    float angleDiff(float destAngle, float srcAngle) noexcept;
    float angleNormalize(float angle) noexcept;
    float remapValClamped(float val, float A, float B, float C, float D) noexcept;
    float smoothStepBounds(float edge0, float edge1, float x) noexcept;
    float bias(float x, float biasAmt) noexcept;
    int timeToTicks(float time) noexcept;
    float ticksToTime(int ticks) noexcept;
    void concatTransforms(const matrix3x4& in1, const matrix3x4& in2, matrix3x4& out);
    void MatrixAngles(const matrix3x4& matrix, Vector& angles);
    void resetMatrix(Entity* entity, matrix3x4* boneCacheData, Vector origin, Vector absAngle, Vector mins, Vector maxs) noexcept;
    void printfColored(int color, const char* text);
    int randomInt(int min, int max) noexcept;
    float randomFloat(float min, float max) noexcept;
    float clampCycle(float clycle) noexcept;
    std::string repeatString(std::string s, int n);unsigned int calculateColor(Color4 color) noexcept;
    unsigned int calculateColor(Color3 color) noexcept;
    unsigned int calculateColor(int r, int g, int b, int a) noexcept;
    bool in_array(const std::string& value, const std::vector<std::string>& array);
    void setAlphaFactor(float newAlphaFactor) noexcept;
    bool getByteFromBytewise(int input, int byte);
    const char* ConvertRankFromNumber(bool shorten, int rank) noexcept;
    int reverse(int in);
    float reverse(float in);
    void getAlertMessage(std::string text);
    const char* TextColors(int x);
    void CheatChatNotification(bool shorten, const char* x, ...);
    void CheatConsoleNotification(Color4 colorek, const char* x, ...);
    float getAlphaFactor() noexcept;
    float approachValueSmooth(float target, float value, float fraction) noexcept;
    float normalizeYaw(float yaw) noexcept;
    float simpleSpline(float value) noexcept;
    float simpleSplineRemapVal(float val, float A, float B, float C, float D) noexcept;
    float simpleSplineRemapValClamped(float val, float A, float B, float C, float D) noexcept;
    Vector lerp(float percent, Vector a, Vector b) noexcept;
    float lerp(float percent, float a, float b) noexcept;
    void convertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) noexcept;
    void healthColor(float fraction, float& outR, float& outG, float& outB) noexcept;
    unsigned int healthColor(float fraction) noexcept;

    constexpr auto units2meters(float units) noexcept
    {
        return units * 0.0254f;
    }

    ImWchar* getFontGlyphRanges() noexcept;

    constexpr std::uint8_t utf8SeqLen(char firstByte) noexcept
    {
        return (firstByte & 0x80) == 0x00 ? 1 :
               (firstByte & 0xE0) == 0xC0 ? 2 :
               (firstByte & 0xF0) == 0xE0 ? 3 :
               (firstByte & 0xF8) == 0xF0 ? 4 :
               0;
    }

    std::string To_UTF8(const std::u16string& s);

    std::string To_UTF8(const std::u32string& s);

    std::u16string To_UTF16(const std::string& s);

    std::u16string To_UTF16(const std::u32string& s);

    std::u32string To_UTF32(const std::string& s);

    std::u32string To_UTF32(const std::u16string& s);

    std::wstring toWideString(const std::string& str) noexcept;
    std::wstring toUpper(std::wstring str) noexcept;

    bool decodeVFONT(std::vector<char>& buffer) noexcept;
    std::vector<char> loadBinaryFile(const std::string& path) noexcept;

    void Draw3DFilledCircleFilled(ImDrawList* drawList, const Vector& origin, float radius, ImU32 color) noexcept;
    void Draw3DFilledCircle(ImDrawList* drawList, const Vector& origin, float radius, ImU32 color) noexcept;

    template <typename T> constexpr auto deg2rad(T degrees) noexcept { return degrees * (std::numbers::pi_v<T> / static_cast<T>(180)); }
    template <typename T> constexpr auto rad2deg(T radians) noexcept { return radians * (static_cast<T>(180) / std::numbers::pi_v<T>); }

    constexpr float angleDiffRad(float a1, float a2) noexcept
    {
        float delta;

        delta = rad2deg(a1 - a2);
        if (a1 > a2)
        {
            if (delta >= PI)
                delta -= PI * 2;
        }
        else
        {
            if (delta <= -PI)
                delta += PI * 2;
        }
        return delta;
    }

    [[nodiscard]] std::size_t calculateVmtLength(const std::uintptr_t* vmt) noexcept;

    constexpr auto isKnife(WeaponId id) noexcept
    {
        return (id >= WeaponId::Bayonet && id <= WeaponId::SkeletonKnife) || id == WeaponId::KnifeT || id == WeaponId::Knife;
    }

    constexpr auto bronzeEventCoinFromViewerPass(WeaponId id) noexcept
    {
        switch (id) {
        using enum WeaponId;
        case Katowice2019ViewerPass: return Katowice2019BronzeCoin;
        case Berlin2019ViewerPass:
        case Berlin2019ViewerPassWith3Tokens: return Berlin2019BronzeCoin;
        case Stockholm2021ViewerPass:
        case Stockholm2021ViewerPassWith3Tokens: return Stockholm2021BronzeCoin;
        case Antwerp2022ViewerPass:
        case Antwerp2022ViewerPassWith3Tokens: return Antwerp2022BronzeCoin;
        case Rio2022ViewerPass:
        case Rio2022ViewerPassWith3Tokens: return Rio2022BronzeCoin;
        case Paris2023ViewerPass:
        case Paris2023ViewerPassWith3Tokens: return Paris2023BronzeCoin;
        default: return WeaponId{};
        }
    }

    bool worldToScreen(const Vector& worldPosition, ImVec2& screenPosition) noexcept;
    bool worldToScreenPixelAligned(const Vector& worldPosition, ImVec2& screenPosition) noexcept;

    [[nodiscard]] constexpr auto isMP5LabRats(WeaponId weaponID, int paintKit) noexcept
    {
        return weaponID == WeaponId::Mp5sd && paintKit == 800;
    }

    Vector calculateRelativeAngle(const Vector& source, const Vector& destination) noexcept;
    class RandomGenerator {
    public:
        using GeneratorType = std::mt19937;
        using result_type = GeneratorType::result_type;

        static constexpr auto min()
        {
            return GeneratorType::min();
        }

        static constexpr auto max()
        {
            return GeneratorType::max();
        }

        auto operator()() const
        {
            std::scoped_lock lock{ mutex };
            return gen();
        }

        template <typename Distribution>
        auto operator()(Distribution&& distribution) const
        {
            std::scoped_lock lock{ mutex };
            return distribution(gen);
        }
    
    private:
        inline static GeneratorType gen{ std::random_device{}() };
        inline static std::mutex mutex;
    };

    class ToUpperConverter {
    public:
        std::wstring_view toUpper(std::wstring_view string)
        {
            assert(string.length() < buffer.size());
            std::size_t length = 0;
            for (auto c : string)
                buffer[length++] = toUpper(c);
            buffer[length] = '\0';
            return { buffer.data(), length };
        }

    private:
        wchar_t toUpper(wchar_t c)
        {
            if (c >= 'a' && c <= 'z') {
                return c - ('a' - 'A');
            }
            else if (c > 127) {
                if (const auto it = cache.find(c); it != cache.end()) {
                    return it->second;
                }
                else {
                    const auto upper = std::towupper(c);
                    cache.emplace(c, upper);
                    return upper;
                }
            }
            return c;
        }

        std::unordered_map<wchar_t, wchar_t> cache;
        std::array<wchar_t, 4096> buffer;
    };

}
