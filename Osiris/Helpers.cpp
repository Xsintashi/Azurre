#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <fstream>
#include <random>
#include <span>
#include <string_view>
#include <unordered_map>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "imgui/imgui.h"

#include "Interfaces.h"
#include "Config.h"
#include "ConfigStructs.h"
#include "GameData.h"
#include "Helpers.h"
#include "Memory.h"
#include "SDK/Client.h"
#include "SDK/ClientMode.h"
#include "SDK/GlobalVars.h"
#include "SDK/Engine.h"
#include "SDK/ClientMode.h"
#include <iostream>

#pragma warning(disable:4996) //syfi build konsole 

static auto rainbowColor(float time, float speed, float alpha) noexcept
{
    constexpr float pi = std::numbers::pi_v<float>;
    return std::array{ std::sin(speed * time) * 0.5f + 0.5f,
                       std::sin(speed * time + 2 * pi / 3) * 0.5f + 0.5f,
                       std::sin(speed * time + 4 * pi / 3) * 0.5f + 0.5f,
                       alpha };
}

static float alphaFactor = 1.0f;

int Helpers::timeToTicks(float time) noexcept { return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick); }
float Helpers::ticksToTime(int ticks) noexcept { return static_cast<float>(ticks * memory->globalVars->intervalPerTick); }

void Helpers::concatTransforms(const matrix3x4& in1, const matrix3x4& in2, matrix3x4& out) {
    if (&in1 == &out) {
        matrix3x4 in1b;
        in1b = in1;
        concatTransforms(in1b, in2, out);
        return;
    }

    if (&in2 == &out) {
        matrix3x4 in2b;
        in2b = in2;
        concatTransforms(in1, in2b, out);
        return;
    }

    out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
    out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
    out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
    out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];

    out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
    out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
    out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
    out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];

    out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
    out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
    out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
    out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

void Helpers::MatrixAngles(const matrix3x4& matrix, Vector& angles) {
    Vector forward, left, up;

    // extract the basis vectors from the matrix. since we only need the z
    // component of the up vector, we don't get x and y.
    forward = { matrix[0][0], matrix[1][0], matrix[2][0] };
    left = { matrix[0][1], matrix[1][1], matrix[2][1] };
    up = { 0.f, 0.f, matrix[2][2] };

    float len = forward.length2D();

    if (len > 0.001f) {
        angles.x = Helpers::rad2deg(std::atan2(-forward.z, len));
        angles.y = Helpers::rad2deg(std::atan2(forward.y, forward.x));
        angles.z = Helpers::rad2deg(std::atan2(left.z, up.z));
    }

    else {
        angles.x = Helpers::rad2deg(std::atan2(-forward.z, len));
        angles.y = Helpers::rad2deg(std::atan2(-left.x, left.y));
        angles.z = 0.f;
    }
}

void Helpers::resetMatrix(Entity* entity, matrix3x4* boneCacheData, Vector origin, Vector absAngle, Vector mins, Vector maxs) noexcept
{
    memcpy(entity->getBoneCache().memory, boneCacheData, std::clamp(entity->getBoneCache().size, 0, MAXSTUDIOBONES) * sizeof(matrix3x4));
    memory->setAbsOrigin(entity, origin);
    memory->setAbsAngle(entity, Vector{ 0.f, absAngle.y, 0.f });
    entity->getCollideable()->setCollisionBounds(mins, maxs);
}

void Helpers::printfColored(int color, const char* text) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, color);
    std::cout << text << std::endl;
    SetConsoleTextAttribute(console, 3);
}

int Helpers::randomInt(int min, int max) noexcept
{
    return rand() % (max - min + 1) + min;
}

float Helpers::randomFloat(float min, float max) noexcept
{
    return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
};

float Helpers::clampCycle(float clycle) noexcept
{
    clycle -= (float)(int)clycle;

    if (clycle < 0.0f)
    {
        clycle += 1.0f;
    }
    else if (clycle > 1.0f)
    {
        clycle -= 1.0f;
    }

    return clycle;
}

float Helpers::approachValSmooth(float target, float value, float fraction)
{
    float delta = target - value;
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    delta *= fraction;
    return value + delta;
}

