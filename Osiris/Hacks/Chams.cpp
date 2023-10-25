#include "Chams.h"

#include <algorithm>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "Animations.h"
#include "Backtrack.h"
#include "Extra.h"
#include "FakeLag.h"
#include "Visuals.h"
#include "../Config.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../Interfaces.h"
#include "../Memory.h"

#include "../InputUtil.h"
#include "../SDK/Constants/ClassId.h"
#include "../SDK/ClientClass.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/ModelRender.h"
#include "../SDK/RenderView.h"
#include "../SDK/StudioRender.h"
#include "../SDK/Input.h"
#include "../SDK/KeyValues.h"
#include "../SDK/Utils.h"

static Material* normal;
static Material* flat;
static Material* animated;
static Material* platinum;
static Material* glass;
static Material* crystal;
static Material* chrome;
static Material* silver;
static Material* gold;
static Material* plastic;
static Material* glow;
static Material* pearlescent;
static Material* metallic;
static Material* smoke;
static Material* smoke2;
static Material* water;
static Material* melt;
static Material* leaves;
static Material* lines;
static Material* snowflakes;

static Material* Glock_18;
static Material* P2000;
static Material* USP_S;
static Material* DualBerettas;
static Material* P250;
static Material* Tec_9;
static Material* Five_Seven;
static Material* CZ_75;
static Material* DesertEagle;
static Material* Revolver;
static Material* Nova;
static Material* XM1014;
static Material* Sawed_off;
static Material* MAG_7;
static Material* M249;
static Material* Negev;
static Material* Mac_10;
static Material* MP9;
static Material* MP7;
static Material* MP5_SD;
static Material* UMP_45;
static Material* P90;
static Material* PP_Bizon;
static Material* GalilAR;
static Material* Famas;
static Material* AK_47;
static Material* M4A4;
static Material* M4A1_S;
static Material* SSG_08;
static Material* SG_553;
static Material* AUG;
static Material* AWP;
static Material* G3SG1;
static Material* SCAR_20;

static Material* custom0;
static Material* custom1;
static Material* custom2;
static Material* custom3;
static Material* custom4;
static Material* custom5;
static Material* custom6;
static Material* custom7;
static Material* custom8;
static Material* custom9;


static constexpr auto dispatchMaterial(int id) noexcept
{
    switch (id) {
    default:
    case 0: return normal;
    case 1: return flat;
    case 2: return animated;
    case 3: return platinum;
    case 4: return glass;
    case 5: return chrome;
    case 6: return crystal;
    case 7: return silver;
    case 8: return gold;
    case 9: return plastic;
    case 10: return glow;
    case 11: return pearlescent;
    case 12: return metallic;
    case 13: return smoke;
    case 14: return smoke2;
    case 15: return water;
    case 16: return leaves;
    case 17: return lines;
    case 18: return snowflakes;
    }
}

Material* dispatchWeaponMaterial(short id) noexcept
{
    switch (id) {
    default:return nullptr;
    case 1: return DesertEagle;
    case 2: return DualBerettas;
    case 3: return Five_Seven;
    case 4: return Glock_18;
    case 7: return AK_47;
    case 8: return AUG;
    case 9: return AWP;
    case 10: return Famas;
    case 11: return G3SG1;
    case 13: return GalilAR;
    case 14: return M249;
    case 16: return M4A4;
    case 17: return Mac_10;
    case 19: return P90;
    case 23: return MP5_SD;
    case 24: return UMP_45;
    case 25: return XM1014;
    case 26: return PP_Bizon;
    case 27: return MAG_7;
    case 28: return Negev;
    case 29: return Sawed_off;
    case 30: return Tec_9;
    //case 31: return nullptr; //ct knife
    case 32: return P2000;
    case 33: return MP7;
    case 34: return MP9;
    case 35: return Nova;
    case 36: return P250;
    case 38: return SCAR_20;
    case 39: return SG_553;
    case 40: return SSG_08;
    //case 42: return nullptr; //ct knife
    //case 59: return nullptr; //t knife
    case 60: return M4A1_S;
    case 61: return USP_S;
    case 63: return CZ_75;
    case 64: return Revolver;
    }
}

Material* dispatchCustomMaterial(int id) noexcept
{
    switch (id) {
        default: return nullptr;
        case 0: return custom0;
        case 1: return custom1;
        case 2: return custom2;
        case 3: return custom3;
        case 4: return custom4;
        case 5: return custom5;
        case 6: return custom6;
        case 7: return custom7;
        case 8: return custom8;
        case 9: return custom9;
    }
}

