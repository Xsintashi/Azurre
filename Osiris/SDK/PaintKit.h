struct String
{
	char* buffer;	//0x0000
	int capacity;	//0x0004
	int growSize;	//0x0008
	int length;		//0x000C
}; //Size=0x0010

struct PaintKit
{
    int ID;
    String name;
    String descriptionString;
    String descriptionTag;
    String pad;
    String pattern;
    String pad1;
    String sLogoMaterial;
    int baseDiffuseOverride;
    int rarity;
    int Style;
    int color1;
    int color2;
    int color3;
    int color4;
    int logoColor1;
    int logoColor2;
    int logoColor3;
    int logoColor4;
    float wearDefault;
    float wearRemapMin;
    float wearRemapMax;
    char fixedSeed;
    char phongExponent;
    char phongAlbedoBoost;
    char phongIntensity;
    float patternScale;
    float patternOffsetXStart;
    float patternOffsetXEnd;
    float patternOffsetYStart;
    float patternOffsetYEnd;
    float patternRotateStart;
    float patternRotateEnd;
    float logoScale;
    float logoOffsetX;
    float logoOffsetY;
    float logoRotation;
    int ignoreWeaponSizeScale;
    int viewModelExponentOverrideSize;
    int onlyFirstMaterial;
    float pearlescent;
    int vmtPath[4];
    int vmtOverrides;
};