Vector Helpers::approach(Vector target, Vector value, float speed) noexcept
{
    Vector diff = (target - value);

    float delta = diff.length();
    if (delta > speed)
        value += diff.normalized() * speed;
    else if (delta < -speed)
        value -= diff.normalized() * speed;
    else
        value = target;

    return value;
}

float Helpers::approach(float target, float value, float speed) noexcept
{
    float delta = target - value;

    if (delta > speed)
        value += speed;
    else if (delta < -speed)
        value -= speed;
    else
        value = target;

    return value;
}

float Helpers::approachAngle(float target, float value, float speed) noexcept
{
    auto anglemod = [](float a)
    {
        a = (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
        return a;
    };
    target = anglemod(target);
    value = anglemod(value);

    float delta = target - value;

    if (speed < 0)
        speed = -speed;

    if (delta < -180)
        delta += 360;
    else if (delta > 180)
        delta -= 360;

    if (delta > speed)
        value += speed;
    else if (delta < -speed)
        value -= speed;
    else
        value = target;

    return value;
}

float Helpers::angleNormalize(float angle) noexcept
{
    angle = fmodf(angle, 360.0f);

    if (angle > 180.f)
        angle -= 360.f;

    if (angle < -180.f)
        angle += 360.f;

    return angle;
}

float Helpers::angleDiff(float destAngle, float srcAngle) noexcept
{
    float delta = std::fmodf(destAngle - srcAngle, 360.0f);

    if (destAngle > srcAngle)
    {
        if (delta >= 180)
            delta -= 360;
    }
    else
    {
        if (delta <= -180)
            delta += 360;
    }
    return delta;
}

float Helpers::bias(float x, float biasAmt) noexcept
{
    static float lastAmt = -1;
    static float lastExponent = 0;
    if (lastAmt != biasAmt)
    {
        lastExponent = log(biasAmt) * -1.4427f;
    }
    return pow(x, lastExponent);
}

float Helpers::smoothStepBounds(float edge0, float edge1, float x) noexcept
{
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
    return x * x * (3 - 2 * x);
}

float Helpers::remapValClamped(float val, float A, float B, float C, float D) noexcept
{
    if (A == B)
        return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    cVal = std::clamp(cVal, 0.0f, 1.0f);

    return C + (D - C) * cVal;
}

std::string Helpers::repeatString(std::string s, int n) {
    std::string repeat;

    for (int i = 0; i < n; i++)
        repeat += s;

    return repeat;
}

unsigned int Helpers::calculateColor(Color4 color) noexcept
{
    color.color[3] *= alphaFactor;
    color.color[3] *= (255.0f - GameData::local().flashDuration) / 255.0f;
    return ImGui::ColorConvertFloat4ToU32(color.rainbow ? rainbowColor(memory->globalVars->realtime, color.rainbowSpeed, color.color[3]) : color.color);
}

unsigned int Helpers::calculateColor(Color3 color) noexcept
{
    return ImGui::ColorConvertFloat4ToU32(color.rainbow ? rainbowColor(memory->globalVars->realtime, color.rainbowSpeed, 1.0f) : ImVec4{ color.color[0], color.color[1], color.color[2], 1.0f});
}

unsigned int Helpers::calculateColor(int r, int g, int b, int a) noexcept
{
    a -= static_cast<int>(a * GameData::local().flashDuration / 255.0f);
    return IM_COL32(r, g, b, a * alphaFactor);
}

bool Helpers::in_array(const std::string& value, const std::vector<std::string>& array)
{
    return std::find(array.begin(), array.end(), value) != array.end();
}

void Helpers::setAlphaFactor(float newAlphaFactor) noexcept
{
    alphaFactor = newAlphaFactor;
}

bool Helpers::getByteFromBytewise(int input, int byte) {
    return (input & 1 << byte) == 1 << byte;
}

static constexpr std::array RanksShort = {
    "Unrank",
    "S1",
    "S2",
    "S3",
    "S4",
    "S5",
    "S6",
    "G1",
    "G2",
    "G3",
    "G4",
    "MG2",
    "MG2",
    "MGE",
    "DMG",
    "LE",
    "LEM",
    "Supreme",
    "Global"
};

static constexpr std::array Ranks = {
    "Unranked/Expired",
    "Silver I",
    "Silver II",
    "Silver III",
    "Silver IV",
    "Silver Elite",
    "Silver Master Elite",
    "Gold Nova I",
    "Gold Nova II",
    "Gold Nova III",
    "Gold Nova Master",
    "Master Guardian I",
    "Master Guardian II",
    "Master Guardian Elite",
    "Distinguished Master Guardian",
    "Legendary Eagle",
    "Legendary Eagle Master",
    "Supreme Master First Class",
    "Global Elite"
};

const char* Helpers::ConvertRankFromNumber(bool shorten, int rank) noexcept {
    if (rank > 18) rank = 18;
    if (shorten)
        return RanksShort[rank];
    else 
        return Ranks[rank];
}

int Helpers::reverse(int in)
{
    return in * -1;
}

float Helpers::reverse(float in)
{
    return in * -1.f;
}

void Helpers::getAlertMessage(std::string text)
{
    char message[0x100] = "\x08\x04\x1A";
    BYTE textsize = (BYTE)text.length();
    memcpy(&message[3], &textsize, 1);
    memcpy(&message[4], text.c_str(), textsize);
    memcpy(&message[4 + textsize], "\x1A\x00\x1A\x00\x1A\x00\x1A\x00", 8);
    interfaces->client->dispatchUserMessage(7, 0, textsize + 12, &message);
}

// COLOR CONSOLE AND CHAT NOTIFICATIONS

static constexpr std::array hexColors = {
    "\x1",
    "\x2",
    "\x3",
    "\x4",
    "\x5",
    "\x6",
    "\x7",
    "\x8",
    "\x9",
    "\xA",
    "\xB",
    "\xC",
    "\xD",
    "\xE",
    "\xF",
    "\x10"
};

const char* Helpers::TextColors(int x) { // R.I.P every1's eyes rn looking at this pshit o_o ;o;
/*                              *\
        \x1  - White
        \x2  - Red
        \x3  - Pale Purple
        \x4  - Green
        \x5  - Pale green
        \x6  - Money green
        \x7  - Light Red
        \x8  - Gray
        \x9  - Yellow
        \xA  - Pale Blue Gray
        \xB  - Turquoise
        \xC  - Blue
        \xD  - Purple
        \xE  - Pink
        \xF  - Light Red 2
        \x10 - Gold
\*                              */                          
    return hexColors[x];
}

void Helpers::CheatChatNotification(bool shorten, const char* x, ...) {
#if defined(_DEBUG)
    memory->clientMode->getHudChat()->printf(0, std::string(" ").append(TextColors(config->style.prefixColor)).append(shorten ? " D" : " Debug").append(" \x8").append("| ").append(" \x01").append(x).append(" \x01").c_str());
    std::cout << "Azurre | " << x << std::endl;
#else
    memory->clientMode->getHudChat()->printf(0, std::string(" ").append(TextColors(config->style.prefixColor)).append(shorten ? " G" : " Azurre").append(" \x8").append("| ").append(" \x01").append(x).append(" \x01").c_str());
#endif
}

void Helpers::CheatConsoleNotification(Color4 colorek, const char* x, ...) {
#if defined(_DEBUG)
    memory->conColorMsg({ static_cast<unsigned char>(calculateColor(colorek)) }, std::string("Azurre | ").append(x).append("\n").c_str());
    std::cout << "Debug | " << x << std::endl;
#else
    memory->conColorMsg({ static_cast<unsigned char>(calculateColor(colorek)) }, std::string("Azurre | ").append(x).append("\n").c_str());
#endif
}

// COLOR CONSOLE AND CHAT NOTIFICATIONS

float Helpers::getAlphaFactor() noexcept
{
    return alphaFactor;
}

float Helpers::approachValueSmooth(float target, float value, float fraction) noexcept
{
    float delta = target - value;
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    delta *= fraction;
    return value + delta;
}

std::string Helpers::To_UTF8(const std::u16string& s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.to_bytes(s);
}

std::string Helpers::To_UTF8(const std::u32string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

std::u16string Helpers::To_UTF16(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.from_bytes(s);
}

std::u16string Helpers::To_UTF16(const std::u32string& s)
{
    std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> conv;
    std::string bytes = conv.to_bytes(s);
    return std::u16string(reinterpret_cast<const char16_t*>(bytes.c_str()), bytes.length() / sizeof(char16_t));
}

std::u32string Helpers::To_UTF32(const std::string& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

std::u32string Helpers::To_UTF32(const std::u16string& s)
{
    const char16_t* pData = s.c_str();
    std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> conv;
    return conv.from_bytes(reinterpret_cast<const char*>(pData), reinterpret_cast<const char*>(pData + s.length()));
}

float Helpers::normalizeYaw(float yaw) noexcept
{
    if (!std::isfinite(yaw))
        return 0.0f;

    if (yaw >= -180.f && yaw <= 180.f)
        return yaw;

    const float rot = std::round(std::abs(yaw / 360.f));

    yaw = (yaw < 0.f) ? yaw + (360.f * rot) : yaw - (360.f * rot);
    return yaw;
}

float Helpers::simpleSpline(float value) noexcept
{
    float valueSquared = value * value;

    return (3 * valueSquared - 2 * valueSquared * value);
}

float Helpers::simpleSplineRemapVal(float val, float A, float B, float C, float D) noexcept
{
    if (A == B)
        return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    return C + (D - C) * simpleSpline(cVal);
}

float Helpers::simpleSplineRemapValClamped(float val, float A, float B, float C, float D) noexcept
{
    if (A == B)
        return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    cVal = std::clamp(cVal, 0.0f, 1.0f);
    return C + (D - C) * simpleSpline(cVal);
}

Vector Helpers::lerp(float percent, Vector a, Vector b) noexcept
{
    return a + (b - a) * percent;
}

float Helpers::lerp(float percent, float a, float b) noexcept
{
    return a + (b - a) * percent;
}

void Helpers::convertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) noexcept
{
    ImGui::ColorConvertHSVtoRGB(h, s, v, outR, outG, outB);
}

void Helpers::healthColor(float fraction, float& outR, float& outG, float& outB) noexcept
{
    constexpr auto greenHue = 1.0f / 3.0f;
    constexpr auto redHue = 0.0f;
    convertHSVtoRGB(std::lerp(redHue, greenHue, fraction), 1.0f, 1.0f, outR, outG, outB);
}

unsigned int Helpers::healthColor(float fraction) noexcept
{
    float r, g, b;
    healthColor(fraction, r, g, b);
    return calculateColor(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
}

ImWchar* Helpers::getFontGlyphRanges() noexcept
{
    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        constexpr ImWchar baseRanges[]{
            0x0100, 0x024F, // Latin Extended-A + Latin Extended-B
            0x0300, 0x03FF, // Combining Diacritical Marks + Greek/Coptic
            0x0600, 0x06FF, // Arabic
            0x0E00, 0x0E7F, // Thai
            0
        };
        builder.AddRanges(baseRanges);
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.AddText("\u9F8D\u738B\u2122");
        builder.BuildRanges(&ranges);
    }
    return ranges.Data;
}

std::wstring Helpers::toWideString(const std::string& str) noexcept
{
    std::wstring wide(str.length(), L'\0');
    if (const auto newLen = std::mbstowcs(wide.data(), str.c_str(), wide.length()); newLen != static_cast<std::size_t>(-1))
        wide.resize(newLen);
    return wide;
}

static void toUpper(std::span<wchar_t> str) noexcept
{
    static std::unordered_map<wchar_t, wchar_t> upperCache;

    for (auto& c : str) {
        if (c >= 'a' && c <= 'z') {
            c -= ('a' - 'A');
        } else if (c > 127) {
            if (const auto it = upperCache.find(c); it != upperCache.end()) {
                c = it->second;
            } else {
                const auto upper = std::towupper(c);
                upperCache.emplace(c, upper);
                c = upper;
            }
        }
    }
}

std::wstring Helpers::toUpper(std::wstring str) noexcept
{
    ::toUpper(str);
    return str;
}

bool Helpers::decodeVFONT(std::vector<char>& buffer) noexcept
{
    constexpr std::string_view tag = "VFONT1";
    unsigned char magic = 0xA7;

    if (buffer.size() <= tag.length())
        return false;

    const auto tagIndex = buffer.size() - tag.length();
    if (std::memcmp(tag.data(), &buffer[tagIndex], tag.length()))
        return false;

    unsigned char saltBytes = buffer[tagIndex - 1];
    const auto saltIndex = tagIndex - saltBytes;
    --saltBytes;

    for (std::size_t i = 0; i < saltBytes; ++i)
        magic ^= (buffer[saltIndex + i] + 0xA7) % 0x100;

    for (std::size_t i = 0; i < saltIndex; ++i) {
        unsigned char xored = buffer[i] ^ magic;
        magic = (buffer[i] + 0xA7) % 0x100;
        buffer[i] = xored;
    }

    buffer.resize(saltIndex);
    return true;
}

std::vector<char> Helpers::loadBinaryFile(const std::string& path) noexcept
{
    std::vector<char> result;
    std::ifstream in{ path, std::ios::binary };
    if (!in)
        return result;
    in.seekg(0, std::ios_base::end);
    result.resize(static_cast<std::size_t>(in.tellg()));
    in.seekg(0, std::ios_base::beg);
    in.read(result.data(), result.size());
    return result;
}

void Helpers::Draw3DFilledCircleFilled(ImDrawList* drawList, const Vector& origin, float radius, ImU32 color) noexcept
{
    float step = (float)M_PI * 2.0f / 45.f;

    std::vector<Vector> points;
    points.reserve(45);
    for (float rotation = 0; rotation < (M_PI * 2.0f); rotation += step)
    {
        Vector point = { radius * cosf(rotation) + origin.x, radius * sinf(rotation) + origin.y, origin.z };
        points.push_back(point);
    }
    std::vector<ImVec2> screenPoints;
    screenPoints.reserve(45);
    for (const auto& point : points)
    {
        ImVec2 screenPoint = { 0.0f,0.0f };
        if (Helpers::worldToScreen(point, screenPoint))
        {
            screenPoints.push_back(screenPoint);
        }
    }
    if (screenPoints.empty())
        return;

    bool aa = false;
    if (drawList->Flags & ImDrawListFlags_AntiAliasedFill)
    {
        drawList->Flags &= ~ImDrawListFlags_AntiAliasedFill;
        aa = true;
    }
    drawList->AddConvexPolyFilled(screenPoints.data(), screenPoints.size(), color);

    if (aa)
        drawList->Flags |= ImDrawListFlags_AntiAliasedFill;
}

std::size_t Helpers::calculateVmtLength(const std::uintptr_t* vmt) noexcept
{
    std::size_t length = 0;
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION memoryInfo;
    while (VirtualQuery(LPCVOID(vmt[length]), &memoryInfo, sizeof(memoryInfo)) && memoryInfo.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
        ++length;
#else
    while (vmt[length])
        ++length;
#endif
    return length;
}

static bool transformWorldPositionToScreenPosition(const Matrix4x4& matrix, const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    const auto w = matrix._41 * worldPosition.x + matrix._42 * worldPosition.y + matrix._43 * worldPosition.z + matrix._44;
    if (w < 0.001f)
        return false;

    screenPosition = ImGui::GetIO().DisplaySize / 2.0f;
    screenPosition.x *= 1.0f + (matrix._11 * worldPosition.x + matrix._12 * worldPosition.y + matrix._13 * worldPosition.z + matrix._14) / w;
    screenPosition.y *= 1.0f - (matrix._21 * worldPosition.x + matrix._22 * worldPosition.y + matrix._23 * worldPosition.z + matrix._24) / w;
    return true;
}

bool Helpers::worldToScreen(const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    return transformWorldPositionToScreenPosition(GameData::toScreenMatrix(), worldPosition, screenPosition);
}

bool Helpers::worldToScreenPixelAligned(const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    const bool onScreen = transformWorldPositionToScreenPosition(GameData::toScreenMatrix(), worldPosition, screenPosition);
    screenPosition = ImFloor(screenPosition);
    return onScreen;
}

Vector Helpers::calculateRelativeAngle(const Vector& source, const Vector& destination) noexcept
{
    Vector delta = destination - source;
    Vector angles{ Helpers::rad2deg(atan2f(-delta.z, std::hypotf(delta.x, delta.y))), Helpers::rad2deg(atan2f(delta.y, delta.x)), 0.f };
    return angles.normalize();;
}