void nChams::initializeCustomMaterials(int id, std::string texture) {
    switch (id)
    {
    case 0: custom0 = interfaces->materialSystem->createMaterial("custom0", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 1: custom1 = interfaces->materialSystem->createMaterial("custom1", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 2: custom2 = interfaces->materialSystem->createMaterial("custom2", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 3: custom3 = interfaces->materialSystem->createMaterial("custom3", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 4: custom4 = interfaces->materialSystem->createMaterial("custom4", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 5: custom5 = interfaces->materialSystem->createMaterial("custom5", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 6: custom6 = interfaces->materialSystem->createMaterial("custom6", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 7: custom7 = interfaces->materialSystem->createMaterial("custom7", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 8: custom8 = interfaces->materialSystem->createMaterial("custom8", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    case 9: custom9 = interfaces->materialSystem->createMaterial("custom9", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    }
}

void nChams::initializeWeaponMaterials(int weapon, std::string texture) {
    switch (weapon) {
        default: 
        case  0: Glock_18 = interfaces->materialSystem->createMaterial("glock", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case  1: P2000 = interfaces->materialSystem->createMaterial("p2000", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case  2: USP_S = interfaces->materialSystem->createMaterial("usp-s", KeyValues::fromString("VertexLitGeneric", std::string(std::string("$basetexture ").append(texture).c_str()).append(texture).c_str())); break;
        case  3: DualBerettas = interfaces->materialSystem->createMaterial("dualberettas", KeyValues::fromString("VertexLitGeneric", std::string(std::string("$basetexture ").append(texture).c_str()).append(texture).c_str())); break;
        case  4: P250 = interfaces->materialSystem->createMaterial("p250", KeyValues::fromString("VertexLitGeneric", std::string(std::string("$basetexture ").append(texture).c_str()).append(texture).c_str())); break;
        case  5: Tec_9 = interfaces->materialSystem->createMaterial("tec9", KeyValues::fromString("VertexLitGeneric", std::string(std::string("$basetexture ").append(texture).c_str()).append(texture).c_str())); break;
        case  6: Five_Seven = interfaces->materialSystem->createMaterial("fiveseven", KeyValues::fromString("VertexLitGeneric", std::string(std::string("$basetexture ").append(texture).c_str()).append(texture).c_str())); break;
        case  7: CZ_75 = interfaces->materialSystem->createMaterial("cz75", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case  8: DesertEagle = interfaces->materialSystem->createMaterial("deagle", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case  9: Revolver = interfaces->materialSystem->createMaterial("revolver", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 10: Nova = interfaces->materialSystem->createMaterial("nova", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 11: XM1014 = interfaces->materialSystem->createMaterial("xm1014", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 12: Sawed_off = interfaces->materialSystem->createMaterial("sawedoff", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 13: MAG_7 = interfaces->materialSystem->createMaterial("mag7", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 14: M249 = interfaces->materialSystem->createMaterial("m249", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 15: Negev = interfaces->materialSystem->createMaterial("negev", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 16: Mac_10 = interfaces->materialSystem->createMaterial("mac10", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 17: MP9 = interfaces->materialSystem->createMaterial("mp9", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 18: MP7 = interfaces->materialSystem->createMaterial("mp7", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 19: MP5_SD = interfaces->materialSystem->createMaterial("mp5sd", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 20: UMP_45 = interfaces->materialSystem->createMaterial("ump45", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 21: P90 = interfaces->materialSystem->createMaterial("p90", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 22: PP_Bizon = interfaces->materialSystem->createMaterial("bizon", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 23: GalilAR = interfaces->materialSystem->createMaterial("galil", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 24: Famas = interfaces->materialSystem->createMaterial("famas", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 25: AK_47 = interfaces->materialSystem->createMaterial("ak47", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 26: M4A4 = interfaces->materialSystem->createMaterial("m4a4", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 27: M4A1_S = interfaces->materialSystem->createMaterial("m4a1-s", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 28: SSG_08 = interfaces->materialSystem->createMaterial("ssg08", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 29: SG_553 = interfaces->materialSystem->createMaterial("sg553", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 30: AUG = interfaces->materialSystem->createMaterial("aug", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 31: AWP = interfaces->materialSystem->createMaterial("awp", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 32: G3SG1 = interfaces->materialSystem->createMaterial("g3sg1", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
        case 33: SCAR_20 = interfaces->materialSystem->createMaterial("scar20", KeyValues::fromString("VertexLitGeneric", std::string("$basetexture ").append(texture).c_str())); break;
    }
}

static void initializeMaterials() noexcept
{
    normal = interfaces->materialSystem->createMaterial("normal", KeyValues::fromString("VertexLitGeneric", nullptr));
    flat = interfaces->materialSystem->createMaterial("flat", KeyValues::fromString("UnlitGeneric", nullptr));
    chrome = interfaces->materialSystem->createMaterial("chrome", KeyValues::fromString("VertexLitGeneric", "$envmap env_cubemap"));
    glow = interfaces->materialSystem->createMaterial("glow", KeyValues::fromString("VertexLitGeneric", "$additive 1 $envmap models/effects/cube_white $envmapfresnel 1 $alpha .8"));
    pearlescent = interfaces->materialSystem->createMaterial("pearlescent", KeyValues::fromString("VertexLitGeneric", "$basetexture vgui/white_additive $ambientonly 1 $phong 1 $pearlescent 3 $basemapalphaphongmask 1"));
    metallic = interfaces->materialSystem->createMaterial("metallic", KeyValues::fromString("VertexLitGeneric", "$basetexture white $ignorez 0 $envmap env_cubemap $normalmapalphaenvmapmask 1 $envmapcontrast 1 $nofog 1 $model 1 $nocull 0 $selfillum 1 $halfambert 1 $znearer 0 $flat 1"));
    smoke = interfaces->materialSystem->createMaterial("smoke", KeyValues::fromString("VertexLitGeneric", "$basetexture models/weapons/customization/paints/anodized_multi/smoke $nofog 1 $envmap env_cubemap $envmaptint [1 1 1] $phong 1 $basemapalphaphongmask 1 $phongboost 0 $rimlight 1 $phongtint [1 1 1] $rimlightexponent 9999999 $rimlightboost 0 $selfillum 1 $ignorez 0"));
    smoke2 = interfaces->materialSystem->createMaterial("smoke2", KeyValues::fromString("VertexLitGeneric", "$basetexture models/weapons/customization/paints/anodized_multi/smoke2 $ambientonly 1 $phong 1 $pearlescent 3 $basemapalphaphongmask 1"));
    water = interfaces->materialSystem->createMaterial("water", KeyValues::fromString("Refract", "$model 1 $refractamount 2.0 $dudvmap dev/water_dudv $normalmap dev/water_normal $surfaceprop water Proxies {  AnimatedTexture  { animatedtexturevar $normalmap animatedtextureframenumvar $bumpframe animatedtextureframerate 30.00  }  TextureScroll  { texturescrollvar $bumptransform texturescrollrate .05 texturescrollangle 45.00  }}"));
    leaves = interfaces->materialSystem->createMaterial("leaves", KeyValues::fromString("VertexLitGeneric", "$basetexture models/props_foliage/urban_tree03_branches $translucent 1"));
    lines = interfaces->materialSystem->createMaterial("lines", KeyValues::fromString("VertexLitGeneric", "$basetexture models/inventory_items/music_kit/darude_01/mp3_detail $additive 1 proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.25 texturescrollangle 90 } }"));
    snowflakes = interfaces->materialSystem->createMaterial("snowflakes", KeyValues::fromString("VertexLitGeneric", "$basetexture dev/snowfield $additive 1 proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.1 } }"));
    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$envmap editor/cube_vertigo $envmapcontrast 1 $basetexture dev/zone_warning proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.6 texturescrollangle 90 } }");
        kv->setString("$envmaptint", "[.7 .7 .7]");
        animated = interfaces->materialSystem->createMaterial("animated", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture models/player/ct_fbi/ct_fbi_glass $envmap env_cubemap");
        kv->setString("$envmaptint", "[.4 .6 .7]");
        platinum = interfaces->materialSystem->createMaterial("platinum", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture detail/dt_metal1 $additive 1 $envmap editor/cube_vertigo");
        kv->setString("$color", "[.05 .05 .05]");
        glass = interfaces->materialSystem->createMaterial("glass", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture black $bumpmap effects/flat_normal $translucent 1 $envmap models/effects/crystal_cube_vertigo_hdr $envmapfresnel 0 $phong 1 $phongexponent 16 $phongboost 2");
        kv->setString("$phongtint", "[.2 .35 .6]");
        crystal = interfaces->materialSystem->createMaterial("crystal", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 2 $phongexponent 8");
        kv->setString("$color2", "[.05 .05 .05]");
        kv->setString("$envmaptint", "[.2 .2 .2]");
        kv->setString("$phongfresnelranges", "[.7 .8 1]");
        kv->setString("$phongtint", "[.8 .9 1]");
        silver = interfaces->materialSystem->createMaterial("silver", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 6 $phongexponent 128 $phongdisablehalflambert 1");
        kv->setString("$color2", "[.18 .15 .06]");
        kv->setString("$envmaptint", "[.6 .5 .2]");
        kv->setString("$phongfresnelranges", "[.7 .8 1]");
        kv->setString("$phongtint", "[.6 .5 .2]");
        gold = interfaces->materialSystem->createMaterial("gold", kv);
    }

    {
        const auto kv = KeyValues::fromString("VertexLitGeneric", "$baseTexture black $bumpmap models/inventory_items/trophy_majors/matte_metal_normal $additive 1 $envmap editor/cube_vertigo $envmapfresnel 1 $normalmapalphaenvmapmask 1 $phong 1 $phongboost 20 $phongexponent 3000 $phongdisablehalflambert 1");
        kv->setString("$phongfresnelranges", "[.1 .4 1]");
        kv->setString("$phongtint", "[.8 .9 1]");
        plastic = interfaces->materialSystem->createMaterial("plastic", kv);
    }
}

void Chams::updateInput() noexcept
{
    config->chamsToggleKey.handleToggle();
}

bool Chams::render(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (config->chamsToggleKey.isSet()) {
        if (!config->chamsToggleKey.isToggled() && !config->chamsHoldKey.isDown())
            return false;
    } else if (config->chamsHoldKey.isSet() && !config->chamsHoldKey.isDown()) {
        return false;
    }

    static bool materialsInitialized = false;
    if (!materialsInitialized) {
        initializeMaterials();
        materialsInitialized = true;
    }

    appliedChams = false;
    this->ctx = ctx;
    this->state = state;
    this->info = &info;
    this->customBoneToWorld = customBoneToWorld;

    if (std::string_view{ info.model->name }.starts_with("models/weapons/v_")) {
        // info.model->name + 17 -> small optimization, skip "models/weapons/v_"
        if (std::strstr(info.model->name + 17, "sleeve"))
            renderSleeves();
        else if (std::strstr(info.model->name + 17, "arms"))
            renderHands();
        else if (!std::strstr(info.model->name + 17, "tablet")
            && !std::strstr(info.model->name + 17, "parachute")
            && !std::strstr(info.model->name + 17, "fists"))
            renderWeapons();
    } else {
        const auto entity = interfaces->entityList->getEntity(info.entityIndex);
        if (entity && !entity->isDormant() && entity->isPlayer())
            renderPlayer(entity);
    }

    return appliedChams;
}

void Chams::applyChams(const std::array<Config::Chams::Material, 8>& chams, int health, const matrix3x4* customMatrix) noexcept
{
    for (const auto& cham : chams) {
        if (!cham.enabled || !cham.ignorez)
            continue;

        Material* material;

        if (cham.material == 19)
            material = dispatchCustomMaterial(config->customChams);
        else
            material = dispatchMaterial(cham.material);

        if (!material)
            continue;

        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        }
        else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(cham.rainbowSpeed);
        }
        else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);
        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, true);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    }

    for (const auto& cham : chams) {
        if (!cham.enabled || cham.ignorez)
            continue;

        Material* material;

        if (cham.material == 19)
            material = dispatchCustomMaterial(config->customChams);
        else
            material = dispatchMaterial(cham.material);

        if (!material)
            continue;

        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        }
        else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(cham.rainbowSpeed);
        }
        else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        if (cham.cover && !appliedChams)
            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, false);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        appliedChams = true;
    }


}

void Chams::applyWeaponChams(const std::array<Config::Chams::Material, 8>& chams, int health, const matrix3x4* customMatrix) noexcept
{
    for (const auto& cham : chams) {
        if (!cham.enabled || !cham.ignorez)
            continue;

        Material* material;

        if (cham.material == 19)
            material = dispatchWeaponMaterial(static_cast<short>(localPlayer->getActiveWeapon()->itemDefinitionIndex()));
        else
            material = dispatchMaterial(cham.material);

        if (!material)
            continue;

        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        }
        else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(cham.rainbowSpeed);
        }
        else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);
        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, true);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    }

    for (const auto& cham : chams) {
        if (!cham.enabled || cham.ignorez)
            continue;

        Material* material;

        if (cham.material == 19)
            material = dispatchWeaponMaterial(static_cast<short>(localPlayer->getActiveWeapon()->itemDefinitionIndex()));
        else
            material = dispatchMaterial(cham.material);

        if (!material)
            continue;

        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        }
        else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(cham.rainbowSpeed);
        }
        else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory->globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        if (cham.cover && !appliedChams)
            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, false);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        appliedChams = true;
    }
}

void Chams::renderPlayer(Entity* player) noexcept
{
    if (!localPlayer)
        return;

    const auto health = player->health();

    if (const auto activeWeapon = player->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::C4 && activeWeapon->c4StartedArming() && std::ranges::any_of(config->chams["Planting"].materials, [](const Config::Chams::Material& mat) { return mat.enabled; })) {
        applyChams(config->chams["Planting"].materials, health);
    } else if (player->isDefusing() && std::ranges::any_of(config->chams["Defusing"].materials, [](const Config::Chams::Material& mat) { return mat.enabled; })) {
        applyChams(config->chams["Defusing"].materials, health);
    }
    else if (player == localPlayer.get()) {
        applyChams(config->chams["Local player"].materials, health);
        renderDesync(health);
        renderFakeLag(health);
        renderAutoPeek(health);
        if (localPlayer->isScoped())
            interfaces->renderView->setBlend((100.f - Visuals::getScopePlayerTransparency()) / 100.f);
    } else if (localPlayer->isOtherEnemy(player)) {
        applyChams(config->chams["Enemies"].materials, health);
        const auto records = Animations::getBacktrackRecords(player->index()); // size = min8 - max13
        if (records && !records->empty() && Backtrack::valid(records->front().simulationTime) && records->back().origin != player->origin()) {
            if (!Backtrack::DrawAllTick()) {
                int lastTick = -1;

                for (int i = static_cast<int>(records->size() - 1U); i >= 0; i--)
                {
                    if (Backtrack::valid(records->at(i).simulationTime) && records->at(i).origin != player->origin())
                    {
                        lastTick = i;
                        break;
                    }
                }

                if (lastTick != -1)
                {
                    if (!appliedChams)
                        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
                    applyChams(config->chams["Backtrack"].materials, health, records->at(lastTick).matrix);
                    interfaces->studioRender->forcedMaterialOverride(nullptr);
                }
            }
            else {
                for (unsigned int i = 0; i < records->size(); i++) {
                    if (records->at(i).origin != player->origin())
                    {
                        if (!appliedChams)
                            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
                        applyChams(config->chams["Backtrack"].materials, health, records->at(i).matrix);
                        interfaces->studioRender->forcedMaterialOverride(nullptr);
                    }
                }
            }
        }
    } else {
        applyChams(config->chams["Allies"].materials, health);
    }
}

void Chams::renderWeapons() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    auto activeWeapon = localPlayer->getActiveWeapon();

    if (!activeWeapon) return;

    applyWeaponChams(config->chams["Weapons"].materials, localPlayer->health());
}

void Chams::renderHands() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(config->chams["Hands"].materials, localPlayer->health());
}

void Chams::renderSleeves() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(config->chams["Sleeves"].materials, localPlayer->health());
}

void Chams::renderAutoPeek(int health) noexcept {

    //if (!localPlayer->isAlive()) return;

    //if (!Extra::isQuickPeekKeyDown()) return;

    //if (!Animations::gotAutoPeekMatrix()) return;

    //auto autoMatrix = Animations::getAutoPeekMatrix();

    //if (!appliedChams)
    //    hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
    //applyChams(config->chams["Auto-Peek"].materials, health, autoMatrix.data());
    //interfaces->studioRender->forcedMaterialOverride(nullptr);
}

void Chams::renderDesync(int health) noexcept
{
    if (Animations::gotFakeMatrix())
    {
        auto fakeMatrix = Animations::getFakeMatrix();
        for (auto& i : fakeMatrix)
        {
            i[0][3] += info->origin.x;
            i[1][3] += info->origin.y;
            i[2][3] += info->origin.z;
        }
        if (!appliedChams)
            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
        applyChams(config->chams["Desync"].materials, health, fakeMatrix.data());
        interfaces->studioRender->forcedMaterialOverride(nullptr);
        for (auto& i : fakeMatrix)
        {
            i[0][3] -= info->origin.x;
            i[1][3] -= info->origin.y;
            i[2][3] -= info->origin.z;
        }
    }
}

void Chams::renderFakeLag(int health) noexcept {

    if (!FakeLag::Enabled()) return;

    if (!localPlayer->isAlive())
        return;

    if (localPlayer->velocity().length2D() < 2.0f)
        return;

    if (Animations::gotFakelagMatrix())
    {
        auto fakelagMatrix = Animations::getFakelagMatrix();
        if (!appliedChams)
            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
        applyChams(config->chams["Fake-lag"].materials, health, fakelagMatrix.data());
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    }
}

