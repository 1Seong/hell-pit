#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <math.h>

static const int kBufferW = 320;
static const int kBufferH = 180;
static const int kWindowScale = 3;
static const int kAppIconResourceId = 201;

struct Vec2
{
    float x;
    float y;
};

struct Input
{
    bool keys[256];
    bool pressed[256];
    bool mouseLeft;
    bool mousePressed;
    int mouseX;
    int mouseY;
    int mousePressedX;
    int mousePressedY;
};

struct Body
{
    Vec2 position;
    Vec2 velocity;
    float halfW;
    float halfH;
    bool grounded;
    bool jumpAvailable;
};

struct RectF
{
    float x;
    float y;
    float w;
    float h;
};

enum PlatformCategory
{
    PlatformSolid
};

enum PlatformShape
{
    PlatformSlab,
    PlatformWedgeLeft,
    PlatformWedgeRight,
    PlatformStepped,
    PlatformJagged
};

struct Platform
{
    RectF bounds;
    PlatformCategory category;
    PlatformShape shape;
    int goreSeed;
    bool active;
};

enum EnemyType
{
    EnemyFlying,
    EnemyWalker,
    EnemyHeartFlying
};

struct Enemy
{
    RectF bounds;
    bool active;
    bool alive;
    float respawnTimer;
    float shotTimer;
    float floatSeed;
    EnemyType type;
    float velocityX;
    float patrolMinX;
    float patrolMaxX;
};

struct Particle
{
    Vec2 position;
    Vec2 velocity;
    float life;
    uint32_t color;
};

struct Worm
{
    Vec2 points[12];
    int pointCount;
    float speed;
    bool active;
};

struct WallTrap
{
    Vec2 position;
    float shotTimer;
    bool firesRight;
    bool active;
};

struct Projectile
{
    Vec2 position;
    Vec2 velocity;
    bool active;
    bool hookable;
};

struct SuperOrb
{
    Vec2 position;
    bool active;
};

struct SawBlade
{
    Vec2 position;
    Vec2 velocity;
    float travelTimer;
    float life;
    float spin;
    bool active;
};

enum ActiveItemType
{
    ActiveNone,
    ActiveDynamite,
    ActiveMiniOrb,
    ActiveBloodChalice,
    ActiveHourglass,
    ActiveSawBlade
};

enum PerkType
{
    PerkNone,
    PerkCrossNecklace,
    PerkParachute,
    PerkDevilHeart,
    PerkAngelSkin,
    PerkBloodBattery,
    PerkContract,
    PerkCount
};

enum AppScreen
{
    ScreenMainMenu,
    ScreenNewGameSelect,
    ScreenIntroCutscene,
    ScreenGameplay,
    ScreenOptions,
    ScreenHelp,
    ScreenCodex,
    ScreenAchievements,
    ScreenCredits
};

enum AchievementType
{
    AchievementStage1,
    AchievementStage2,
    AchievementStage3,
    AchievementEscape,
    AchievementHookVeteran,
    AchievementCount
};

enum SfxType
{
    SfxUiClick,
    SfxHookFire,
    SfxHookHit,
    SfxHookDash,
    SfxSuperDash,
    SfxHit,
    SfxHeal,
    SfxDeath,
    SfxStageClear,
    SfxPerkRoulette,
    SfxPerkConfirm,
    SfxShopEnter,
    SfxJump,
    SfxDynamite,
    SfxHourglass,
    SfxSawBlade,
    SfxReached,
    SfxCount
};

struct ShopOffer
{
    ActiveItemType item;
    int uses;
    const char* name;
    const char* description;
};

struct AfterImage
{
    Vec2 position;
    float life;
};

struct SlashTrail
{
    Vec2 start;
    Vec2 end;
    float life;
};

struct SfxVoice
{
    bool active;
    SfxType type;
    float age;
    float duration;
    double phaseA;
    double phaseB;
    uint32_t seed;
};

enum HookMode
{
    HookIdle,
    HookFlying,
    HookPullTerrain,
    HookPullEnemy,
    HookSuperOrb,
    HookShopPortal
};

struct Hook
{
    HookMode mode;
    Vec2 position;
    Vec2 direction;
    Vec2 target;
    Vec2 dashStart;
    float distance;
    float pauseTimer;
    float dashTimer;
    float dashDuration;
    bool vault;
    bool dashing;
};

static uint32_t gPixels[kBufferW * kBufferH];
static uint32_t gCrtScratch[kBufferW * kBufferH];
static BITMAPINFO gBitmapInfo;
static Input gInput;
static bool gRunning = true;
static bool gPlayerFacingRight = true;
static const int kAudioSampleRate = 44100;
static const int kAudioChannels = 2;
static const int kAudioBufferSamples = 512;
static const int kAudioBufferCount = 4;
static HWAVEOUT gAudioDevice = 0;
static WAVEHDR gAudioHeaders[kAudioBufferCount] = {};
static int16_t gAudioSamples[kAudioBufferCount][kAudioBufferSamples * kAudioChannels] = {};
static double gAudioPhaseA = 0.0;
static double gAudioPhaseB = 0.0;
static double gAudioPhaseC = 0.0;
static double gAudioPhaseD = 0.0;
static double gAudioPhaseE = 0.0;
static double gAudioMelodyPhase = 0.0;
static double gAudioLeadPhase = 0.0;
static double gAudioBassPhase = 0.0;
static double gAudioArpPhase = 0.0;
static double gAudioSubPhase = 0.0;
static double gAudioOrbitPhase = 0.0;
static double gAudioMelodyTime = 0.0;
static const int kAudioEchoSamples = 13230;
static float gAudioEchoL[kAudioEchoSamples] = {};
static float gAudioEchoR[kAudioEchoSamples] = {};
static int gAudioEchoIndex = 0;
static const int kMaxSfxVoices = 24;
static SfxVoice gSfxVoices[kMaxSfxVoices] = {};
static const int kSfxHookFireSampleCount = 17640;
static const int kSfxHookHitSampleCount = 11025;
static const int kSfxJumpSampleCount = 11025;
static const int kSfxDashSampleCount = 17640;
static float gSfxHookFireSample[kSfxHookFireSampleCount] = {};
static float gSfxHookHitSample[kSfxHookHitSampleCount] = {};
static float gSfxJumpSample[kSfxJumpSampleCount] = {};
static float gSfxDashSample[kSfxDashSampleCount] = {};
static bool gSfxSamplesReady = false;
static uint32_t gAudioNoiseState = 0x7A341316u;
static float gAudioLowNoise = 0.0f;
static float gAudioRumble = 0.0f;
static float gAudioHiss = 0.0f;
static float gAudioMetal = 0.0f;
static bool gAudioRunning = false;
static Hook gHook = {};
static float gHookCooldownTimer = 0.0f;
static AfterImage gAfterImages[24];
static Vec2 gCloakSegments[5];
static SlashTrail gSlashTrail = {};
static Vec2 gCrossFlashPosition = {};
static float gCrossFlashTimer = 0.0f;
static Particle gParticles[128];
static Worm gWorms[8];
static WallTrap gTraps[12];
static Projectile gProjectiles[32];
static SuperOrb gSuperOrbs[12];
static SawBlade gSawBlades[8];
static const int kMaxPlatforms = 64;
static const int kMaxEnemies = 40;
static Platform gPlatforms[kMaxPlatforms];
static Enemy gEnemies[kMaxEnemies];
static RectF gSideWalls[] =
{
    { 0.0f, 0.0f, 6.0f, (float)kBufferH },
    { (float)kBufferW - 6.0f, 0.0f, 6.0f, (float)kBufferH }
};
static uint32_t gRandomState = 0xC0FFEEu;
static float gMapTime = 0.0f;
static AppScreen gScreen = ScreenMainMenu;
static AppScreen gHelpReturnScreen = ScreenMainMenu;
static bool gOptionsFromGameplay = false;
static bool gHelpStartsGame = false;
static int gHelpPage = 0;
static float gIntroTimer = 0.0f;
static float gBrightness = 1.0f;
static float gMasterSound = 1.0f;
static int gCrosshairStyle = 0;
static char gSaveFileName[MAX_PATH] = "hellpit_save.ini";
static float gScrollSpeed = 0.0f;
static float gLastPlatformCenterX = kBufferW * 0.5f;
static int gRowsSinceEnemy = 0;
static float gDevourerY = 250.0f;
static const float kCameraLockY = 105.0f;
static float gHitStopTimer = 0.0f;
static float gCameraShakeTimer = 0.0f;
static int gRenderOffsetX = 0;
static int gRenderOffsetY = 0;
static bool gPlayerDead = false;
static float gPlayerDeathTimer = 0.0f;
static int gDeathStageMeters = 0;
static Vec2 gCrossReviveEffectPosition = {};
static float gCrossReviveEffectTimer = 0.0f;
static float gInvulnerableTimer = 0.0f;
static float gSuperJumpTimer = 0.0f;
static float gTimeStopTimer = 0.0f;
static float gWormSpawnTimer = 3.5f;
static bool gDevourerFastForward = false;
static float gDevourerPauseTimer = 0.0f;
static const int kMaxHeartUnits = 6;
static const int kHeartCount = 3;
static const int kFullHeartUnits = 2;
static const int kMaxHealth = kMaxHeartUnits;
static const float kPerkRouletteSeconds = 2.15f;
static int gPlayerHealth = kMaxHealth;
static int gCombo = 0;
static float gComboTimer = 0.0f;
static float gComboPulse = 0.0f;
static float gHeightPixels = 0.0f;
static float gViewHeightPixels = 0.0f;
static const int kHeightMarkerInterval = 200;
static int gNextHeightMilestone = kHeightMarkerInterval;
static int gReachedMeters = 0;
static float gReachedBannerTimer = 0.0f;
static const float kPixelsPerMeter = 6.0f;
static const float kInvulnerableSeconds = 1.0f;
static const float kComboSeconds = 3.0f;
static const float kSuperOrbHitRadius = 11.0f;
static const float kHookMissCooldownSeconds = 0.42f;
static const int kChapterCount = 4;
static const float kChapterLengthMeters = 2000.0f;
static const float kShopPortalMeters = 1000.0f;
static const int kShopPrices[kChapterCount] = { 50, 60, 70, 100 };

struct ChapterConfig
{
    const char* name;
    int rowObjects;
    bool allowWorms;
    bool allowTraps;
    bool allowHearts;
    bool canSpawnSingleObject;
    bool pipeWalls;
    float superOrbChance;
    float monsterBaseSpeed;
    float monsterAccelDelay;
    float monsterMaxSpeed;
    float monsterCatchupSpeed;
};

static const ChapterConfig gChapterConfigs[kChapterCount] =
{
    { "CHAMBER", 3, false, false, true,  false, false, 0.045f, 36.0f, 10.0f,  76.0f, 118.0f },
    { "MARCH",   2, true,  true,  true,  false, false, 0.034f, 58.0f,  6.6f, 116.0f, 154.0f },
    { "DREAM",   2, true,  true,  true,  false, true,  0.026f, 64.0f,  5.6f, 128.0f, 166.0f },
    { "DREAD",   2, true,  true,  false, true,  true,  0.026f, 64.0f,  5.8f, 126.0f, 168.0f }
};

static int gChapter = 0;
static bool gInfiniteMode = false;
static int gCoins = 0;
static ActiveItemType gActiveItem = ActiveNone;
static int gActiveItemUses = 0;
static bool gRunUsedActiveItem = false;
static bool gPerkOwned[PerkCount] = {};
static int gPerkGrantChapterMask = 0;
static bool gCrossReviveUsed = false;
static int gAirJumpsUsed = 0;
static float gParachuteHoldTimer = 0.0f;
static bool gPerkRouletteOpen = false;
static float gPerkRouletteTimer = 0.0f;
static bool gPerkRouletteWaitingConfirm = false;
static PerkType gPerkRouletteFinal = PerkNone;
static PerkType gQueuedPerkRoulette = PerkNone;
static bool gShopUsed[kChapterCount] = {};
static bool gStartShopUsed[kChapterCount] = {};
static bool gFreeShopClaimed = false;
static bool gShopOpen = false;
static bool gShopCurrentFree = false;
static bool gShopCurrentStart = false;
static bool gHookShopFree = false;
static bool gHookShopStart = false;
static bool gStartPortalTouchArmed = false;
static int gShopHover = 0;
static float gDealAcceptedTimer = 0.0f;
static ShopOffer gShopOffers[3] = {};
static bool gPreserveRunStateOnRestart = false;
static bool gCheckpointValid = false;
static int gCheckpointChapter = 0;
static bool gCheckpointInfiniteMode = false;
static int gCheckpointCoins = 0;
static ActiveItemType gCheckpointActiveItem = ActiveNone;
static int gCheckpointActiveItemUses = 0;
static bool gCheckpointRunUsedActiveItem = false;
static bool gCheckpointPerkOwned[PerkCount] = {};
static int gCheckpointPerkGrantChapterMask = 0;
static bool gCheckpointCrossReviveUsed = false;
static bool gCheckpointShopUsed[kChapterCount] = {};
static bool gCheckpointStartShopUsed[kChapterCount] = {};
static bool gCheckpointFreeShopClaimed = false;
static int gBestStageMeters[kChapterCount] = {};
static bool gAchievementUnlocked[AchievementCount] = {};
static bool gGameClearedEver = false;
static AchievementType gAchievementPopup = AchievementCount;
static float gAchievementPopupTimer = 0.0f;
static AchievementType gAchievementQueue[8] = {};
static int gAchievementQueueCount = 0;
static bool gChapterMonsterAwake = false;
static float gChapterChaseTimer = 0.0f;
static float gChapterFirstRowMeters = 14.0f;
static float gChapterBannerTimer = 0.0f;
static bool gChapterTransitionActive = false;
static bool gChapterTransitionWaitingConfirm = false;
static bool gEndingActive = false;
static float gEndingTimer = 0.0f;
static const float kChapterTransitionSeconds = 1.35f;
static const float kChapterTransitionMoveSeconds = 0.45f;
static float gChapterTransitionTimer = 0.0f;
static Vec2 gChapterTransitionStart = {};
static Vec2 gChapterTransitionTarget = {};
static int gPendingChapter = 0;

static const Body kPlayerStart =
{
    { kBufferW * 0.5f, 156.0f },
    { 0.0f, 0.0f },
    3.0f,
    4.0f,
    true,
    true
};
static Body gPlayer = kPlayerStart;
static void RestartGame();
static void SpawnBurst(Vec2 center, uint32_t color, int count);
static void ResolveBranchDividerOverlap();
static void FireTrapProjectileAt(const WallTrap& trap, float travelOffset);
static void SavePersistentData();
static void KillPlayer();
static void ResetCloak();
static bool MenuClicked(float x, float y, float w, float h, bool enabled = true);
static void DrawMenuButton(int x, int y, int w, int h, const char* label,
                           bool enabled, uint32_t textColor, uint32_t red,
                           uint32_t dark);
static void DrawCustomCursor(uint32_t white, uint32_t red, uint32_t dark);
static void DrawCrosshairAt(int x, int y, int style, bool cooldown,
                            uint32_t white, uint32_t red, uint32_t dark);

static uint32_t Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint32_t)b | ((uint32_t)g << 8) | ((uint32_t)r << 16);
}

static void ClearPressedInput()
{
    for (int i = 0; i < 256; ++i)
        gInput.pressed[i] = false;
    gInput.mousePressed = false;
}

static float Clamp(float v, float minV, float maxV)
{
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}

static float SmoothStep(float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static float AbsFloat(float v)
{
    return v < 0.0f ? -v : v;
}

static float Random01()
{
    gRandomState = gRandomState * 1664525u + 1013904223u;
    return (float)((gRandomState >> 8) & 0x00FFFFFFu) / 16777215.0f;
}

static float AudioNoise01()
{
    gAudioNoiseState = gAudioNoiseState * 1664525u + 1013904223u;
    return (float)((gAudioNoiseState >> 8) & 0x00FFFFFFu) / 16777215.0f;
}

static float VoiceNoise(SfxVoice& voice)
{
    voice.seed = voice.seed * 1664525u + 1013904223u;
    return (float)((voice.seed >> 8) & 0x00FFFFFFu) / 8388607.5f - 1.0f;
}

static float SfxDuration(SfxType type)
{
    switch (type)
    {
        case SfxUiClick: return 0.075f;
        case SfxHookFire: return 0.40f;
        case SfxHookHit: return 0.20f;
        case SfxHookDash: return 0.40f;
        case SfxSuperDash: return 1.05f;
        case SfxHit: return 0.22f;
        case SfxHeal: return 0.36f;
        case SfxDeath: return 1.22f;
        case SfxStageClear: return 1.38f;
        case SfxPerkRoulette: return kPerkRouletteSeconds;
        case SfxPerkConfirm: return 0.68f;
        case SfxShopEnter: return 0.78f;
        case SfxJump: return 0.25f;
        case SfxDynamite: return 0.72f;
        case SfxHourglass: return 5.0f;
        case SfxSawBlade: return 0.86f;
        case SfxReached: return 0.55f;
        default: return 0.20f;
    }
}

static void PlaySfx(SfxType type)
{
    int slot = 0;
    for (int i = 0; i < kMaxSfxVoices; ++i)
    {
        if (!gSfxVoices[i].active)
        {
            slot = i;
            break;
        }
        if (gSfxVoices[i].age > gSfxVoices[slot].age)
            slot = i;
    }

    SfxVoice& voice = gSfxVoices[slot];
    voice.active = true;
    voice.type = type;
    voice.age = 0.0f;
    voice.duration = SfxDuration(type);
    voice.phaseA = 0.0;
    voice.phaseB = 0.0;
    voice.seed = gAudioNoiseState ^ ((uint32_t)type * 0x9E3779B9u) ^ 0xA53C91D7u;
}

static float SampleNoise(uint32_t& state)
{
    state = state * 1664525u + 1013904223u;
    return (float)((state >> 8) & 0x00FFFFFFu) / 8388607.5f - 1.0f;
}

static void BakeSfxSamples()
{
    if (gSfxSamplesReady) return;
    const double twoPi = 6.28318530717958647692;

    uint32_t hookNoise = 0xA145927Bu;
    double hookMetalA = 0.0;
    double hookMetalB = 0.0;
    double hookMass = 0.0;
    double hookThud = 0.0;
    float hookFiltered = 0.0f;
    float hookLow = 0.0f;
    float hookMassNoise = 0.0f;
    float hookMid = 0.0f;
    for (int i = 0; i < kSfxHookFireSampleCount; ++i)
    {
        float time = (float)i / (float)kAudioSampleRate;
        float t = time / 0.40f;
        float tail = 1.0f - SmoothStep(t);
        float snap = 1.0f - SmoothStep(time / 0.020f);
        float heavy = 1.0f - SmoothStep(time / 0.120f);
        float launchPunch = 1.0f - SmoothStep(time / 0.052f);
        float dryCrack = 1.0f - SmoothStep(time / 0.008f);
        float rattle =
            (1.0f - SmoothStep(fabsf((time - 0.030f) * 1000.0f) / 14.0f)) * 0.36f +
            (1.0f - SmoothStep(fabsf((time - 0.056f) * 1000.0f) / 17.0f)) * 0.30f +
            (1.0f - SmoothStep(fabsf((time - 0.090f) * 1000.0f) / 23.0f)) * 0.24f +
            (1.0f - SmoothStep(fabsf((time - 0.138f) * 1000.0f) / 34.0f)) * 0.18f;
        float noise = SampleNoise(hookNoise);
        hookLow += (noise - hookLow) * 0.018f;
        hookMid += (noise - hookMid) * 0.090f;
        hookMassNoise += (hookLow - hookMassNoise) * 0.050f;
        hookFiltered += ((noise - hookMid) - hookFiltered) * 0.46f;
        hookThud += twoPi * 28.0 / (double)kAudioSampleRate;
        hookMass += twoPi * 46.0 / (double)kAudioSampleRate;
        hookMetalA += twoPi * (215.0 + 18.0 * snap) / (double)kAudioSampleRate;
        hookMetalB += twoPi * (390.0 + 35.0 * rattle) / (double)kAudioSampleRate;
        float thud = ((float)sin(hookThud) * 0.70f + hookMassNoise * 1.35f) * heavy;
        float recoil = (float)sin(hookMass) * launchPunch * 0.32f;
        float chain = ((float)sin(hookMetalA) * 0.16f + (float)sin(hookMetalB) * 0.11f) * rattle * tail;
        float cable = hookFiltered * (0.18f * snap + 0.13f * tail);
        float crack = (noise - hookLow) * dryCrack * 0.16f;
        float muzzle = (hookMid - hookLow) * launchPunch * 0.36f;
        float drag = (noise - hookFiltered) * tail * 0.025f;
        gSfxHookFireSample[i] = Clamp((float)tanh((thud + recoil + muzzle + crack + cable + chain + drag) * 2.85f) * 0.94f, -1.0f, 1.0f);
    }

    uint32_t hookHitNoise = 0xB7714D2Fu;
    double hookHitThunk = 0.0;
    double hookHitLockA = 0.0;
    double hookHitLockB = 0.0;
    float hookHitLow = 0.0f;
    float hookHitMid = 0.0f;
    float hookHitAir = 0.0f;
    for (int i = 0; i < kSfxHookHitSampleCount; ++i)
    {
        float time = (float)i / (float)kAudioSampleRate;
        float t = time / 0.25f;
        float env = 1.0f - SmoothStep((time - 0.095f) / 0.145f);
        float bite = 1.0f - SmoothStep(time / 0.026f);
        float sink = 1.0f - SmoothStep(time / 0.080f);
        float lock =
            (1.0f - SmoothStep(fabsf((time - 0.040f) * 1000.0f) / 18.0f)) * 0.38f +
            (1.0f - SmoothStep(fabsf((time - 0.082f) * 1000.0f) / 28.0f)) * 0.24f;
        float noise = SampleNoise(hookHitNoise);
        hookHitLow += (noise - hookHitLow) * 0.020f;
        hookHitMid += (noise - hookHitMid) * 0.110f;
        hookHitAir += ((noise - hookHitMid) - hookHitAir) * 0.38f;
        hookHitThunk += twoPi * 42.0 / (double)kAudioSampleRate;
        hookHitLockA += twoPi * 265.0 / (double)kAudioSampleRate;
        hookHitLockB += twoPi * 520.0 / (double)kAudioSampleRate;
        float wetBite = (noise - hookHitLow) * bite * 0.28f;
        float thunk = ((float)sin(hookHitThunk) * 0.46f + hookHitLow * 1.20f) * sink;
        float clank = ((float)sin(hookHitLockA) * 0.15f +
                       (float)sin(hookHitLockB) * 0.09f +
                       hookHitAir * 0.22f) * lock;
        float embed = (hookHitMid - hookHitLow) * sink * 0.18f;
        gSfxHookHitSample[i] = Clamp((float)tanh((wetBite + thunk + clank + embed) * 2.55f) * env * 0.86f, -1.0f, 1.0f);
    }

    uint32_t jumpNoise = 0x6C8E9CF5u;
    double jumpSub = 0.0;
    double jumpBody = 0.0;
    float jumpLowNoise = 0.0f;
    for (int i = 0; i < kSfxJumpSampleCount; ++i)
    {
        float time = (float)i / (float)kAudioSampleRate;
        float t = time / 0.25f;
        float env = 1.0f - SmoothStep(t);
        float punch = 1.0f - SmoothStep(time / 0.075f);
        float noise = SampleNoise(jumpNoise);
        jumpLowNoise += (noise - jumpLowNoise) * 0.08f;
        jumpSub += twoPi * (46.0 + 12.0 * punch) / (double)kAudioSampleRate;
        jumpBody += twoPi * (92.0 - 18.0 * t) / (double)kAudioSampleRate;
        float sub = (float)sin(jumpSub) * 0.72f * punch;
        float body = (float)sin(jumpBody) * 0.30f * env;
        float cloth = jumpLowNoise * env * 0.22f;
        gSfxJumpSample[i] = Clamp((float)tanh((sub + body + cloth) * 1.25f) * 0.72f, -1.0f, 1.0f);
    }

    uint32_t dashNoise = 0xD32451B9u;
    double dashBlade = 0.0;
    double dashEdge = 0.0;
    double dashRing = 0.0;
    double dashGlass = 0.0;
    double dashShimmerA = 0.0;
    double dashShimmerB = 0.0;
    double dashTailRise = 0.0;
    float dashFiltered = 0.0f;
    float dashMid = 0.0f;
    float dashAirState = 0.0f;
    for (int i = 0; i < kSfxDashSampleCount; ++i)
    {
        float time = (float)i / (float)kAudioSampleRate;
        float t = time / 0.40f;
        float env = 1.0f - SmoothStep((time - 0.190f) / 0.190f);
        float attack = SmoothStep(time / 0.003f);
        float cutEnv = attack * (1.0f - SmoothStep((time - 0.070f) / 0.120f));
        float trailEnv = SmoothStep(time / 0.010f) * (1.0f - SmoothStep((time - 0.150f) / 0.230f));
        float shineEnv = attack * (1.0f - SmoothStep((time - 0.075f) / 0.075f));
        float shimmerEnv = attack * (1.0f - SmoothStep((time - 0.105f) / 0.155f));
        float tailEnv = SmoothStep((time - 0.065f) / 0.055f) *
                        (1.0f - SmoothStep((time - 0.230f) / 0.160f));
        float biteEnv =
            (1.0f - SmoothStep(fabsf((time - 0.018f) * 1000.0f) / 18.0f)) * 0.55f +
            (1.0f - SmoothStep(fabsf((time - 0.052f) * 1000.0f) / 34.0f)) * 0.30f;
        float noise = SampleNoise(dashNoise);
        dashMid += (noise - dashMid) * 0.070f;
        dashFiltered += ((noise - dashMid) - dashFiltered) * 0.52f;
        dashAirState += (dashFiltered - dashAirState) * 0.10f;
        float air = (dashFiltered - dashAirState) * trailEnv * 0.54f;
        dashBlade += twoPi * 6644.0 / (double)kAudioSampleRate;
        dashEdge += twoPi * 9956.0 / (double)kAudioSampleRate;
        dashRing += twoPi * 4435.0 / (double)kAudioSampleRate;
        dashGlass += twoPi * 13289.0 / (double)kAudioSampleRate;
        dashShimmerA += twoPi * 8869.0 / (double)kAudioSampleRate;
        dashShimmerB += twoPi * 11840.0 / (double)kAudioSampleRate;
        float tailRiseFreq = 13289.0f + 3980.0f * SmoothStep((time - 0.070f) / 0.230f);
        dashTailRise += twoPi * tailRiseFreq / (double)kAudioSampleRate;
        float blade = (float)sin(dashBlade) * cutEnv * 0.20f;
        float edge = (float)sin(dashEdge) * shineEnv * 0.145f;
        float ring = (float)sin(dashRing) * shineEnv * 0.040f;
        float glass = (float)sin(dashGlass) * shineEnv * 0.060f;
        float shimmer = ((float)sin(dashShimmerA) * 0.11f +
                         (float)sin(dashShimmerB) * 0.075f) * shimmerEnv;
        float tail = ((float)sin(dashShimmerA) * 0.045f +
                      (float)sin(dashRing) * 0.035f +
                      (float)sin(dashTailRise) * 0.052f +
                      (dashFiltered - dashAirState) * 0.10f) * tailEnv;
        float scrape = (noise - dashMid) * cutEnv * 0.42f;
        float bite = (noise - dashMid) * biteEnv * 0.16f;
        gSfxDashSample[i] = Clamp((float)tanh((air + blade + edge + ring + glass + shimmer + tail + scrape + bite) * 1.76f) * env * 0.88f, -1.0f, 1.0f);
    }

    gSfxSamplesReady = true;
}

static float ReadSfxSample(const float* samples, int sampleCount, float age)
{
    float position = age * (float)kAudioSampleRate;
    int index = (int)position;
    if (index < 0 || index >= sampleCount - 1) return 0.0f;
    float frac = position - (float)index;
    return samples[index] * (1.0f - frac) + samples[index + 1] * frac;
}

static float RenderSfxVoice(SfxVoice& voice, double twoPi)
{
    if (!voice.active) return 0.0f;

    const float dt = 1.0f / (float)kAudioSampleRate;
    float t = voice.age / voice.duration;
    if (t >= 1.0f)
    {
        voice.active = false;
        return 0.0f;
    }

    float out = 0.0f;
    float n = VoiceNoise(voice);
    if (voice.type == SfxHookFire)
    {
        out = ReadSfxSample(gSfxHookFireSample, kSfxHookFireSampleCount, voice.age);
        voice.age += dt;
        return Clamp(out, -1.0f, 1.0f);
    }
    if (voice.type == SfxHookHit)
    {
        out = ReadSfxSample(gSfxHookHitSample, kSfxHookHitSampleCount, voice.age);
        voice.age += dt;
        return Clamp(out, -1.0f, 1.0f);
    }
    if (voice.type == SfxJump)
    {
        out = ReadSfxSample(gSfxJumpSample, kSfxJumpSampleCount, voice.age);
        voice.age += dt;
        return Clamp(out, -1.0f, 1.0f);
    }
    if (voice.type == SfxHookDash)
    {
        out = ReadSfxSample(gSfxDashSample, kSfxDashSampleCount, voice.age);
        voice.age += dt;
        return Clamp(out, -1.0f, 1.0f);
    }
    switch (voice.type)
    {
        case SfxUiClick:
        {
            float env = 1.0f - SmoothStep(t);
            float freq = 740.0f - 160.0f * t;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            out = (float)sin(voice.phaseA) * env * 0.26f + n * env * 0.035f;
        } break;

        case SfxHookFire:
        {
            float env = 1.0f - SmoothStep(t);
            float snap = 1.0f - SmoothStep(t / 0.22f);
            float freq = 260.0f + 520.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (92.0f - 24.0f * t) / (double)kAudioSampleRate;
            float whip = (float)sin(voice.phaseA) * 0.32f + n * 0.16f;
            float body = (float)sin(voice.phaseB) * 0.42f;
            out = (float)tanh((whip + body) * 1.8f) * env * 0.68f + n * snap * 0.08f;
        } break;

        case SfxHookHit:
        {
            float env = 1.0f - SmoothStep(t);
            float clack = 1.0f - SmoothStep(t / 0.18f);
            voice.phaseA += twoPi * (115.0f - 42.0f * t) / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (620.0f - 280.0f * t) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 0.90 + sin(voice.phaseB) * 0.20 + n * 0.28f) * 2.4f) * env * 0.78f +
                  n * clack * 0.10f;
        } break;

        case SfxHookDash:
        {
            float env = 1.0f - SmoothStep(t);
            float cut = 1.0f - SmoothStep(t / 0.42f);
            float freq = 118.0f - 52.0f * t;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (760.0f - 430.0f * SmoothStep(t)) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 1.05 + sin(voice.phaseB) * 0.26 + n * (0.26f * cut)) * 2.8) * env * 0.86f;
        } break;

        case SfxSuperDash:
        {
            float env = SmoothStep(t / 0.08f) * (1.0f - SmoothStep((t - 0.86f) / 0.14f));
            float launch = 1.0f - SmoothStep(t / 0.22f);
            float freq = 84.0f + 420.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (48.0f + 20.0f * sin(voice.age * 19.0f)) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 0.62 + sin(voice.phaseB) * 0.78 + n * 0.18f) * 2.0) * env * 0.88f +
                  (float)sin(voice.phaseB) * launch * 0.18f;
        } break;

        case SfxHit:
        {
            float env = 1.0f - SmoothStep(t);
            float freq = 95.0f - 36.0f * t;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 1.1 + n * 0.48f) * 2.6) * env * 0.70f;
        } break;

        case SfxHeal:
        {
            float env = SmoothStep(t / 0.12f) * (1.0f - SmoothStep((t - 0.72f) / 0.28f));
            float freq = t < 0.45f ? 523.25f : 659.25f;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 2.0f) / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * 0.42f + (float)sin(voice.phaseB) * 0.08f) * env * 0.62f;
        } break;

        case SfxDeath:
        {
            float env = 1.0f - SmoothStep(t);
            float freq = 72.0f - 48.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (34.0f - 12.0f * t) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 1.05 + sin(voice.phaseB) * 0.70 + n * (0.28f + t * 0.40f)) * 3.2) * env * 1.00f;
        } break;

        case SfxStageClear:
        {
            float env = SmoothStep(t / 0.08f) * (1.0f - SmoothStep((t - 0.86f) / 0.14f));
            float freq = t < 0.28f ? 261.63f : (t < 0.56f ? 392.0f : 523.25f);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 0.5f) / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * 0.44f + (float)sin(voice.phaseB) * 0.30f + n * 0.035f) * env * 0.84f;
        } break;

        case SfxPerkRoulette:
        {
            float tick = (float)sin(voice.age * 95.0f);
            float tickGate = tick > 0.83f ? (tick - 0.83f) * 5.8f : 0.0f;
            float env = 1.0f - SmoothStep((t - 0.72f) / 0.28f);
            voice.phaseA += twoPi * (260.0f + 120.0f * sin(voice.age * 7.0f)) / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * 0.24f + n * 0.13f) * tickGate * env * 0.48f;
        } break;

        case SfxPerkConfirm:
        {
            float env = SmoothStep(t / 0.06f) * (1.0f - SmoothStep((t - 0.74f) / 0.26f));
            float freq = 246.94f + 392.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 0.5f) / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * 0.46f + (float)sin(voice.phaseB) * 0.32f + n * 0.04f) * env * 0.82f;
        } break;

        case SfxShopEnter:
        {
            float env = 1.0f - SmoothStep(t);
            float freq = 150.0f - 58.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 1.25f + 12.0f * sin(voice.age * 11.0f)) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 0.82 + sin(voice.phaseB) * 0.28 + n * 0.08f) * 1.9) * env * 0.74f;
        } break;

        case SfxJump:
        {
            float env = 1.0f - SmoothStep(t);
            float freq = 128.0f + 82.0f * SmoothStep(t);
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (54.0f + 18.0f * t) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 0.42 + sin(voice.phaseB) * 0.30 + n * 0.055f) * 1.5) * env * 0.56f;
        } break;

        case SfxDynamite:
        {
            float env = 1.0f - SmoothStep(t);
            float boom = 1.0f - SmoothStep(t / 0.55f);
            voice.phaseA += twoPi * (52.0f - 20.0f * t) / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (118.0f - 64.0f * t) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 1.15 + sin(voice.phaseB) * 0.38 + n * (0.72f - t * 0.30f)) * 2.9) * env * 1.00f +
                  n * boom * 0.16f;
        } break;

        case SfxHourglass:
        {
            float env = SmoothStep(t / 0.04f) * (1.0f - SmoothStep((t - 0.96f) / 0.04f));
            float tick = (float)sin(voice.age * 6.2831853f * 2.0f);
            float tickGate = tick > 0.90f ? (tick - 0.90f) * 10.0f : 0.0f;
            voice.phaseA += twoPi * 1760.0 / (double)kAudioSampleRate;
            voice.phaseB += twoPi * 58.0 / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * tickGate * 0.32f +
                   (float)sin(voice.phaseB) * 0.12f) * env * 0.52f;
        } break;

        case SfxSawBlade:
        {
            float env = SmoothStep(t / 0.05f) * (1.0f - SmoothStep((t - 0.82f) / 0.18f));
            float freq = 260.0f + 980.0f * t;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 0.37f + 44.0f * sin(voice.age * 20.0f)) / (double)kAudioSampleRate;
            out = (float)tanh((sin(voice.phaseA) * 0.38 + sin(voice.phaseB) * 0.36 + n * 0.26f) * 2.4) * env * 0.76f;
        } break;

        case SfxReached:
        {
            float env = SmoothStep(t / 0.06f) * (1.0f - SmoothStep((t - 0.76f) / 0.24f));
            float freq = t < 0.36f ? 659.25f : 987.77f;
            voice.phaseA += twoPi * freq / (double)kAudioSampleRate;
            voice.phaseB += twoPi * (freq * 2.0f) / (double)kAudioSampleRate;
            out = ((float)sin(voice.phaseA) * 0.42f + (float)sin(voice.phaseB) * 0.055f) * env * 0.66f;
        } break;

        default:
            break;
    }

    voice.age += dt;
    if (voice.phaseA > twoPi) voice.phaseA -= twoPi;
    if (voice.phaseB > twoPi) voice.phaseB -= twoPi;
    return Clamp(out, -1.0f, 1.0f);
}

static void FillAudioBuffer(int bufferIndex)
{
    const double twoPi = 6.28318530717958647692;
    float master = Clamp(gMasterSound, 0.0f, 1.0f);
    bool gameplayMusic = gScreen == ScreenGameplay;
    float gameplayWeight = gameplayMusic ? 1.0f : 0.78f;
    float danger = 0.0f;
    if (gameplayMusic)
        danger = Clamp(((float)kBufferH - gDevourerY) / 170.0f, 0.0f, 1.0f);
    float volume = master * (gameplayMusic ? (0.50f + danger * 0.12f) : 0.58f) * gameplayWeight;
    int audioChapter = gameplayMusic ? gChapter : 0;
    if (audioChapter < 0) audioChapter = 0;
    if (audioChapter >= kChapterCount) audioChapter = kChapterCount - 1;
    if (gPlayerDead || gEndingActive || gPerkRouletteOpen)
        volume *= 0.82f;
    if (gEndingActive && gEndingTimer < 0.22f)
        volume *= 0.10f + SmoothStep(gEndingTimer / 0.22f) * 0.90f;
    if (gTimeStopTimer > 0.0f)
        volume *= 0.50f;

    for (int i = 0; i < kAudioBufferSamples; ++i)
    {
        gAudioPhaseA += twoPi * (25.0 + danger * 2.0) / (double)kAudioSampleRate;
        gAudioPhaseB += twoPi * (37.5 + danger * 1.2) / (double)kAudioSampleRate;
        gAudioPhaseC += twoPi * 61.0 / (double)kAudioSampleRate;
        gAudioPhaseD += twoPi * (78.0 + danger * 2.0) / (double)kAudioSampleRate;
        gAudioPhaseE += twoPi * 0.115 / (double)kAudioSampleRate;
        double subBaseFreq = 31.0;
        double orbitBaseFreq = 62.0;
        if (gameplayMusic && audioChapter == 1)
        {
            subBaseFreq = 30.0;
            orbitBaseFreq = 66.0;
        }
        else if (gameplayMusic && audioChapter == 2)
        {
            subBaseFreq = 32.0;
            orbitBaseFreq = 64.0;
        }
        else if (gameplayMusic && audioChapter >= 3)
        {
            subBaseFreq = 28.5;
            orbitBaseFreq = 58.0;
        }
        gAudioSubPhase += twoPi * (subBaseFreq + danger * 2.5) / (double)kAudioSampleRate;
        double orbitFreq = orbitBaseFreq + sin(gAudioPhaseE * 1.00) * 3.4 + sin(gAudioPhaseE * 1.90) * 1.0 + danger * 1.5;
        gAudioOrbitPhase += twoPi * orbitFreq / (double)kAudioSampleRate;
        gAudioMelodyTime += 1.0 / (double)kAudioSampleRate;

        static const double leadNotes[] = {
            261.63, 0.0, 246.94, 207.65, 196.00, 0.0, 174.61, 196.00,
            207.65, 0.0, 233.08, 207.65, 155.56, 174.61, 196.00, 0.0,
            261.63, 311.13, 293.66, 246.94, 207.65, 0.0, 196.00, 174.61,
            155.56, 0.0, 174.61, 196.00, 207.65, 246.94, 233.08, 0.0
        };
        static const double bassNotes[] = {
            65.41, 61.74, 51.91, 58.27, 43.65, 51.91, 49.00, 58.27
        };
        static const double arpNotes[] = {
            392.00, 311.13, 261.63, 233.08, 392.00, 349.23, 293.66, 246.94
        };

        double leadBeat = gAudioMelodyTime / 0.42;
        int leadIndex = ((int)leadBeat) & 31;
        double leadPos = leadBeat - (double)((int)leadBeat);
        double leadFreq = leadNotes[leadIndex] * 0.8909;
        double leadEnv = SmoothStep((float)(leadPos / 0.10)) *
                         (1.0 - SmoothStep((float)((leadPos - 0.70) / 0.30)));
        if (leadFreq > 1.0)
            gAudioLeadPhase += twoPi * leadFreq * (1.0 + sin(gAudioPhaseE * 0.9) * 0.002) / (double)kAudioSampleRate;

        double bassBeat = gAudioMelodyTime / 1.68;
        int bassIndex = ((int)bassBeat) & 7;
        double bassPos = bassBeat - (double)((int)bassBeat);
        double bassFreq = bassNotes[bassIndex];
        double bassPunch = 1.0 - SmoothStep((float)(bassPos / 0.18));
        gAudioBassPhase += twoPi * bassFreq / (double)kAudioSampleRate;

        double arpBeat = gAudioMelodyTime / 0.21;
        int arpIndex = ((int)arpBeat) & 7;
        double arpPos = arpBeat - (double)((int)arpBeat);
        double arpEnv = SmoothStep((float)(arpPos / 0.08)) *
                        (1.0 - SmoothStep((float)((arpPos - 0.50) / 0.50)));
        gAudioArpPhase += twoPi * arpNotes[arpIndex] / (double)kAudioSampleRate;

        double screamLength = 22.0;
        if (gameplayMusic && audioChapter == 1) screamLength = 26.0;
        else if (gameplayMusic && audioChapter == 2) screamLength = 30.0;
        else if (gameplayMusic && audioChapter >= 3) screamLength = 18.0;
        double screamCycle = gAudioMelodyTime / screamLength;
        double screamPos = screamCycle - (double)((int)screamCycle);
        double screamEnv = SmoothStep((float)((screamPos - 0.08) / 0.42)) *
                           (1.0 - SmoothStep((float)((screamPos - 0.78) / 0.22)));
        double screamFreq = (audioChapter >= 3 ? 39.0 : 44.0) + danger * 14.0 +
                            sin(gAudioPhaseE * 3.3) * 4.0 +
                            (1.0 - screamPos) * 7.0;
        gAudioMelodyPhase += twoPi * screamFreq / (double)kAudioSampleRate;

        if (gAudioPhaseA > twoPi) gAudioPhaseA -= twoPi;
        if (gAudioPhaseB > twoPi) gAudioPhaseB -= twoPi;
        if (gAudioPhaseC > twoPi) gAudioPhaseC -= twoPi;
        if (gAudioPhaseD > twoPi) gAudioPhaseD -= twoPi;
        if (gAudioPhaseE > twoPi) gAudioPhaseE -= twoPi;
        if (gAudioMelodyPhase > twoPi) gAudioMelodyPhase -= twoPi;
        if (gAudioLeadPhase > twoPi) gAudioLeadPhase -= twoPi;
        if (gAudioBassPhase > twoPi) gAudioBassPhase -= twoPi;
        if (gAudioArpPhase > twoPi) gAudioArpPhase -= twoPi;
        if (gAudioSubPhase > twoPi) gAudioSubPhase -= twoPi;
        if (gAudioOrbitPhase > twoPi) gAudioOrbitPhase -= twoPi;

        float noise = AudioNoise01() * 2.0f - 1.0f;
        gAudioLowNoise += (noise - gAudioLowNoise) * 0.004f;
        gAudioRumble += (gAudioLowNoise - gAudioRumble) * 0.0018f;
        gAudioHiss += (noise - gAudioHiss) * 0.035f;
        gAudioMetal += (gAudioHiss - gAudioMetal) * 0.014f;

        double slowPulse = sin(gAudioPhaseE * 1.35);
        double dreadSweep = sin(gAudioPhaseE);
        double metalPulse = sin(gAudioPhaseD + sin(gAudioPhaseE * 4.0) * 0.9);
        double drone =
            sin(gAudioPhaseA + dreadSweep * 0.15) * 0.26 +
            sin(gAudioPhaseB + slowPulse * 0.18) * 0.12 +
            sin(gAudioPhaseC - dreadSweep * 0.20) * 0.05;
        float breath = (float)(0.86 + 0.14 * slowPulse);
        float pitAir = gAudioRumble * 0.38f + gAudioLowNoise * 0.08f;
        float metal = (float)metalPulse * 0.018f + (gAudioHiss - gAudioMetal) * 0.045f;

        double bassWave =
            sin(gAudioBassPhase) * 0.62 +
            sin(gAudioBassPhase * 2.0 + 0.25) * 0.18 +
            sin(gAudioBassPhase * 3.0) * 0.05;
        double leadWave = 0.0;
        if (leadFreq > 1.0)
        {
            leadWave =
                sin(gAudioLeadPhase) * 0.55 +
                sin(gAudioLeadPhase * 2.0 + 0.7) * 0.20 +
                sin(gAudioLeadPhase * 3.0 - 0.4) * 0.10;
            leadWave = tanh(leadWave * 1.45);
        }
        double arpWave =
            sin(gAudioArpPhase) * 0.34 +
            sin(gAudioArpPhase * 2.0) * 0.10;

        double screamWave =
            sin(gAudioMelodyPhase) * 0.62 +
            sin(gAudioMelodyPhase * 2.0 + gAudioPhaseE * 9.0) * 0.24 +
            sin(gAudioMelodyPhase * 3.0 - gAudioPhaseE * 5.0) * 0.14;
        screamWave = tanh(screamWave * 3.2);
        double subWave =
            sin(gAudioSubPhase) * 0.74 +
            sin(gAudioSubPhase * 2.0 + 0.45) * 0.18 +
            sin(gAudioSubPhase * 3.0 - 0.2) * 0.06;
        double subPressure = tanh(subWave * 2.1);
        double orbitWave =
            sin(gAudioOrbitPhase + sin(gAudioPhaseE * 1.10) * 0.10) * 0.84 +
            sin(gAudioOrbitPhase * 0.5 + gAudioPhaseE * 0.55) * 0.11;
        double orbitPulse = 0.84 + 0.16 * sin(gAudioPhaseE * 0.44 + sin(gAudioPhaseE * 0.26));
        double orbitResonance = tanh(orbitWave * 0.48) * orbitPulse;
        float contaminatedMetal =
            (float)(sin(gAudioPhaseD * 2.0 + sin(gAudioPhaseE * 5.0) * 1.7) * 0.10 +
                    sin(gAudioPhaseC * 3.0 - sin(gAudioPhaseE * 3.0) * 1.1) * 0.06);
        float chapterTone = 0.0f;
        if (gameplayMusic && audioChapter == 1)
        {
            double rustPulse = 0.48 + 0.52 * sin(gAudioPhaseE * 0.86 + sin(gAudioPhaseE * 1.8));
            double rustGate = 0.62 + 0.38 * sin(gAudioMelodyTime * 2.35);
            double rust =
                sin(gAudioPhaseD * 2.0 + sin(gAudioPhaseE * 2.2) * 1.1) * 0.20 +
                sin(gAudioPhaseC * 2.5 - gAudioPhaseE * 1.1) * 0.10 +
                sin(gAudioOrbitPhase * 1.35 + gAudioPhaseE * 0.7) * 0.08;
            chapterTone = (float)tanh(rust * 1.45) * (float)rustPulse * (float)rustGate * 0.34f;
        }
        else if (gameplayMusic && audioChapter == 2)
        {
            double pipePulse = 0.74 + 0.26 * sin(gAudioPhaseE * 0.25);
            double pipeBend = sin(gAudioPhaseE * 0.42) * 0.18;
            double pipe =
                sin(gAudioOrbitPhase * 0.72 + pipeBend) * 0.16 +
                sin(gAudioOrbitPhase * 1.08 + sin(gAudioPhaseE * 0.9) * 0.10) * 0.06;
            chapterTone = (float)tanh(pipe * 0.72) * (float)pipePulse * 0.18f;
        }
        else if (gameplayMusic && audioChapter >= 3)
        {
            double abyssPulse = 0.74 + 0.26 * sin(gAudioPhaseE * 0.48 + sin(gAudioPhaseE * 0.8));
            double abyss =
                sin(gAudioSubPhase * 0.5 - gAudioPhaseE * 0.6) * 0.20 +
                sin(gAudioOrbitPhase * 0.75 + gAudioPhaseE * 0.5) * 0.08;
            chapterTone = (float)tanh(abyss * 1.35) * (float)abyssPulse * 0.24f;
        }
        float sample = 0.0f;
        if (gameplayMusic)
        {
            float lowCrush = (float)tanh((gAudioRumble * 1.6f + (float)subPressure * 1.4f) * 1.9f);
            float screamWeight = 0.18f + danger * 0.24f;
            if (audioChapter == 1) screamWeight *= 0.82f;
            else if (audioChapter == 2) screamWeight *= 0.70f;
            else if (audioChapter >= 3) screamWeight *= 1.18f;
            float belowScream = (float)screamWave * (float)screamEnv * screamWeight;
            float throat = (gAudioHiss - gAudioMetal) * (0.020f + danger * 0.025f);
            float metalWeight = 0.08f + danger * 0.05f;
            if (audioChapter == 1) metalWeight = 0.24f + danger * 0.10f;
            else if (audioChapter == 2) metalWeight = 0.10f + danger * 0.05f;
            else if (audioChapter >= 3) metalWeight = 0.12f + danger * 0.07f;
            if (audioChapter == 0)
            {
                sample = (float)subPressure * (0.48f + danger * 0.14f) +
                         (float)drone * 0.06f +
                         (float)orbitResonance * (0.170f + danger * 0.030f) +
                         lowCrush * (0.18f + danger * 0.10f) +
                         gAudioRumble * (0.10f + danger * 0.07f) +
                         metal * 0.22f +
                         contaminatedMetal * metalWeight +
                         belowScream +
                         throat;
            }
            else if (audioChapter == 1)
            {
                double factoryBeat = sin(gAudioMelodyTime * 3.1 + sin(gAudioPhaseE * 2.3));
                float pressHit = factoryBeat > 0.42 ? (float)((factoryBeat - 0.42) * 0.34) : 0.0f;
                float rustAir = (gAudioHiss - gAudioMetal) * 0.035f;
                sample = (float)subPressure * (0.28f + danger * 0.08f) +
                         (float)orbitResonance * (0.12f + danger * 0.02f) +
                         lowCrush * (0.10f + danger * 0.06f) +
                         metal * (0.42f + danger * 0.10f) +
                         contaminatedMetal * (0.38f + danger * 0.12f) +
                         chapterTone * 1.38f +
                         pressHit +
                         belowScream * 0.55f +
                         rustAir;
            }
            else if (audioChapter == 2)
            {
                double pipeWhoom = 0.5 + 0.5 * sin(gAudioPhaseE * 0.34 - sin(gAudioPhaseE * 0.12));
                float hollow = (float)tanh(((float)orbitResonance * 0.85f + (float)drone * 0.28f) * 0.78f);
                float pressureWave = (float)subPressure * (0.13f + (float)pipeWhoom * 0.05f);
                sample = pressureWave +
                         hollow * (0.17f + danger * 0.025f) +
                         chapterTone * 0.95f +
                         gAudioRumble * (0.09f + danger * 0.035f) +
                         lowCrush * (0.045f + danger * 0.025f) +
                         contaminatedMetal * 0.05f +
                         belowScream * 0.28f +
                         throat * 0.35f;
            }
            else
            {
                float abyssBase = (float)subPressure * (0.58f + danger * 0.18f);
                float abyssThroat = (float)screamWave * (float)screamEnv * (0.28f + danger * 0.36f);
                float deepDrag = (float)tanh(((float)subPressure * 1.6f + gAudioRumble * 2.2f) * 1.45f);
                sample = abyssBase +
                         deepDrag * (0.24f + danger * 0.12f) +
                         (float)orbitResonance * (0.10f + danger * 0.03f) +
                         chapterTone * 1.35f +
                         metal * (0.18f + danger * 0.08f) +
                         contaminatedMetal * (0.16f + danger * 0.10f) +
                         abyssThroat +
                         throat;
            }
        }
        else
        {
            sample = (float)drone * breath + pitAir + metal;
            sample += (float)bassWave * (0.20f + (float)bassPunch * 0.08f);
            sample += (float)leadWave * (float)leadEnv * 0.54f;
            sample += (float)arpWave * (float)arpEnv * 0.115f;
        }

        if (!gameplayMusic && danger > 0.35f)
        {
            float pulse = (float)sin(gAudioPhaseA * 0.42);
            if (pulse > 0.88f)
                sample += (pulse - 0.88f) * danger * 0.22f;
        }

        float leadPan = gameplayMusic ? 0.0f : (float)sin(gAudioPhaseE * 2.2) * 0.18f;
        float pan = (float)(sin(gAudioPhaseE * 3.0) * 0.13 + sin(gAudioPhaseB * 0.07) * 0.06);
        if (gameplayMusic)
            pan *= 0.32f;
        float left = sample + metal * 0.28f - pan * 0.08f - (float)leadWave * (float)leadEnv * leadPan * 0.10f;
        float right = sample - metal * 0.28f + pan * 0.08f + (float)leadWave * (float)leadEnv * leadPan * 0.10f;
        float echoL = gAudioEchoL[gAudioEchoIndex];
        float echoR = gAudioEchoR[gAudioEchoIndex];
        float echoFeedback = gameplayMusic ? 0.06f : 0.24f;
        float echoWet = gameplayMusic ? 0.045f : 0.20f;
        gAudioEchoL[gAudioEchoIndex] = left + echoR * echoFeedback;
        gAudioEchoR[gAudioEchoIndex] = right + echoL * echoFeedback;
        ++gAudioEchoIndex;
        if (gAudioEchoIndex >= kAudioEchoSamples)
            gAudioEchoIndex = 0;
        left += echoL * echoWet;
        right += echoR * echoWet;
        float sfx = 0.0f;
        for (int voiceIndex = 0; voiceIndex < kMaxSfxVoices; ++voiceIndex)
            if (gSfxVoices[voiceIndex].active)
                sfx += RenderSfxVoice(gSfxVoices[voiceIndex], twoPi);
        sfx = Clamp(sfx, -1.2f, 1.2f);
        left = left * volume + sfx * master * 0.82f;
        right = right * volume + sfx * master * 0.82f;
        left = Clamp(left, -0.88f, 0.88f);
        right = Clamp(right, -0.88f, 0.88f);
        int outIndex = i * kAudioChannels;
        gAudioSamples[bufferIndex][outIndex + 0] = (int16_t)(left * 32767.0f);
        gAudioSamples[bufferIndex][outIndex + 1] = (int16_t)(right * 32767.0f);
    }
}

static void QueueAudioBuffer(int bufferIndex)
{
    if (!gAudioDevice) return;
    WAVEHDR& header = gAudioHeaders[bufferIndex];
    if (header.dwFlags & WHDR_PREPARED)
        waveOutUnprepareHeader(gAudioDevice, &header, sizeof(header));
    FillAudioBuffer(bufferIndex);
    header = {};
    header.lpData = (LPSTR)gAudioSamples[bufferIndex];
    header.dwBufferLength = sizeof(gAudioSamples[bufferIndex]);
    waveOutPrepareHeader(gAudioDevice, &header, sizeof(header));
    waveOutWrite(gAudioDevice, &header, sizeof(header));
}

static void StartAudio()
{
    if (gAudioRunning) return;
    BakeSfxSamples();
    WAVEFORMATEX format = {};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = kAudioChannels;
    format.nSamplesPerSec = kAudioSampleRate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (WORD)(format.nChannels * format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    if (waveOutOpen(&gAudioDevice, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
    {
        gAudioDevice = 0;
        return;
    }
    gAudioRunning = true;
    for (int i = 0; i < kAudioBufferCount; ++i)
        QueueAudioBuffer(i);
}

static void UpdateAudio()
{
    if (!gAudioRunning || !gAudioDevice) return;
    for (int i = 0; i < kAudioBufferCount; ++i)
    {
        if (gAudioHeaders[i].dwFlags & WHDR_DONE)
            QueueAudioBuffer(i);
    }
}

static void StopAudio()
{
    if (!gAudioDevice) return;
    waveOutReset(gAudioDevice);
    for (int i = 0; i < kAudioBufferCount; ++i)
    {
        if (gAudioHeaders[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(gAudioDevice, &gAudioHeaders[i], sizeof(gAudioHeaders[i]));
        gAudioHeaders[i] = {};
    }
    waveOutClose(gAudioDevice);
    gAudioDevice = 0;
    gAudioRunning = false;
}

static const ChapterConfig& CurrentChapterConfig()
{
    int index = gChapter;
    if (index < 0) index = 0;
    if (index >= kChapterCount) index = kChapterCount - 1;
    return gChapterConfigs[index];
}

static float ChapterStartMeters(int chapter)
{
    return (float)chapter * kChapterLengthMeters;
}

static float CurrentChapterLocalMeters()
{
    return gViewHeightPixels / kPixelsPerMeter - ChapterStartMeters(gChapter);
}

static int CurrentChapterProgressMeters()
{
    float local = gHeightPixels / kPixelsPerMeter - ChapterStartMeters(gChapter);
    if (gInfiniteMode && gChapter == kChapterCount - 1)
    {
        if (local < 0.0f) local = 0.0f;
    }
    else
    {
        local = Clamp(local, 0.0f, kChapterLengthMeters);
    }
    return (int)(local + 0.5f);
}

static void PipeProfileForMeters(float meters, float* centerOut, float* halfWidthOut)
{
    float local = meters - ChapterStartMeters(gChapter);
    if (local < 0.0f) local = 0.0f;
    int segment = (int)(local / 170.0f);
    float phase = (local - (float)segment * 170.0f) / 170.0f;
    phase = SmoothStep(phase);
    float anchors[] = { -46.0f, 38.0f, -28.0f, 50.0f, -8.0f, 28.0f };
    float from = anchors[segment % 6];
    float to = anchors[(segment + 1) % 6];
    float center = kBufferW * 0.5f + from + (to - from) * phase;

    float halfWidth = gChapter == 2 ? 66.0f : 116.0f;
    if (gChapter == 2)
    {
        halfWidth = 62.0f;
    }
    else if (gChapter == 3)
    {
        bool wideSection = ((int)(local / 300.0f) & 1) == 0;
        float sectionPhase = local - (float)((int)(local / 300.0f)) * 300.0f;
        float blend = SmoothStep(sectionPhase / 85.0f);
        float narrow = 86.0f;
        float wide = 145.0f;
        halfWidth = wideSection
            ? narrow + (wide - narrow) * blend
            : wide + (narrow - wide) * blend;

        if (local < 220.0f)
        {
            float entry = SmoothStep(local / 220.0f);
            float normalHalf = (float)kBufferW * 0.5f - 6.0f;
            center = kBufferW * 0.5f + (center - kBufferW * 0.5f) * entry;
            halfWidth = normalHalf + (halfWidth - normalHalf) * entry;
        }
    }

    *centerOut = center;
    *halfWidthOut = halfWidth;
}

static float PipeCenterForMeters(float meters)
{
    float center = kBufferW * 0.5f;
    float halfWidth = 100.0f;
    PipeProfileForMeters(meters, &center, &halfWidth);
    return center;
}

static float BranchStartMetersForChapter(int chapter)
{
    return ChapterStartMeters(chapter);
}

static bool ChapterHasBranchDividers(int chapter);

static float BranchAmountForMeters(float meters)
{
    if (!ChapterHasBranchDividers(gChapter)) return 0.0f;
    float local = meters - BranchStartMetersForChapter(gChapter);
    if (local < 0.0f) return 0.0f;
    float cycle = 420.0f;
    float start = 125.0f;
    float end = 295.0f;
    float phase = local - (float)((int)(local / cycle)) * cycle;
    if (phase <= start || phase >= end) return 0.0f;
    float open = SmoothStep((phase - start) / 46.0f);
    float close = 1.0f - SmoothStep((phase - (end - 52.0f)) / 52.0f);
    return Clamp(open * close, 0.0f, 1.0f);
}

static int BranchSideForMeters(float meters)
{
    float local = meters - ChapterStartMeters(gChapter);
    float cycle = 420.0f;
    int index = (int)(local / cycle);
    return (index & 1) ? -1 : 1;
}

static bool ChapterHasBranchDividers(int chapter)
{
    (void)chapter;
    return false;
}

static bool BranchDividerForScreenYWithThreshold(float y, float threshold, RectF* rect)
{
    if (!ChapterHasBranchDividers(gChapter)) return false;
    float meters = gViewHeightPixels / kPixelsPerMeter +
                   (kCameraLockY - y) / kPixelsPerMeter;
    float branch = BranchAmountForMeters(meters);
    if (branch <= threshold) return false;
    float center = PipeCenterForMeters(meters) +
                   (float)BranchSideForMeters(meters) * 18.0f * branch;
    float half = branch * 8.5f;
    *rect = { center - half, y - 5.0f, half * 2.0f, 10.0f };
    return true;
}

static bool BranchDividerForScreenY(float y, RectF* rect)
{
    return BranchDividerForScreenYWithThreshold(y, 0.18f, rect);
}

static void ApplyChapterWalls(float meters)
{
    const ChapterConfig& config = CurrentChapterConfig();
    if (!config.pipeWalls)
    {
        gSideWalls[0] = { 0.0f, 0.0f, 6.0f, (float)kBufferH };
        gSideWalls[1] = { (float)kBufferW - 6.0f, 0.0f, 6.0f, (float)kBufferH };
        return;
    }

    float center = kBufferW * 0.5f;
    float halfWidth = 100.0f;
    PipeProfileForMeters(meters, &center, &halfWidth);
    float leftEdge = Clamp(center - halfWidth, 8.0f, (float)kBufferW - 80.0f);
    float rightEdge = Clamp(center + halfWidth, 80.0f, (float)kBufferW - 8.0f);
    if (rightEdge - leftEdge < 112.0f)
    {
        rightEdge = leftEdge + 112.0f;
        if (rightEdge > (float)kBufferW - 8.0f)
        {
            rightEdge = (float)kBufferW - 8.0f;
            leftEdge = rightEdge - 112.0f;
        }
    }

    gSideWalls[0] = { 0.0f, 0.0f, leftEdge, (float)kBufferH };
    gSideWalls[1] = { rightEdge, 0.0f, (float)kBufferW - rightEdge, (float)kBufferH };
}

static float PlayfieldLeft()
{
    return gSideWalls[0].x + gSideWalls[0].w;
}

static float PlayfieldRight()
{
    return gSideWalls[1].x;
}

static void PlayfieldBoundsForScreenY(float y, float* leftOut, float* rightOut)
{
    const ChapterConfig& config = CurrentChapterConfig();
    if (!config.pipeWalls)
    {
        *leftOut = PlayfieldLeft();
        *rightOut = PlayfieldRight();
        return;
    }

    float meters = gViewHeightPixels / kPixelsPerMeter +
                   (kCameraLockY - y) / kPixelsPerMeter;
    float center = kBufferW * 0.5f;
    float halfWidth = 100.0f;
    PipeProfileForMeters(meters, &center, &halfWidth);
    float left = Clamp(center - halfWidth, 8.0f, (float)kBufferW - 80.0f);
    float right = Clamp(center + halfWidth, 80.0f, (float)kBufferW - 8.0f);
    if (right - left < 112.0f)
    {
        right = left + 112.0f;
        if (right > (float)kBufferW - 8.0f)
        {
            right = (float)kBufferW - 8.0f;
            left = right - 112.0f;
        }
    }
    *leftOut = left;
    *rightOut = right;
}

static int NextHeightMilestoneAfter(float heightPixels)
{
    int meters = (int)(heightPixels / kPixelsPerMeter);
    return (meters / kHeightMarkerInterval + 1) * kHeightMarkerInterval;
}

static int CoinMultiplier()
{
    int multiplier = 1 + gCombo / 10;
    return multiplier < 1 ? 1 : multiplier;
}

static float CurrentPlayerMeters()
{
    return gViewHeightPixels / kPixelsPerMeter;
}

static float ChapterProgress()
{
    float localMeters = CurrentPlayerMeters() - ChapterStartMeters(gChapter);
    return Clamp(localMeters / kChapterLengthMeters, 0.0f, 1.0f);
}

static bool StartShopPortalPosition(Vec2* position)
{
    if (gChapter < 0 || gChapter >= kChapterCount) return false;
    if (gChapter == 0)
    {
        if (gFreeShopClaimed) return false;
    }
    else
    {
        if (gStartShopUsed[gChapter]) return false;
    }

    float startMeters = ChapterStartMeters(gChapter);
    float currentMeters = gViewHeightPixels / kPixelsPerMeter;
    float y = 88.0f + (currentMeters - startMeters) * kPixelsPerMeter;
    if (y < -28.0f || y > (float)kBufferH + 36.0f) return false;

    float left, right;
    PlayfieldBoundsForScreenY(y, &left, &right);
    *position = { right - 30.0f, y };
    return true;
}

static bool MidShopPortalPosition(Vec2* position)
{
    if (gChapter < 0 || gChapter >= kChapterCount) return false;
    if (gShopUsed[gChapter]) return false;
    float portalMeters = ChapterStartMeters(gChapter) + kShopPortalMeters;
    float currentMeters = gViewHeightPixels / kPixelsPerMeter;
    float y = kCameraLockY - (portalMeters - currentMeters) * kPixelsPerMeter;
    if (y < -28.0f || y > (float)kBufferH + 36.0f) return false;
    float left, right;
    PlayfieldBoundsForScreenY(y, &left, &right);
    *position = { (left + right) * 0.5f, y };
    return true;
}

static bool ShopPortalPosition(Vec2* position, bool* freePortal, bool* startPortal)
{
    if (StartShopPortalPosition(position))
    {
        if (freePortal) *freePortal = gChapter == 0;
        if (startPortal) *startPortal = true;
        return true;
    }

    if (MidShopPortalPosition(position))
    {
        if (freePortal) *freePortal = false;
        if (startPortal) *startPortal = false;
        return true;
    }

    if (freePortal) *freePortal = false;
    if (startPortal) *startPortal = false;
    return false;
}

static int CurrentShopPrice()
{
    return gShopCurrentFree ? 0 : kShopPrices[gChapter];
}

static int BoolArrayToMask(const bool* values, int count)
{
    int mask = 0;
    for (int i = 0; i < count; ++i)
        if (values[i]) mask |= (1 << i);
    return mask;
}

static void MaskToBoolArray(int mask, bool* values, int count)
{
    for (int i = 0; i < count; ++i)
        values[i] = (mask & (1 << i)) != 0;
}

static bool HasPerk(PerkType perk)
{
    return perk > PerkNone && perk < PerkCount && gPerkOwned[perk];
}

static int PerkArrayToMask(const bool* values)
{
    int mask = 0;
    for (int i = 1; i < PerkCount; ++i)
        if (values[i]) mask |= (1 << i);
    return mask;
}

static void MaskToPerkArray(int mask, bool* values)
{
    values[PerkNone] = false;
    for (int i = 1; i < PerkCount; ++i)
        values[i] = (mask & (1 << i)) != 0;
}

static void CopyPerks(bool* dst, const bool* src)
{
    for (int i = 0; i < PerkCount; ++i)
        dst[i] = src[i];
}

static const char* PerkName(PerkType perk)
{
    switch (perk)
    {
        case PerkCrossNecklace: return "CROSS CHARM";
        case PerkParachute: return "PARACHUTE";
        case PerkDevilHeart: return "DEVIL HEART";
        case PerkAngelSkin: return "ANGEL SKIN";
        case PerkBloodBattery: return "BLOOD BATTERY";
        case PerkContract: return "CONTRACT";
        default: return "NO PERK";
    }
}

static const char* PerkDescription(PerkType perk)
{
    switch (perk)
    {
        case PerkCrossNecklace: return "REVIVE ONCE INTO A SUPER DASH";
        case PerkParachute: return "HOLD SPACE AFTER JUMP TO SLOW FALL";
        case PerkDevilHeart: return "STRONGER JUMP WITH A SHORT BURST";
        case PerkAngelSkin: return "DAMAGE TAKEN BECOMES HALF HEART";
        case PerkBloodBattery: return "GAIN ONE EXTRA AIR JUMP";
        case PerkContract: return "ACTIVE ITEMS GAIN ONE EXTRA USE";
        default: return "UNKNOWN CONTRACT";
    }
}

static const char* PerkRouletteDescription(PerkType perk)
{
    switch (perk)
    {
        case PerkCrossNecklace: return "REVIVE ON DEATH ONCE";
        case PerkParachute: return "HOLD SPACE TO SLOW FALL";
        case PerkDevilHeart: return "STRONGER JUMP BURST";
        case PerkAngelSkin: return "DAMAGE IS HALF HEART";
        case PerkBloodBattery: return "GAIN ONE AIR JUMP";
        case PerkContract: return "ACTIVE ITEMS PLUS ONE";
        default: return "";
    }
}

static const char* AchievementName(AchievementType achievement)
{
    switch (achievement)
    {
        case AchievementStage1: return "I";
        case AchievementStage2: return "II";
        case AchievementStage3: return "III";
        case AchievementEscape: return "ESCAPE";
        case AchievementHookVeteran: return "HOOK VETERAN";
        default: return "UNKNOWN";
    }
}

static const char* AchievementDescription(AchievementType achievement)
{
    switch (achievement)
    {
        case AchievementStage1: return "CLEAR STAGE 1";
        case AchievementStage2: return "CLEAR STAGE 2";
        case AchievementStage3: return "CLEAR STAGE 3";
        case AchievementEscape: return "INFINITE MODE UNLOCKED";
        case AchievementHookVeteran: return "CLEAR WITHOUT ACTIVE ITEMS";
        default: return "";
    }
}

static int AchievementMask()
{
    int mask = 0;
    for (int i = 0; i < AchievementCount; ++i)
        if (gAchievementUnlocked[i]) mask |= (1 << i);
    return mask;
}

static void ApplyAchievementMask(int mask)
{
    for (int i = 0; i < AchievementCount; ++i)
        gAchievementUnlocked[i] = (mask & (1 << i)) != 0;
}

static bool InfiniteModeUnlocked()
{
    return gGameClearedEver || gAchievementUnlocked[AchievementEscape];
}

static void QueueAchievementPopup(AchievementType achievement)
{
    if (gAchievementPopup == AchievementCount)
    {
        gAchievementPopup = achievement;
        gAchievementPopupTimer = 3.4f;
        PlaySfx(SfxReached);
        return;
    }
    if (gAchievementQueueCount >= (int)(sizeof(gAchievementQueue) / sizeof(gAchievementQueue[0])))
        return;
    gAchievementQueue[gAchievementQueueCount++] = achievement;
}

static void UnlockAchievement(AchievementType achievement)
{
    if (achievement < 0 || achievement >= AchievementCount) return;
    if (gAchievementUnlocked[achievement]) return;
    gAchievementUnlocked[achievement] = true;
    QueueAchievementPopup(achievement);
    SavePersistentData();
}

static void UpdateAchievementPopup(float dt)
{
    if (gAchievementPopup == AchievementCount)
    {
        if (gAchievementQueueCount <= 0) return;
        gAchievementPopup = gAchievementQueue[0];
        for (int i = 1; i < gAchievementQueueCount; ++i)
            gAchievementQueue[i - 1] = gAchievementQueue[i];
        --gAchievementQueueCount;
        gAchievementPopupTimer = 3.4f;
        PlaySfx(SfxReached);
        return;
    }
    gAchievementPopupTimer -= dt;
    if (gAchievementPopupTimer <= 0.0f)
    {
        gAchievementPopup = AchievementCount;
        gAchievementPopupTimer = 0.0f;
    }
}

static PerkType GrantRandomPerkForChapter(int chapter)
{
    if (chapter <= 0 || chapter >= kChapterCount) return PerkNone;
    int chapterBit = 1 << chapter;
    if ((gPerkGrantChapterMask & chapterBit) != 0) return PerkNone;

    PerkType candidates[PerkCount];
    int count = 0;
    for (int i = 1; i < PerkCount; ++i)
        if (!gPerkOwned[i])
            candidates[count++] = (PerkType)i;
    if (count <= 0)
    {
        gPerkGrantChapterMask |= chapterBit;
        return PerkNone;
    }

    int index = (int)(Random01() * (float)count);
    if (index < 0) index = 0;
    if (index >= count) index = count - 1;
    PerkType perk = candidates[index];
    gPerkOwned[perk] = true;
    gPerkGrantChapterMask |= chapterBit;
    if (perk == PerkContract && gActiveItem != ActiveNone)
        ++gActiveItemUses;
    return perk;
}

static void WriteIniInt(const char* section, const char* key, int value)
{
    char text[32];
    wsprintfA(text, "%d", value);
    WritePrivateProfileStringA(section, key, text, gSaveFileName);
}

static void InitializeSaveFilePath()
{
    DWORD length = GetModuleFileNameA(0, gSaveFileName, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        lstrcpyA(gSaveFileName, "hellpit_save.ini");
        return;
    }

    int slash = -1;
    for (int i = (int)length - 1; i >= 0; --i)
    {
        if (gSaveFileName[i] == '\\' || gSaveFileName[i] == '/')
        {
            slash = i;
            break;
        }
    }

    if (slash < 0)
    {
        lstrcpyA(gSaveFileName, "hellpit_save.ini");
        return;
    }

    const char* fileName = "hellpit_save.ini";
    int fileLength = lstrlenA(fileName);
    if (slash + 1 + fileLength >= MAX_PATH)
    {
        lstrcpyA(gSaveFileName, "hellpit_save.ini");
        return;
    }

    gSaveFileName[slash + 1] = 0;
    lstrcatA(gSaveFileName, fileName);
}

static void SavePersistentData()
{
    WriteIniInt("save", "version", 1);
    WriteIniInt("save", "checkpoint_valid", gCheckpointValid ? 1 : 0);
    WriteIniInt("save", "chapter", gCheckpointChapter);
    WriteIniInt("save", "infinite_mode", gCheckpointInfiniteMode ? 1 : 0);
    WriteIniInt("save", "coins", gCheckpointCoins);
    WriteIniInt("save", "active_item", (int)gCheckpointActiveItem);
    WriteIniInt("save", "active_uses", gCheckpointActiveItemUses);
    WriteIniInt("save", "used_active", gCheckpointRunUsedActiveItem ? 1 : 0);
    WriteIniInt("save", "perks", PerkArrayToMask(gCheckpointPerkOwned));
    WriteIniInt("save", "perk_chapters", gCheckpointPerkGrantChapterMask);
    WriteIniInt("save", "cross_revive_used", gCheckpointCrossReviveUsed ? 1 : 0);
    WriteIniInt("save", "free_shop", gCheckpointFreeShopClaimed ? 1 : 0);
    WriteIniInt("save", "mid_shops", BoolArrayToMask(gCheckpointShopUsed, kChapterCount));
    WriteIniInt("save", "start_shops", BoolArrayToMask(gCheckpointStartShopUsed, kChapterCount));
    for (int i = 0; i < kChapterCount; ++i)
    {
        char key[16];
        wsprintfA(key, "best_%d", i);
        WriteIniInt("save", key, gBestStageMeters[i]);
    }
    WriteIniInt("options", "brightness", (int)(Clamp(gBrightness, 0.0f, 1.0f) * 100.0f + 0.5f));
    WriteIniInt("options", "master_sound", (int)(Clamp(gMasterSound, 0.0f, 1.0f) * 100.0f + 0.5f));
    WriteIniInt("options", "crosshair", gCrosshairStyle);
    WriteIniInt("achievements", "unlocked", AchievementMask());
    WriteIniInt("achievements", "game_cleared", gGameClearedEver ? 1 : 0);
}

static void LoadPersistentData()
{
    int version = GetPrivateProfileIntA("save", "version", 0, gSaveFileName);
    gBrightness = Clamp((float)GetPrivateProfileIntA("options", "brightness", 100, gSaveFileName) / 100.0f,
                        0.0f, 1.0f);
    gMasterSound = Clamp((float)GetPrivateProfileIntA("options", "master_sound", 100, gSaveFileName) / 100.0f,
                         0.0f, 1.0f);
    gCrosshairStyle = GetPrivateProfileIntA("options", "crosshair", 0, gSaveFileName);
    if (gCrosshairStyle < 0) gCrosshairStyle = 0;
    if (gCrosshairStyle > 3) gCrosshairStyle = 3;

    if (version <= 0) return;

    gCheckpointValid = GetPrivateProfileIntA("save", "checkpoint_valid", 0, gSaveFileName) != 0;
    gCheckpointChapter = GetPrivateProfileIntA("save", "chapter", 0, gSaveFileName);
    if (gCheckpointChapter < 0) gCheckpointChapter = 0;
    if (gCheckpointChapter >= kChapterCount) gCheckpointChapter = kChapterCount - 1;
    gCheckpointInfiniteMode = GetPrivateProfileIntA("save", "infinite_mode", 0, gSaveFileName) != 0;
    gCheckpointCoins = GetPrivateProfileIntA("save", "coins", 0, gSaveFileName);
    gCheckpointActiveItem = (ActiveItemType)GetPrivateProfileIntA("save", "active_item", (int)ActiveNone, gSaveFileName);
    if (gCheckpointActiveItem < ActiveNone || gCheckpointActiveItem > ActiveSawBlade)
        gCheckpointActiveItem = ActiveNone;
    gCheckpointActiveItemUses = GetPrivateProfileIntA("save", "active_uses", 0, gSaveFileName);
    gCheckpointRunUsedActiveItem = GetPrivateProfileIntA("save", "used_active", 0, gSaveFileName) != 0;
    MaskToPerkArray(GetPrivateProfileIntA("save", "perks", 0, gSaveFileName),
                    gCheckpointPerkOwned);
    gCheckpointPerkGrantChapterMask = GetPrivateProfileIntA("save", "perk_chapters", 0, gSaveFileName);
    gCheckpointCrossReviveUsed = GetPrivateProfileIntA("save", "cross_revive_used", 0, gSaveFileName) != 0;
    gCheckpointFreeShopClaimed = GetPrivateProfileIntA("save", "free_shop", 0, gSaveFileName) != 0;
    MaskToBoolArray(GetPrivateProfileIntA("save", "mid_shops", 0, gSaveFileName),
                    gCheckpointShopUsed, kChapterCount);
    MaskToBoolArray(GetPrivateProfileIntA("save", "start_shops", 0, gSaveFileName),
                    gCheckpointStartShopUsed, kChapterCount);
    for (int i = 0; i < kChapterCount; ++i)
    {
        char key[16];
        wsprintfA(key, "best_%d", i);
        gBestStageMeters[i] = GetPrivateProfileIntA("save", key, 0, gSaveFileName);
    }
    ApplyAchievementMask(GetPrivateProfileIntA("achievements", "unlocked", 0, gSaveFileName));
    gGameClearedEver = GetPrivateProfileIntA("achievements", "game_cleared", 0, gSaveFileName) != 0;
}

static void BuildShopOffers()
{
    ShopOffer catalog[] =
    {
        { ActiveDynamite, 5, "DYNAMITE", "JUMP BLAST" },
        { ActiveMiniOrb, 2, "MINI ORB", "DASH ORB" },
        { ActiveBloodChalice, 3, "BLOOD CUP", "HEAL ONE" },
        { ActiveHourglass, 2, "HOURGLASS", "STOP TIME" },
        { ActiveSawBlade, 5, "SAWBLADE", "CUT ANCHOR" }
    };
    int count = (int)(sizeof(catalog) / sizeof(catalog[0]));
    for (int i = count - 1; i > 0; --i)
    {
        int swapIndex = (int)(Random01() * (float)(i + 1));
        if (swapIndex < 0) swapIndex = 0;
        if (swapIndex > i) swapIndex = i;
        ShopOffer temp = catalog[i];
        catalog[i] = catalog[swapIndex];
        catalog[swapIndex] = temp;
    }
    for (int i = 0; i < 3; ++i)
        gShopOffers[i] = catalog[i];
}

static bool RectOverlapsRect(const RectF& a, const RectF& b)
{
    return a.x + a.w > b.x &&
           a.x < b.x + b.w &&
           a.y + a.h > b.y &&
           a.y < b.y + b.h;
}

static RectF SuperOrbBounds(Vec2 position, float padding)
{
    float radius = kSuperOrbHitRadius + padding;
    return { position.x - radius, position.y - radius, radius * 2.0f, radius * 2.0f };
}

static bool SuperOrbSpawnBlocked(Vec2 position)
{
    RectF bounds = SuperOrbBounds(position, 7.0f);
    for (int platform = 0; platform < kMaxPlatforms; ++platform)
        if (gPlatforms[platform].active &&
            RectOverlapsRect(bounds, gPlatforms[platform].bounds))
            return true;
    for (int enemy = 0; enemy < kMaxEnemies; ++enemy)
        if (gEnemies[enemy].active && gEnemies[enemy].alive &&
            RectOverlapsRect(bounds, gEnemies[enemy].bounds))
            return true;
    for (int orb = 0; orb < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++orb)
        if (gSuperOrbs[orb].active &&
            RectOverlapsRect(bounds, SuperOrbBounds(gSuperOrbs[orb].position, 4.0f)))
            return true;
    return false;
}

static bool EnemySpawnBlocked(const RectF& enemyBounds)
{
    RectF padded =
    {
        enemyBounds.x - 10.0f,
        enemyBounds.y - 14.0f,
        enemyBounds.w + 20.0f,
        enemyBounds.h + 28.0f
    };
    for (int platform = 0; platform < kMaxPlatforms; ++platform)
        if (gPlatforms[platform].active &&
            RectOverlapsRect(padded, gPlatforms[platform].bounds))
            return true;
    for (int enemy = 0; enemy < kMaxEnemies; ++enemy)
        if (gEnemies[enemy].active && gEnemies[enemy].alive &&
            RectOverlapsRect(padded, gEnemies[enemy].bounds))
            return true;
    for (int orb = 0; orb < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++orb)
        if (gSuperOrbs[orb].active &&
            RectOverlapsRect(padded, SuperOrbBounds(gSuperOrbs[orb].position, 6.0f)))
            return true;
    return false;
}

static Platform* AddPlatform(RectF bounds)
{
    for (int i = 0; i < kMaxPlatforms; ++i)
    {
        if (gPlatforms[i].active) continue;
        gPlatforms[i].bounds = bounds;
        gPlatforms[i].category = PlatformSolid;
        gPlatforms[i].shape = (PlatformShape)((int)(Random01() * 5.0f) % 5);
        gPlatforms[i].goreSeed = (int)(Random01() * 100000.0f);
        gPlatforms[i].active = true;
        return &gPlatforms[i];
    }
    return 0;
}

static float SpawnEnemyAtRow(float y, float preferredCenterX, bool fullyRandom)
{
    for (int i = 0; i < kMaxEnemies; ++i)
    {
        if (gEnemies[i].active) continue;
        float rowLeft, rowRight;
        PlayfieldBoundsForScreenY(y, &rowLeft, &rowRight);
        float left = rowLeft + 14.0f;
        float right = rowRight - 14.0f;
        float centerX = Clamp(preferredCenterX, left, right);
        RectF bounds = {};
        bool foundClearSpot = false;
        for (int attempt = 0; attempt < 8; ++attempt)
        {
            centerX = fullyRandom
                ? left + Random01() * (right - left)
                : preferredCenterX + (Random01() * 2.0f - 1.0f) * 48.0f;
            centerX = Clamp(centerX, left, right);
            bounds =
            {
                centerX - 4.5f,
                y - 5.5f,
                9.0f,
                11.0f
            };
            if (!EnemySpawnBlocked(bounds))
            {
                foundClearSpot = true;
                break;
            }
        }
        if (!foundClearSpot)
        {
            centerX = Clamp(preferredCenterX, left, right);
            bounds = { centerX - 4.5f, y - 21.5f, 9.0f, 11.0f };
            if (EnemySpawnBlocked(bounds))
                return preferredCenterX;
        }
        gEnemies[i].bounds =
            bounds;
        gEnemies[i].active = true;
        gEnemies[i].alive = true;
        gEnemies[i].respawnTimer = 0.0f;
        gEnemies[i].shotTimer = 0.35f + Random01() * 0.95f;
        gEnemies[i].floatSeed = Random01() * 100.0f;
        float heartChance = 0.12f;
        if (gChapter == 1) heartChance = 0.08f;
        else if (gChapter >= 2) heartChance = 0.06f;
        gEnemies[i].type =
            CurrentChapterConfig().allowHearts && Random01() < heartChance
            ? EnemyHeartFlying
            : EnemyFlying;
        gEnemies[i].velocityX = 0.0f;
        gEnemies[i].patrolMinX = centerX - 4.5f;
        gEnemies[i].patrolMaxX = centerX - 4.5f;
        return centerX;
    }
    return preferredCenterX;
}

static void SpawnWalker(const RectF& platform)
{
    if (platform.w < 34.0f) return;

    for (int i = 0; i < kMaxEnemies; ++i)
    {
        if (gEnemies[i].active) continue;
        float x = platform.x + (platform.w - 9.0f) * Random01();
        gEnemies[i].bounds = { x, platform.y - 11.0f, 9.0f, 11.0f };
        gEnemies[i].active = true;
        gEnemies[i].alive = true;
        gEnemies[i].respawnTimer = 0.0f;
        gEnemies[i].shotTimer = 99.0f;
        gEnemies[i].floatSeed = Random01() * 100.0f;
        gEnemies[i].type = EnemyWalker;
        gEnemies[i].velocityX = Random01() < 0.5f ? -28.0f : 28.0f;
        gEnemies[i].patrolMinX = platform.x;
        gEnemies[i].patrolMaxX = platform.x + platform.w - 9.0f;
        return;
    }
}

static void SpawnWormRow(float y, float centerX)
{
    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
    {
        Worm& worm = gWorms[i];
        if (worm.active) continue;
        worm.active = true;
        worm.pointCount = 8 + (int)(Random01() * 4.0f);
        worm.speed = 30.0f + Random01() * 10.0f;
        for (int point = 0; point < worm.pointCount; ++point)
            worm.points[point] = { centerX - (float)point * 6.0f, y };
        return;
    }
}

static void SpawnTrapRow(float y, float preferredX)
{
    float leftWall, rightWall;
    PlayfieldBoundsForScreenY(y, &leftWall, &rightWall);
    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
    {
        WallTrap& trap = gTraps[i];
        if (trap.active) continue;
        trap.active = true;
        trap.firesRight = preferredX < (leftWall + rightWall) * 0.5f;
        trap.position = { trap.firesRight ? leftWall + 3.0f : rightWall - 3.0f, y };
        trap.shotTimer = 0.02f + Random01() * 0.06f;
        FireTrapProjectileAt(trap, 24.0f + Random01() * 20.0f);
        FireTrapProjectileAt(trap, 86.0f + Random01() * 28.0f);
        return;
    }
}

static void SpawnSuperOrb(float y, float centerX)
{
    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
    {
        SuperOrb& orb = gSuperOrbs[i];
        if (orb.active) continue;
        float rowLeft, rowRight;
        PlayfieldBoundsForScreenY(y, &rowLeft, &rowRight);
        float left = rowLeft + kSuperOrbHitRadius + 8.0f;
        float right = rowRight - kSuperOrbHitRadius - 8.0f;
        if (right <= left) return;
        for (int attempt = 0; attempt < 12; ++attempt)
        {
            Vec2 candidate =
            {
                attempt == 0
                    ? Clamp(centerX, left, right)
                    : left + Random01() * (right - left),
                y + (attempt == 0 ? 0.0f : (Random01() * 2.0f - 1.0f) * 18.0f)
            };
            if (SuperOrbSpawnBlocked(candidate)) continue;
            orb.active = true;
            orb.position = candidate;
            return;
        }
        return;
    }
}

static void SpawnTraversalRow(float y)
{
    bool spawnedEnemy = false;
    const ChapterConfig& config = CurrentChapterConfig();
    int objectCount = config.rowObjects;
    if (config.canSpawnSingleObject && Random01() < 0.36f)
        objectCount = 1;

    float rowLeft, rowRight;
    PlayfieldBoundsForScreenY(y, &rowLeft, &rowRight);
    float playLeft = rowLeft + 12.0f;
    float playRight = rowRight - 12.0f;
    RectF divider = {};
    bool branchRow = BranchDividerForScreenYWithThreshold(y, 0.05f, &divider);
    if (branchRow) objectCount = 2;
    if (playRight - playLeft < 80.0f)
    {
        playLeft = 24.0f;
        playRight = (float)kBufferW - 24.0f;
        branchRow = false;
    }

    for (int objectIndex = 0; objectIndex < objectCount; ++objectIndex)
    {
        float span = playRight - playLeft;
        float bandMin = playLeft + span * (float)objectIndex / (float)objectCount + 5.0f;
        float bandMax = playLeft + span * (float)(objectIndex + 1) / (float)objectCount - 5.0f;
        if (branchRow)
        {
            if (objectIndex == 0)
            {
                bandMin = playLeft;
                bandMax = divider.x - 14.0f;
            }
            else
            {
                bandMin = divider.x + divider.w + 14.0f;
                bandMax = playRight;
            }
            if (bandMax - bandMin < 28.0f)
            {
                bandMin = playLeft + span * (float)objectIndex / 2.0f + 5.0f;
                bandMax = playLeft + span * (float)(objectIndex + 1) / 2.0f - 5.0f;
            }
        }
        else if (objectCount == 1)
        {
            bandMin = playLeft;
            bandMax = playRight;
        }
        float centerX = bandMin + Random01() * (bandMax - bandMin);
        float objectY = y + (Random01() * 2.0f - 1.0f) * 16.0f;
        float roll = Random01();
        float platformChance = config.pipeWalls ? 0.40f : 0.34f;
        float enemyChance = config.pipeWalls ? 0.38f : 0.36f;
        float orbThreshold = 1.0f - config.superOrbChance;
        if (gChapter == 1)
        {
            platformChance = 0.25f;
            enemyChance = 0.46f;
        }
        else if (gChapter == 2)
        {
            platformChance = 0.28f;
            enemyChance = 0.46f;
        }
        else if (gChapter >= 3)
        {
            platformChance = 0.30f;
            enemyChance = 0.47f;
        }
        if (branchRow)
        {
            platformChance = gChapter >= 3 ? 0.38f : 0.44f;
            enemyChance = 1.0f - platformChance;
            orbThreshold = 1.0f;
        }
        if (!config.allowTraps && roll >= platformChance + enemyChance &&
            roll < orbThreshold)
            roll = platformChance + Random01() * (enemyChance - 0.01f);

        if (roll < platformChance)
        {
            float width = 26.0f + Random01() * 42.0f;
            if (config.pipeWalls && Random01() < 0.42f)
                width = 18.0f + Random01() * 28.0f;
            float x = Clamp(centerX - width * 0.5f, playLeft - 7.0f,
                            playRight + 7.0f - width);
            RectF platform = { x, objectY, width, 7.0f + Random01() * 5.0f };
            float walkerChance = 0.48f + (float)gChapter * 0.08f;
            if (walkerChance > 0.72f) walkerChance = 0.72f;
            if (AddPlatform(platform) && Random01() < walkerChance)
                SpawnWalker(platform);
        }
        else if (roll < platformChance + enemyChance)
        {
            SpawnEnemyAtRow(objectY, centerX, true);
            spawnedEnemy = true;
        }
        else if (roll < orbThreshold)
        {
            if (config.allowTraps)
                SpawnTrapRow(objectY, centerX);
            else
            {
                SpawnEnemyAtRow(objectY, centerX, true);
                spawnedEnemy = true;
            }
        }
        else
        {
            SpawnSuperOrb(objectY, centerX);
        }

        gLastPlatformCenterX = centerX;
    }

    gRowsSinceEnemy = spawnedEnemy ? 0 : gRowsSinceEnemy + 1;
}

static float HighestTraversalY()
{
    float highest = (float)kBufferH;
    for (int i = 0; i < kMaxPlatforms; ++i)
        if (gPlatforms[i].active && gPlatforms[i].bounds.y < highest)
            highest = gPlatforms[i].bounds.y;
    for (int i = 0; i < kMaxEnemies; ++i)
        if (gEnemies[i].active && gEnemies[i].bounds.y < highest)
            highest = gEnemies[i].bounds.y;
    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        if (gTraps[i].active && gTraps[i].position.y < highest)
            highest = gTraps[i].position.y;
    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
        if (gSuperOrbs[i].active && gSuperOrbs[i].position.y < highest)
            highest = gSuperOrbs[i].position.y;
    return highest;
}

static void ReclaimOffscreenTraversal()
{
    const float reclaimY = (float)kBufferH + 420.0f;
    for (int i = 0; i < kMaxPlatforms; ++i)
        if (gPlatforms[i].active && gPlatforms[i].bounds.y > reclaimY)
            gPlatforms[i].active = false;

    for (int i = 0; i < kMaxEnemies; ++i)
        if (gEnemies[i].active && gEnemies[i].bounds.y > reclaimY)
            gEnemies[i].active = false;

    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        if (gTraps[i].active && gTraps[i].position.y > reclaimY)
            gTraps[i].active = false;

    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
        if (gSuperOrbs[i].active && gSuperOrbs[i].position.y > reclaimY)
            gSuperOrbs[i].active = false;

    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
        if (gSawBlades[i].active && gSawBlades[i].position.y > reclaimY)
            gSawBlades[i].active = false;
}

static void ClearHazardsForChapterTransition()
{
    for (int i = 0; i < kMaxEnemies; ++i)
    {
        Enemy& enemy = gEnemies[i];
        if (!enemy.active) continue;
        if (enemy.alive)
        {
            Vec2 center =
            {
                enemy.bounds.x + enemy.bounds.w * 0.5f,
                enemy.bounds.y + enemy.bounds.h * 0.5f
            };
            if (center.y > -32.0f && center.y < (float)kBufferH + 32.0f)
                SpawnBurst(center, Rgb(214, 18, 42), 10);
        }
        enemy.active = false;
        enemy.alive = false;
    }

    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
    {
        Worm& worm = gWorms[i];
        if (!worm.active) continue;
        for (int point = 0; point < worm.pointCount; point += 3)
        {
            if (worm.points[point].y > -28.0f &&
                worm.points[point].y < (float)kBufferH + 28.0f)
                SpawnBurst(worm.points[point], Rgb(184, 16, 48), 5);
        }
        worm.active = false;
    }

    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        gTraps[i].active = false;
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        if (gProjectiles[i].active)
            SpawnBurst(gProjectiles[i].position, Rgb(245, 102, 48), 3);
        gProjectiles[i].active = false;
    }
}

static void BeginEnding()
{
    if (gEndingActive) return;
    if ((int)kChapterLengthMeters > gBestStageMeters[gChapter])
        gBestStageMeters[gChapter] = (int)kChapterLengthMeters;
    gGameClearedEver = true;
    UnlockAchievement(AchievementEscape);
    if (!gRunUsedActiveItem)
        UnlockAchievement(AchievementHookVeteran);
    SavePersistentData();
    ClearHazardsForChapterTransition();
    gEndingActive = true;
    gEndingTimer = 0.0f;
    gChapterMonsterAwake = false;
    gScrollSpeed = 0.0f;
    gDevourerFastForward = false;
    gDevourerPauseTimer = 0.0f;
    gTimeStopTimer = 0.0f;
    gHitStopTimer = 0.0f;
    gHook.mode = HookIdle;
    gPlayer.velocity = { 0.0f, 0.0f };
    gPlayer.grounded = true;
    gPlayer.jumpAvailable = true;
    gCameraShakeTimer = 0.34f;
    SpawnBurst(gPlayer.position, Rgb(228, 66, 82), 28);
    SpawnBurst({ gPlayer.position.x, gPlayer.position.y - 10.0f },
               Rgb(255, 226, 174), 18);
    PlaySfx(SfxStageClear);
}

static void BeginChapterTransition()
{
    if (gChapter >= kChapterCount - 1 || gChapterTransitionActive) return;
    if (gChapter == 0) UnlockAchievement(AchievementStage1);
    else if (gChapter == 1) UnlockAchievement(AchievementStage2);
    else if (gChapter == 2) UnlockAchievement(AchievementStage3);
    gPendingChapter = gChapter + 1;
    gQueuedPerkRoulette = GrantRandomPerkForChapter(gPendingChapter);
    gCheckpointChapter = gPendingChapter;
    gCheckpointValid = true;
    gCheckpointInfiniteMode = gInfiniteMode;
    gCheckpointCoins = gCoins;
    gCheckpointActiveItem = gActiveItem;
    gCheckpointActiveItemUses = gActiveItemUses;
    gCheckpointRunUsedActiveItem = gRunUsedActiveItem;
    CopyPerks(gCheckpointPerkOwned, gPerkOwned);
    gCheckpointPerkGrantChapterMask = gPerkGrantChapterMask;
    gCheckpointCrossReviveUsed = gCrossReviveUsed;
    gCheckpointFreeShopClaimed = gFreeShopClaimed;
    if ((int)kChapterLengthMeters > gBestStageMeters[gChapter])
        gBestStageMeters[gChapter] = (int)kChapterLengthMeters;
    for (int i = 0; i < kChapterCount; ++i)
    {
        gCheckpointShopUsed[i] = gShopUsed[i];
        gCheckpointStartShopUsed[i] = gStartShopUsed[i];
    }
    SavePersistentData();
    ClearHazardsForChapterTransition();
    gChapterTransitionActive = true;
    gChapterTransitionWaitingConfirm = false;
    gChapterTransitionTimer = kChapterTransitionSeconds;
    gPreserveRunStateOnRestart = true;
    gChapterMonsterAwake = false;
    gScrollSpeed = 0.0f;
    gDevourerFastForward = false;
    gDevourerPauseTimer = 0.0f;
    gTimeStopTimer = 0.0f;
    gHook.mode = HookIdle;
    gPlayer.velocity = { 0.0f, 0.0f };
    gChapterTransitionStart = gPlayer.position;
    gChapterTransitionTarget =
    {
        Clamp(gPlayer.position.x,
              PlayfieldLeft() + gPlayer.halfW + 8.0f,
              PlayfieldRight() - gPlayer.halfW - 8.0f),
        Clamp(gPlayer.position.y, 86.0f, 126.0f)
    };
    gPlayer.grounded = true;
    gPlayer.jumpAvailable = true;
    gCameraShakeTimer = 0.34f;
    SpawnBurst({ gPlayer.position.x, gPlayer.position.y + 7.0f },
               Rgb(190, 18, 42), 34);
    PlaySfx(SfxStageClear);
    PlaySfx(SfxDynamite);
}

static void RestartGame()
{
    bool restartHeld = gInput.keys['R'];
    bool preserveRunState = gPreserveRunStateOnRestart;
    bool preservedInfiniteMode = preserveRunState ? gInfiniteMode : gCheckpointInfiniteMode;
    int preservedCoins = preserveRunState ? gCoins : gCheckpointCoins;
    ActiveItemType preservedItem = preserveRunState ? gActiveItem : gCheckpointActiveItem;
    int preservedUses = preserveRunState ? gActiveItemUses : gCheckpointActiveItemUses;
    bool preservedRunUsedActiveItem = preserveRunState ? gRunUsedActiveItem : gCheckpointRunUsedActiveItem;
    bool preservedPerks[PerkCount];
    CopyPerks(preservedPerks, preserveRunState ? gPerkOwned : gCheckpointPerkOwned);
    int preservedPerkGrantChapterMask = preserveRunState ? gPerkGrantChapterMask : gCheckpointPerkGrantChapterMask;
    bool preservedCrossReviveUsed = preserveRunState ? gCrossReviveUsed : gCheckpointCrossReviveUsed;
    bool preservedFreeShopClaimed = preserveRunState ? gFreeShopClaimed : gCheckpointFreeShopClaimed;
    bool preservedShopUsed[kChapterCount];
    bool preservedStartShopUsed[kChapterCount];
    for (int i = 0; i < kChapterCount; ++i)
    {
        preservedShopUsed[i] = preserveRunState ? gShopUsed[i] : gCheckpointShopUsed[i];
        preservedStartShopUsed[i] = preserveRunState ? gStartShopUsed[i] : gCheckpointStartShopUsed[i];
    }
    gPreserveRunStateOnRestart = false;

    int targetChapter = gCheckpointValid ? gCheckpointChapter : 0;
    gChapter = targetChapter;
    gInfiniteMode = gCheckpointValid || preserveRunState ? preservedInfiniteMode : false;
    gPlayer = kPlayerStart;
    gPlayerFacingRight = true;
    ResetCloak();
    gHook = {};
    gHookCooldownTimer = 0.0f;
    gSlashTrail = {};
    gRandomState = 0xC0FFEEu + (uint32_t)targetChapter * 0x9E3779B9u;
    gMapTime = 0.0f;
    gScrollSpeed = 0.0f;
    gHitStopTimer = 0.0f;
    gCameraShakeTimer = 0.0f;
    gRenderOffsetX = 0;
    gRenderOffsetY = 0;
    gPlayerDead = false;
    gPlayerDeathTimer = 0.0f;
    gCrossReviveEffectTimer = 0.0f;
    gInvulnerableTimer = 0.0f;
    gSuperJumpTimer = 0.0f;
    gTimeStopTimer = 0.0f;
    gAirJumpsUsed = 0;
    gParachuteHoldTimer = 0.0f;
    gWormSpawnTimer = 3.5f;
    gDevourerFastForward = false;
    gDevourerPauseTimer = 0.0f;
    gPlayerHealth = kMaxHealth;
    gCombo = 0;
    gComboTimer = 0.0f;
    gComboPulse = 0.0f;
    gHeightPixels = ChapterStartMeters(gChapter) * kPixelsPerMeter;
    gViewHeightPixels = gHeightPixels;
    gNextHeightMilestone = NextHeightMilestoneAfter(gHeightPixels);
    gReachedMeters = 0;
    gReachedBannerTimer = 0.0f;
    gLastPlatformCenterX = kBufferW * 0.5f;
    gRowsSinceEnemy = 0;
    gDevourerY = 250.0f;
    gCoins = gCheckpointValid || preserveRunState ? preservedCoins : 0;
    gActiveItem = gCheckpointValid || preserveRunState ? preservedItem : ActiveNone;
    gActiveItemUses = gCheckpointValid || preserveRunState ? preservedUses : 0;
    gRunUsedActiveItem = gCheckpointValid || preserveRunState ? preservedRunUsedActiveItem : false;
    if (gCheckpointValid || preserveRunState)
    {
        CopyPerks(gPerkOwned, preservedPerks);
        gPerkGrantChapterMask = preservedPerkGrantChapterMask;
        gCrossReviveUsed = preservedCrossReviveUsed;
    }
    else
    {
        for (int i = 0; i < PerkCount; ++i)
            gPerkOwned[i] = false;
        gPerkGrantChapterMask = 0;
        gCrossReviveUsed = false;
    }
    gFreeShopClaimed = gCheckpointValid || preserveRunState ? preservedFreeShopClaimed : false;
    for (int i = 0; i < kChapterCount; ++i)
    {
        gShopUsed[i] = gCheckpointValid || preserveRunState ? preservedShopUsed[i] : false;
        gStartShopUsed[i] = gCheckpointValid || preserveRunState ? preservedStartShopUsed[i] : false;
    }
    gShopOpen = false;
    gShopCurrentFree = false;
    gShopCurrentStart = false;
    gHookShopFree = false;
    gHookShopStart = false;
    gStartPortalTouchArmed = false;
    gShopHover = 0;
    gDealAcceptedTimer = 0.0f;
    gChapterMonsterAwake = false;
    gChapterChaseTimer = 0.0f;
    gChapterFirstRowMeters = ChapterStartMeters(gChapter) + 14.0f;
    gChapterBannerTimer = 3.0f;
    gChapterTransitionActive = false;
    gChapterTransitionWaitingConfirm = false;
    gEndingActive = false;
    gEndingTimer = 0.0f;
    gChapterTransitionTimer = 0.0f;
    gPendingChapter = gChapter;
    if (preserveRunState && gQueuedPerkRoulette != PerkNone)
    {
        gPerkRouletteOpen = true;
        gPerkRouletteTimer = kPerkRouletteSeconds;
        gPerkRouletteWaitingConfirm = false;
        gPerkRouletteFinal = gQueuedPerkRoulette;
        gQueuedPerkRoulette = PerkNone;
        PlaySfx(SfxPerkRoulette);
        ClearPressedInput();
    }
    else
    {
        gPerkRouletteOpen = false;
        gPerkRouletteTimer = 0.0f;
        gPerkRouletteWaitingConfirm = false;
        gPerkRouletteFinal = PerkNone;
        if (!preserveRunState)
            gQueuedPerkRoulette = PerkNone;
    }
    ApplyChapterWalls(gViewHeightPixels / kPixelsPerMeter);

    for (int i = 0; i < kMaxPlatforms; ++i) gPlatforms[i] = {};
    for (int i = 0; i < kMaxEnemies; ++i) gEnemies[i] = {};
    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
        gWorms[i] = {};
    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        gTraps[i] = {};
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
        gProjectiles[i] = {};
    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
        gSuperOrbs[i] = {};
    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
        gSawBlades[i] = {};
    for (int i = 0; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
        gParticles[i] = {};
    for (int i = 0; i < (int)(sizeof(gAfterImages) / sizeof(gAfterImages[0])); ++i)
        gAfterImages[i] = {};

    float startLeft = PlayfieldLeft();
    float startRight = PlayfieldRight();
    Platform* startPlatform = AddPlatform(
        { startLeft, 160.0f, startRight - startLeft, 20.0f });
    if (startPlatform) startPlatform->shape = PlatformSlab;
    for (float y = 78.0f; y >= -100.0f; y -= 86.0f)
        SpawnTraversalRow(y);

    for (int i = 0; i < 256; ++i)
    {
        gInput.keys[i] = false;
        gInput.pressed[i] = false;
    }
    gInput.keys['R'] = restartHeld;
    gInput.mouseLeft = false;
    gInput.mousePressed = false;
}

static void UpdateCameraAndDevourer(float dt)
{
    if (gShopOpen) return;
    if (gPlayerDead) return;
    if (gEndingActive) return;
    gMapTime += dt;
    bool timeStopped = gTimeStopTimer > 0.0f;

    if (gChapterTransitionActive)
    {
        if (!gChapterTransitionWaitingConfirm)
        {
            gChapterTransitionTimer -= dt;
            if (gChapterTransitionTimer <= 0.0f)
            {
                gChapterTransitionTimer = 0.0f;
                gChapterTransitionWaitingConfirm = true;
            }
        }
        if (gChapterTransitionWaitingConfirm &&
            (gInput.pressed[VK_RETURN] ||
             MenuClicked(112.0f, 132.0f, 96.0f, 20.0f)))
        {
            gInput.pressed[VK_RETURN] = false;
            gInput.mousePressed = false;
            gChapter = gPendingChapter;
            RestartGame();
        }
        return;
    }

    float heightMetersF = gViewHeightPixels / kPixelsPerMeter;
    if (!gChapterMonsterAwake && heightMetersF >= gChapterFirstRowMeters)
    {
        gChapterMonsterAwake = true;
        gChapterChaseTimer = 0.0f;
    }

    const ChapterConfig& config = CurrentChapterConfig();
    if (!gChapterMonsterAwake)
        gScrollSpeed = 0.0f;
    else
    {
        if (!timeStopped)
            gChapterChaseTimer += dt;
        float accelerationTime = gChapterChaseTimer - config.monsterAccelDelay;
        gScrollSpeed = accelerationTime > 0.0f
            ? config.monsterBaseSpeed + accelerationTime
            : config.monsterBaseSpeed;
        if (gScrollSpeed > config.monsterMaxSpeed)
            gScrollSpeed = config.monsterMaxSpeed;
    }

    float devourerDistance = gDevourerY - gPlayer.position.y;
    if (devourerDistance > 125.0f) gDevourerFastForward = true;
    if (devourerDistance < 82.0f) gDevourerFastForward = false;
    if (gChapterMonsterAwake && gDevourerFastForward &&
        gScrollSpeed < config.monsterCatchupSpeed)
        gScrollSpeed = config.monsterCatchupSpeed;

    float cameraShift = kCameraLockY - gPlayer.position.y;
    gViewHeightPixels += cameraShift;
    float chapterFloorPixels = ChapterStartMeters(gChapter) * kPixelsPerMeter;
    if (gViewHeightPixels < chapterFloorPixels)
        gViewHeightPixels = chapterFloorPixels;
    if (gViewHeightPixels > gHeightPixels)
    {
        gHeightPixels = gViewHeightPixels;
        int heightMeters = (int)(gHeightPixels / kPixelsPerMeter);
        while (heightMeters >= gNextHeightMilestone)
        {
            gReachedMeters = gNextHeightMilestone;
            gReachedBannerTimer = 2.4f;
            PlaySfx(SfxReached);
            gNextHeightMilestone += kHeightMarkerInterval;
        }
    }
    gPlayer.position.y = kCameraLockY;
    ApplyChapterWalls(gViewHeightPixels / kPixelsPerMeter);
    gPlayer.position.x = Clamp(gPlayer.position.x,
                               PlayfieldLeft() + gPlayer.halfW,
                               PlayfieldRight() - gPlayer.halfW);
    ResolveBranchDividerOverlap();

    for (int i = 0; i < kMaxPlatforms; ++i)
    {
        if (!gPlatforms[i].active) continue;
        gPlatforms[i].bounds.y += cameraShift;
    }

    for (int i = 0; i < kMaxEnemies; ++i)
    {
        if (!gEnemies[i].active) continue;
        gEnemies[i].bounds.y += cameraShift;
    }
    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
    {
        Worm& worm = gWorms[i];
        if (!worm.active) continue;
        for (int point = 0; point < worm.pointCount; ++point)
            worm.points[point].y += cameraShift;
    }
    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        if (gTraps[i].active) gTraps[i].position.y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
        if (gProjectiles[i].active) gProjectiles[i].position.y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
        if (gSuperOrbs[i].active) gSuperOrbs[i].position.y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
        if (gSawBlades[i].active) gSawBlades[i].position.y += cameraShift;

    if (gHook.mode != HookIdle)
    {
        gHook.position.y += cameraShift;
        gHook.target.y += cameraShift;
        gHook.dashStart.y += cameraShift;
    }

    if (gSlashTrail.life > 0.0f)
    {
        gSlashTrail.start.y += cameraShift;
        gSlashTrail.end.y += cameraShift;
    }
    if (gCrossFlashTimer > 0.0f)
        gCrossFlashPosition.y += cameraShift;
    if (gCrossReviveEffectTimer > 0.0f)
        gCrossReviveEffectPosition.y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gAfterImages) / sizeof(gAfterImages[0])); ++i)
        if (gAfterImages[i].life > 0.0f)
            gAfterImages[i].position.y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gCloakSegments) / sizeof(gCloakSegments[0])); ++i)
        gCloakSegments[i].y += cameraShift;
    for (int i = 0; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
        if (gParticles[i].life > 0.0f)
            gParticles[i].position.y += cameraShift;

    gDevourerY += cameraShift;
    if (CurrentChapterLocalMeters() >= kChapterLengthMeters &&
        !(gInfiniteMode && gChapter == kChapterCount - 1))
    {
        if (gChapter < kChapterCount - 1)
            BeginChapterTransition();
        else
            BeginEnding();
        return;
    }

    if (!timeStopped && gDevourerPauseTimer > 0.0f)
    {
        gDevourerPauseTimer -= dt;
        if (gDevourerPauseTimer < 0.0f) gDevourerPauseTimer = 0.0f;
    }
    else if (!timeStopped)
    {
        gDevourerY -= gScrollSpeed * dt;
    }

    for (int i = 0; i < kMaxPlatforms; ++i)
        if (gPlatforms[i].active && gPlatforms[i].bounds.y >= gDevourerY)
            gPlatforms[i].active = false;
    for (int i = 0; i < kMaxEnemies; ++i)
        if (gEnemies[i].active && gEnemies[i].bounds.y >= gDevourerY)
        {
            gEnemies[i].active = false;
            gEnemies[i].alive = false;
        }
    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
        if (gWorms[i].active && gWorms[i].points[0].y >= gDevourerY)
            gWorms[i].active = false;
    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
        if (gTraps[i].active && gTraps[i].position.y >= gDevourerY)
            gTraps[i].active = false;
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
        if (gProjectiles[i].active && gProjectiles[i].position.y >= gDevourerY)
            gProjectiles[i].active = false;
    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
        if (gSuperOrbs[i].active && gSuperOrbs[i].position.y >= gDevourerY)
            gSuperOrbs[i].active = false;
    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
        if (gSawBlades[i].active && gSawBlades[i].position.y >= gDevourerY)
            gSawBlades[i].active = false;

    if (!gPlayerDead && gPlayer.position.y + gPlayer.halfH >= gDevourerY)
    {
        KillPlayer();
        return;
    }

    ReclaimOffscreenTraversal();
    float highest = HighestTraversalY();
    while (highest > -55.0f)
    {
        float baseSpacing = 72.0f;
        float randomSpacing = 32.0f;
        if (gChapter == 1)
        {
            baseSpacing = 80.0f;
            randomSpacing = 36.0f;
        }
        else if (gChapter == 2)
        {
            baseSpacing = 84.0f;
            randomSpacing = 40.0f;
        }
        else if (gChapter >= 3)
        {
            baseSpacing = 86.0f;
            randomSpacing = 42.0f;
        }
        highest -= baseSpacing + Random01() * randomSpacing;
        SpawnTraversalRow(highest);
    }
}

static float MoveTowards(float current, float target, float maxDelta)
{
    if (current < target)
    {
        current += maxDelta;
        return current > target ? target : current;
    }
    if (current > target)
    {
        current -= maxDelta;
        return current < target ? target : current;
    }
    return target;
}

static int AbsInt(int v)
{
    return v < 0 ? -v : v;
}

static float SqrtApprox(float v)
{
    if (v <= 0.0f) return 0.0f;

    float x = 1.0f;
    while (x * x < v)
        x *= 2.0f;

    for (int i = 0; i < 6; ++i)
        x = 0.5f * (x + v / x);
    return x;
}

static Vec2 Sub(Vec2 a, Vec2 b)
{
    return { a.x - b.x, a.y - b.y };
}

static Vec2 Normalize(Vec2 v)
{
    float len = SqrtApprox(v.x * v.x + v.y * v.y);
    if (len <= 0.0001f) return { 1.0f, 0.0f };
    return { v.x / len, v.y / len };
}

static float Length(Vec2 v)
{
    return SqrtApprox(v.x * v.x + v.y * v.y);
}

static float FloatBob(float seed)
{
    int phase = ((int)(gMapTime * 34.0f + seed * 19.0f)) % 48;
    float t = phase < 24 ? (float)phase / 24.0f : (float)(48 - phase) / 24.0f;
    return (t * 2.0f - 1.0f) * 1.35f;
}

static bool PointInRect(Vec2 point, const RectF& rect)
{
    return point.x >= rect.x && point.x <= rect.x + rect.w &&
           point.y >= rect.y && point.y <= rect.y + rect.h;
}

static bool Overlaps(const Body& body, const RectF& rect)
{
    return body.position.x + body.halfW > rect.x &&
           body.position.x - body.halfW < rect.x + rect.w &&
           body.position.y + body.halfH > rect.y &&
           body.position.y - body.halfH < rect.y + rect.h;
}

static bool MovePlayerAgainstBranchDivider(float amountX, float amountY)
{
    bool collided = false;
    for (int y = -8; y < kBufferH + 8; y += 8)
    {
        RectF divider;
        if (!BranchDividerForScreenY((float)y + 4.0f, &divider)) continue;
        divider.y = (float)y;
        divider.h = 8.0f;
        if (!Overlaps(gPlayer, divider)) continue;

        collided = true;
        if (amountX > 0.0f)
            gPlayer.position.x = divider.x - gPlayer.halfW;
        else if (amountX < 0.0f)
            gPlayer.position.x = divider.x + divider.w + gPlayer.halfW;
        else
        {
            float dividerCenter = divider.x + divider.w * 0.5f;
            if (gPlayer.position.x < dividerCenter)
                gPlayer.position.x = divider.x - gPlayer.halfW;
            else
                gPlayer.position.x = divider.x + divider.w + gPlayer.halfW;
        }
    }
    if (collided)
    {
        if (amountX != 0.0f) gPlayer.velocity.x = 0.0f;
        if (amountY != 0.0f) gPlayer.velocity.x = 0.0f;
    }
    return collided;
}

static void ResolveBranchDividerOverlap()
{
    RectF divider;
    if (!BranchDividerForScreenY(gPlayer.position.y, &divider)) return;
    divider.y = 0.0f;
    divider.h = (float)kBufferH;
    if (!Overlaps(gPlayer, divider)) return;
    float dividerCenter = divider.x + divider.w * 0.5f;
    if (gPlayer.position.x < dividerCenter)
        gPlayer.position.x = divider.x - gPlayer.halfW;
    else
        gPlayer.position.x = divider.x + divider.w + gPlayer.halfW;
    gPlayer.velocity.x = 0.0f;
}

static bool SegmentHitsBranchDivider(Vec2 start, Vec2 end)
{
    Vec2 delta = Sub(end, start);
    int steps = (int)(Length(delta) * 0.35f) + 2;
    if (steps < 8) steps = 8;
    for (int i = 0; i <= steps; ++i)
    {
        float t = (float)i / (float)steps;
        Vec2 point =
        {
            start.x + delta.x * t,
            start.y + delta.y * t
        };
        RectF divider;
        if (BranchDividerForScreenY(point.y, &divider) &&
            PointInRect(point, divider))
            return true;
    }
    return false;
}

static bool MovePlayerX(float amount)
{
    bool collided = false;
    gPlayer.position.x += amount;

    for (int i = 0; i < (int)(sizeof(gPlatforms) / sizeof(gPlatforms[0])); ++i)
    {
        if (!gPlatforms[i].active) continue;
        const RectF& rect = gPlatforms[i].bounds;
        if (!Overlaps(gPlayer, rect)) continue;

        collided = true;

        if (amount > 0.0f)
            gPlayer.position.x = rect.x - gPlayer.halfW;
        else if (amount < 0.0f)
            gPlayer.position.x = rect.x + rect.w + gPlayer.halfW;
        gPlayer.velocity.x = 0.0f;
    }

    for (int i = 0; i < (int)(sizeof(gSideWalls) / sizeof(gSideWalls[0])); ++i)
    {
        const RectF& wall = gSideWalls[i];
        if (!Overlaps(gPlayer, wall)) continue;
        collided = true;
        if (amount > 0.0f)
            gPlayer.position.x = wall.x - gPlayer.halfW;
        else if (amount < 0.0f)
            gPlayer.position.x = wall.x + wall.w + gPlayer.halfW;
        gPlayer.velocity.x = 0.0f;
    }
    if (MovePlayerAgainstBranchDivider(amount, 0.0f))
        collided = true;
    return collided;
}

static bool MovePlayerY(float amount)
{
    bool collided = false;
    gPlayer.grounded = false;
    gPlayer.position.y += amount;

    for (int i = 0; i < (int)(sizeof(gPlatforms) / sizeof(gPlatforms[0])); ++i)
    {
        if (!gPlatforms[i].active) continue;
        const RectF& rect = gPlatforms[i].bounds;
        if (!Overlaps(gPlayer, rect)) continue;

        collided = true;

        if (amount > 0.0f)
        {
            gPlayer.position.y = rect.y - gPlayer.halfH;
            gPlayer.grounded = true;
            gPlayer.jumpAvailable = true;
            gAirJumpsUsed = 0;
            gParachuteHoldTimer = 0.0f;
        }
        else if (amount < 0.0f)
        {
            gPlayer.position.y = rect.y + rect.h + gPlayer.halfH;
        }
        gPlayer.velocity.y = 0.0f;
    }
    if (MovePlayerAgainstBranchDivider(0.0f, amount))
        collided = true;
    return collided;
}

static void TriggerHookMissCooldown()
{
    gHookCooldownTimer = kHookMissCooldownSeconds;
}

static bool HookOnCooldown()
{
    return gHookCooldownTimer > 0.0f;
}

static void StartHook()
{
    if (HookOnCooldown()) return;
    Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
    gPlayerFacingRight = mouse.x >= gPlayer.position.x;
    Vec2 hookAnchor =
    {
        gPlayer.position.x,
        gPlayer.position.y - 7.0f
    };
    gHook.mode = HookFlying;
    gHook.direction = Normalize(Sub(mouse, hookAnchor));
    gHook.position =
    {
        hookAnchor.x + gHook.direction.x * 7.0f,
        hookAnchor.y + gHook.direction.y * 7.0f
    };
    gHook.distance = 0.0f;
    gHook.vault = false;
    gHook.dashing = false;
    PlaySfx(SfxHookFire);
}

static void AddAfterImage(Vec2 position)
{
    int slot = 0;
    for (int i = 1; i < (int)(sizeof(gAfterImages) / sizeof(gAfterImages[0])); ++i)
        if (gAfterImages[i].life < gAfterImages[slot].life) slot = i;

    gAfterImages[slot].position = position;
    gAfterImages[slot].life = 0.16f;
}

static void ResetCloak()
{
    Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
    Vec2 center = { gPlayer.position.x, gPlayer.position.y - 7.0f };
    Vec2 forward = Normalize(Sub(mouse, center));
    if (gSuperJumpTimer > 0.0f)
        forward = { 0.0f, -1.0f };
    for (int i = 0; i < (int)(sizeof(gCloakSegments) / sizeof(gCloakSegments[0])); ++i)
        gCloakSegments[i] =
        {
            center.x - forward.x * (7.0f + (float)i * 2.7f),
            center.y - forward.y * (7.0f + (float)i * 2.7f)
        };
}

static void UpdateCloak(float dt)
{
    Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
    Vec2 center = { gPlayer.position.x, gPlayer.position.y - 7.0f };
    Vec2 forward = Normalize(Sub(mouse, center));
    if (gSuperJumpTimer > 0.0f)
        forward = { 0.0f, -1.0f };
    Vec2 anchor =
    {
        center.x - forward.x * 7.0f,
        center.y - forward.y * 7.0f
    };
    float segmentLength = 2.7f;
    for (int i = 0; i < (int)(sizeof(gCloakSegments) / sizeof(gCloakSegments[0])); ++i)
    {
        Vec2 target = i == 0 ? anchor : gCloakSegments[i - 1];
        Vec2 offset = Sub(gCloakSegments[i], target);
        float length = Length(offset);
        if (length < 0.001f) offset = { -forward.x, -forward.y };
        else offset = { offset.x / length, offset.y / length };

        Vec2 desired =
        {
            target.x + offset.x * segmentLength - forward.x * 0.35f * (float)i,
            target.y + offset.y * segmentLength - forward.y * 0.35f * (float)i
        };
        float follow = Clamp(dt * (24.0f - (float)i * 1.7f), 0.0f, 1.0f);
        gCloakSegments[i].x += (desired.x - gCloakSegments[i].x) * follow;
        gCloakSegments[i].y += (desired.y - gCloakSegments[i].y) * follow;
        gCloakSegments[i].x -= gPlayer.velocity.x * dt * (0.07f + (float)i * 0.02f);
        gCloakSegments[i].y -= gPlayer.velocity.y * dt * (0.035f + (float)i * 0.012f);
    }
}

static void UpdateEffects(float dt)
{
    UpdateCloak(dt);

    for (int i = 0; i < (int)(sizeof(gAfterImages) / sizeof(gAfterImages[0])); ++i)
    {
        gAfterImages[i].life -= dt;
        if (gAfterImages[i].life < 0.0f) gAfterImages[i].life = 0.0f;
    }

    gSlashTrail.life -= dt;
    if (gSlashTrail.life < 0.0f) gSlashTrail.life = 0.0f;
    gCrossFlashTimer -= dt;
    if (gCrossFlashTimer < 0.0f) gCrossFlashTimer = 0.0f;
    gCrossReviveEffectTimer -= dt;
    if (gCrossReviveEffectTimer < 0.0f) gCrossReviveEffectTimer = 0.0f;

    gCameraShakeTimer -= dt;
    if (gCameraShakeTimer < 0.0f) gCameraShakeTimer = 0.0f;
    gInvulnerableTimer -= dt;
    if (gInvulnerableTimer < 0.0f) gInvulnerableTimer = 0.0f;
    bool timeStopped = gTimeStopTimer > 0.0f;
    gTimeStopTimer -= dt;
    if (gTimeStopTimer < 0.0f) gTimeStopTimer = 0.0f;
    float previousSuperJumpTimer = gSuperJumpTimer;
    gSuperJumpTimer -= dt;
    if (gSuperJumpTimer < 0.0f) gSuperJumpTimer = 0.0f;
    if (previousSuperJumpTimer > 0.0f && gSuperJumpTimer <= 0.0f)
    {
        gPlayer.velocity.y = -60.0f;
        gInvulnerableTimer = kInvulnerableSeconds;
    }

    if (previousSuperJumpTimer <= 0.0f && !timeStopped)
        gComboTimer -= dt;
    if (gComboTimer <= 0.0f)
    {
        gComboTimer = 0.0f;
        gCombo = 0;
    }
    gComboPulse -= dt;
    if (gComboPulse < 0.0f) gComboPulse = 0.0f;
    gReachedBannerTimer -= dt;
    if (gReachedBannerTimer < 0.0f) gReachedBannerTimer = 0.0f;
    gChapterBannerTimer -= dt;
    if (gChapterBannerTimer < 0.0f) gChapterBannerTimer = 0.0f;
    gDealAcceptedTimer -= dt;
    if (gDealAcceptedTimer < 0.0f) gDealAcceptedTimer = 0.0f;

    if (timeStopped)
        return;

    for (int i = 0; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
    {
        Particle& particle = gParticles[i];
        if (particle.life <= 0.0f) continue;
        particle.life -= dt;
        particle.position.x += particle.velocity.x * dt;
        particle.position.y += particle.velocity.y * dt;
        particle.velocity.y += 75.0f * dt;
        if (particle.life < 0.0f) particle.life = 0.0f;
    }

    for (int i = 0; i < kMaxEnemies; ++i)
    {
        Enemy& enemy = gEnemies[i];
        if (!enemy.active || enemy.alive) continue;
        enemy.respawnTimer -= dt;
        if (enemy.respawnTimer <= 0.0f && enemy.bounds.y < gDevourerY)
        {
            enemy.alive = true;
            enemy.respawnTimer = 0.0f;
            enemy.shotTimer = 0.25f + Random01() * 0.85f;
        }
    }
}

static bool SlashHitsRect(Vec2 start, Vec2 end, const RectF& rect)
{
    RectF expanded = { rect.x - 5.0f, rect.y - 5.0f, rect.w + 10.0f, rect.h + 10.0f };
    Vec2 delta = Sub(end, start);
    int steps = (int)(Length(delta) * 0.5f) + 1;
    for (int i = 0; i <= steps; ++i)
    {
        float t = (float)i / (float)steps;
        Vec2 point = { start.x + delta.x * t, start.y + delta.y * t };
        if (PointInRect(point, expanded)) return true;
    }
    return false;
}

static void SpawnBurst(Vec2 center, uint32_t color, int count)
{
    for (int burst = 0; burst < count; ++burst)
    {
        int slot = 0;
        for (int i = 1; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
            if (gParticles[i].life < gParticles[slot].life) slot = i;

        float speed = 45.0f + Random01() * 70.0f;
        Vec2 direction = Normalize(
            { Random01() * 2.0f - 1.0f, Random01() * 2.0f - 1.0f });
        gParticles[slot].position = center;
        gParticles[slot].velocity =
        {
            direction.x * speed,
            direction.y * speed - 18.0f
        };
        gParticles[slot].life = 0.28f + Random01() * 0.20f;
        gParticles[slot].color = color;
    }
}

static void SpawnEnemyBurst(Vec2 center)
{
    SpawnBurst(center, Rgb(235, 18, 30), 14);
}

static void SpawnSuperDashFlame(Vec2 position)
{
    Vec2 center = { position.x, position.y - 7.0f };
    for (int flame = 0; flame < 4; ++flame)
    {
        int slot = 0;
        for (int i = 1; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
            if (gParticles[i].life < gParticles[slot].life) slot = i;

        Vec2 direction = Normalize(
            { Random01() * 2.0f - 1.0f, Random01() * 2.0f - 1.0f });
        float radius = 4.0f + Random01() * 8.0f;
        gParticles[slot].position =
        {
            center.x + direction.x * radius,
            center.y + direction.y * radius
        };
        gParticles[slot].velocity =
        {
            direction.x * (24.0f + Random01() * 34.0f),
            direction.y * 26.0f - 42.0f - Random01() * 58.0f
        };
        gParticles[slot].life = 0.16f + Random01() * 0.22f;
        gParticles[slot].color = flame == 0
            ? Rgb(248, 176, 255)
            : (flame == 1 ? Rgb(202, 82, 255) : Rgb(112, 30, 220));
    }
}

static bool SpawnSawBlade()
{
    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
    {
        SawBlade& blade = gSawBlades[i];
        if (blade.active) continue;
        blade.active = true;
        blade.position = { gPlayer.position.x, gPlayer.position.y - 13.0f };
        blade.velocity = { 0.0f, -245.0f };
        blade.travelTimer = 0.22f;
        blade.life = 7.0f;
        blade.spin = 0.0f;
        SpawnBurst(blade.position, Rgb(210, 210, 220), 8);
        return true;
    }
    return false;
}

static void UseActiveItem()
{
    if (gActiveItem == ActiveNone || gActiveItemUses <= 0) return;

    bool used = false;
    if (gActiveItem == ActiveDynamite)
    {
        --gActiveItemUses;
        used = true;
        gPlayer.velocity.y = -225.0f;
        gPlayer.grounded = false;
        gPlayer.jumpAvailable = false;
        gCameraShakeTimer = 0.20f;
        SpawnBurst(gPlayer.position, Rgb(255, 112, 36), 24);
        PlaySfx(SfxDynamite);
    }
    else if (gActiveItem == ActiveMiniOrb)
    {
        --gActiveItemUses;
        used = true;
        gPlayer.velocity = { 0.0f, -390.0f };
        gPlayer.grounded = false;
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
        gSuperJumpTimer = 1.35f;
        gInvulnerableTimer = kInvulnerableSeconds;
        gHitStopTimer = 0.08f;
        gCameraShakeTimer = 0.10f;
        AddAfterImage(gPlayer.position);
        PlaySfx(SfxSuperDash);
    }
    else if (gActiveItem == ActiveBloodChalice)
    {
        if (gPlayerHealth >= kMaxHealth) return;
        --gActiveItemUses;
        used = true;
        gPlayerHealth += kFullHeartUnits;
        if (gPlayerHealth > kMaxHealth)
            gPlayerHealth = kMaxHealth;
        SpawnBurst(gPlayer.position, Rgb(225, 28, 48), 12);
        PlaySfx(SfxHeal);
    }
    else if (gActiveItem == ActiveHourglass)
    {
        --gActiveItemUses;
        used = true;
        gTimeStopTimer = 5.0f;
        gCameraShakeTimer = 0.08f;
        SpawnBurst(gPlayer.position, Rgb(210, 210, 225), 18);
        PlaySfx(SfxHourglass);
    }
    else if (gActiveItem == ActiveSawBlade)
    {
        if (!SpawnSawBlade()) return;
        --gActiveItemUses;
        used = true;
        gCameraShakeTimer = 0.05f;
        PlaySfx(SfxSawBlade);
    }

    if (used)
        gRunUsedActiveItem = true;
    if (gActiveItemUses <= 0)
        gActiveItem = ActiveNone;
}

static void KillPlayer()
{
    if (gPlayerDead) return;
    if (HasPerk(PerkCrossNecklace) && !gCrossReviveUsed)
    {
        gCrossReviveUsed = true;
        gPlayerHealth = kFullHeartUnits;
        gInvulnerableTimer = kInvulnerableSeconds;
        gSuperJumpTimer = 1.25f;
        gPlayer.velocity = { 0.0f, -390.0f };
        gPlayer.grounded = false;
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
        gHook.mode = HookIdle;
        gHitStopTimer = 0.10f;
        gCameraShakeTimer = 0.24f;
        gCrossReviveEffectPosition = gPlayer.position;
        gCrossReviveEffectTimer = 0.62f;
        ClearPressedInput();
        SpawnBurst(gPlayer.position, Rgb(238, 226, 184), 26);
        SpawnBurst(gPlayer.position, Rgb(168, 36, 54), 18);
        PlaySfx(SfxSuperDash);
        return;
    }
    gPlayerDead = true;
    gPlayerDeathTimer = 0.0f;
    gDeathStageMeters = CurrentChapterProgressMeters();
    if (gDeathStageMeters > gBestStageMeters[gChapter])
    {
        gBestStageMeters[gChapter] = gDeathStageMeters;
        SavePersistentData();
    }
    gPlayer.velocity = { 0.0f, 0.0f };
    gHook.mode = HookIdle;
    ClearPressedInput();
    gCameraShakeTimer = 0.20f;
    SpawnBurst(gPlayer.position, Rgb(92, 235, 220), 24);
    PlaySfx(SfxDeath);
}

static void DamagePlayer()
{
    if (gPlayerDead || gInvulnerableTimer > 0.0f) return;
    int damage = HasPerk(PerkAngelSkin) ? 1 : kFullHeartUnits;
    gPlayerHealth -= damage;
    if (gPlayerHealth <= 0)
    {
        KillPlayer();
        return;
    }

    gInvulnerableTimer = kInvulnerableSeconds;
    gCameraShakeTimer = 0.14f;
    gHook.mode = HookIdle;
    gPlayer.velocity.y = -95.0f;
    SpawnBurst(gPlayer.position, Rgb(92, 235, 220), 10);
    PlaySfx(SfxHit);
}

static void AddCombo()
{
    ++gCombo;
    gComboTimer = kComboSeconds;
    gComboPulse = 0.24f;
}

static void RewardEnemyKill()
{
    AddCombo();
    gCoins += CoinMultiplier();
    gDevourerPauseTimer = 0.22f;
    gDevourerY += 4.0f;
}

static void KillEnemy(Enemy& enemy)
{
    if (!enemy.active || !enemy.alive) return;
    Vec2 center =
    {
        enemy.bounds.x + enemy.bounds.w * 0.5f,
        enemy.bounds.y + enemy.bounds.h * 0.5f
    };
    enemy.alive = false;
    enemy.respawnTimer = 1.3f;
    SpawnEnemyBurst(center);
    if (enemy.type == EnemyHeartFlying && gPlayerHealth < kMaxHealth)
    {
        gPlayerHealth += kFullHeartUnits;
        if (gPlayerHealth > kMaxHealth)
            gPlayerHealth = kMaxHealth;
        PlaySfx(SfxHeal);
    }
    RewardEnemyKill();
}

static void CutWorm(Worm& worm, int cutIndex)
{
    if (!worm.active || cutIndex < 0 || cutIndex >= worm.pointCount) return;
    Vec2 cutPoint = worm.points[cutIndex];
    SpawnEnemyBurst(cutPoint);
    RewardEnemyKill();

    int tailCount = worm.pointCount - cutIndex - 1;
    if (tailCount >= 2)
    {
        for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
        {
            Worm& tail = gWorms[i];
            if (tail.active || &tail == &worm) continue;
            tail = {};
            tail.active = true;
            tail.pointCount = tailCount;
            tail.speed = worm.speed + 3.0f;
            for (int point = 0; point < tailCount; ++point)
                tail.points[point] = worm.points[cutIndex + 1 + point];
            break;
        }
    }

    worm.pointCount = cutIndex;
    if (worm.pointCount < 2) worm.active = false;
}

static void KillEnemiesInSlash(Vec2 start, Vec2 end)
{
    for (int i = 0; i < (int)(sizeof(gEnemies) / sizeof(gEnemies[0])); ++i)
    {
        if (gEnemies[i].alive && SlashHitsRect(start, end, gEnemies[i].bounds))
            KillEnemy(gEnemies[i]);
    }

    for (int wormIndex = 0; wormIndex < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++wormIndex)
    {
        Worm& worm = gWorms[wormIndex];
        if (!worm.active) continue;
        for (int point = 0; point < worm.pointCount; ++point)
        {
            RectF hitBox = { worm.points[point].x - 3.0f, worm.points[point].y - 3.0f, 6.0f, 6.0f };
            if (SlashHitsRect(start, end, hitBox))
            {
                CutWorm(worm, point);
                break;
            }
        }
    }

    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (!projectile.active) continue;
        RectF hitBox = { projectile.position.x - 4.0f, projectile.position.y - 4.0f, 8.0f, 8.0f };
        if (SlashHitsRect(start, end, hitBox))
        {
            projectile.active = false;
            SpawnEnemyBurst(projectile.position);
            AddCombo();
        }
    }
}

static void FireTrapProjectileAt(const WallTrap& trap, float travelOffset)
{
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (projectile.active) continue;
        float direction = trap.firesRight ? 1.0f : -1.0f;
        projectile.active = true;
        projectile.position =
        {
            trap.position.x + direction * (4.0f + travelOffset),
            trap.position.y
        };
        projectile.velocity = { direction * 56.0f, 0.0f };
        projectile.hookable = true;
        return;
    }
}

static void FireTrapProjectile(const WallTrap& trap)
{
    FireTrapProjectileAt(trap, 0.0f);
}

static float EnemyShotInterval()
{
    float totalMeters = gHeightPixels / kPixelsPerMeter;
    float climbFactor = Clamp(totalMeters / 4200.0f, 0.0f, 1.0f);
    float chapterFactor = (float)gChapter * 0.18f;
    if (gChapter == 1) chapterFactor += 0.25f;
    if (gChapter == 2) chapterFactor += 0.24f;
    if (gChapter >= 3) chapterFactor += 0.22f;
    float interval = 2.35f - climbFactor * 0.90f - chapterFactor;
    if (interval < 1.00f) interval = 1.00f;
    return interval;
}

static void FireEnemyProjectile(Enemy& enemy)
{
    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (projectile.active) continue;
        bool hookable = Random01() < 0.16f;
        Vec2 center =
        {
            enemy.bounds.x + enemy.bounds.w * 0.5f,
            enemy.bounds.y + enemy.bounds.h * 0.5f
        };
        Vec2 direction = Normalize(Sub(gPlayer.position, center));
        float speedFactor = Clamp((gHeightPixels / kPixelsPerMeter) / 4200.0f,
                                  0.0f, 1.0f);
        float speed = hookable
            ? 56.0f + speedFactor * 22.0f
            : 64.0f + speedFactor * 28.0f;
        projectile.active = true;
        projectile.hookable = hookable;
        projectile.position = center;
        projectile.velocity = { direction.x * speed, direction.y * speed };
        return;
    }
}

static void UpdateSawBlades(float dt)
{
    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
    {
        SawBlade& blade = gSawBlades[i];
        if (!blade.active) continue;

        blade.life -= dt;
        if (blade.life <= 0.0f)
        {
            blade.active = false;
            continue;
        }

        blade.spin += dt * 24.0f;
        if (blade.travelTimer > 0.0f)
        {
            blade.travelTimer -= dt;
            blade.position.x += blade.velocity.x * dt;
            blade.position.y += blade.velocity.y * dt;
            if (blade.travelTimer <= 0.0f)
            {
                blade.travelTimer = 0.0f;
                blade.velocity = { 0.0f, 0.0f };
            }
        }

        RectF bladeBox =
        {
            blade.position.x - 7.0f,
            blade.position.y - 7.0f,
            14.0f,
            14.0f
        };

        for (int enemyIndex = 0; enemyIndex < kMaxEnemies; ++enemyIndex)
        {
            Enemy& enemy = gEnemies[enemyIndex];
            if (enemy.active && enemy.alive &&
                RectOverlapsRect(bladeBox, enemy.bounds))
                KillEnemy(enemy);
        }

        for (int wormIndex = 0; wormIndex < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++wormIndex)
        {
            Worm& worm = gWorms[wormIndex];
            if (!worm.active) continue;
            for (int point = 0; point < worm.pointCount; ++point)
            {
                RectF hitBox =
                {
                    worm.points[point].x - 3.0f,
                    worm.points[point].y - 3.0f,
                    6.0f,
                    6.0f
                };
                if (RectOverlapsRect(bladeBox, hitBox))
                {
                    CutWorm(worm, point);
                    break;
                }
            }
        }

        for (int projectileIndex = 0; projectileIndex < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++projectileIndex)
        {
            Projectile& projectile = gProjectiles[projectileIndex];
            if (!projectile.active) continue;
            RectF hitBox =
            {
                projectile.position.x - 4.0f,
                projectile.position.y - 4.0f,
                8.0f,
                8.0f
            };
            if (RectOverlapsRect(bladeBox, hitBox))
            {
                projectile.active = false;
                SpawnEnemyBurst(projectile.position);
                AddCombo();
            }
        }
    }
}

static void UpdateActors(float dt)
{
    UpdateSawBlades(dt);

    if (CurrentChapterConfig().allowWorms && gChapterMonsterAwake)
    {
        gWormSpawnTimer -= dt;
        if (gWormSpawnTimer <= 0.0f)
        {
            float wormY = -65.0f - Random01() * 35.0f;
            float rowLeft, rowRight;
            PlayfieldBoundsForScreenY(wormY, &rowLeft, &rowRight);
            SpawnWormRow(wormY,
                         rowLeft + 25.0f +
                         Random01() * (rowRight - rowLeft - 50.0f));
            if (gChapter == 3)
                gWormSpawnTimer = 1.8f + Random01() * 1.8f;
            else if (gChapter == 2)
                gWormSpawnTimer = 2.5f + Random01() * 1.8f;
            else if (gChapter == 1)
                gWormSpawnTimer = 3.2f + Random01() * 2.0f;
            else
                gWormSpawnTimer = 3.8f + Random01() * 2.4f;
        }
    }

    for (int i = 0; i < kMaxEnemies; ++i)
    {
        Enemy& enemy = gEnemies[i];
        if (!enemy.active || !enemy.alive) continue;
        if (enemy.type == EnemyWalker)
        {
            enemy.bounds.x += enemy.velocityX * dt;
            if (enemy.bounds.x <= enemy.patrolMinX)
            {
                enemy.bounds.x = enemy.patrolMinX;
                enemy.velocityX = 28.0f;
            }
            else if (enemy.bounds.x >= enemy.patrolMaxX)
            {
                enemy.bounds.x = enemy.patrolMaxX;
                enemy.velocityX = -28.0f;
            }
        }
        else if (gChapterMonsterAwake && !gPlayerDead &&
                 enemy.bounds.y > -15.0f && enemy.bounds.y < (float)kBufferH + 8.0f)
        {
            enemy.shotTimer -= dt;
            if (enemy.shotTimer <= 0.0f)
            {
                FireEnemyProjectile(enemy);
                enemy.shotTimer = EnemyShotInterval() + Random01() * 0.75f;
            }
        }
    }

    if (!gPlayerDead)
    {
        for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
        {
            Worm& worm = gWorms[i];
            if (!worm.active || worm.pointCount < 2) continue;
            Vec2 direction = Normalize(Sub(gPlayer.position, worm.points[0]));
            worm.points[0].x += direction.x * worm.speed * dt;
            worm.points[0].y += direction.y * worm.speed * dt;
            for (int point = 1; point < worm.pointCount; ++point)
            {
                Vec2 offset = Sub(worm.points[point], worm.points[point - 1]);
                Vec2 followDirection = Normalize(offset);
                worm.points[point] =
                {
                    worm.points[point - 1].x + followDirection.x * 6.0f,
                    worm.points[point - 1].y + followDirection.y * 6.0f
                };
            }
        }
    }

    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
    {
        WallTrap& trap = gTraps[i];
        if (!trap.active) continue;
        trap.shotTimer -= dt;
        if (trap.shotTimer <= 0.0f)
        {
            FireTrapProjectile(trap);
            trap.shotTimer = 0.62f + Random01() * 0.28f;
        }
    }

    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (!projectile.active) continue;
        projectile.position.x += projectile.velocity.x * dt;
        projectile.position.y += projectile.velocity.y * dt;
        float topLimit = projectile.hookable ? -460.0f : -12.0f;
        float bottomLimit = projectile.hookable ? (float)kBufferH + 460.0f :
                            (float)kBufferH + 12.0f;
        if (projectile.position.x < 5.0f || projectile.position.x > (float)kBufferW - 5.0f ||
            projectile.position.y < topLimit || projectile.position.y > bottomLimit)
            projectile.active = false;
    }
}

static void CheckPlayerDamage()
{
    if (gPlayerDead || gHook.dashing) return;
    bool superJumping = gSuperJumpTimer > 0.0f;
    bool invulnerable = gInvulnerableTimer > 0.0f;

    for (int i = 0; i < kMaxEnemies; ++i)
    {
        if (gEnemies[i].active && gEnemies[i].alive &&
            Overlaps(gPlayer, gEnemies[i].bounds))
        {
            if (superJumping)
                KillEnemy(gEnemies[i]);
            else if (!invulnerable)
            {
                DamagePlayer();
                return;
            }
        }
    }

    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
    {
        Worm& worm = gWorms[i];
        if (!worm.active) continue;
        for (int point = 0; point < worm.pointCount; ++point)
        {
            RectF hitBox = { worm.points[point].x - 3.0f, worm.points[point].y - 3.0f, 6.0f, 6.0f };
            if (Overlaps(gPlayer, hitBox))
            {
                if (superJumping)
                {
                    CutWorm(worm, point);
                    break;
                }
                if (!invulnerable)
                {
                    DamagePlayer();
                    return;
                }
            }
        }
    }

    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (!projectile.active) continue;
        RectF hitBox = { projectile.position.x - 4.0f, projectile.position.y - 4.0f, 8.0f, 8.0f };
        if (Overlaps(gPlayer, hitBox))
        {
            if (superJumping)
            {
                projectile.active = false;
                SpawnEnemyBurst(projectile.position);
                AddCombo();
            }
            else if (!invulnerable)
            {
                projectile.active = false;
                DamagePlayer();
                return;
            }
        }
    }
}

static void BeginDash()
{
    if (SegmentHitsBranchDivider(gPlayer.position, gHook.target))
    {
        gHook.mode = HookIdle;
        gHook.dashing = false;
        gCameraShakeTimer = 0.05f;
        ResolveBranchDividerOverlap();
        return;
    }

    gHook.dashing = true;
    gHook.dashTimer = 0.0f;
    gHook.dashDuration = 0.045f;
    gHook.dashStart = gPlayer.position;
    gSlashTrail.start = gHook.dashStart;
    gSlashTrail.end = gHook.target;
    gSlashTrail.life = 0.14f;
    gCameraShakeTimer = 0.12f;
    AddAfterImage(gPlayer.position);
    KillEnemiesInSlash(gHook.dashStart, gHook.target);
    PlaySfx(SfxHookDash);
}

static void AttachHookToCombatTarget(Vec2 center)
{
    gHook.mode = HookPullEnemy;
    gHook.position = center;
    gHook.pauseTimer = 0.0f;
    gHook.dashing = false;
    gHitStopTimer = 0.11f;
    gHook.vault = gInput.keys[VK_SPACE] ||
                  gInput.keys['W'] ||
                  gInput.keys[VK_UP];
    PlaySfx(SfxHookHit);

    if (gHook.vault)
    {
        gHook.target = { center.x, center.y - 25.0f };
    }
    else
    {
        gHook.target =
        {
            center.x + gHook.direction.x * 30.0f,
            center.y + gHook.direction.y * 30.0f
        };
    }
}

static void AttachHookToEnemy(Enemy& enemy)
{
    Vec2 center =
    {
        enemy.bounds.x + enemy.bounds.w * 0.5f,
        enemy.bounds.y + enemy.bounds.h * 0.5f
    };
    AttachHookToCombatTarget(center);
    KillEnemy(enemy);
}

static void AttachHookToPlatform(const Platform& platform)
{
    const RectF& rect = platform.bounds;
    gHook.mode = HookPullTerrain;
    gHook.pauseTimer = 0.0f;
    gHook.dashing = false;
    gHook.vault = false;
    gInvulnerableTimer = kInvulnerableSeconds;
    PlaySfx(SfxHookHit);
    float targetX = Clamp(gHook.position.x, rect.x + gPlayer.halfW + 1.0f,
                          rect.x + rect.w - gPlayer.halfW - 1.0f);
    for (int i = 0; i < kMaxEnemies; ++i)
    {
        const Enemy& enemy = gEnemies[i];
        if (!enemy.active || !enemy.alive || enemy.type != EnemyWalker) continue;
        if (enemy.bounds.y + enemy.bounds.h < rect.y - 1.0f ||
            enemy.bounds.y > rect.y + 2.0f)
            continue;
        float unsafeLeft = enemy.bounds.x - gPlayer.halfW - 4.0f;
        float unsafeRight = enemy.bounds.x + enemy.bounds.w + gPlayer.halfW + 4.0f;
        if (targetX >= unsafeLeft && targetX <= unsafeRight)
        {
            float leftCandidate = Clamp(unsafeLeft - 1.0f,
                                        rect.x + gPlayer.halfW + 1.0f,
                                        rect.x + rect.w - gPlayer.halfW - 1.0f);
            float rightCandidate = Clamp(unsafeRight + 1.0f,
                                         rect.x + gPlayer.halfW + 1.0f,
                                         rect.x + rect.w - gPlayer.halfW - 1.0f);
            targetX = AbsInt((int)(targetX - leftCandidate)) <
                      AbsInt((int)(rightCandidate - targetX))
                ? leftCandidate
                : rightCandidate;
        }
    }
    gHook.target =
    {
        targetX,
        rect.y - gPlayer.halfH
    };
}

static void AttachHookToShopPortal(Vec2 center, bool freePortal, bool startPortal)
{
    gHook.mode = HookShopPortal;
    gHook.position = center;
    gHook.target = center;
    gHook.dashing = false;
    gHookShopFree = freePortal;
    gHookShopStart = startPortal;
    PlaySfx(SfxHookHit);
}

static void OpenShopPortal(bool freePortal, bool startPortal)
{
    gShopCurrentFree = freePortal;
    gShopCurrentStart = startPortal;
    gHookShopFree = false;
    gHookShopStart = false;
    BuildShopOffers();
    gShopOpen = true;
    gShopHover = 0;
    gStartPortalTouchArmed = false;
    gHook.mode = HookIdle;
    gPlayer.velocity = { 0.0f, 0.0f };
    gCameraShakeTimer = 0.08f;
    PlaySfx(SfxShopEnter);
}

static void CheckPlayerShopPortalTouch()
{
    if (gShopOpen || gPlayerDead || gChapterTransitionActive) return;
    Vec2 portal;
    bool freePortal = false;
    bool startPortal = false;
    if (!ShopPortalPosition(&portal, &freePortal, &startPortal)) return;

    RectF portalBounds =
    {
        portal.x - 14.0f,
        portal.y - 17.0f,
        28.0f,
        34.0f
    };
    bool touching = Overlaps(gPlayer, portalBounds);
    if (startPortal && !gStartPortalTouchArmed)
    {
        if (!touching) gStartPortalTouchArmed = true;
        return;
    }
    if (touching)
        OpenShopPortal(freePortal, startPortal);
}

static void UpdateHookFlight(float dt)
{
    const float hookSpeed = 520.0f;
    float step = hookSpeed * dt;
    gHook.position.x += gHook.direction.x * step;
    gHook.position.y += gHook.direction.y * step;
    gHook.distance += step;

    for (int i = 0; i < (int)(sizeof(gEnemies) / sizeof(gEnemies[0])); ++i)
    {
        if (gEnemies[i].alive && PointInRect(gHook.position, gEnemies[i].bounds))
        {
            AttachHookToEnemy(gEnemies[i]);
            return;
        }
    }

    for (int wormIndex = 0; wormIndex < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++wormIndex)
    {
        Worm& worm = gWorms[wormIndex];
        if (!worm.active) continue;
        for (int point = 0; point < worm.pointCount; ++point)
        {
            Vec2 delta = Sub(gHook.position, worm.points[point]);
            if (delta.x * delta.x + delta.y * delta.y <= 16.0f)
            {
                Vec2 hitPoint = worm.points[point];
                AttachHookToCombatTarget(hitPoint);
                CutWorm(worm, point);
                return;
            }
        }
    }

    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
    {
        SawBlade& blade = gSawBlades[i];
        if (!blade.active) continue;
        Vec2 delta = Sub(gHook.position, blade.position);
        if (delta.x * delta.x + delta.y * delta.y <= 64.0f)
        {
            Vec2 hitPoint = blade.position;
            blade.active = false;
            SpawnBurst(hitPoint, Rgb(210, 210, 220), 10);
            AttachHookToCombatTarget(hitPoint);
            return;
        }
    }

    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        Projectile& projectile = gProjectiles[i];
        if (!projectile.active || !projectile.hookable) continue;
        Vec2 delta = Sub(gHook.position, projectile.position);
        if (delta.x * delta.x + delta.y * delta.y <= 49.0f)
        {
            Vec2 hitPoint = projectile.position;
            projectile.active = false;
            SpawnEnemyBurst(hitPoint);
            AttachHookToCombatTarget(hitPoint);
            AddCombo();
            return;
        }
    }

    RectF divider;
    if (BranchDividerForScreenY(gHook.position.y, &divider) &&
        PointInRect(gHook.position, divider))
    {
        gHook.mode = HookIdle;
        TriggerHookMissCooldown();
        return;
    }

    Vec2 portal;
    bool freePortal = false;
    bool startPortal = false;
    if (ShopPortalPosition(&portal, &freePortal, &startPortal))
    {
        Vec2 delta = Sub(gHook.position, portal);
        if (delta.x * delta.x + delta.y * delta.y <= 100.0f)
        {
            AttachHookToShopPortal(portal, freePortal, startPortal);
            return;
        }
    }

    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
    {
        SuperOrb& orb = gSuperOrbs[i];
        if (!orb.active) continue;
        Vec2 delta = Sub(gHook.position, orb.position);
        if (delta.x * delta.x + delta.y * delta.y <=
            kSuperOrbHitRadius * kSuperOrbHitRadius)
        {
            gHook.mode = HookSuperOrb;
            gHook.position = orb.position;
            gHook.target = orb.position;
            gHook.dashing = false;
            orb.active = false;
            SpawnBurst(gHook.position, Rgb(190, 92, 255), 10);
            AddCombo();
            PlaySfx(SfxHookHit);
            return;
        }
    }

    for (int i = 0; i < (int)(sizeof(gPlatforms) / sizeof(gPlatforms[0])); ++i)
    {
        if (gPlatforms[i].active && gPlatforms[i].category == PlatformSolid &&
            PointInRect(gHook.position, gPlatforms[i].bounds))
        {
            AttachHookToPlatform(gPlatforms[i]);
            return;
        }
    }

    if (gHook.distance >= 190.0f ||
        gHook.position.x < 0.0f || gHook.position.x >= (float)kBufferW ||
        gHook.position.y < 0.0f || gHook.position.y >= (float)kBufferH)
    {
        gHook.mode = HookIdle;
        TriggerHookMissCooldown();
    }
}

static void FinishDash()
{
    HookMode completedMode = gHook.mode;
    Vec2 dashDirection = Normalize(Sub(gHook.target, gHook.dashStart));
    gPlayer.position = gHook.target;
    ResolveBranchDividerOverlap();

    if (completedMode == HookPullTerrain)
    {
        gPlayer.velocity = { 0.0f, 0.0f };
        gPlayer.grounded = true;
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
    }
    else if (gHook.vault)
    {
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
        gPlayer.velocity.x = dashDirection.x * 70.0f;
        gPlayer.velocity.y = -155.0f;
        gPlayer.grounded = false;
    }
    else
    {
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
        gPlayer.velocity.x = dashDirection.x * 105.0f;
        gPlayer.velocity.y = dashDirection.y * 105.0f;
    }
    if (completedMode == HookPullEnemy)
        gInvulnerableTimer = kInvulnerableSeconds;
    gHook.mode = HookIdle;
    gHook.dashing = false;
}

static void UpdateHookPull(float dt)
{
    if (gHook.mode == HookShopPortal)
    {
        const float portalPullSpeed = 380.0f;
        Vec2 toTarget = Sub(gHook.target, gPlayer.position);
        float distance = Length(toTarget);
        float step = portalPullSpeed * dt;
        if (distance <= step + 0.5f)
        {
            gPlayer.position = gHook.target;
            gPlayer.velocity = { 0.0f, 0.0f };
            gPlayer.grounded = false;
            gPlayer.jumpAvailable = true;
            gAirJumpsUsed = 0;
            OpenShopPortal(gHookShopFree, gHookShopStart);
            return;
        }
        Vec2 direction = Normalize(toTarget);
        gPlayer.velocity = { direction.x * portalPullSpeed,
                             direction.y * portalPullSpeed };
        gPlayer.position.x += gPlayer.velocity.x * dt;
        gPlayer.position.y += gPlayer.velocity.y * dt;
        gPlayer.grounded = false;
        return;
    }

    if (gHook.mode == HookSuperOrb)
    {
        const float orbPullSpeed = 400.0f;
        Vec2 toTarget = Sub(gHook.target, gPlayer.position);
        float distance = Length(toTarget);
        float step = orbPullSpeed * dt;
        if (distance <= step + 0.5f)
        {
            gPlayer.position = gHook.target;
            gPlayer.velocity = { 0.0f, -390.0f };
            gPlayer.grounded = false;
            gPlayer.jumpAvailable = true;
            gAirJumpsUsed = 0;
            gSuperJumpTimer = 1.35f;
            gInvulnerableTimer = kInvulnerableSeconds;
            gHitStopTimer = 0.12f;
            gCameraShakeTimer = 0.10f;
            gHook.mode = HookIdle;
            PlaySfx(SfxSuperDash);
            return;
        }
        Vec2 direction = Normalize(toTarget);
        gPlayer.velocity = { direction.x * orbPullSpeed, direction.y * orbPullSpeed };
        gPlayer.position.x += gPlayer.velocity.x * dt;
        gPlayer.position.y += gPlayer.velocity.y * dt;
        gPlayer.grounded = false;
        return;
    }

    if (gHook.mode == HookPullTerrain)
    {
        const float platformPullSpeed = 320.0f;
        Vec2 toTarget = Sub(gHook.target, gPlayer.position);
        float distance = Length(toTarget);
        float step = platformPullSpeed * dt;

        if (distance <= step + 0.5f)
        {
            gPlayer.position = gHook.target;
            gPlayer.velocity = { 0.0f, 0.0f };
            gPlayer.grounded = true;
            gPlayer.jumpAvailable = true;
            gAirJumpsUsed = 0;
            gInvulnerableTimer = kInvulnerableSeconds;
            gHook.mode = HookIdle;
            return;
        }

        Vec2 direction = Normalize(toTarget);
        gPlayer.velocity = { direction.x * platformPullSpeed,
                             direction.y * platformPullSpeed };
        gPlayer.position.x += gPlayer.velocity.x * dt;
        gPlayer.position.y += gPlayer.velocity.y * dt;
        gPlayer.grounded = false;
        return;
    }

    gPlayer.velocity = { 0.0f, 0.0f };

    if (!gHook.dashing)
    {
        gHook.pauseTimer -= dt;
        if (gHook.pauseTimer <= 0.0f) BeginDash();
        return;
    }

    Vec2 previous = gPlayer.position;
    gHook.dashTimer += dt;
    float t = Clamp(gHook.dashTimer / gHook.dashDuration, 0.0f, 1.0f);
    gPlayer.position =
    {
        gHook.dashStart.x + (gHook.target.x - gHook.dashStart.x) * t,
        gHook.dashStart.y + (gHook.target.y - gHook.dashStart.y) * t
    };
    AddAfterImage(previous);
    AddAfterImage({ (previous.x + gPlayer.position.x) * 0.5f,
                    (previous.y + gPlayer.position.y) * 0.5f });

    if (t >= 1.0f) FinishDash();
}

static void UpdateHook(float dt)
{
    UpdateEffects(dt);
    if (gHookCooldownTimer > 0.0f)
    {
        gHookCooldownTimer -= dt;
        if (gHookCooldownTimer < 0.0f) gHookCooldownTimer = 0.0f;
    }

    if (gInput.mousePressed)
    {
        if (gHook.mode == HookIdle)
            StartHook();
        else
        {
            if (gHook.mode == HookFlying)
                TriggerHookMissCooldown();
            gHook.mode = HookIdle;
        }
        gInput.mousePressed = false;
    }

    if (gHook.mode == HookFlying)
        UpdateHookFlight(dt);
}

static void AcceptShopOffer(int index)
{
    if (index < 0 || index >= 3) return;
    int price = CurrentShopPrice();
    if (gCoins < price) return;
    PlaySfx(SfxUiClick);
    gCoins -= price;
    gActiveItem = gShopOffers[index].item;
    gActiveItemUses = gShopOffers[index].uses + (HasPerk(PerkContract) ? 1 : 0);
    if (gShopCurrentFree)
        gFreeShopClaimed = true;
    else if (gShopCurrentStart)
        gStartShopUsed[gChapter] = true;
    else
        gShopUsed[gChapter] = true;
    gShopOpen = false;
    gDealAcceptedTimer = 1.45f;
    SpawnBurst(gPlayer.position, Rgb(190, 92, 255), 16);
    PlaySfx(SfxPerkConfirm);

    if (gCheckpointValid && gCheckpointChapter == gChapter)
    {
        gCheckpointCoins = gCoins;
        gCheckpointActiveItem = gActiveItem;
        gCheckpointActiveItemUses = gActiveItemUses;
        CopyPerks(gCheckpointPerkOwned, gPerkOwned);
        gCheckpointPerkGrantChapterMask = gPerkGrantChapterMask;
        gCheckpointCrossReviveUsed = gCrossReviveUsed;
        gCheckpointFreeShopClaimed = gFreeShopClaimed;
        for (int i = 0; i < kChapterCount; ++i)
        {
            gCheckpointShopUsed[i] = gShopUsed[i];
            gCheckpointStartShopUsed[i] = gStartShopUsed[i];
        }
    }
    SavePersistentData();
}

static void UpdateShopInput()
{
    if (gInput.pressed[VK_ESCAPE])
    {
        gShopOpen = false;
        gHook.mode = HookIdle;
        gInput.pressed[VK_ESCAPE] = false;
        gInput.mousePressed = false;
        return;
    }

    int cardW = 58;
    int cardH = 58;
    int startX = 66;
    int y = 54;
    gShopHover = -1;
    for (int i = 0; i < 3; ++i)
    {
        RectF card = { (float)(startX + i * 64), (float)y, (float)cardW, (float)cardH };
        Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
        if (PointInRect(mouse, card))
            gShopHover = i;
    }

    if (gInput.mousePressed)
    {
        if (MenuClicked(121.0f, 154.0f, 78.0f, 18.0f))
        {
            gShopOpen = false;
            gHook.mode = HookIdle;
        }
        else if (gShopHover >= 0)
            AcceptShopOffer(gShopHover);
        gInput.mousePressed = false;
    }
}

static bool MouseInRect(float x, float y, float w, float h)
{
    Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
    RectF rect = { x, y, w, h };
    return PointInRect(mouse, rect);
}

static bool MenuClicked(float x, float y, float w, float h, bool enabled)
{
    if (!enabled || !gInput.mousePressed) return false;
    Vec2 click = { (float)gInput.mousePressedX, (float)gInput.mousePressedY };
    RectF rect = { x, y, w, h };
    bool clicked = PointInRect(click, rect);
    if (clicked)
        PlaySfx(SfxUiClick);
    return clicked;
}

static void ResetRunProgress()
{
    gCheckpointValid = false;
    gCheckpointChapter = 0;
    gCheckpointInfiniteMode = false;
    gCheckpointCoins = 0;
    gCheckpointActiveItem = ActiveNone;
    gCheckpointActiveItemUses = 0;
    gCheckpointRunUsedActiveItem = false;
    for (int i = 0; i < PerkCount; ++i)
    {
        gCheckpointPerkOwned[i] = false;
        gPerkOwned[i] = false;
    }
    gCheckpointPerkGrantChapterMask = 0;
    gPerkGrantChapterMask = 0;
    gCheckpointCrossReviveUsed = false;
    gCrossReviveUsed = false;
    gQueuedPerkRoulette = PerkNone;
    gPerkRouletteOpen = false;
    gPerkRouletteTimer = 0.0f;
    gPerkRouletteWaitingConfirm = false;
    gPerkRouletteFinal = PerkNone;
    gCheckpointFreeShopClaimed = false;
    for (int i = 0; i < kChapterCount; ++i)
    {
        gCheckpointShopUsed[i] = false;
        gCheckpointStartShopUsed[i] = false;
        gShopUsed[i] = false;
        gStartShopUsed[i] = false;
    }
    gFreeShopClaimed = false;
    gCoins = 0;
    gActiveItem = ActiveNone;
    gActiveItemUses = 0;
    gInfiniteMode = false;
    gRunUsedActiveItem = false;
    gAirJumpsUsed = 0;
    gParachuteHoldTimer = 0.0f;
    gPreserveRunStateOnRestart = false;
}

static void CreateChapterStartCheckpoint()
{
    gCheckpointValid = true;
    gCheckpointChapter = gChapter;
    gCheckpointInfiniteMode = gInfiniteMode;
    gCheckpointCoins = gCoins;
    gCheckpointActiveItem = gActiveItem;
    gCheckpointActiveItemUses = gActiveItemUses;
    gCheckpointRunUsedActiveItem = gRunUsedActiveItem;
    CopyPerks(gCheckpointPerkOwned, gPerkOwned);
    gCheckpointPerkGrantChapterMask = gPerkGrantChapterMask;
    gCheckpointCrossReviveUsed = gCrossReviveUsed;
    gCheckpointFreeShopClaimed = gFreeShopClaimed;
    for (int i = 0; i < kChapterCount; ++i)
    {
        gCheckpointShopUsed[i] = gShopUsed[i];
        gCheckpointStartShopUsed[i] = gStartShopUsed[i];
    }
}

static void StartNewGameFromMenu(bool infiniteMode)
{
    ResetRunProgress();
    gInfiniteMode = infiniteMode;
    gCheckpointValid = true;
    gCheckpointChapter = 0;
    gCheckpointInfiniteMode = gInfiniteMode;
    RestartGame();
    CreateChapterStartCheckpoint();
    SavePersistentData();
    gHelpStartsGame = true;
    gHelpReturnScreen = ScreenGameplay;
    gHelpPage = 0;
    gIntroTimer = 0.0f;
    gScreen = ScreenIntroCutscene;
}

static void ContinueGameFromMenu()
{
    if (!gCheckpointValid) return;
    RestartGame();
    gScreen = ScreenGameplay;
}

static void UpdateMainMenuInput()
{
    if (MenuClicked(108.0f, 62.0f, 104.0f, 14.0f))
    {
        gScreen = ScreenNewGameSelect;
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(108.0f, 77.0f, 104.0f, 14.0f, gCheckpointValid))
    {
        ContinueGameFromMenu();
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(108.0f, 92.0f, 104.0f, 14.0f))
    {
        gOptionsFromGameplay = false;
        gScreen = ScreenOptions;
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(108.0f, 107.0f, 104.0f, 14.0f))
    {
        gScreen = ScreenCodex;
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(108.0f, 122.0f, 104.0f, 14.0f))
    {
        gScreen = ScreenAchievements;
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(108.0f, 137.0f, 104.0f, 14.0f))
    {
        gRunning = false;
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(248.0f, 160.0f, 62.0f, 14.0f))
    {
        gScreen = ScreenCredits;
        gInput.mousePressed = false;
        return;
    }
}

static void UpdateNewGameSelectInput()
{
    if (MenuClicked(82.0f, 70.0f, 156.0f, 22.0f))
    {
        StartNewGameFromMenu(false);
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(82.0f, 102.0f, 156.0f, 22.0f, InfiniteModeUnlocked()))
    {
        StartNewGameFromMenu(true);
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(112.0f, 146.0f, 96.0f, 18.0f) || gInput.pressed[VK_ESCAPE])
    {
        gScreen = ScreenMainMenu;
        gInput.mousePressed = false;
        gInput.pressed[VK_ESCAPE] = false;
        return;
    }
}

static void UpdateAchievementsInput()
{
    if (MenuClicked(26.0f, 150.0f, 78.0f, 18.0f))
    {
        for (int i = 0; i < AchievementCount; ++i)
            gAchievementUnlocked[i] = false;
        gGameClearedEver = false;
        gAchievementPopup = AchievementCount;
        gAchievementPopupTimer = 0.0f;
        gAchievementQueueCount = 0;
        SavePersistentData();
        gInput.mousePressed = false;
        return;
    }
    if (MenuClicked(216.0f, 150.0f, 78.0f, 18.0f) || gInput.pressed[VK_ESCAPE])
    {
        gScreen = ScreenMainMenu;
        gInput.mousePressed = false;
        gInput.pressed[VK_ESCAPE] = false;
        return;
    }
}

static bool UpdateSlider(float x, float y, float w, float* value)
{
    if (!gInput.mouseLeft || !MouseInRect(x - 4.0f, y - 6.0f, w + 8.0f, 14.0f)) return false;
    float newValue = Clamp(((float)gInput.mouseX - x) / w, 0.0f, 1.0f);
    if ((newValue - *value) < 0.001f && (*value - newValue) < 0.001f) return false;
    *value = newValue;
    return true;
}

static void UpdateOptionsInput()
{
    bool changed = false;
    changed = UpdateSlider(110.0f, 54.0f, 118.0f, &gBrightness) || changed;
    changed = UpdateSlider(110.0f, 78.0f, 118.0f, &gMasterSound) || changed;
    if (MenuClicked(126.0f, 96.0f, 20.0f, 16.0f))
    {
        gCrosshairStyle = (gCrosshairStyle + 3) % 4;
        changed = true;
    }
    if (MenuClicked(206.0f, 96.0f, 20.0f, 16.0f))
    {
        gCrosshairStyle = (gCrosshairStyle + 1) % 4;
        changed = true;
    }
    if (changed)
        SavePersistentData();

    if (MenuClicked(34.0f, 150.0f, 74.0f, 18.0f))
    {
        gHelpStartsGame = false;
        gHelpReturnScreen = ScreenOptions;
        gHelpPage = 0;
        gScreen = ScreenHelp;
        gInput.mousePressed = false;
        return;
    }

    if (gOptionsFromGameplay && MenuClicked(123.0f, 150.0f, 74.0f, 18.0f))
    {
        gScreen = ScreenGameplay;
        gInput.mousePressed = false;
        return;
    }

    if (MenuClicked(212.0f, 150.0f, 74.0f, 18.0f))
    {
        gScreen = ScreenMainMenu;
        gOptionsFromGameplay = false;
        gInput.mousePressed = false;
        return;
    }

    if (!gOptionsFromGameplay && MenuClicked(123.0f, 150.0f, 74.0f, 18.0f))
    {
        gScreen = ScreenMainMenu;
        gInput.mousePressed = false;
        return;
    }
}

static void UpdateHelpInput()
{
    const int kHelpPageCount = 4;
    if (MenuClicked(24.0f, 152.0f, 58.0f, 18.0f, gHelpPage > 0) ||
        gInput.pressed[VK_LEFT] || gInput.pressed['A'])
    {
        if (gHelpPage > 0) --gHelpPage;
        gInput.mousePressed = false;
        gInput.pressed[VK_LEFT] = false;
        gInput.pressed['A'] = false;
        return;
    }

    if (MenuClicked(172.0f, 152.0f, 58.0f, 18.0f, gHelpPage < kHelpPageCount - 1) ||
        gInput.pressed[VK_RIGHT] || gInput.pressed['D'])
    {
        if (gHelpPage < kHelpPageCount - 1) ++gHelpPage;
        gInput.mousePressed = false;
        gInput.pressed[VK_RIGHT] = false;
        gInput.pressed['D'] = false;
        return;
    }

    if (MenuClicked(236.0f, 152.0f, 58.0f, 18.0f) ||
        gInput.pressed[VK_ESCAPE])
    {
        gScreen = gHelpStartsGame ? ScreenGameplay : gHelpReturnScreen;
        gHelpStartsGame = false;
        gHelpPage = 0;
        gInput.mousePressed = false;
        gInput.pressed[VK_ESCAPE] = false;
    }
}

static void FinishIntroCutscene()
{
    gIntroTimer = 0.0f;
    gHelpStartsGame = true;
    gHelpReturnScreen = ScreenGameplay;
    gHelpPage = 0;
    gScreen = ScreenHelp;
    ClearPressedInput();
}

static void UpdateIntroCutsceneInput(float dt)
{
    gIntroTimer += dt;
    bool skip =
        gInput.mousePressed ||
        gInput.pressed[VK_RETURN] ||
        gInput.pressed[VK_SPACE] ||
        gInput.pressed[VK_ESCAPE];
    if (skip || gIntroTimer >= 7.2f)
        FinishIntroCutscene();
}

static void UpdateCodexInput()
{
    if (MenuClicked(248.0f, 154.0f, 58.0f, 18.0f) || gInput.pressed[VK_ESCAPE])
    {
        gScreen = ScreenMainMenu;
        gInput.mousePressed = false;
        gInput.pressed[VK_ESCAPE] = false;
    }
}

static void UpdateCreditsInput()
{
    if (MenuClicked(112.0f, 142.0f, 96.0f, 18.0f) || gInput.pressed[VK_ESCAPE])
    {
        gScreen = ScreenMainMenu;
        gInput.mousePressed = false;
        gInput.pressed[VK_ESCAPE] = false;
    }
}

static void UpdateMenu(float dt)
{
    gMapTime += dt;
    if (gScreen == ScreenMainMenu)
        UpdateMainMenuInput();
    else if (gScreen == ScreenNewGameSelect)
        UpdateNewGameSelectInput();
    else if (gScreen == ScreenIntroCutscene)
        UpdateIntroCutsceneInput(dt);
    else if (gScreen == ScreenOptions)
        UpdateOptionsInput();
    else if (gScreen == ScreenHelp)
        UpdateHelpInput();
    else if (gScreen == ScreenCodex)
        UpdateCodexInput();
    else if (gScreen == ScreenAchievements)
        UpdateAchievementsInput();
    else if (gScreen == ScreenCredits)
        UpdateCreditsInput();

    gInput.mousePressed = false;
}

static void UpdatePhysics(float dt)
{
    UpdateAchievementPopup(dt);

    if (gPerkRouletteOpen)
    {
        UpdateEffects(dt);
        if (!gPerkRouletteWaitingConfirm)
        {
            gPerkRouletteTimer -= dt;
            if (gPerkRouletteTimer <= 0.0f)
            {
                gPerkRouletteTimer = 0.0f;
                gPerkRouletteWaitingConfirm = true;
                PlaySfx(SfxPerkConfirm);
            }
            ClearPressedInput();
        }
        else
        {
            bool confirmed = MenuClicked(112.0f, 148.0f, 96.0f, 18.0f) ||
                             gInput.pressed[VK_RETURN];
            if (confirmed)
            {
                gPerkRouletteOpen = false;
                gPerkRouletteWaitingConfirm = false;
                gPerkRouletteFinal = PerkNone;
            }
            gInput.pressed[VK_ESCAPE] = false;
            gInput.pressed['R'] = false;
            ClearPressedInput();
        }
        return;
    }

    if (gInput.pressed[VK_ESCAPE])
    {
        gOptionsFromGameplay = true;
        gScreen = ScreenOptions;
        gInput.pressed[VK_ESCAPE] = false;
        gInput.mousePressed = false;
        return;
    }

    if (gShopOpen)
    {
        UpdateEffects(dt);
        UpdateShopInput();
        return;
    }

    if (gEndingActive)
    {
        UpdateEffects(dt);
        gEndingTimer += dt;
        if (gEndingTimer > 1.0f) gEndingTimer = 1.0f;
        if (MenuClicked(112.0f, 132.0f, 96.0f, 20.0f) ||
            gInput.pressed[VK_RETURN] ||
            gInput.pressed[VK_ESCAPE])
        {
            gInput.mousePressed = false;
            gInput.pressed[VK_RETURN] = false;
            gInput.pressed[VK_ESCAPE] = false;
            gEndingActive = false;
            gScreen = ScreenMainMenu;
        }
        return;
    }

    if (gInput.pressed['R'])
    {
        RestartGame();
        return;
    }

    if (gChapterTransitionActive)
    {
        UpdateEffects(dt);
        float elapsed = kChapterTransitionSeconds - gChapterTransitionTimer;
        if (gChapterTransitionWaitingConfirm)
            elapsed = kChapterTransitionSeconds;
        float t = SmoothStep(elapsed / kChapterTransitionMoveSeconds);
        gPlayer.position =
        {
            gChapterTransitionStart.x +
                (gChapterTransitionTarget.x - gChapterTransitionStart.x) * t,
            gChapterTransitionStart.y +
                (gChapterTransitionTarget.y - gChapterTransitionStart.y) * t
        };
        gPlayer.velocity = { 0.0f, 0.0f };
        gPlayer.grounded = true;
        gPlayer.jumpAvailable = true;
        return;
    }

    if (gPlayerDead)
    {
        UpdateEffects(dt);
        gPlayerDeathTimer += dt;
        if (gPlayerDeathTimer > 1.0f) gPlayerDeathTimer = 1.0f;
        bool deathInputReady = gPlayerDeathTimer >= 0.25f;
        if (deathInputReady &&
            (MenuClicked(56.0f, 132.0f, 92.0f, 20.0f) || gInput.pressed['R']))
        {
            gInput.mousePressed = false;
            gInput.pressed['R'] = false;
            RestartGame();
            return;
        }
        if (deathInputReady &&
            (MenuClicked(172.0f, 132.0f, 92.0f, 20.0f) || gInput.pressed[VK_ESCAPE]))
        {
            gInput.mousePressed = false;
            gInput.pressed[VK_ESCAPE] = false;
            gScreen = ScreenMainMenu;
            return;
        }
        return;
    }

    if (gInput.pressed['Q'])
    {
        UseActiveItem();
        gInput.pressed['Q'] = false;
    }

    bool timeStopped = gTimeStopTimer > 0.0f;
    if (!timeStopped)
    {
        UpdateActors(dt);
        CheckPlayerDamage();
        if (gPlayerDead) return;
    }

    UpdateHook(dt);

    if (gHitStopTimer > 0.0f)
        return;

    if (gHook.mode == HookPullTerrain || gHook.mode == HookPullEnemy ||
        gHook.mode == HookSuperOrb || gHook.mode == HookShopPortal)
    {
        UpdateHookPull(dt);
        CheckPlayerShopPortalTouch();
        if (gShopOpen) return;
        gInput.pressed[VK_SPACE] = false;
        gInput.pressed['W'] = false;
        gInput.pressed[VK_UP] = false;
        if (!timeStopped)
            CheckPlayerDamage();
        return;
    }

    if (gSuperJumpTimer > 0.0f)
    {
        AddAfterImage(gPlayer.position);
        SpawnSuperDashFlame(gPlayer.position);
        gPlayer.velocity = { 0.0f, -390.0f };
        gPlayer.position.y += gPlayer.velocity.y * dt;
        gPlayer.grounded = false;
        gPlayer.jumpAvailable = true;
        gAirJumpsUsed = 0;
        gInput.pressed[VK_SPACE] = false;
        gInput.pressed['W'] = false;
        gInput.pressed[VK_UP] = false;
        CheckPlayerShopPortalTouch();
        if (gShopOpen) return;
        if (!timeStopped)
            CheckPlayerDamage();
        return;
    }

    bool left = gInput.keys['A'] || gInput.keys[VK_LEFT];
    bool right = gInput.keys['D'] || gInput.keys[VK_RIGHT];
    float move = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    if (move > 0.0f)
        gPlayerFacingRight = true;
    else if (move < 0.0f)
        gPlayerFacingRight = false;

    const float moveSpeed = 88.0f;
    float acceleration = gPlayer.grounded ? 1250.0f : 760.0f;
    float deceleration = gPlayer.grounded ? 1500.0f : 500.0f;
    float rate = move == 0.0f ? deceleration : acceleration;
    gPlayer.velocity.x = MoveTowards(gPlayer.velocity.x, move * moveSpeed, rate * dt);

    bool jumpPressed = gInput.pressed[VK_SPACE] ||
                       gInput.pressed['W'] ||
                       gInput.pressed[VK_UP];
    bool canDoubleJump = HasPerk(PerkBloodBattery) &&
                         !gPlayer.grounded &&
                         !gPlayer.jumpAvailable &&
                         gAirJumpsUsed < 1;
    if (jumpPressed && (gPlayer.jumpAvailable || canDoubleJump))
    {
        bool devilJump = HasPerk(PerkDevilHeart);
        gPlayer.velocity.y = devilJump ? -202.0f : -158.0f;
        gPlayer.grounded = false;
        gPlayer.jumpAvailable = false;
        if (canDoubleJump)
            ++gAirJumpsUsed;
        else
            gAirJumpsUsed = 0;
        gParachuteHoldTimer = 0.0f;
        if (devilJump)
        {
            AddAfterImage(gPlayer.position);
            AddAfterImage({ gPlayer.position.x, gPlayer.position.y + 8.0f });
            SpawnBurst({ gPlayer.position.x, gPlayer.position.y + 5.0f },
                       Rgb(238, 34, 60), 14);
            SpawnBurst({ gPlayer.position.x, gPlayer.position.y + 9.0f },
                       Rgb(146, 36, 112), 8);
        }
        PlaySfx(SfxJump);
    }

    gInput.pressed[VK_SPACE] = false;
    gInput.pressed['W'] = false;
    gInput.pressed[VK_UP] = false;

    gPlayer.velocity.y += 345.0f * dt;
    bool parachuteHeld = HasPerk(PerkParachute) &&
                         !gPlayer.grounded &&
                         !gPlayer.jumpAvailable &&
                         gPlayer.velocity.y > 0.0f &&
                         (gInput.keys[VK_SPACE] || gInput.keys['W'] || gInput.keys[VK_UP]);
    if (parachuteHeld)
    {
        gParachuteHoldTimer += dt;
        float weaken = Clamp(gParachuteHoldTimer / 2.2f, 0.0f, 1.0f);
        float fallCap = 44.0f + weaken * 92.0f;
        if (gPlayer.velocity.y > fallCap)
            gPlayer.velocity.y = MoveTowards(gPlayer.velocity.y, fallCap, 620.0f * dt);
        if (((int)(gMapTime * 18.0f) & 1) == 0)
            SpawnBurst({ gPlayer.position.x, gPlayer.position.y + 7.0f },
                       Rgb(118, 52, 72), 1);
    }
    else if (gPlayer.grounded || gPlayer.jumpAvailable)
    {
        gParachuteHoldTimer = 0.0f;
    }
    else
    {
        gParachuteHoldTimer = MoveTowards(gParachuteHoldTimer, 0.0f, dt * 0.35f);
    }
    gPlayer.velocity.y = Clamp(gPlayer.velocity.y, -215.0f, 190.0f);

    MovePlayerX(gPlayer.velocity.x * dt);
    MovePlayerY(gPlayer.velocity.y * dt);
    CheckPlayerShopPortalTouch();
    if (gShopOpen) return;
    if (!timeStopped)
        CheckPlayerDamage();
}

static void PutPixel(int x, int y, uint32_t color)
{
    x += gRenderOffsetX;
    y += gRenderOffsetY;
    if (x < 0 || x >= kBufferW || y < 0 || y >= kBufferH) return;
    gPixels[y * kBufferW + x] = color;
}

static void Clear(uint32_t color)
{
    for (int i = 0; i < kBufferW * kBufferH; ++i)
        gPixels[i] = color;
}

static void FillRect(int x, int y, int w, int h, uint32_t color)
{
    x += gRenderOffsetX;
    y += gRenderOffsetY;
    int x0 = (int)Clamp((float)x, 0.0f, (float)kBufferW);
    int y0 = (int)Clamp((float)y, 0.0f, (float)kBufferH);
    int x1 = (int)Clamp((float)(x + w), 0.0f, (float)kBufferW);
    int y1 = (int)Clamp((float)(y + h), 0.0f, (float)kBufferH);

    for (int py = y0; py < y1; ++py)
        for (int px = x0; px < x1; ++px)
            gPixels[py * kBufferW + px] = color;
}

static float EdgeFunction(Vec2 a, Vec2 b, Vec2 point)
{
    return (point.x - a.x) * (b.y - a.y) -
           (point.y - a.y) * (b.x - a.x);
}

static void FillTriangle(Vec2 a, Vec2 b, Vec2 c, uint32_t color)
{
    float minX = a.x < b.x ? a.x : b.x;
    if (c.x < minX) minX = c.x;
    float maxX = a.x > b.x ? a.x : b.x;
    if (c.x > maxX) maxX = c.x;
    float minY = a.y < b.y ? a.y : b.y;
    if (c.y < minY) minY = c.y;
    float maxY = a.y > b.y ? a.y : b.y;
    if (c.y > maxY) maxY = c.y;

    int x0 = (int)Clamp(minX, 0.0f, (float)(kBufferW - 1));
    int x1 = (int)Clamp(maxX + 1.0f, 0.0f, (float)(kBufferW - 1));
    int y0 = (int)Clamp(minY, 0.0f, (float)(kBufferH - 1));
    int y1 = (int)Clamp(maxY + 1.0f, 0.0f, (float)(kBufferH - 1));

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            Vec2 point = { (float)x + 0.5f, (float)y + 0.5f };
            float e0 = EdgeFunction(a, b, point);
            float e1 = EdgeFunction(b, c, point);
            float e2 = EdgeFunction(c, a, point);
            bool hasNegative = e0 < 0.0f || e1 < 0.0f || e2 < 0.0f;
            bool hasPositive = e0 > 0.0f || e1 > 0.0f || e2 > 0.0f;
            if (!(hasNegative && hasPositive)) PutPixel(x, y, color);
        }
    }
}

static void FillCircle(int cx, int cy, int radius, uint32_t color)
{
    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            if (x * x + y * y <= radius * radius)
                PutPixel(cx + x, cy + y, color);
        }
    }
}

static void DrawCooldownRing(int cx, int cy, int radius, float remainingRatio,
                             uint32_t activeColor, uint32_t backColor)
{
    remainingRatio = Clamp(remainingRatio, 0.0f, 1.0f);
    const int segments = 32;
    for (int i = 0; i < segments; ++i)
    {
        float angle = -1.5707963f + 6.2831853f * (float)i / (float)segments;
        int x = cx + (int)((float)cos(angle) * (float)radius);
        int y = cy + (int)((float)sin(angle) * (float)radius);
        PutPixel(x, y, backColor);
        if ((float)i / (float)segments <= remainingRatio)
            PutPixel(x, y, activeColor);
    }
    for (int i = 0; i < segments; i += 4)
    {
        float angle = -1.5707963f + 6.2831853f * (float)i / (float)segments;
        int x = cx + (int)((float)cos(angle) * (float)(radius + 1));
        int y = cy + (int)((float)sin(angle) * (float)(radius + 1));
        PutPixel(x, y, backColor);
    }
}

static void DrawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = AbsInt(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -AbsInt(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;)
    {
        PutPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = err * 2;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

static void DrawHookSprite(Vec2 tip, Vec2 direction, uint32_t color)
{
    direction = Normalize(direction);
    if (direction.x == 0.0f && direction.y == 0.0f)
        direction = { 1.0f, 0.0f };
    Vec2 right = { -direction.y, direction.x };
    uint32_t darkMetal = Rgb(18, 12, 22);
    uint32_t highlight = Rgb(232, 222, 214);

    Vec2 base =
    {
        tip.x - direction.x * 7.0f,
        tip.y - direction.y * 7.0f
    };
    Vec2 spine =
    {
        tip.x - direction.x * 3.0f,
        tip.y - direction.y * 3.0f
    };
    Vec2 barb =
    {
        base.x + right.x * 5.0f - direction.x * 2.0f,
        base.y + right.y * 5.0f - direction.y * 2.0f
    };
    Vec2 clawTip =
    {
        tip.x + direction.x * 2.0f,
        tip.y + direction.y * 2.0f
    };
    Vec2 leftEdge =
    {
        base.x - right.x * 2.0f,
        base.y - right.y * 2.0f
    };
    Vec2 rightEdge =
    {
        base.x + right.x * 2.0f,
        base.y + right.y * 2.0f
    };

    FillTriangle(leftEdge, rightEdge, clawTip, darkMetal);
    DrawLine((int)base.x, (int)base.y, (int)clawTip.x, (int)clawTip.y, color);
    DrawLine((int)spine.x, (int)spine.y, (int)barb.x, (int)barb.y, color);
    DrawLine((int)barb.x, (int)barb.y,
             (int)(barb.x - right.x * 2.0f + direction.x * 2.0f),
             (int)(barb.y - right.y * 2.0f + direction.y * 2.0f),
             color);
    PutPixel((int)clawTip.x, (int)clawTip.y, highlight);
    PutPixel((int)(base.x - right.x), (int)(base.y - right.y), highlight);
}

static int PlatformGoreAttachY(const Platform& platform, int px)
{
    const RectF& rect = platform.bounds;
    int x = (int)rect.x;
    int y = (int)rect.y;
    int w = (int)rect.w;
    int h = (int)rect.h;
    if (h < 5) h = 5;
    if (w <= 1) return y + h - 1;

    float u = Clamp(((float)px - (float)x) / (float)(w - 1), 0.0f, 1.0f);
    switch (platform.shape)
    {
    case PlatformWedgeLeft:
        return y + 2 + (int)((float)(h - 3) * (1.0f - u));
    case PlatformWedgeRight:
        return y + 2 + (int)((float)(h - 3) * u);
    case PlatformStepped:
        if (px < x + w / 3) return y + h - 1;
        if (px < x + (w * 2) / 3) return y + 3 + ((h - 3) * 2) / 3 - 1;
        return y + 3 + (h - 3) / 3;
    case PlatformJagged:
    {
        int local = (px - x) % 8;
        if (local < 0) local += 8;
        int distanceFromCenter = AbsInt(local - 4);
        return y + 3 + ((h - 4) * (4 - distanceFromCenter)) / 4;
    }
    default:
        return y + h - 1;
    }
}

static void DrawPlatformShape(const Platform& platform, uint32_t color, uint32_t edgeColor)
{
    const RectF& rect = platform.bounds;
    int x = (int)rect.x;
    int y = (int)rect.y;
    int w = (int)rect.w;
    int h = (int)rect.h;
    if (h < 5) h = 5;

    switch (platform.shape)
    {
    case PlatformWedgeLeft:
        FillRect(x, y, w, 3, edgeColor);
        FillTriangle({ (float)x, (float)(y + 2) },
                     { (float)(x + w), (float)(y + 2) },
                     { (float)x, (float)(y + h) }, color);
        break;

    case PlatformWedgeRight:
        FillRect(x, y, w, 3, edgeColor);
        FillTriangle({ (float)x, (float)(y + 2) },
                     { (float)(x + w), (float)(y + 2) },
                     { (float)(x + w), (float)(y + h) }, color);
        break;

    case PlatformStepped:
        FillRect(x, y, w, 3, edgeColor);
        FillRect(x, y + 3, w / 3, h - 3, color);
        FillRect(x + w / 3, y + 3, w / 3, (h - 3) * 2 / 3, color);
        FillRect(x + (w * 2) / 3, y + 3, w - (w * 2) / 3, (h - 3) / 3 + 1, color);
        break;

    case PlatformJagged:
        FillRect(x, y, w, 4, edgeColor);
        for (int tooth = x; tooth < x + w; tooth += 8)
            FillTriangle({ (float)tooth, (float)(y + 3) },
                         { (float)(tooth + 8), (float)(y + 3) },
                         { (float)(tooth + 4), (float)(y + h) }, color);
        break;

    default:
        FillRect(x, y, w, h, color);
        DrawLine(x, y, x + w - 1, y, edgeColor);
        break;
    }

    uint32_t shadow = Rgb(24, 15, 25);
    uint32_t darkCrack = Rgb(42, 24, 38);
    DrawLine(x, y + h - 1, x + w - 1, y + h - 1, shadow);
    DrawLine(x, y + 1, x, y + h - 2, shadow);
    DrawLine(x + w - 1, y + 1, x + w - 1, y + h - 2, shadow);
    for (int bolt = x + 6; bolt < x + w - 4; bolt += 14)
    {
        PutPixel(bolt, y + 1, edgeColor);
        if (h > 7) PutPixel(bolt + 1, y + h - 3, darkCrack);
    }
    if (w > 30)
    {
        int crack = x + (w * 3) / 5;
        DrawLine(crack, y + 2, crack - 2, y + h - 2, darkCrack);
        PutPixel(crack + 2, y + 3, darkCrack);
    }

    uint32_t blood = Rgb(122, 3, 22);
    uint32_t bloodLight = Rgb(184, 14, 34);
    uint32_t flesh = Rgb(96, 43, 48);
    int seed = platform.goreSeed == 0 ? x * 19 + y * 31 + w * 7 : platform.goreSeed;
    int clots = w > 48 ? 4 : (w > 28 ? 3 : 2);
    for (int i = 0; i < clots; ++i)
    {
        int hash = seed + i * 7919;
        int span = w > 10 ? w - 8 : w;
        int gx = x + 4 + (hash & 0x7fffffff) % (span > 1 ? span : 1);
        int drip = 2 + ((hash >> 5) & 3);
        uint32_t clotColor = (i & 1) ? blood : flesh;
        int attachY = PlatformGoreAttachY(platform, gx);
        FillCircle(gx, attachY, 2, clotColor);
        DrawLine(gx, attachY, gx, attachY + drip, blood);
        if (((hash >> 9) & 3) == 0)
            PutPixel(gx + 1, attachY + drip + 1, bloodLight);
    }
    if (w > 22)
    {
        int meatX = x + 5 + (seed & 0x7fffffff) % (w - 12);
        int attachY = PlatformGoreAttachY(platform, meatX);
        FillTriangle({ (float)meatX - 4.0f, (float)attachY },
                     { (float)meatX + 5.0f, (float)attachY },
                     { (float)meatX + 1.0f, (float)attachY + 6.0f },
                     flesh);
        DrawLine(meatX - 3, attachY, meatX + 3, attachY + 3, blood);
    }
}

static uint32_t BlendColor(uint32_t from, uint32_t to, float amount);

static uint32_t PlayerSpriteColor(char code, uint32_t accentColor)
{
    switch (code)
    {
    case 'K': return Rgb(10, 10, 16);      // outline
    case 'W': return Rgb(104, 18, 28);     // legacy hair
    case 'w': return Rgb(104, 18, 28);     // legacy hair
    case 'S': return Rgb(122, 78, 64);     // legacy face
    case 's': return Rgb(122, 78, 64);     // legacy face
    case 'M': return Rgb(38, 42, 52);      // legacy mask
    case 'm': return Rgb(18, 20, 28);      // legacy mask shade
    case 'V': return Rgb(132, 166, 172);   // legacy visor
    case 'C': return Rgb(36, 30, 38);      // scorched hull
    case 'c': return Rgb(36, 30, 38);      // hull
    case 'L': return Rgb(156, 132, 112);   // bone-metal highlight
    case 'A': return accentColor;          // jump-state core / engine
    case 'a': return BlendColor(accentColor, Rgb(0, 0, 0), 0.46f);
    case 'P': return Rgb(24, 18, 25);      // engine block
    case 'p': return Rgb(24, 18, 25);      // engine block
    case 'B': return Rgb(9, 7, 12);        // thruster / deep metal
    case 'G': return Rgb(218, 194, 150);   // glove / hand
    default: return 0;
    }
}

static bool BloodBatteryAirJumpReady()
{
    return HasPerk(PerkBloodBattery) &&
           !gPlayer.grounded &&
           !gPlayer.jumpAvailable &&
           gAirJumpsUsed < 1;
}

static void DrawPlayerLongScarf(uint32_t accentColor, Vec2 center, Vec2 forward)
{
    uint32_t scarfShade = BlendColor(accentColor, Rgb(0, 0, 0), 0.42f);
    uint32_t scarfDark = BlendColor(accentColor, Rgb(0, 0, 0), 0.62f);
    Vec2 previous =
    {
        center.x - forward.x * 7.0f,
        center.y - forward.y * 7.0f
    };
    for (int i = 0; i < (int)(sizeof(gCloakSegments) / sizeof(gCloakSegments[0])); ++i)
    {
        Vec2 point = gCloakSegments[i];
        uint32_t color = i < 2 ? accentColor : (i < 5 ? scarfShade : scarfDark);
        int x0 = (int)previous.x;
        int y0 = (int)previous.y;
        int x1 = (int)point.x;
        int y1 = (int)point.y;
        DrawLine(x0, y0, x1, y1, color);
        if (AbsInt(x1 - x0) >= AbsInt(y1 - y0))
        {
            DrawLine(x0, y0 - 1, x1, y1 - 1, color);
            DrawLine(x0, y0 + 1, x1, y1 + 1, color);
        }
        else
        {
            DrawLine(x0 - 1, y0, x1 - 1, y1, color);
            DrawLine(x0 + 1, y0, x1 + 1, y1, color);
        }
        previous = point;
    }
}

static void DrawPlayerHeroSprite(uint32_t accentColor, Vec2 forward, bool boosting)
{
    forward = Normalize(forward);
    Vec2 right = { -forward.y, forward.x };
    Vec2 center = { gPlayer.position.x, gPlayer.position.y - 7.0f };
    bool hookAction = gHook.mode == HookFlying ||
                      gHook.mode == HookPullTerrain ||
                      gHook.mode == HookPullEnemy ||
                      gHook.mode == HookSuperOrb ||
                      gHook.mode == HookShopPortal;
    bool dashAction = gHook.dashing || gSuperJumpTimer > 0.0f;
    bool falling = !gPlayer.grounded && gPlayer.velocity.y > 28.0f && !boosting;
    bool jumping = !gPlayer.grounded && gPlayer.velocity.y < -24.0f && !boosting;
    float noseLength = dashAction ? 12.6f : (hookAction ? 11.6f : (falling ? 9.0f : 10.0f));
    float tailLength = dashAction ? 5.4f : (jumping ? 8.4f : 7.0f);
    float wingSpan = dashAction ? 5.9f : (falling ? 8.4f : (hookAction ? 7.7f : 7.0f));
    float bodyBack = dashAction ? 0.2f : (falling ? -1.5f : -1.0f);
    uint32_t outline = Rgb(8, 8, 14);
    uint32_t hull = PlayerSpriteColor('C', accentColor);
    uint32_t hullLight = PlayerSpriteColor('L', accentColor);
    uint32_t deep = PlayerSpriteColor('B', accentColor);
    uint32_t canopy = Rgb(152, 42, 54);
    uint32_t canopyDark = Rgb(56, 12, 20);
    uint32_t ember = BlendColor(accentColor, Rgb(238, 18, 22), 0.38f);
    uint32_t scarfAccent = BloodBatteryAirJumpReady()
        ? Rgb(180, 46, 76)
        : accentColor;

    DrawPlayerLongScarf(scarfAccent, center, forward);

    Vec2 nose = { center.x + forward.x * noseLength, center.y + forward.y * noseLength };
    Vec2 tail = { center.x - forward.x * tailLength, center.y - forward.y * tailLength };
    Vec2 leftWing = { center.x + forward.x * bodyBack - right.x * wingSpan,
                      center.y + forward.y * bodyBack - right.y * wingSpan };
    Vec2 rightWing = { center.x + forward.x * bodyBack + right.x * wingSpan,
                       center.y + forward.y * bodyBack + right.y * wingSpan };
    Vec2 leftNose = { center.x + forward.x * 4.8f - right.x * 2.8f,
                      center.y + forward.y * 4.8f - right.y * 2.8f };
    Vec2 rightNose = { center.x + forward.x * 4.8f + right.x * 2.8f,
                       center.y + forward.y * 4.8f + right.y * 2.8f };

    FillTriangle(nose, leftWing, rightWing, outline);
    FillTriangle({ tail.x - right.x * 3.0f, tail.y - right.y * 3.0f },
                 { tail.x + right.x * 3.0f, tail.y + right.y * 3.0f },
                 { center.x, center.y }, outline);
    FillTriangle({ nose.x - forward.x * 1.0f, nose.y - forward.y * 1.0f },
                 { leftWing.x + right.x * 1.0f, leftWing.y + right.y * 1.0f },
                 { center.x, center.y }, hull);
    FillTriangle({ nose.x - forward.x * 1.0f, nose.y - forward.y * 1.0f },
                 { center.x, center.y },
                 { rightWing.x - right.x * 1.0f, rightWing.y - right.y * 1.0f },
                 hull);
    FillTriangle(center,
                 { tail.x - right.x * 2.0f, tail.y - right.y * 2.0f },
                 { tail.x + right.x * 2.0f, tail.y + right.y * 2.0f },
                 deep);

    DrawLine((int)nose.x, (int)nose.y, (int)leftWing.x, (int)leftWing.y, outline);
    DrawLine((int)nose.x, (int)nose.y, (int)rightWing.x, (int)rightWing.y, outline);
    DrawLine((int)leftWing.x, (int)leftWing.y, (int)tail.x, (int)tail.y, outline);
    DrawLine((int)rightWing.x, (int)rightWing.y, (int)tail.x, (int)tail.y, outline);
    DrawLine((int)leftNose.x, (int)leftNose.y, (int)center.x, (int)center.y, hullLight);
    DrawLine((int)rightNose.x, (int)rightNose.y, (int)center.x, (int)center.y, hullLight);
    DrawLine((int)leftWing.x, (int)leftWing.y,
             (int)(leftWing.x - right.x * 2.0f - forward.x * 1.0f),
             (int)(leftWing.y - right.y * 2.0f - forward.y * 1.0f),
             Rgb(164, 24, 36));
    DrawLine((int)rightWing.x, (int)rightWing.y,
             (int)(rightWing.x + right.x * 2.0f - forward.x * 1.0f),
             (int)(rightWing.y + right.y * 2.0f - forward.y * 1.0f),
             Rgb(164, 24, 36));

    Vec2 canopyTip = { center.x + forward.x * 5.8f, center.y + forward.y * 5.8f };
    Vec2 canopyBack = { center.x + forward.x * 0.8f, center.y + forward.y * 0.8f };
    Vec2 canopyLeft = { canopyBack.x - right.x * 2.0f, canopyBack.y - right.y * 2.0f };
    Vec2 canopyRight = { canopyBack.x + right.x * 2.0f, canopyBack.y + right.y * 2.0f };
    FillTriangle(canopyTip, canopyLeft, canopyRight, canopy);
    DrawLine((int)canopyLeft.x, (int)canopyLeft.y,
             (int)canopyRight.x, (int)canopyRight.y, canopyDark);

    Vec2 coreA = { center.x - right.x * 1.4f, center.y - right.y * 1.4f };
    Vec2 coreB = { center.x + right.x * 1.4f, center.y + right.y * 1.4f };
    Vec2 coreC = { center.x - forward.x * 2.6f, center.y - forward.y * 2.6f };
    FillTriangle(coreA, coreB, coreC, boosting ? accentColor : ember);
    Vec2 engineA = { tail.x - right.x * 1.8f, tail.y - right.y * 1.8f };
    Vec2 engineB = { tail.x + right.x * 1.8f, tail.y + right.y * 1.8f };
    Vec2 engineTip =
    {
        tail.x - forward.x * (boosting ? 5.0f : 2.5f),
        tail.y - forward.y * (boosting ? 5.0f : 2.5f)
    };
    FillTriangle(engineA, engineB, engineTip, boosting ? accentColor : Rgb(104, 16, 26));
    if (dashAction)
    {
        DrawLine((int)(tail.x - right.x * 2.8f), (int)(tail.y - right.y * 2.8f),
                 (int)(tail.x - forward.x * 8.0f - right.x * 1.2f),
                 (int)(tail.y - forward.y * 8.0f - right.y * 1.2f),
                 BlendColor(accentColor, Rgb(255, 255, 255), 0.18f));
        DrawLine((int)(tail.x + right.x * 2.8f), (int)(tail.y + right.y * 2.8f),
                 (int)(tail.x - forward.x * 8.0f + right.x * 1.2f),
                 (int)(tail.y - forward.y * 8.0f + right.y * 1.2f),
                 BlendColor(accentColor, Rgb(255, 255, 255), 0.18f));
    }
    else if (jumping)
    {
        PutPixel((int)(tail.x - right.x * 2.0f), (int)(tail.y - right.y * 2.0f), accentColor);
        PutPixel((int)(tail.x + right.x * 2.0f), (int)(tail.y + right.y * 2.0f), accentColor);
    }
    PutPixel((int)nose.x, (int)nose.y, Rgb(222, 186, 142));
}

static void DrawHaloRing(int cx, int cy, uint32_t light, uint32_t shade)
{
    DrawLine(cx - 6, cy - 1, cx - 3, cy - 3, shade);
    DrawLine(cx - 3, cy - 3, cx + 3, cy - 3, shade);
    DrawLine(cx + 3, cy - 3, cx + 6, cy - 1, shade);
    DrawLine(cx - 7, cy, cx - 3, cy + 2, light);
    DrawLine(cx - 3, cy + 2, cx + 3, cy + 2, light);
    DrawLine(cx + 3, cy + 2, cx + 7, cy, light);
    PutPixel(cx - 6, cy - 1, light);
    PutPixel(cx + 6, cy - 1, light);
}

static void DrawFallenAngelEnemy(const RectF& enemy, uint32_t bodyColor,
                                 uint32_t wingColor, uint32_t eyeColor)
{
    int cx = (int)(enemy.x + enemy.w * 0.5f);
    int cy = (int)(enemy.y + enemy.h * 0.5f);
    int flap = ((int)(gMapTime * 10.0f + enemy.x * 0.13f)) & 3;
    int wingLift = flap == 0 ? -2 : (flap == 1 ? -1 : (flap == 2 ? 1 : 0));
    int wingSpread = flap == 2 ? 2 : (flap == 0 ? -1 : 0);
    uint32_t bodyShadow = BlendColor(bodyColor, Rgb(0, 0, 0), 0.42f);
    uint32_t bodyLight = BlendColor(bodyColor, Rgb(255, 180, 150), 0.24f);
    DrawHaloRing(cx, cy - 11, Rgb(228, 68, 42), Rgb(82, 8, 18));
    PutPixel(cx + 8, cy - 10, Rgb(228, 68, 42));
    PutPixel(cx - 8, cy - 8, Rgb(82, 8, 18));
    FillTriangle({ (float)cx - 4.0f, (float)cy },
                 { (float)(cx - 9 - wingSpread), (float)(cy - 5 + wingLift) },
                 { (float)(cx - 8 - wingSpread), (float)(cy + 4 + wingLift) }, wingColor);
    DrawLine(cx - 5, cy - 1, cx - 9 - wingSpread, cy - 5 + wingLift, BlendColor(wingColor, Rgb(255, 160, 140), 0.16f));
    DrawLine(cx - 6, cy + 2, cx - 8 - wingSpread, cy + 4 + wingLift, bodyShadow);
    FillTriangle({ (float)cx + 4.0f, (float)cy },
                 { (float)(cx + 9 + wingSpread), (float)(cy - 5 + wingLift) },
                 { (float)(cx + 8 + wingSpread), (float)(cy + 4 + wingLift) }, wingColor);
    DrawLine(cx + 5, cy - 1, cx + 9 + wingSpread, cy - 5 + wingLift, BlendColor(wingColor, Rgb(255, 160, 140), 0.16f));
    DrawLine(cx + 6, cy + 2, cx + 8 + wingSpread, cy + 4 + wingLift, bodyShadow);
    FillCircle(cx, cy, 6, bodyColor);
    FillRect(cx - 5, cy + 1, 11, 5, bodyShadow);
    FillCircle(cx - 2, cy - 2, 2, bodyLight);
    FillCircle(cx, cy, 4, eyeColor);
    FillCircle(cx, cy, 2, Rgb(22, 4, 8));
    DrawLine(cx - 5, cy + 5, cx - 2, cy + 9, wingColor);
    DrawLine(cx + 5, cy + 5, cx + 2, cy + 9, wingColor);
}

static void DrawPureAngelEnemy(const RectF& enemy, uint32_t bodyColor,
                               uint32_t wingColor, uint32_t eyeColor)
{
    int cx = (int)(enemy.x + enemy.w * 0.5f);
    int cy = (int)(enemy.y + enemy.h * 0.5f);
    int flap = ((int)(gMapTime * 9.0f + enemy.x * 0.11f)) & 3;
    int wingLift = flap == 0 ? -2 : (flap == 1 ? -1 : (flap == 2 ? 1 : 0));
    int wingSpread = flap == 2 ? 2 : (flap == 0 ? -1 : 0);
    uint32_t bodyShadow = BlendColor(bodyColor, Rgb(80, 18, 34), 0.24f);
    uint32_t bodyLight = BlendColor(bodyColor, Rgb(255, 246, 220), 0.34f);
    DrawHaloRing(cx, cy - 11, Rgb(255, 222, 120), Rgb(164, 108, 54));
    FillTriangle({ (float)cx - 3.0f, (float)cy },
                 { (float)(cx - 8 - wingSpread), (float)(cy - 6 + wingLift) },
                 { (float)(cx - 8 - wingSpread), (float)(cy + 3 + wingLift) }, wingColor);
    DrawLine(cx - 5, cy - 1, cx - 8 - wingSpread, cy - 6 + wingLift, bodyLight);
    FillTriangle({ (float)cx + 3.0f, (float)cy },
                 { (float)(cx + 8 + wingSpread), (float)(cy - 6 + wingLift) },
                 { (float)(cx + 8 + wingSpread), (float)(cy + 3 + wingLift) }, wingColor);
    DrawLine(cx + 5, cy - 1, cx + 8 + wingSpread, cy - 6 + wingLift, bodyLight);
    FillCircle(cx, cy, 6, bodyColor);
    FillRect(cx - 5, cy + 2, 11, 4, bodyShadow);
    FillCircle(cx - 2, cy - 2, 2, bodyLight);
    FillCircle(cx - 2, cy - 1, 2, eyeColor);
    FillCircle(cx + 2, cy - 1, 2, eyeColor);
    FillTriangle({ (float)cx - 4.0f, (float)cy },
                 { (float)cx + 4.0f, (float)cy },
                 { (float)cx, (float)cy + 5.0f },
                 eyeColor);
    PutPixel(cx - 1, cy - 2, Rgb(255, 220, 232));
    PutPixel(cx + 1, cy - 2, Rgb(255, 220, 232));
}

static void DrawCrawlerEnemy(const RectF& enemy, uint32_t bodyColor,
                             uint32_t edgeColor, uint32_t eyeColor,
                             bool facingRight)
{
    int x = (int)enemy.x;
    int y = (int)enemy.y;
    int crawlFrame = ((int)(gMapTime * 12.0f + enemy.x * 0.17f)) & 3;
    int bob = (crawlFrame == 1 || crawlFrame == 2) ? 1 : 0;
    int legKick = (crawlFrame & 1) ? 1 : -1;
    int cy = y + 8 + bob;
    int side = facingRight ? 1 : -1;
    int tailX = facingRight ? x + 3 : x + 13;
    int midX = facingRight ? x + 7 : x + 9;
    int headX = facingRight ? x + 12 : x + 4;
    uint32_t shadow = BlendColor(bodyColor, Rgb(0, 0, 0), 0.48f);
    uint32_t light = BlendColor(bodyColor, Rgb(255, 170, 110), 0.20f);
    FillCircle(tailX, cy + 1, 4, shadow);
    FillCircle(midX, cy, 5, bodyColor);
    FillCircle(headX, cy - 1, 5, edgeColor);
    FillRect((facingRight ? x + 2 : x + 4), cy + 2, 11, 3, shadow);
    FillCircle(midX - side * 2, cy - 2, 2, light);
    FillCircle(headX - side, cy - 3, 2, light);
    FillTriangle({ (float)(headX + side * 4), (float)(cy - 2) },
                 { (float)(headX + side * 4), (float)(cy + 2) },
                 { (float)(headX + side * 8), (float)cy },
                 Rgb(42, 2, 6));
    PutPixel(headX + side * 3, cy - 2, eyeColor);
    for (int crease = 0; crease < 3; ++crease)
    {
        int lx = facingRight ? x + 4 + crease * 3 : x + 12 - crease * 3;
        DrawLine(lx, cy - 3, lx - side, cy + 3, shadow);
    }
    DrawLine(x + 2, cy + 5, x + 5, cy + 5 + legKick, shadow);
    DrawLine(x + 6, cy + 5, x + 9, cy + 5 - legKick, shadow);
    DrawLine(x + 10, cy + 5, x + 13, cy + 5 + legKick, shadow);
}

static void DrawWallMawTrap(const WallTrap& trap, uint32_t flesh,
                            uint32_t edge, uint32_t eye, uint32_t barrel)
{
    int x = (int)trap.position.x;
    int y = (int)trap.position.y;
    int side = trap.firesRight ? 1 : -1;
    FillCircle(x, y, 6, flesh);
    FillCircle(x + side * 2, y - 2, 2, eye);
    FillRect(x - (trap.firesRight ? 1 : 7), y + 2, 8, 4, Rgb(34, 2, 6));
    DrawLine(x, y + 1, x + side * 12, y + 1, barrel);
    DrawLine(x + side * 3, y + 5, x + side * 7, y + 9, edge);
    DrawLine(x + side * 3, y - 5, x + side * 7, y - 9, edge);
}

static void PipeEdgesForScreenY(int y, float* leftOut, float* rightOut)
{
    float meters = gViewHeightPixels / kPixelsPerMeter +
                   (kCameraLockY - (float)y) / kPixelsPerMeter;
    float center = kBufferW * 0.5f;
    float halfWidth = 100.0f;
    PipeProfileForMeters(meters, &center, &halfWidth);
    float left = Clamp(center - halfWidth, 8.0f, (float)kBufferW - 80.0f);
    float right = Clamp(center + halfWidth, 80.0f, (float)kBufferW - 8.0f);
    if (right - left < 112.0f)
    {
        right = left + 112.0f;
        if (right > (float)kBufferW - 8.0f)
        {
            right = (float)kBufferW - 8.0f;
            left = right - 112.0f;
        }
    }
    *leftOut = left;
    *rightOut = right;
}

static void DrawPipeWalls(uint32_t color, uint32_t edgeColor, uint32_t veinColor)
{
    for (int y = -12; y < kBufferH + 12; y += 10)
    {
        int y1 = y + 10;
        float left0, right0, left1, right1;
        PipeEdgesForScreenY(y, &left0, &right0);
        PipeEdgesForScreenY(y1, &left1, &right1);

        FillTriangle({ 0.0f, (float)y }, { left0, (float)y },
                     { left1, (float)y1 }, color);
        FillTriangle({ 0.0f, (float)y }, { left1, (float)y1 },
                     { 0.0f, (float)y1 }, color);
        FillTriangle({ right0, (float)y }, { (float)kBufferW, (float)y },
                     { (float)kBufferW, (float)y1 }, color);
        FillTriangle({ right0, (float)y }, { (float)kBufferW, (float)y1 },
                     { right1, (float)y1 }, color);

        DrawLine((int)left0, y, (int)left1, y1, edgeColor);
        DrawLine((int)right0, y, (int)right1, y1, edgeColor);
        if (((y / 10) & 3) == 0)
        {
            DrawLine((int)(left0 - 10.0f), y + 2, (int)(left1 - 2.0f), y1 - 2, veinColor);
            DrawLine((int)(right0 + 10.0f), y + 2, (int)(right1 + 2.0f), y1 - 2, veinColor);
        }

        if (ChapterHasBranchDividers(gChapter))
        {
            float meters = gViewHeightPixels / kPixelsPerMeter +
                           (kCameraLockY - (float)y) / kPixelsPerMeter;
            float branch = BranchAmountForMeters(meters);
            if (branch > 0.0f)
            {
                float center0 = PipeCenterForMeters(meters);
                float center1 = PipeCenterForMeters(
                    gViewHeightPixels / kPixelsPerMeter +
                    (kCameraLockY - (float)y1) / kPixelsPerMeter);
                int side = BranchSideForMeters(meters);
                center0 += (float)side * 18.0f * branch;
                center1 += (float)side * 18.0f * branch;
                float half = branch * 8.0f;
                FillTriangle({ center0 - half, (float)y },
                             { center0 + half, (float)y },
                             { center1 + half, (float)y1 }, color);
                FillTriangle({ center0 - half, (float)y },
                             { center1 + half, (float)y1 },
                             { center1 - half, (float)y1 }, color);
                DrawLine((int)(center0 - half), y, (int)(center1 - half), y1, edgeColor);
                DrawLine((int)(center0 + half), y, (int)(center1 + half), y1, edgeColor);
            }
        }
    }
}

static void DrawDigit(int x, int y, int digit, int scale, uint32_t color)
{
    static const uint8_t masks[10] =
    {
        0x3F, 0x06, 0x5B, 0x4F, 0x66,
        0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };
    if (digit < 0 || digit > 9) return;
    uint8_t mask = masks[digit];
    int thickness = scale;
    int length = 3 * scale;
    if (mask & 0x01) FillRect(x + scale, y, length, thickness, color);
    if (mask & 0x02) FillRect(x + 4 * scale, y + scale, thickness, length, color);
    if (mask & 0x04) FillRect(x + 4 * scale, y + 5 * scale, thickness, length, color);
    if (mask & 0x08) FillRect(x + scale, y + 8 * scale, length, thickness, color);
    if (mask & 0x10) FillRect(x, y + 5 * scale, thickness, length, color);
    if (mask & 0x20) FillRect(x, y + scale, thickness, length, color);
    if (mask & 0x40) FillRect(x + scale, y + 4 * scale, length, thickness, color);
}

static void DrawHeartIcon(int x, int y, int units, uint32_t color, uint32_t emptyColor)
{
    if (units < 0) units = 0;
    if (units > kFullHeartUnits) units = kFullHeartUnits;
    FillRect(x + 1, y, 2, 2, emptyColor);
    FillRect(x + 5, y, 2, 2, emptyColor);
    FillRect(x, y + 2, 8, 3, emptyColor);
    FillRect(x + 1, y + 5, 6, 2, emptyColor);
    FillRect(x + 2, y + 7, 4, 1, emptyColor);
    PutPixel(x + 3, y + 8, emptyColor);

    if (units <= 0) return;
    if (units >= kFullHeartUnits)
    {
        FillRect(x + 1, y, 2, 2, color);
        FillRect(x + 5, y, 2, 2, color);
        FillRect(x, y + 2, 8, 3, color);
        FillRect(x + 1, y + 5, 6, 2, color);
        FillRect(x + 2, y + 7, 4, 1, color);
        PutPixel(x + 3, y + 8, color);
        return;
    }

    FillRect(x + 1, y, 2, 2, color);
    FillRect(x, y + 2, 4, 3, color);
    FillRect(x + 1, y + 5, 3, 2, color);
    FillRect(x + 2, y + 7, 2, 1, color);
    PutPixel(x + 3, y + 8, color);
}

static void DrawPerkIcon(int x, int y, PerkType perk, uint32_t white,
                         uint32_t red, uint32_t dark)
{
    uint32_t gold = Rgb(220, 166, 74);
    uint32_t purple = Rgb(182, 84, 245);
    uint32_t pale = Rgb(210, 210, 225);
    uint32_t blood = Rgb(190, 18, 42);
    switch (perk)
    {
        case PerkCrossNecklace:
            FillCircle(x + 8, y + 3, 3, Rgb(88, 48, 28));
            DrawLine(x + 8, y + 5, x + 8, y + 15, gold);
            DrawLine(x + 4, y + 9, x + 12, y + 9, gold);
            FillRect(x + 7, y + 8, 3, 6, Rgb(116, 72, 36));
            PutPixel(x + 8, y + 2, white);
            PutPixel(x + 11, y + 8, Rgb(255, 220, 128));
            break;
        case PerkParachute:
            FillTriangle({ (float)x + 1.0f, (float)y + 9.0f },
                         { (float)x + 8.0f, (float)y + 1.0f },
                         { (float)x + 15.0f, (float)y + 9.0f },
                         Rgb(116, 48, 78));
            DrawLine(x + 2, y + 9, x + 14, y + 9, Rgb(190, 112, 132));
            DrawLine(x + 5, y + 4, x + 5, y + 10, pale);
            DrawLine(x + 11, y + 4, x + 11, y + 10, pale);
            DrawLine(x + 2, y + 9, x + 8, y + 15, pale);
            DrawLine(x + 14, y + 9, x + 8, y + 15, pale);
            FillRect(x + 6, y + 14, 5, 2, dark);
            break;
        case PerkDevilHeart:
            DrawLine(x + 3, y + 2, x, y, red);
            DrawLine(x + 13, y + 2, x + 16, y, red);
            FillCircle(x + 6, y + 6, 4, Rgb(112, 4, 18));
            FillCircle(x + 10, y + 6, 4, Rgb(112, 4, 18));
            FillCircle(x + 6, y + 6, 3, blood);
            FillCircle(x + 10, y + 6, 3, blood);
            FillTriangle({ (float)x + 2.0f, (float)y + 8.0f },
                         { (float)x + 14.0f, (float)y + 8.0f },
                         { (float)x + 8.0f, (float)y + 15.0f }, blood);
            DrawLine(x + 8, y + 3, x + 6, y + 8, Rgb(255, 116, 82));
            PutPixel(x + 10, y + 5, Rgb(255, 180, 118));
            break;
        case PerkAngelSkin:
            FillTriangle({ (float)x + 8.0f, (float)y + 1.0f },
                         { (float)x + 14.0f, (float)y + 4.0f },
                         { (float)x + 11.0f, (float)y + 14.0f }, Rgb(98, 86, 104));
            FillTriangle({ (float)x + 8.0f, (float)y + 1.0f },
                         { (float)x + 2.0f, (float)y + 4.0f },
                         { (float)x + 5.0f, (float)y + 14.0f }, pale);
            DrawLine(x + 8, y + 2, x + 8, y + 15, white);
            DrawLine(x + 4, y + 5, x + 12, y + 5, gold);
            PutPixel(x + 6, y + 3, white);
            break;
        case PerkBloodBattery:
            FillRect(x + 4, y + 3, 9, 12, Rgb(18, 14, 24));
            DrawLine(x + 5, y + 2, x + 12, y + 2, pale);
            DrawLine(x + 4, y + 3, x + 4, y + 14, white);
            FillRect(x + 6, y + 7, 5, 6, blood);
            DrawLine(x + 9, y + 4, x + 6, y + 9, red);
            DrawLine(x + 6, y + 9, x + 10, y + 9, red);
            DrawLine(x + 10, y + 9, x + 7, y + 14, Rgb(255, 116, 82));
            break;
        case PerkContract:
            FillRect(x + 3, y + 2, 10, 13, Rgb(154, 108, 74));
            FillRect(x + 5, y + 3, 8, 11, Rgb(72, 28, 34));
            DrawLine(x + 6, y + 5, x + 11, y + 5, gold);
            DrawLine(x + 6, y + 8, x + 10, y + 8, red);
            DrawLine(x + 6, y + 11, x + 11, y + 11, gold);
            FillCircle(x + 13, y + 13, 2, blood);
            PutPixel(x + 14, y + 12, Rgb(255, 112, 92));
            break;
        default:
            DrawLine(x + 4, y + 4, x + 12, y + 12, Rgb(70, 54, 64));
            DrawLine(x + 12, y + 4, x + 4, y + 12, Rgb(70, 54, 64));
            break;
    }
    PutPixel(x + 14, y + 2, BlendColor(white, red, 0.18f));
    PutPixel(x + 1, y + 15, BlendColor(dark, Rgb(0, 0, 0), 0.28f));
}

static uint8_t GlyphRow(char character, int row)
{
    static const uint8_t digits[][7] =
    {
        {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
        {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
        {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
        {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
    };
    if (character >= '0' && character <= '9') return digits[character - '0'][row];
    switch (character)
    {
    case 'A': { static const uint8_t g[7] = {14,17,17,31,17,17,17}; return g[row]; }
    case 'B': { static const uint8_t g[7] = {30,17,17,30,17,17,30}; return g[row]; }
    case 'C': { static const uint8_t g[7] = {15,16,16,16,16,16,15}; return g[row]; }
    case 'D': { static const uint8_t g[7] = {30,17,17,17,17,17,30}; return g[row]; }
    case 'E': { static const uint8_t g[7] = {31,16,16,30,16,16,31}; return g[row]; }
    case 'F': { static const uint8_t g[7] = {31,16,16,30,16,16,16}; return g[row]; }
    case 'G': { static const uint8_t g[7] = {15,16,16,19,17,17,15}; return g[row]; }
    case 'H': { static const uint8_t g[7] = {17,17,17,31,17,17,17}; return g[row]; }
    case 'I': { static const uint8_t g[7] = {14,4,4,4,4,4,14}; return g[row]; }
    case 'J': { static const uint8_t g[7] = {7,2,2,2,18,18,12}; return g[row]; }
    case 'K': { static const uint8_t g[7] = {17,18,20,24,20,18,17}; return g[row]; }
    case 'L': { static const uint8_t g[7] = {16,16,16,16,16,16,31}; return g[row]; }
    case 'M': { static const uint8_t g[7] = {17,27,21,21,17,17,17}; return g[row]; }
    case 'N': { static const uint8_t g[7] = {17,25,21,19,17,17,17}; return g[row]; }
    case 'O': { static const uint8_t g[7] = {14,17,17,17,17,17,14}; return g[row]; }
    case 'P': { static const uint8_t g[7] = {30,17,17,30,16,16,16}; return g[row]; }
    case 'Q': { static const uint8_t g[7] = {14,17,17,17,21,18,13}; return g[row]; }
    case 'R': { static const uint8_t g[7] = {30,17,17,30,20,18,17}; return g[row]; }
    case 'S': { static const uint8_t g[7] = {15,16,16,14,1,1,30}; return g[row]; }
    case 'T': { static const uint8_t g[7] = {31,4,4,4,4,4,4}; return g[row]; }
    case 'U': { static const uint8_t g[7] = {17,17,17,17,17,17,14}; return g[row]; }
    case 'V': { static const uint8_t g[7] = {17,17,17,17,17,10,4}; return g[row]; }
    case 'W': { static const uint8_t g[7] = {17,17,17,21,21,21,10}; return g[row]; }
    case 'X': { static const uint8_t g[7] = {17,17,10,4,10,17,17}; return g[row]; }
    case 'Y': { static const uint8_t g[7] = {17,17,10,4,4,4,4}; return g[row]; }
    case 'Z': { static const uint8_t g[7] = {31,1,2,4,8,16,31}; return g[row]; }
    case '-': { static const uint8_t g[7] = {0,0,0,31,0,0,0}; return g[row]; }
    case ':': { static const uint8_t g[7] = {0,4,4,0,4,4,0}; return g[row]; }
    case '/': { static const uint8_t g[7] = {1,1,2,4,8,16,16}; return g[row]; }
    case '%': { static const uint8_t g[7] = {17,18,4,8,16,9,17}; return g[row]; }
    case '(': { static const uint8_t g[7] = {2,4,8,8,8,4,2}; return g[row]; }
    case ')': { static const uint8_t g[7] = {8,4,2,2,2,4,8}; return g[row]; }
    case 'X' + 32: return GlyphRow('X', row);
    }
    return 0;
}

static int TextWidth(const char* text, int scale)
{
    int count = 0;
    while (text[count] != 0) ++count;
    return count > 0 ? count * 6 * scale - scale : 0;
}

static void DrawItalicText(int x, int y, const char* text, int scale, uint32_t color)
{
    for (int character = 0; text[character] != 0; ++character)
    {
        if (text[character] == ' ') continue;
        for (int row = 0; row < 7; ++row)
        {
            uint8_t bits = GlyphRow(text[character], row);
            int shear = ((6 - row) * scale) / 3;
            for (int column = 0; column < 5; ++column)
            {
                if (bits & (1 << (4 - column)))
                    FillRect(x + character * 6 * scale + column * scale + shear,
                             y + row * scale, scale, scale, color);
            }
        }
    }
}

static uint32_t BlendColor(uint32_t from, uint32_t to, float amount)
{
    amount = Clamp(amount, 0.0f, 1.0f);
    int fromR = (from >> 16) & 255;
    int fromG = (from >> 8) & 255;
    int fromB = from & 255;
    int toR = (to >> 16) & 255;
    int toG = (to >> 8) & 255;
    int toB = to & 255;
    return Rgb(
        (uint8_t)(fromR + (toR - fromR) * amount),
        (uint8_t)(fromG + (toG - fromG) * amount),
        (uint8_t)(fromB + (toB - fromB) * amount));
}

static float LerpFloat(float a, float b, float t)
{
    return a + (b - a) * t;
}

static uint32_t Hash2D(int x, int y, int seed)
{
    uint32_t h = (uint32_t)(x * 374761393 + y * 668265263 + seed * 1442695041);
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static float ValueNoise(float x, float y, int seed)
{
    int xi = (int)x;
    int yi = (int)y;
    float tx = x - (float)xi;
    float ty = y - (float)yi;
    tx = SmoothStep(tx);
    ty = SmoothStep(ty);

    float a = (float)(Hash2D(xi, yi, seed) & 255u) / 255.0f;
    float b = (float)(Hash2D(xi + 1, yi, seed) & 255u) / 255.0f;
    float c = (float)(Hash2D(xi, yi + 1, seed) & 255u) / 255.0f;
    float d = (float)(Hash2D(xi + 1, yi + 1, seed) & 255u) / 255.0f;
    return LerpFloat(LerpFloat(a, b, tx), LerpFloat(c, d, tx), ty);
}

static void BlendPixelRaw(int x, int y, uint32_t color, float amount)
{
    if (x < 0 || x >= kBufferW || y < 0 || y >= kBufferH) return;
    amount = Clamp(amount, 0.0f, 1.0f);
    int index = y * kBufferW + x;
    gPixels[index] = BlendColor(gPixels[index], color, amount);
}

static void BlendCircleRaw(int cx, int cy, int radius, uint32_t color, float amount)
{
    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            int distanceSq = x * x + y * y;
            if (distanceSq > radius * radius) continue;
            float falloff = 1.0f - (float)distanceSq / (float)(radius * radius + 1);
            BlendPixelRaw(cx + x, cy + y, color, amount * falloff);
        }
    }
}

static void DrawHeightMarkers(uint32_t color)
{
    float currentMeters = gViewHeightPixels / kPixelsPerMeter;
    int baseMarker = ((int)currentMeters / kHeightMarkerInterval) *
                     kHeightMarkerInterval;
    for (int marker = baseMarker - kHeightMarkerInterval;
         marker <= baseMarker + kHeightMarkerInterval * 2;
         marker += kHeightMarkerInterval)
    {
        if (marker <= 0) continue;
        int y = (int)(kCameraLockY - ((float)marker - currentMeters) * kPixelsPerMeter);
        if (y < 4 || y > kBufferH - 10) continue;
        char label[16];
        wsprintfA(label, "-%dM", marker);
        int width = TextWidth(label, 1);
        DrawItalicText(9, y, label, 1, color);
        DrawItalicText(kBufferW - 9 - width, y, label, 1, color);
        DrawLine(6, y + 3, 8, y + 3, color);
        DrawLine(kBufferW - 9, y + 3, kBufferW - 7, y + 3, color);
    }
}

static void DrawHud(uint32_t white, uint32_t red, uint32_t dark, uint32_t background)
{
    for (int heart = 0; heart < kHeartCount; ++heart)
    {
        int units = gPlayerHealth - heart * kFullHeartUnits;
        DrawHeartIcon(10 + heart * 12, 9, units,
                      Rgb(225, 28, 48), Rgb(55, 35, 48));
    }

    char coinText[24];
    wsprintfA(coinText, "COIN %d", gCoins);
    DrawItalicText(9, 22, coinText, 1, Rgb(220, 166, 74));

    int itemX = 235;
    int itemY = 2;
    FillRect(itemX, itemY, 17, 17, dark);
    DrawLine(itemX, itemY, itemX + 16, itemY, white);
    DrawLine(itemX, itemY + 16, itemX + 16, itemY + 16, white);
    DrawLine(itemX, itemY, itemX, itemY + 16, white);
    DrawLine(itemX + 16, itemY, itemX + 16, itemY + 16, white);
    if (gActiveItem == ActiveDynamite)
    {
        FillRect(itemX + 5, itemY + 5, 4, 9, Rgb(124, 14, 18));
        FillRect(itemX + 9, itemY + 4, 4, 10, Rgb(190, 38, 28));
        DrawLine(itemX + 5, itemY + 5, itemX + 12, itemY + 4, white);
        DrawLine(itemX + 11, itemY + 3, itemX + 14, itemY + 1, Rgb(255, 205, 72));
        PutPixel(itemX + 14, itemY + 1, Rgb(255, 98, 32));
    }
    else if (gActiveItem == ActiveMiniOrb)
    {
        FillCircle(itemX + 9, itemY + 9, 5, Rgb(74, 18, 104));
        FillCircle(itemX + 9, itemY + 9, 3, Rgb(182, 84, 245));
        DrawLine(itemX + 4, itemY + 9, itemX + 14, itemY + 9, Rgb(218, 132, 255));
        DrawLine(itemX + 9, itemY + 4, itemX + 9, itemY + 14, Rgb(118, 46, 182));
        PutPixel(itemX + 9, itemY + 9, white);
    }
    else if (gActiveItem == ActiveBloodChalice)
    {
        DrawLine(itemX + 5, itemY + 5, itemX + 13, itemY + 5, white);
        FillRect(itemX + 6, itemY + 6, 7, 5, Rgb(122, 6, 26));
        FillRect(itemX + 7, itemY + 6, 5, 3, Rgb(210, 18, 48));
        DrawLine(itemX + 9, itemY + 11, itemX + 9, itemY + 14, white);
        DrawLine(itemX + 6, itemY + 14, itemX + 12, itemY + 14, white);
        PutPixel(itemX + 10, itemY + 7, Rgb(255, 92, 92));
    }
    else if (gActiveItem == ActiveHourglass)
    {
        DrawLine(itemX + 5, itemY + 4, itemX + 12, itemY + 4, Rgb(220, 166, 74));
        DrawLine(itemX + 5, itemY + 13, itemX + 12, itemY + 13, Rgb(220, 166, 74));
        DrawLine(itemX + 6, itemY + 5, itemX + 11, itemY + 12, white);
        DrawLine(itemX + 11, itemY + 5, itemX + 6, itemY + 12, white);
        FillRect(itemX + 8, itemY + 7, 2, 4, Rgb(220, 166, 74));
    }
    else if (gActiveItem == ActiveSawBlade)
    {
        FillRect(itemX + 5, itemY + 5, 8, 8, Rgb(180, 188, 196));
        FillRect(itemX + 7, itemY + 7, 4, 4, dark);
        PutPixel(itemX + 9, itemY + 3, white);
        PutPixel(itemX + 9, itemY + 14, white);
        PutPixel(itemX + 3, itemY + 9, white);
        PutPixel(itemX + 14, itemY + 9, white);
    }
    char itemText[8];
    wsprintfA(itemText, "Q %d", gActiveItemUses);
    DrawItalicText(itemX - 1, itemY + 19, itemText, 1, white);

    if (gCombo > 0)
    {
        int labelScale = 1;
        int numberScale = gComboPulse > 0.12f ? 2 : 1;
        char comboValue[16];
        wsprintfA(comboValue, "%d", gCombo);
        int labelWidth = TextWidth("COMBO", labelScale);
        int numberWidth = TextWidth(comboValue, numberScale);
        DrawItalicText(kBufferW - 10 - labelWidth, 3, "COMBO", labelScale, red);
        DrawItalicText(kBufferW - 10 - numberWidth,
                       numberScale == 2 ? 14 : 15,
                       comboValue, numberScale, white);

        int gaugeX = kBufferW - 58;
        int gaugeY = numberScale == 2 ? 34 : 27;
        FillRect(gaugeX, gaugeY, 48, 4, dark);
        int fill = (int)(46.0f * Clamp(gComboTimer / kComboSeconds, 0.0f, 1.0f));
        FillRect(gaugeX + 1, gaugeY + 1, fill, 2, red);
        char multText[12];
        wsprintfA(multText, "X%d COIN", CoinMultiplier());
        DrawItalicText(gaugeX - 2, gaugeY + 5, multText, 1, Rgb(220, 166, 74));
    }

    if (gReachedBannerTimer > 0.0f)
    {
        float elapsed = 2.4f - gReachedBannerTimer;
        float fade = elapsed < 0.35f ? elapsed / 0.35f : 1.0f;
        if (gReachedBannerTimer < 0.65f) fade = gReachedBannerTimer / 0.65f;
        uint32_t bannerColor = BlendColor(background, white, fade);
        char banner[32];
        float localMeters = (float)gReachedMeters - ChapterStartMeters(gChapter);
        int percent = (int)(Clamp(localMeters / kChapterLengthMeters, 0.0f, 1.0f) * 100.0f + 0.5f);
        wsprintfA(banner, "%dM REACHED (%d%%)", gReachedMeters, percent);
        int width = TextWidth(banner, 1);
        DrawItalicText((kBufferW - width) / 2, 3, banner, 1, bannerColor);
    }

    if (gChapterBannerTimer > 0.0f)
    {
        float elapsed = 3.0f - gChapterBannerTimer;
        float fade = elapsed < 0.45f ? elapsed / 0.45f : 1.0f;
        if (gChapterBannerTimer < 0.75f) fade = gChapterBannerTimer / 0.75f;
        uint32_t titleColor = BlendColor(background, Rgb(235, 66, 82), fade);
        const char* title = CurrentChapterConfig().name;
        int width = TextWidth(title, 3);
        DrawItalicText((kBufferW - width) / 2, 18, title, 3, titleColor);
    }

    if (gTimeStopTimer > 0.0f)
    {
        int seconds = (int)gTimeStopTimer + 1;
        if (seconds > 5) seconds = 5;
        char timerText[12];
        wsprintfA(timerText, "TIME %d", seconds);
        int width = TextWidth(timerText, 1);
        int x = (int)gPlayer.position.x - width / 2;
        int y = (int)gPlayer.position.y - 24;
        if (x < 4) x = 4;
        if (x + width > kBufferW - 4) x = kBufferW - 4 - width;
        if (y < 35) y = 35;
        DrawItalicText(x, y, timerText, 1, Rgb(235, 235, 230));
        FillRect(x, y + 9, width, 3, dark);
        int fill = (int)((float)(width - 2) * Clamp(gTimeStopTimer / 5.0f, 0.0f, 1.0f));
        FillRect(x + 1, y + 10, fill, 1, Rgb(235, 235, 230));
    }
}

static void DrawOfferIcon(int x, int y, ActiveItemType item, uint32_t white)
{
    if (item == ActiveDynamite)
    {
        FillRect(x + 15, y + 15, 7, 23, Rgb(118, 12, 18));
        FillRect(x + 23, y + 13, 8, 25, Rgb(184, 32, 28));
        FillRect(x + 32, y + 16, 6, 21, Rgb(130, 16, 22));
        DrawLine(x + 15, y + 15, x + 37, y + 16, white);
        DrawLine(x + 15, y + 37, x + 37, y + 37, Rgb(52, 8, 12));
        DrawLine(x + 27, y + 12, x + 35, y + 5, Rgb(255, 205, 72));
        FillCircle(x + 37, y + 4, 3, Rgb(255, 102, 30));
        PutPixel(x + 38, y + 3, white);
    }
    else if (item == ActiveMiniOrb)
    {
        FillCircle(x + 26, y + 25, 12, Rgb(64, 12, 96));
        FillCircle(x + 26, y + 25, 8, Rgb(150, 64, 225));
        FillCircle(x + 26, y + 25, 4, Rgb(224, 168, 255));
        DrawLine(x + 10, y + 25, x + 42, y + 25, Rgb(205, 112, 255));
        DrawLine(x + 26, y + 9, x + 26, y + 41, Rgb(104, 38, 178));
        DrawLine(x + 17, y + 16, x + 35, y + 34, white);
        PutPixel(x + 26, y + 25, white);
    }
    else if (item == ActiveBloodChalice)
    {
        DrawLine(x + 16, y + 17, x + 38, y + 17, white);
        FillRect(x + 18, y + 18, 18, 12, Rgb(116, 4, 26));
        FillRect(x + 20, y + 18, 14, 6, Rgb(205, 18, 48));
        DrawLine(x + 18, y + 30, x + 26, y + 36, Rgb(178, 132, 120));
        DrawLine(x + 36, y + 30, x + 28, y + 36, Rgb(178, 132, 120));
        DrawLine(x + 27, y + 30, x + 27, y + 41, white);
        DrawLine(x + 19, y + 41, x + 35, y + 41, white);
        PutPixel(x + 30, y + 21, Rgb(255, 96, 96));
    }
    else if (item == ActiveHourglass)
    {
        DrawLine(x + 17, y + 12, x + 36, y + 12, Rgb(220, 166, 74));
        DrawLine(x + 17, y + 40, x + 36, y + 40, Rgb(220, 166, 74));
        DrawLine(x + 20, y + 13, x + 33, y + 39, white);
        DrawLine(x + 33, y + 13, x + 20, y + 39, white);
        FillRect(x + 25, y + 20, 4, 13, Rgb(220, 166, 74));
        FillRect(x + 23, y + 34, 8, 3, Rgb(220, 166, 74));
    }
    else if (item == ActiveSawBlade)
    {
        FillRect(x + 18, y + 16, 19, 19, Rgb(178, 188, 196));
        FillRect(x + 23, y + 21, 9, 9, Rgb(42, 38, 48));
        PutPixel(x + 27, y + 10, white);
        PutPixel(x + 27, y + 41, white);
        PutPixel(x + 12, y + 26, white);
        PutPixel(x + 42, y + 26, white);
        DrawLine(x + 17, y + 14, x + 37, y + 36, white);
        DrawLine(x + 37, y + 14, x + 17, y + 36, white);
    }
}

static void DrawShopOverlay(uint32_t white, uint32_t red, uint32_t dark)
{
    if (!gShopOpen && gDealAcceptedTimer <= 0.0f) return;

    if (gShopOpen)
    {
        FillRect(0, 0, kBufferW, kBufferH, Rgb(5, 0, 8));
        int titleW = TextWidth("DEMON DEAL", 2);
        DrawItalicText((kBufferW - titleW) / 2, 18, "DEMON DEAL", 2, red);
        int price = CurrentShopPrice();
        char priceText[24];
        wsprintfA(priceText, "PRICE %d COIN", price);
        int priceW = TextWidth(priceText, 1);
        DrawItalicText((kBufferW - priceW) / 2, 38, priceText, 1, white);

        int cardW = 58;
        int cardH = 58;
        int startX = 66;
        int y = 54;
        for (int i = 0; i < 3; ++i)
        {
            int x = startX + i * 64;
            uint32_t edge = i == gShopHover ? Rgb(235, 66, 82) : Rgb(94, 58, 104);
            FillRect(x, y, cardW, cardH, dark);
            DrawLine(x, y, x + cardW - 1, y, edge);
            DrawLine(x, y + cardH - 1, x + cardW - 1, y + cardH - 1, edge);
            DrawLine(x, y, x, y + cardH - 1, edge);
            DrawLine(x + cardW - 1, y, x + cardW - 1, y + cardH - 1, edge);
            DrawOfferIcon(x + 2, y + 5, gShopOffers[i].item, white);
        }

        int hover = gShopHover >= 0 ? gShopHover : 0;
        int nameW = TextWidth(gShopOffers[hover].name, 1);
        int descW = TextWidth(gShopOffers[hover].description, 1);
        DrawItalicText((kBufferW - nameW) / 2, 123, gShopOffers[hover].name, 1, red);
        DrawItalicText((kBufferW - descW) / 2, 137, gShopOffers[hover].description, 1, white);
        if (gCoins < price)
            DrawItalicText(32, 158, "NEED COIN", 1, Rgb(220, 166, 74));
        DrawMenuButton(121, 154, 78, 18, "LEAVE", true, white, red, dark);
        DrawItalicText(210, 158, "ESC", 1, Rgb(150, 112, 126));
    }

    if (gDealAcceptedTimer > 0.0f &&
        (((int)(gDealAcceptedTimer * 14.0f)) & 1))
    {
        int dealW = TextWidth("DEAL ACCEPTED", 1);
        DrawItalicText((kBufferW - dealW) / 2, 82, "DEAL ACCEPTED", 1, red);
    }
}

static void ApplyCrtEffect()
{
    for (int i = 0; i < kBufferW * kBufferH; ++i)
        gCrtScratch[i] = gPixels[i];

    for (int y = 0; y < kBufferH; ++y)
    {
        float ny = ((float)y / (float)(kBufferH - 1)) * 2.0f - 1.0f;
        for (int x = 0; x < kBufferW; ++x)
        {
            float nx = ((float)x / (float)(kBufferW - 1)) * 2.0f - 1.0f;
            int index = y * kBufferW + x;
            int leftIndex = y * kBufferW + (x > 0 ? x - 1 : x);
            int rightIndex = y * kBufferW + (x < kBufferW - 1 ? x + 1 : x);

            uint32_t left = gCrtScratch[leftIndex];
            uint32_t center = gCrtScratch[index];
            uint32_t right = gCrtScratch[rightIndex];

            int centerR = (center >> 16) & 255;
            int centerG = (center >> 8) & 255;
            int centerB = center & 255;
            int r = (centerR + ((left >> 16) & 255) * 3) / 4;
            int g = centerG;
            int b = (centerB + (int)(right & 255) * 3) / 4;
            if (gTimeStopTimer > 0.0f)
            {
                int gray = (r * 30 + g * 59 + b * 11) / 100;
                r = gray;
                g = gray;
                b = gray;
            }

            float edge = nx * nx * 0.72f + ny * ny * 0.92f;
            float vignette = 1.0f - Clamp(edge * 0.28f, 0.0f, 0.34f);
            float scanline = (y & 1) ? 0.90f : 1.03f;
            int noiseHash = (x * 17 + y * 43 + (int)(gMapTime * 90.0f)) & 15;
            float noise = 0.985f + (float)noiseHash * 0.002f;
            float strength = vignette * scanline * noise;

            int brightnessLift = 6 + (int)(gBrightness * 12.0f);
            r = (int)((float)r * strength) + brightnessLift;
            g = (int)((float)g * strength) + brightnessLift;
            b = (int)((float)b * strength) + brightnessLift;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            gPixels[index] = Rgb((uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
    }

    uint32_t glow = Rgb(28, 8, 22);
    DrawLine(0, 0, kBufferW - 1, 0, glow);
    DrawLine(0, kBufferH - 1, kBufferW - 1, kBufferH - 1, glow);
    DrawLine(0, 0, 0, kBufferH - 1, glow);
    DrawLine(kBufferW - 1, 0, kBufferW - 1, kBufferH - 1, glow);
}

static void DrawRisingBottomMist(uint32_t red, uint32_t ember, uint32_t dark)
{
    float devourerMistTop = gDevourerY - 74.0f;
    float screenMistTop = (float)kBufferH - 72.0f;
    float chosenTop = devourerMistTop < screenMistTop ? devourerMistTop : screenMistTop;
    int mistTop = (int)Clamp(chosenTop, 48.0f, (float)kBufferH - 20.0f);
    int mistBottom = kBufferH;
    float height = (float)(mistBottom - mistTop);
    if (height <= 1.0f) return;

    for (int y = mistTop; y < mistBottom; ++y)
    {
        float depth = (float)(y - mistTop) / height;
        float fadeIn = SmoothStep(depth);
        float baseAlpha = fadeIn * (0.055f + depth * depth * 0.36f);
        for (int x = 0; x < kBufferW; x += 2)
        {
            float wave = ValueNoise((float)x * 0.018f + gMapTime * 0.42f,
                                    (float)y * 0.026f - gMapTime * 0.55f, 5);
            float flowX = (float)x * 0.030f + gMapTime * 0.58f + wave * 2.6f;
            float flowY = (float)y * 0.046f - gMapTime * 1.28f;
            float coarse = ValueNoise(flowX, flowY, 11);
            float wisps = ValueNoise((float)x * 0.074f - gMapTime * 0.52f,
                                     (float)y * 0.030f + gMapTime * 0.24f, 23);
            float plume = coarse * 0.70f + wisps * 0.30f;
            if (plume < 0.32f) continue;

            float alpha = baseAlpha * (plume - 0.24f);
            uint32_t fogColor = BlendColor(dark, red, 0.36f + wisps * 0.30f);
            BlendPixelRaw(x, y, fogColor, alpha);
            BlendPixelRaw(x + 1, y, fogColor, alpha * 0.58f);
        }
    }

    int time = (int)(gMapTime * 60.0f);
    for (int i = 0; i < 16; ++i)
    {
        uint32_t h = Hash2D(i, time / 12, 91);
        int cycle = 176 + (int)(h % 132u);
        int phase = (time + (int)(h >> 8)) % cycle;
        float rise = (float)phase / (float)cycle;
        int driftPhase = (time / 3 + i * 9) & 31;
        int drift = ((driftPhase < 16 ? driftPhase : 32 - driftPhase) - 8) / 2;
        int x = (int)(h % (uint32_t)kBufferW) + drift;
        int y = kBufferH - 3 - (int)(rise * 104.0f);
        if (y < mistTop - 8 || y >= kBufferH) continue;
        float alpha = (1.0f - rise) * 0.94f;
        uint32_t flame = Rgb(255, 84, 18);
        BlendPixelRaw(x, y, flame, alpha);
        if ((h & 2u) != 0)
            BlendPixelRaw(x + 1, y, Rgb(220, 28, 8), alpha * 0.46f);
        if ((h & 4u) != 0)
            BlendPixelRaw(x, y + 1, Rgb(125, 8, 4), alpha * 0.48f);
    }
}

static void ApplyVerticalMapLighting(uint32_t topGlow)
{
    uint32_t bottomShade = Rgb(0, 0, 0);
    uint32_t bottomRed = Rgb(26, 0, 5);
    for (int y = 0; y < kBufferH; ++y)
    {
        float t = (float)y / (float)(kBufferH - 1);
        float topAmount = (1.0f - t) * (1.0f - t) * 0.20f;
        float redAmount = t * t * 0.055f;
        float bottomAmount = t * t * 0.36f;
        for (int x = 0; x < kBufferW; ++x)
        {
            int index = y * kBufferW + x;
            uint32_t color = gPixels[index];
            color = BlendColor(color, topGlow, topAmount);
            color = BlendColor(color, bottomRed, redAmount);
            color = BlendColor(color, bottomShade, bottomAmount);
            gPixels[index] = color;
        }
    }
}

static void DrawTopLightLeaks(uint32_t lightColor)
{
    for (int y = 0; y < 42; ++y)
    {
        float t = 1.0f - (float)y / 42.0f;
        float alpha = t * t * 0.15f;
        for (int x = 0; x < kBufferW; x += 2)
        {
            BlendPixelRaw(x, y, lightColor, alpha);
            BlendPixelRaw(x + 1, y, lightColor, alpha * 0.82f);
        }
    }

    for (int beam = 0; beam < 3; ++beam)
    {
        float seed = (float)beam * 47.0f;
        float wobble = ValueNoise(gMapTime * 0.10f + seed, seed * 0.07f, 131) - 0.5f;
        float center = (float)kBufferW * (0.18f + 0.31f * (float)beam) + wobble * 44.0f;
        float width = 48.0f + (float)beam * 12.0f;
        float slant = -28.0f + (float)beam * 21.0f;

        for (int y = 0; y < 132; y += 2)
        {
            float vertical = 1.0f - (float)y / 132.0f;
            vertical *= vertical;
            float rowCenter = center + slant * ((float)y / 132.0f);
            for (int x = (int)(rowCenter - width); x <= (int)(rowCenter + width); x += 2)
            {
                float dx = ((float)x - rowCenter) / width;
                float shape = 1.0f - dx * dx;
                if (shape <= 0.0f) continue;
                float shimmer = ValueNoise((float)x * 0.035f + gMapTime * 0.16f,
                                           (float)y * 0.050f, 211);
                float alpha = shape * vertical * (0.125f + shimmer * 0.065f);
                BlendPixelRaw(x, y, lightColor, alpha);
                BlendPixelRaw(x + 1, y, lightColor, alpha * 0.82f);
            }
        }
    }
}

static float BackgroundPipeBulge(float screenY)
{
    float depth = Clamp(screenY / (float)kBufferH, 0.0f, 1.0f);
    return 8.0f + depth * 20.0f;
}

static void DrawPerspectivePipeBackdrop(uint32_t bg, uint32_t dark,
                                        uint32_t veinColor, uint32_t red)
{
    uint32_t farRib = BlendColor(bg, veinColor, 0.28f);
    uint32_t nearRib = BlendColor(bg, veinColor, 0.42f);
    uint32_t boltColor = BlendColor(bg, red, 0.34f);

    int ringOffset = (int)(gViewHeightPixels * 0.42f) % 34;
    for (int ring = ringOffset - 42; ring < kBufferH + 48; ring += 34)
    {
        float depth = Clamp((float)ring / (float)kBufferH, 0.0f, 1.0f);
        float bulge = BackgroundPipeBulge((float)ring);
        uint32_t ribColor = depth > 0.55f ? nearRib : farRib;

        int previousX = 0;
        int previousY = ring;
        for (int x = 8; x < kBufferW; x += 8)
        {
            float u = ((float)x / (float)(kBufferW - 1)) * 2.0f - 1.0f;
            float curve = 1.0f - u * u;
            int y = ring - (int)(bulge * curve);
            DrawLine(previousX, previousY, x, y, ribColor);
            if ((ring / 34) & 1)
                PutPixel(x, y + 1, BlendColor(bg, ribColor, 0.58f));
            previousX = x;
            previousY = y;
        }
        DrawLine(previousX, previousY, kBufferW - 1, ring, ribColor);

        for (int bolt = 0; bolt < 5; ++bolt)
        {
            int bx = 42 + bolt * 59;
            float u = ((float)bx / (float)(kBufferW - 1)) * 2.0f - 1.0f;
            int by = ring - (int)(bulge * (1.0f - u * u));
            if (by < 0 || by >= kBufferH) continue;
            PutPixel(bx, by, boltColor);
            PutPixel(bx + 1, by, BlendColor(bg, boltColor, 0.7f));
        }
    }

}

static void DrawCenteredText(int y, const char* text, int scale, uint32_t color)
{
    int width = TextWidth(text, scale);
    DrawItalicText((kBufferW - width) / 2, y, text, scale, color);
}

static void DrawMenuButton(int x, int y, int w, int h, const char* label,
                           bool enabled, uint32_t textColor, uint32_t red,
                           uint32_t dark)
{
    bool hover = enabled && MouseInRect((float)x, (float)y, (float)w, (float)h);
    int textNoiseX = hover ? (((int)(gMapTime * 34.0f) & 1) ? 1 : 0) : 0;
    int textNoiseY = hover ? (((int)(gMapTime * 49.0f) & 3) == 0 ? -1 : 0) : 0;
    uint32_t edge = enabled ? (hover ? red : Rgb(96, 58, 92)) : Rgb(42, 34, 42);
    uint32_t fill = hover ? Rgb(38, 7, 22) : dark;
    FillRect(x, y, w, h, fill);
    if (hover)
    {
        FillRect(x + 2, y + 2, 2, h - 4, Rgb(92, 12, 34));
        FillRect(x + w - 4, y + 2, 2, h - 4, Rgb(54, 8, 24));
    }
    DrawLine(x, y, x + w - 1, y, edge);
    DrawLine(x, y + h - 1, x + w - 1, y + h - 1, edge);
    DrawLine(x, y, x, y + h - 1, edge);
    DrawLine(x + w - 1, y, x + w - 1, y + h - 1, edge);
    if (hover)
    {
        PutPixel(x - 1, y + 2, edge);
        PutPixel(x + w, y + h - 3, edge);
    }
    uint32_t color = enabled ? textColor : Rgb(78, 68, 78);
    int width = TextWidth(label, 1);
    if (hover)
        DrawItalicText(x + (w - width) / 2 - 1, y + 5, label, 1,
                       BlendColor(dark, red, 0.38f));
    DrawItalicText(x + (w - width) / 2 + textNoiseX, y + 5 + textNoiseY,
                   label, 1, color);
}

static void DrawMenuFarArchitecture(uint32_t bg, uint32_t red, uint32_t dark)
{
    uint32_t wall = BlendColor(bg, Rgb(28, 9, 22), 0.72f);
    uint32_t pipe = BlendColor(bg, Rgb(48, 20, 34), 0.62f);
    uint32_t seam = BlendColor(bg, Rgb(96, 40, 56), 0.45f);
    int scroll = (int)(gMapTime * 8.0f) % 48;

    for (int i = 0; i < 5; ++i)
    {
        int px = 18 + i * 67 + ((i & 1) ? 9 : -5);
        FillRect(px, -8, 10 + (i % 2) * 5, kBufferH + 16, pipe);
        DrawLine(px + 1, 0, px + 1, kBufferH - 1, BlendColor(pipe, bg, 0.35f));
        DrawLine(px + 9 + (i % 2) * 5, 0, px + 9 + (i % 2) * 5, kBufferH - 1, seam);
        for (int y = scroll - 48; y < kBufferH; y += 48)
        {
            DrawLine(px - 2, y, px + 13 + (i % 2) * 5, y + 4, seam);
            PutPixel(px + 4, y + 2, BlendColor(seam, red, 0.35f));
        }
    }

    for (int eye = 0; eye < 9; ++eye)
    {
        int ex = 22 + (eye * 37 + (int)(Hash2D(eye, 4, 911) % 18u)) % (kBufferW - 44);
        int ey = 26 + (int)(Hash2D(eye, 7, 913) % 108u);
        float blink = ((int)(gMapTime * 2.0f) + eye) % 7 == 0 ? 0.25f : 1.0f;
        uint32_t iris = BlendColor(Rgb(98, 10, 30), red, blink * 0.55f);
        BlendCircleRaw(ex, ey, 9, iris, 0.06f * blink);
        DrawLine(ex - 4, ey, ex + 4, ey, BlendColor(dark, red, 0.38f * blink));
        PutPixel(ex, ey, BlendColor(bg, Rgb(255, 120, 92), 0.65f * blink));
    }

    for (int rib = -scroll; rib < kBufferH + 16; rib += 24)
    {
        DrawLine(0, rib, 42, rib + 9, wall);
        DrawLine(kBufferW - 1, rib + 2, kBufferW - 46, rib + 10, wall);
    }
}

static void DrawMenuLuxuryMist(uint32_t red, uint32_t white, uint32_t particleDark)
{
    uint32_t fog = BlendColor(Rgb(44, 0, 16), red, 0.34f);
    for (int band = 0; band < 5; ++band)
    {
        int baseY = 132 + band * 10;
        int drift = (int)(sin(gMapTime * (0.7f + band * 0.13f) + band) * 13.0f);
        for (int x = -24; x < kBufferW + 28; x += 22)
        {
            int cx = x + drift + (band & 1) * 9;
            BlendCircleRaw(cx, baseY + (x & 7), 18 + band * 4, fog,
                           0.030f + band * 0.006f);
        }
    }

    for (int p = 0; p < 18; ++p)
    {
        uint32_t h = Hash2D(p, (int)(gMapTime * 9.0f), 1231);
        int x = (int)((h & 255u) * kBufferW / 255u);
        int y = 106 + (int)(((h >> 8) & 63u) * 74u / 63u);
        int rise = (int)(gMapTime * (10.0f + (p % 4) * 2.0f)) % 72;
        y -= rise;
        if (y < 76) y += 72;
        uint32_t ember = ((p + (int)gMapTime) & 3)
            ? BlendColor(particleDark, Rgb(255, 62, 28), 0.58f)
            : BlendColor(white, Rgb(255, 88, 42), 0.72f);
        PutPixel(x, y, ember);
        if ((p & 3) == 0) PutPixel(x, y + 1, BlendColor(ember, red, 0.55f));
    }
}

static void DrawMenuBackground(uint32_t bg, uint32_t white, uint32_t red,
                               uint32_t dark, uint32_t particleDark)
{
    Clear(bg);
    DrawMenuFarArchitecture(bg, red, dark);
    int gridOffsetY = (int)(gMapTime * 18.0f) & 15;
    for (int y = gridOffsetY - 16; y < kBufferH; y += 16)
    {
        float bulge = BackgroundPipeBulge((float)y);
        int previousX = 0;
        int previousY = y;
        for (int x = 6; x < kBufferW; x += 6)
        {
            float u = ((float)x / (float)(kBufferW - 1)) * 2.0f - 1.0f;
            int curveY = y - (int)(bulge * (1.0f - u * u));
            DrawLine(previousX, previousY, x, curveY, dark);
            previousX = x;
            previousY = curveY;
        }
        DrawLine(previousX, previousY, kBufferW - 1, y, dark);
    }

    DrawRisingBottomMist(red, Rgb(255, 84, 18), particleDark);
    DrawMenuLuxuryMist(red, white, particleDark);
    ApplyVerticalMapLighting(BlendColor(bg, white, 0.16f));
    DrawTopLightLeaks(BlendColor(bg, white, 0.60f));
}

static void DrawMenuPanel(int x, int y, int w, int h, uint32_t edge,
                          uint32_t dark)
{
    FillRect(x + 2, y + 2, w, h, Rgb(2, 0, 4));
    FillRect(x, y, w, h, Rgb(8, 2, 10));
    DrawLine(x, y, x + w - 1, y, edge);
    DrawLine(x, y + h - 1, x + w - 1, y + h - 1, BlendColor(edge, dark, 0.45f));
    DrawLine(x, y, x, y + h - 1, BlendColor(edge, dark, 0.20f));
    DrawLine(x + w - 1, y, x + w - 1, y + h - 1, BlendColor(edge, dark, 0.20f));
    PutPixel(x + 3, y + 3, BlendColor(edge, Rgb(255, 220, 180), 0.25f));
    PutPixel(x + w - 4, y + 3, edge);
}

static void DrawLockedNoise(int x, int y, int w, int h, uint32_t color)
{
    uint32_t dim = BlendColor(color, Rgb(0, 0, 0), 0.42f);
    DrawLine(x + 5, y + h - 6, x + w - 6, y + 5, dim);
    DrawLine(x + 6, y + h - 5, x + w - 5, y + 6,
             BlendColor(dim, Rgb(0, 0, 0), 0.20f));
    int cx = x + w / 2;
    int cy = y + h / 2 + 3;
    DrawLine(cx - 4, cy - 3, cx - 4, cy - 6, color);
    DrawLine(cx + 4, cy - 3, cx + 4, cy - 6, color);
    DrawLine(cx - 4, cy - 6, cx + 4, cy - 6, color);
    FillRect(cx - 5, cy - 2, 11, 7, dim);
    PutPixel(cx, cy + 1, color);
}

static void DrawTooltipBox(int x, int y, int w, int h, const char* title,
                           const char* desc, uint32_t white, uint32_t red,
                           uint32_t dark)
{
    DrawMenuPanel(x, y, w, h, red, dark);
    FillRect(x + 3, y + 3, w - 6, 1, Rgb(54, 8, 22));
    DrawItalicText(x + 8, y + 7, title, 1, red);
    DrawItalicText(x + 8, y + 18, desc, 1, white);
}

static void DrawHellPitTitle(uint32_t red, uint32_t white)
{
    int titleWidth = TextWidth("HELL PIT", 4);
    int titleX = (kBufferW - titleWidth) / 2;
    int pulse = (int)(gMapTime * 5.0f) & 1;
    uint32_t shade = Rgb(34, 0, 12);
    uint32_t deep = Rgb(92, 4, 22);
    uint32_t hot = BlendColor(Rgb(255, 76, 68), white,
                              0.08f + 0.06f * (float)pulse);
    DrawItalicText(titleX + 3, 22, "HELL PIT", 4, Rgb(12, 0, 5));
    DrawItalicText(titleX + 2, 21, "HELL PIT", 4, shade);
    DrawItalicText(titleX + 1, 20, "HELL PIT", 4, deep);
    DrawItalicText(titleX, 18, "HELL PIT", 4, red);
    DrawItalicText(titleX, 17, "HELL PIT", 4, hot);
}

static void DrawSlider(int x, int y, int w, float value, uint32_t white,
                       uint32_t red, uint32_t dark)
{
    FillRect(x, y, w, 4, dark);
    DrawLine(x, y + 1, x + w, y + 1, Rgb(92, 62, 82));
    int fill = (int)((float)w * Clamp(value, 0.0f, 1.0f));
    FillRect(x, y, fill, 4, red);
    int knobX = x + fill;
    FillRect(knobX - 2, y - 4, 5, 12, white);
}

static const char* ActiveItemMenuName(ActiveItemType item)
{
    switch (item)
    {
    case ActiveDynamite: return "DYNAMITE";
    case ActiveMiniOrb: return "MINI ORB";
    case ActiveBloodChalice: return "BLOOD CUP";
    case ActiveHourglass: return "HOURGLASS";
    case ActiveSawBlade: return "SAWBLADE";
    default: return "UNKNOWN";
    }
}

static const char* ActiveItemMenuDescription(ActiveItemType item)
{
    switch (item)
    {
    case ActiveDynamite: return "BLAST JUMP  5 USES";
    case ActiveMiniOrb: return "SUPER DASH  2 USES";
    case ActiveBloodChalice: return "HEAL HEART  3 USES";
    case ActiveHourglass: return "STOP TIME  2 USES";
    case ActiveSawBlade: return "CUT ANCHOR  5 USES";
    default: return "LOCKED";
    }
}

static void RenderMainMenu(uint32_t bg, uint32_t white, uint32_t red,
                           uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawHellPitTitle(red, white);
    DrawCenteredText(50, "BLOOD DEBT BELOW", 1, Rgb(190, 142, 132));
    DrawMenuPanel(101, 58, 118, 96, Rgb(62, 20, 42), dark);
    DrawMenuButton(108, 62, 104, 14, "NEW GAME", true, white, red, dark);
    DrawMenuButton(108, 77, 104, 14, "CONTINUE", gCheckpointValid, white, red, dark);
    DrawMenuButton(108, 92, 104, 14, "OPTIONS", true, white, red, dark);
    DrawMenuButton(108, 107, 104, 14, "CODEX", true, white, red, dark);
    DrawMenuButton(108, 122, 104, 14, "ACHIEVEMENTS", true, white, red, dark);
    DrawMenuButton(108, 137, 104, 14, "EXIT GAME", true, white, red, dark);
    DrawMenuButton(248, 160, 62, 14, "CREDIT", true, white, red, dark);
    if (!gCheckpointValid)
        DrawItalicText(218, 81, "NO SAVE", 1, Rgb(110, 78, 82));
}

static void RenderNewGameSelect(uint32_t bg, uint32_t white, uint32_t red,
                                uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(54, 28, 212, 126, red, dark);
    DrawCenteredText(42, "SELECT MODE", 2, red);
    DrawMenuButton(82, 70, 156, 22, "NORMAL MODE", true, white, red, dark);
    bool infiniteUnlocked = InfiniteModeUnlocked();
    DrawMenuButton(82, 102, 156, 22, "INFINITE MODE", infiniteUnlocked, white, red, dark);
    if (!infiniteUnlocked && MouseInRect(82.0f, 102.0f, 156.0f, 22.0f))
        DrawCenteredText(130, "CLEAR GAME TO UNLOCK", 1, Rgb(220, 166, 74));
    else if (infiniteUnlocked)
        DrawCenteredText(130, "ENDLESS DREAD AWAITS", 1, Rgb(190, 142, 132));
    DrawMenuButton(112, 146, 96, 18, "BACK", true, white, red, dark);
}

static void DrawAchievementIcon(int x, int y, AchievementType achievement,
                                bool unlocked, uint32_t white,
                                uint32_t red, uint32_t dark)
{
    uint32_t color = unlocked ? red : Rgb(74, 50, 64);
    uint32_t light = unlocked ? white : Rgb(112, 86, 96);
    FillRect(x, y, 18, 18, dark);
    DrawLine(x, y, x + 17, y, color);
    DrawLine(x, y + 17, x + 17, y + 17, color);
    DrawLine(x, y, x, y + 17, color);
    DrawLine(x + 17, y, x + 17, y + 17, color);
    if (achievement == AchievementStage1)
    {
        FillRect(x + 8, y + 5, 2, 9, light);
        DrawLine(x + 6, y + 4, x + 11, y + 4, color);
        DrawLine(x + 6, y + 14, x + 11, y + 14, color);
    }
    else if (achievement == AchievementStage2)
    {
        FillRect(x + 6, y + 5, 2, 9, light);
        FillRect(x + 10, y + 5, 2, 9, light);
        DrawLine(x + 5, y + 4, x + 12, y + 4, color);
        DrawLine(x + 5, y + 14, x + 12, y + 14, color);
    }
    else if (achievement == AchievementStage3)
    {
        FillRect(x + 4, y + 5, 2, 9, light);
        FillRect(x + 8, y + 5, 2, 9, light);
        FillRect(x + 12, y + 5, 2, 9, light);
        DrawLine(x + 3, y + 4, x + 14, y + 4, color);
        DrawLine(x + 3, y + 14, x + 14, y + 14, color);
    }
    else if (achievement == AchievementEscape)
    {
        uint32_t sun = unlocked ? Rgb(255, 194, 78) : Rgb(118, 86, 58);
        DrawLine(x + 2, y + 12, x + 16, y + 12, light);
        FillCircle(x + 9, y + 11, 5, sun);
        FillRect(x + 2, y + 11, 15, 5, dark);
        DrawLine(x + 9, y + 2, x + 9, y + 6, sun);
        DrawLine(x + 4, y + 5, x + 6, y + 7, sun);
        DrawLine(x + 14, y + 5, x + 12, y + 7, sun);
        DrawLine(x + 3, y + 9, x + 5, y + 10, sun);
        DrawLine(x + 15, y + 9, x + 13, y + 10, sun);
        PutPixel(x + 9, y + 7, light);
    }
    else if (achievement == AchievementHookVeteran)
    {
        DrawHookSprite({ (float)x + 10.0f, (float)y + 8.0f },
                       Normalize({ 1.0f, -0.25f }), light);
        DrawLine(x + 3, y + 13, x + 10, y + 8, color);
    }
}

static void RenderAchievements(uint32_t bg, uint32_t white, uint32_t red,
                               uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(14, 12, 292, 158, red, dark);
    DrawCenteredText(20, "ACHIEVEMENTS", 2, red);
    for (int i = 0; i < AchievementCount; ++i)
    {
        int y = 46 + i * 20;
        bool unlocked = gAchievementUnlocked[i];
        bool hover = MouseInRect(22.0f, (float)y - 6.0f, 270.0f, 18.0f);
        if (hover)
        {
            FillRect(22, y - 6, 270, 18, Rgb(26, 4, 14));
            DrawLine(24, y - 6, 288, y - 6, red);
        }
        DrawAchievementIcon(24, y - 4, (AchievementType)i, unlocked, white, red, dark);
        if (!unlocked)
            DrawLockedNoise(24, y - 4, 18, 18, Rgb(82, 54, 66));
        DrawItalicText(50, y, AchievementName((AchievementType)i), 1,
                       unlocked ? white : Rgb(120, 84, 96));
        DrawItalicText(134, y, AchievementDescription((AchievementType)i), 1,
                       unlocked ? Rgb(220, 166, 74) : Rgb(92, 66, 76));
    }
    DrawMenuButton(26, 150, 78, 18, "RESET", true, white, red, dark);
    DrawMenuButton(216, 150, 78, 18, "BACK", true, white, red, dark);
}

static void RenderOptions(uint32_t bg, uint32_t white, uint32_t red,
                          uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(18, 10, 284, 162, red, dark);
    DrawCenteredText(18, "OPTIONS", 2, red);
    DrawItalicText(48, 48, "BRIGHTNESS", 1, white);
    DrawSlider(110, 54, 118, gBrightness, white, red, dark);
    char brightText[8];
    wsprintfA(brightText, "%d%%", (int)(gBrightness * 100.0f + 0.5f));
    DrawItalicText(236, 49, brightText, 1, white);

    DrawItalicText(42, 72, "MASTER SOUND", 1, white);
    DrawSlider(110, 78, 118, gMasterSound, white, red, dark);
    char soundText[8];
    wsprintfA(soundText, "%d%%", (int)(gMasterSound * 100.0f + 0.5f));
    DrawItalicText(236, 73, soundText, 1, white);

    DrawItalicText(42, 99, "CROSSHAIR", 1, white);
    DrawMenuButton(126, 96, 20, 16, "", true, white, red, dark);
    {
        bool hover = MouseInRect(126.0f, 96.0f, 20.0f, 16.0f);
        uint32_t arrow = hover ? red : white;
        uint32_t arrowShadow = BlendColor(dark, Rgb(0, 0, 0), 0.35f);
        DrawLine(138, 100, 134, 104, arrowShadow);
        DrawLine(134, 104, 138, 108, arrowShadow);
        DrawLine(137, 100, 133, 104, arrow);
        DrawLine(133, 104, 137, 108, arrow);
    }
    FillRect(156, 95, 40, 18, Rgb(12, 4, 14));
    DrawLine(156, 95, 195, 95, Rgb(96, 58, 92));
    DrawLine(156, 112, 195, 112, Rgb(96, 58, 92));
    DrawLine(156, 95, 156, 112, Rgb(96, 58, 92));
    DrawLine(195, 95, 195, 112, Rgb(96, 58, 92));
    FillRect(159, 98, 34, 12, Rgb(7, 1, 9));
    PutPixel(160, 97, red);
    PutPixel(191, 110, red);
    DrawCrosshairAt(176, 104, gCrosshairStyle, false, white, red, dark);
    DrawMenuButton(206, 96, 20, 16, "", true, white, red, dark);
    {
        bool hover = MouseInRect(206.0f, 96.0f, 20.0f, 16.0f);
        uint32_t arrow = hover ? red : white;
        uint32_t arrowShadow = BlendColor(dark, Rgb(0, 0, 0), 0.35f);
        DrawLine(214, 100, 218, 104, arrowShadow);
        DrawLine(218, 104, 214, 108, arrowShadow);
        DrawLine(213, 100, 217, 104, arrow);
        DrawLine(217, 104, 213, 108, arrow);
    }

    if (gOptionsFromGameplay)
    {
        DrawItalicText(42, 124, "PERKS", 1, Rgb(180, 128, 142));
        int slot = 0;
        PerkType hoverPerk = PerkNone;
        for (int i = 1; i < PerkCount; ++i)
        {
            if (!gPerkOwned[i]) continue;
            int x = 86 + slot * 19;
            bool hover = MouseInRect((float)x, 120.0f, 17.0f, 17.0f);
            if (hover) hoverPerk = (PerkType)i;
            FillRect(x, 120, 17, 17, dark);
            uint32_t edge = hover ? red : Rgb(96, 58, 92);
            DrawLine(x, 120, x + 16, 120, edge);
            DrawLine(x, 136, x + 16, 136, edge);
            DrawLine(x, 120, x, 136, edge);
            DrawLine(x + 16, 120, x + 16, 136, edge);
            DrawPerkIcon(x, 120, (PerkType)i, white, red, dark);
            ++slot;
        }
        if (slot == 0)
            DrawItalicText(86, 124, "NONE", 1, Rgb(110, 78, 82));
        else if (hoverPerk != PerkNone)
        {
            DrawTooltipBox(38, 34, 236, 28, PerkName(hoverPerk),
                           PerkRouletteDescription(hoverPerk), white, red, dark);
        }
    }

    DrawMenuButton(34, 150, 74, 18, "HELP", true, white, red, dark);
    DrawMenuButton(123, 150, 74, 18, gOptionsFromGameplay ? "RESUME" : "BACK",
                   true, white, red, dark);
    DrawMenuButton(212, 150, 74, 18, "MAIN", true, white, red, dark);
}

static void DrawHelpCard(int x, int y, int w, int h, const char* title,
                         uint32_t white, uint32_t red, uint32_t dark)
{
    DrawMenuPanel(x, y, w, h, Rgb(86, 28, 56), dark);
    DrawItalicText(x + 7, y + 6, title, 1, red);
    DrawLine(x + 7, y + 17, x + w - 8, y + 17, Rgb(62, 30, 52));
}

static void DrawHelpKeyIcon(int x, int y, int w, const char* label, uint32_t white,
                            uint32_t red, uint32_t dark)
{
    FillRect(x, y, w, 18, dark);
    DrawLine(x, y, x + w - 1, y, red);
    DrawLine(x, y + 17, x + w - 1, y + 17, Rgb(96, 58, 92));
    DrawLine(x, y, x, y + 17, Rgb(96, 58, 92));
    DrawLine(x + w - 1, y, x + w - 1, y + 17, Rgb(96, 58, 92));
    int textW = TextWidth(label, 1);
    DrawItalicText(x + (w - textW) / 2, y + 6, label, 1, white);
}

static void DrawHelpMouseIcon(int x, int y, uint32_t white, uint32_t red,
                              uint32_t dark)
{
    FillRect(x + 6, y + 1, 17, 24, dark);
    FillRect(x + 8, y + 3, 13, 20, Rgb(18, 8, 18));
    DrawLine(x + 6, y + 1, x + 22, y + 1, white);
    DrawLine(x + 6, y + 1, x + 6, y + 24, white);
    DrawLine(x + 22, y + 1, x + 22, y + 24, white);
    DrawLine(x + 6, y + 24, x + 22, y + 24, white);
    FillRect(x + 8, y + 3, 6, 8, red);
    FillRect(x + 15, y + 3, 6, 8, Rgb(42, 26, 40));
    DrawLine(x + 14, y + 3, x + 14, y + 12, Rgb(96, 58, 92));
    FillRect(x + 13, y + 14, 3, 5, Rgb(96, 58, 92));
    DrawLine(x + 9, y + 12, x + 20, y + 12, Rgb(96, 58, 92));
}

static void DrawHelpPlatformIcon(int x, int y, uint32_t red)
{
    FillRect(x, y + 12, 48, 8, Rgb(82, 65, 84));
    DrawLine(x, y + 12, x + 47, y + 12, Rgb(152, 126, 148));
    DrawLine(x + 4, y + 19, x + 44, y + 19, Rgb(42, 28, 44));
}

static void DrawHelpOrbIcon(int cx, int cy, uint32_t purple, uint32_t white)
{
    int radiusPulse = 1;
    BlendCircleRaw(cx, cy, (int)kSuperOrbHitRadius + radiusPulse, purple, 0.24f);
    FillCircle(cx, cy, 7, Rgb(62, 10, 92));
    FillTriangle({ (float)cx, (float)cy - 10.0f },
                 { (float)cx - 8.0f, (float)cy },
                 { (float)cx, (float)cy + 10.0f }, Rgb(132, 48, 210));
    FillTriangle({ (float)cx, (float)cy - 10.0f },
                 { (float)cx + 8.0f, (float)cy },
                 { (float)cx, (float)cy + 10.0f }, purple);
    FillCircle(cx, cy, 3, Rgb(230, 176, 255));
    PutPixel(cx, cy, white);
    DrawLine(cx - (int)kSuperOrbHitRadius - radiusPulse, cy, cx - 7, cy, Rgb(212, 128, 255));
    DrawLine(cx + 7, cy, cx + (int)kSuperOrbHitRadius + radiusPulse, cy, Rgb(212, 128, 255));
    DrawLine(cx, cy - (int)kSuperOrbHitRadius - radiusPulse, cx, cy - 7, Rgb(212, 128, 255));
}

static void DrawHelpBulletIcon(int x, int y, bool hookable, uint32_t white,
                               uint32_t red)
{
    uint32_t color = hookable ? Rgb(245, 175, 70) : Rgb(210, 52, 42);
    BlendCircleRaw(x + 13, y + 9, 10, color, 0.18f);
    FillCircle(x + 13, y + 9, 5, color);
    DrawLine(x, y + 9, x + 7, y + 9, BlendColor(color, Rgb(0, 0, 0), 0.35f));
    if (!hookable)
    {
        DrawLine(x + 27, y + 4, x + 35, y + 14, red);
        DrawLine(x + 35, y + 4, x + 27, y + 14, red);
    }
}

static void DrawHelpPortalIcon(int cx, int cy, uint32_t white)
{
    uint32_t purple = Rgb(178, 72, 238);
    uint32_t outer = Rgb(58, 10, 72);
    BlendCircleRaw(cx, cy, 18, purple, 0.22f);
    FillCircle(cx, cy, 13, outer);
    FillRect(cx - 10, cy - 14, 21, 29, outer);
    FillCircle(cx, cy, 10, purple);
    FillRect(cx - 7, cy - 11, 15, 23, purple);
    FillCircle(cx - 2, cy - 3, 3, BlendColor(purple, white, 0.20f));
    DrawLine(cx - 11, cy - 10, cx - 15, cy - 15, purple);
    DrawLine(cx + 11, cy - 10, cx + 15, cy - 15, purple);
    DrawLine(cx - 12, cy + 11, cx - 16, cy + 15, outer);
    DrawLine(cx + 12, cy + 11, cx + 16, cy + 15, outer);
}

static void DrawHelpPageDots(uint32_t white, uint32_t red)
{
    for (int i = 0; i < 4; ++i)
        FillCircle(105 + i * 11, 162, 2, i == gHelpPage ? red : Rgb(96, 58, 92));
    char pageText[8];
    wsprintfA(pageText, "%d/4", gHelpPage + 1);
    DrawItalicText(145, 157, pageText, 1, white);
}

static void RenderHelp(uint32_t bg, uint32_t white, uint32_t red,
                       uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(12, 8, 296, 164, red, dark);
    DrawCenteredText(16, "HELP", 2, red);

    if (gHelpPage == 0)
    {
        DrawHelpCard(22, 36, 86, 50, "MOVE", white, red, dark);
        DrawHelpKeyIcon(31, 58, 22, "A", white, red, dark);
        DrawHelpKeyIcon(57, 58, 22, "D", white, red, dark);
        DrawItalicText(27, 94, "LEFT / RIGHT", 1, white);

        DrawHelpCard(117, 36, 86, 50, "JUMP", white, red, dark);
        DrawHelpKeyIcon(130, 58, 46, "SPACE", white, red, dark);
        DrawItalicText(122, 96, "KILLS RESET JUMP", 1, white);

        DrawHelpCard(212, 36, 74, 50, "HOOK", white, red, dark);
        DrawHelpMouseIcon(232, 55, white, red, dark);
        DrawItalicText(217, 96, "LEFT CLICK", 1, white);

        DrawItalicText(36, 121, "AIM WITH MOUSE. HOOK TARGETS TO KEEP CLIMBING.", 1, red);
        DrawItalicText(36, 134, "Q: USE ACTIVE ITEM     ESC: PAUSE / OPTIONS", 1, white);
    }
    else if (gHelpPage == 1)
    {
        DrawHelpCard(22, 36, 82, 82, "PLATFORM", white, red, dark);
        DrawHelpPlatformIcon(39, 58, red);
        DrawItalicText(28, 89, "HOOK: PULL UP", 1, white);
        DrawItalicText(28, 101, "LAND SAFELY", 1, Rgb(220, 166, 74));

        DrawHelpCard(119, 36, 82, 82, "ENEMY", white, red, dark);
        DrawFallenAngelEnemy({ 148.0f, 63.0f, 24.0f, 20.0f }, Rgb(130, 9, 20),
                             Rgb(92, 18, 34), Rgb(245, 190, 110));
        DrawItalicText(125, 89, "HOOK: SLASH", 1, white);
        DrawItalicText(125, 101, "KILL = JUMP", 1, Rgb(220, 166, 74));

        DrawHelpCard(216, 36, 82, 82, "ORB", white, red, dark);
        DrawHelpOrbIcon(257, 74, Rgb(182, 84, 245), white);
        DrawItalicText(222, 89, "HOOK: DASH", 1, white);
        DrawItalicText(222, 101, "GO UP FAST", 1, Rgb(220, 166, 74));

        DrawItalicText(28, 130, "RULE: HOOKABLE OBJECTS KEEP YOUR CLIMB ALIVE.", 1, white);
        DrawItalicText(28, 142, "KILL ENEMIES OR HIT ORBS TO EXTEND COMBO.", 1, red);
    }
    else if (gHelpPage == 2)
    {
        DrawHelpCard(22, 36, 82, 82, "BULLETS", white, red, dark);
        DrawHelpBulletIcon(35, 57, true, white, red);
        DrawHelpBulletIcon(35, 77, false, white, red);
        DrawItalicText(28, 99, "GOLD = HOOK", 1, white);
        DrawItalicText(28, 109, "RED = DANGER", 1, Rgb(220, 166, 74));

        DrawHelpCard(119, 36, 82, 82, "SHOP", white, red, dark);
        DrawHelpPortalIcon(160, 74, white);
        DrawItalicText(125, 89, "TOUCH PORTAL", 1, white);
        DrawItalicText(125, 101, "BUY ITEM", 1, Rgb(220, 166, 74));

        DrawHelpCard(216, 36, 82, 82, "RUN", white, red, dark);
        DrawItalicText(222, 60, "CLIMB 2000", 1, white);
        DrawItalicText(222, 74, "METERS TO", 1, Rgb(220, 166, 74));
        DrawItalicText(222, 88, "NEXT RING.", 1, white);

        DrawItalicText(28, 134, "NEW SHOP ITEM REPLACES OLD ACTIVE.", 1, red);
    }
    else
    {
        DrawHelpCard(38, 38, 244, 82, "PERKS", white, red, dark);
        DrawPerkIcon(60, 64, PerkCrossNecklace, white, red, dark);
        DrawPerkIcon(88, 64, PerkDevilHeart, white, red, dark);
        DrawPerkIcon(116, 64, PerkBloodBattery, white, red, dark);
        DrawPerkIcon(144, 64, PerkAngelSkin, white, red, dark);
        DrawPerkIcon(172, 64, PerkParachute, white, red, dark);
        DrawPerkIcon(200, 64, PerkContract, white, red, dark);
        DrawItalicText(54, 92, "PASSIVE BONUSES GAINED FROM STAGE 2+.", 1, white);
        DrawItalicText(54, 105, "PERKS ARE SAVED AT CHECKPOINTS.", 1, Rgb(220, 166, 74));

        DrawItalicText(34, 132, "PERKS STACK WITH ACTIVE ITEMS.", 1, red);
    }

    DrawHelpPageDots(white, red);
    DrawMenuButton(24, 152, 58, 18, "PREV", gHelpPage > 0, white, red, dark);
    DrawMenuButton(172, 152, 58, 18, "NEXT", gHelpPage < 3, white, red, dark);
    DrawMenuButton(236, 152, 58, 18,
                   gHelpStartsGame ? "START" : "BACK",
                   true, white, red, dark);
}

static void RenderCodex(uint32_t bg, uint32_t white, uint32_t red,
                        uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(8, 6, 304, 168, Rgb(86, 28, 56), dark);
    DrawCenteredText(10, "CODEX", 2, red);
    DrawItalicText(16, 31, "ACTIVE ITEMS", 1, white);

    ActiveItemType items[] =
    {
        ActiveDynamite,
        ActiveMiniOrb,
        ActiveBloodChalice,
        ActiveHourglass,
        ActiveSawBlade
    };
    ActiveItemType hoverItem = ActiveNone;
    PerkType hoverPerk = PerkNone;
    for (int i = 0; i < 5; ++i)
    {
        int x = 12 + i * 60;
        int y = 44;
        bool hover = MouseInRect((float)x, (float)y, 52.0f, 43.0f);
        if (hover) hoverItem = items[i];
        uint32_t edge = hover ? red : Rgb(96, 58, 92);
        DrawMenuPanel(x, y, 52, 43, edge, dark);
        if (hover) BlendCircleRaw(x + 26, y + 22, 24, red, 0.10f);
        DrawOfferIcon(x - 1, y - 4, items[i], white);
    }

    DrawItalicText(16, 95, "PERKS", 1, white);
    for (int i = 1; i < PerkCount; ++i)
    {
        int index = i - 1;
        int x = 12 + index * 51;
        int y = 108;
        bool hover = MouseInRect((float)x, (float)y, 42.0f, 30.0f);
        if (hover) hoverPerk = (PerkType)i;
        uint32_t edge = hover ? red : Rgb(96, 58, 92);
        DrawMenuPanel(x, y, 42, 30, edge, dark);
        uint32_t perkWhite = gPerkOwned[i] ? white : Rgb(106, 82, 94);
        uint32_t perkRed = gPerkOwned[i] ? red : Rgb(82, 54, 66);
        DrawPerkIcon(x + 13, y + 7, (PerkType)i, perkWhite, perkRed, dark);
        if (!gPerkOwned[i])
            DrawLockedNoise(x, y, 42, 30, Rgb(82, 54, 66));
        if (hover) BlendCircleRaw(x + 21, y + 15, 17, red, 0.08f);
    }

    if (hoverPerk != PerkNone)
    {
        DrawTooltipBox(14, 140, 220, 30, PerkName(hoverPerk),
                       PerkDescription(hoverPerk), white, red, dark);
    }
    else if (hoverItem != ActiveNone)
    {
        DrawTooltipBox(14, 140, 220, 30, ActiveItemMenuName(hoverItem),
                       ActiveItemMenuDescription(hoverItem), white, red, dark);
    }
    else
    {
        DrawItalicText(16, 151, "HOVER ENTRY FOR DETAILS", 1, Rgb(160, 118, 128));
    }
    DrawMenuButton(248, 154, 58, 18, "BACK", true, white, red, dark);
}

static void RenderCredits(uint32_t bg, uint32_t white, uint32_t red,
                          uint32_t dark, uint32_t particleDark)
{
    DrawMenuBackground(bg, white, red, dark, particleDark);
    DrawMenuPanel(36, 30, 248, 112, red, dark);
    DrawCenteredText(45, "CREDIT", 2, red);
    DrawCenteredText(76, "DEVELOPED BY", 1, white);
    DrawCenteredText(99, "CHOI SEONG WON", 1, Rgb(228, 66, 82));
    DrawCenteredText(116, "SOLO DEVELOPER", 1, white);
    DrawMenuButton(112, 142, 96, 18, "BACK", true, white, red, dark);
}

static float CutsceneAlpha(float t, float start, float end)
{
    float inA = SmoothStep((t - start) / 0.42f);
    float outA = 1.0f - SmoothStep((t - end) / 0.42f);
    return Clamp(inA < outA ? inA : outA, 0.0f, 1.0f);
}

static void DrawIntroShip(Vec2 center, float angle, uint32_t color,
                          uint32_t dark)
{
    Vec2 forward = { (float)cos(angle), (float)sin(angle) };
    Vec2 right = { -forward.y, forward.x };
    float noseLength = 10.0f;
    float tailLength = 7.0f;
    float wingSpan = 7.0f;
    float bodyBack = -1.0f;
    uint32_t outline = Rgb(8, 8, 14);
    uint32_t hull = PlayerSpriteColor('C', color);
    uint32_t hullLight = PlayerSpriteColor('L', color);
    uint32_t deep = PlayerSpriteColor('B', color);
    uint32_t canopy = Rgb(152, 42, 54);
    uint32_t canopyDark = Rgb(56, 12, 20);
    uint32_t ember = BlendColor(color, Rgb(238, 18, 22), 0.38f);

    Vec2 nose = { center.x + forward.x * noseLength, center.y + forward.y * noseLength };
    Vec2 tail = { center.x - forward.x * tailLength, center.y - forward.y * tailLength };
    Vec2 leftWing = { center.x + forward.x * bodyBack - right.x * wingSpan,
                      center.y + forward.y * bodyBack - right.y * wingSpan };
    Vec2 rightWing = { center.x + forward.x * bodyBack + right.x * wingSpan,
                       center.y + forward.y * bodyBack + right.y * wingSpan };
    Vec2 leftNose = { center.x + forward.x * 4.8f - right.x * 2.8f,
                      center.y + forward.y * 4.8f - right.y * 2.8f };
    Vec2 rightNose = { center.x + forward.x * 4.8f + right.x * 2.8f,
                       center.y + forward.y * 4.8f + right.y * 2.8f };

    FillTriangle(nose, leftWing, rightWing, outline);
    FillTriangle({ tail.x - right.x * 3.0f, tail.y - right.y * 3.0f },
                 { tail.x + right.x * 3.0f, tail.y + right.y * 3.0f },
                 { center.x, center.y }, outline);
    FillTriangle({ nose.x - forward.x * 1.0f, nose.y - forward.y * 1.0f },
                 { leftWing.x + right.x * 1.0f, leftWing.y + right.y * 1.0f },
                 { center.x, center.y }, hull);
    FillTriangle({ nose.x - forward.x * 1.0f, nose.y - forward.y * 1.0f },
                 { center.x, center.y },
                 { rightWing.x - right.x * 1.0f, rightWing.y - right.y * 1.0f },
                 hull);
    FillTriangle(center,
                 { tail.x - right.x * 2.0f, tail.y - right.y * 2.0f },
                 { tail.x + right.x * 2.0f, tail.y + right.y * 2.0f },
                 deep);

    DrawLine((int)nose.x, (int)nose.y, (int)leftWing.x, (int)leftWing.y, outline);
    DrawLine((int)nose.x, (int)nose.y, (int)rightWing.x, (int)rightWing.y, outline);
    DrawLine((int)leftWing.x, (int)leftWing.y, (int)tail.x, (int)tail.y, outline);
    DrawLine((int)rightWing.x, (int)rightWing.y, (int)tail.x, (int)tail.y, outline);
    DrawLine((int)leftNose.x, (int)leftNose.y, (int)center.x, (int)center.y, hullLight);
    DrawLine((int)rightNose.x, (int)rightNose.y, (int)center.x, (int)center.y, hullLight);
    DrawLine((int)leftWing.x, (int)leftWing.y,
             (int)(leftWing.x - right.x * 2.0f - forward.x * 1.0f),
             (int)(leftWing.y - right.y * 2.0f - forward.y * 1.0f),
             Rgb(164, 24, 36));
    DrawLine((int)rightWing.x, (int)rightWing.y,
             (int)(rightWing.x + right.x * 2.0f - forward.x * 1.0f),
             (int)(rightWing.y + right.y * 2.0f - forward.y * 1.0f),
             Rgb(164, 24, 36));

    Vec2 canopyTip = { center.x + forward.x * 5.8f, center.y + forward.y * 5.8f };
    Vec2 canopyBack = { center.x + forward.x * 0.8f, center.y + forward.y * 0.8f };
    Vec2 canopyLeft = { canopyBack.x - right.x * 2.0f, canopyBack.y - right.y * 2.0f };
    Vec2 canopyRight = { canopyBack.x + right.x * 2.0f, canopyBack.y + right.y * 2.0f };
    FillTriangle(canopyTip, canopyLeft, canopyRight, canopy);
    DrawLine((int)canopyLeft.x, (int)canopyLeft.y,
             (int)canopyRight.x, (int)canopyRight.y, canopyDark);

    Vec2 coreA = { center.x - right.x * 1.4f, center.y - right.y * 1.4f };
    Vec2 coreB = { center.x + right.x * 1.4f, center.y + right.y * 1.4f };
    Vec2 coreC = { center.x - forward.x * 2.6f, center.y - forward.y * 2.6f };
    FillTriangle(coreA, coreB, coreC, ember);
    Vec2 engineA = { tail.x - right.x * 1.8f, tail.y - right.y * 1.8f };
    Vec2 engineB = { tail.x + right.x * 1.8f, tail.y + right.y * 1.8f };
    Vec2 engineTip = { tail.x - forward.x * 3.0f, tail.y - forward.y * 3.0f };
    FillTriangle(engineA, engineB, engineTip, Rgb(104, 16, 26));
    PutPixel((int)nose.x, (int)nose.y, Rgb(222, 186, 142));
}

static void DrawIntroText(int y, const char* text, float alpha,
                          uint32_t color, uint32_t bg, int scale = 1)
{
    if (alpha <= 0.0f) return;
    DrawCenteredText(y, text, scale, BlendColor(bg, color, alpha));
}

static void RenderIntroCutscene(uint32_t bg, uint32_t white, uint32_t red,
                                uint32_t dark, uint32_t particleDark)
{
    uint32_t black = Rgb(1, 0, 3);
    Clear(black);

    float t = gIntroTimer;
    float pitWake = SmoothStep((t - 2.4f) / 1.2f);
    float fall = SmoothStep((t - 1.0f) / 4.8f);

    for (int y = 0; y < kBufferH; ++y)
    {
        float depth = (float)y / (float)(kBufferH - 1);
        uint32_t shade = BlendColor(black, Rgb(18, 2, 10), depth * 0.42f);
        for (int x = 0; x < kBufferW; ++x)
            gPixels[y * kBufferW + x] = shade;
    }

    for (int band = 0; band < 6; ++band)
    {
        int baseY = 134 + band * 8;
        int drift = (int)(sin(t * (0.65f + band * 0.08f) + band) * 18.0f);
        for (int x = -30; x < kBufferW + 34; x += 24)
        {
            BlendCircleRaw(x + drift + (band & 1) * 12, baseY + (x & 5),
                           20 + band * 5, BlendColor(particleDark, red, 0.38f),
                           0.026f + pitWake * 0.018f);
        }
    }

    for (int eye = 0; eye < 7; ++eye)
    {
        int ex = 30 + eye * 43 + (eye % 2) * 9;
        int ey = 102 + (eye % 3) * 15;
        float a = pitWake * (0.28f + 0.08f * (float)(eye % 2));
        BlendCircleRaw(ex, ey, 10, red, 0.09f * a);
        DrawLine(ex - 5, ey, ex + 5, ey, BlendColor(black, red, a));
        if (a > 0.16f) PutPixel(ex, ey, BlendColor(black, white, a));
    }

    Vec2 ship =
    {
        160.0f + (float)sin(t * 1.35f) * 18.0f,
        -18.0f + fall * 126.0f
    };
    if (t > 0.8f && t < 6.2f)
    {
        float trailA = CutsceneAlpha(t, 0.8f, 6.0f);
        for (int i = 0; i < 5; ++i)
        {
            int ty = (int)(ship.y - 18.0f - i * 9.0f);
            BlendCircleRaw((int)ship.x - i % 2, ty, 5 - i / 2,
                           BlendColor(red, Rgb(255, 160, 72), 0.18f),
                           0.10f * trailA);
        }
        DrawIntroShip(ship, 1.57f + sin(t * 2.1f) * 0.28f,
                      Rgb(144, 38, 46), Rgb(8, 8, 14));
    }

    DrawIntroText(44, "YOU SOLD YOUR WAY OUT.", CutsceneAlpha(t, 0.35f, 2.35f),
                  white, black);
    DrawIntroText(66, "THE PIT WANTS THE REST.", CutsceneAlpha(t, 2.20f, 4.55f),
                  Rgb(228, 66, 82), black);
    DrawIntroText(68, "CLIMB.", CutsceneAlpha(t, 4.75f, 6.95f),
                  Rgb(255, 76, 68), black, 2);

    float skipA = SmoothStep((t - 0.8f) / 0.8f) *
                  (0.55f + 0.25f * sin(t * 4.0f));
    DrawCenteredText(158, "CLICK / ENTER / SPACE TO SKIP", 1,
                     BlendColor(black, Rgb(150, 112, 126), skipA));
}

static void RenderMenuScreen()
{
    uint32_t bg = Rgb(5, 1, 7);
    uint32_t white = Rgb(235, 220, 205);
    uint32_t red = Rgb(210, 18, 32);
    uint32_t dark = Rgb(20, 6, 17);
    uint32_t particleDark = Rgb(112, 5, 20);

    if (gScreen == ScreenMainMenu)
        RenderMainMenu(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenNewGameSelect)
        RenderNewGameSelect(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenIntroCutscene)
        RenderIntroCutscene(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenOptions)
        RenderOptions(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenHelp)
        RenderHelp(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenCodex)
        RenderCodex(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenAchievements)
        RenderAchievements(bg, white, red, dark, particleDark);
    else if (gScreen == ScreenCredits)
        RenderCredits(bg, white, red, dark, particleDark);

    DrawCustomCursor(white, red, dark);
    ApplyCrtEffect();
}

static void DarkenScreen(float amount)
{
    amount = Clamp(amount, 0.0f, 1.0f);
    for (int i = 0; i < kBufferW * kBufferH; ++i)
        gPixels[i] = BlendColor(gPixels[i], Rgb(0, 0, 0), amount);
}

static void TintScreen(uint32_t color, float amount)
{
    amount = Clamp(amount, 0.0f, 1.0f);
    for (int i = 0; i < kBufferW * kBufferH; ++i)
        gPixels[i] = BlendColor(gPixels[i], color, amount);
}

static void RenderEndingLightLeak(float amount, uint32_t white, uint32_t red, uint32_t dark)
{
    amount = Clamp(amount, 0.0f, 1.0f);
    if (amount <= 0.0f) return;

    uint32_t gold = BlendColor(Rgb(255, 190, 96), white, 0.35f);
    BlendCircleRaw(kBufferW / 2, -18, 82, gold, 0.42f * amount);
    BlendCircleRaw(kBufferW / 2, -8, 46, white, 0.30f * amount);
    for (int i = 0; i < 5; ++i)
    {
        int x0 = 34 + i * 56;
        int wobble = ((int)(gEndingTimer * 70.0f) + i * 17) % 15 - 7;
        uint32_t ray = BlendColor(dark, gold, 0.42f + 0.05f * (float)(i % 2));
        FillTriangle({ (float)(x0 - 9 + wobble), 0.0f },
                     { (float)(x0 + 13 + wobble), 0.0f },
                     { (float)(x0 + 34 - i * 4), 118.0f },
                     ray);
    }
    TintScreen(gold, 0.055f * amount);
}

static void RenderDeathOverlay(uint32_t white, uint32_t red, uint32_t dark)
{
    float fade = Clamp(gPlayerDeathTimer / 0.22f, 0.0f, 1.0f);
    DarkenScreen(0.82f * fade);

    uint32_t deepRed = Rgb(54, 0, 8);
    uint32_t titleRed = BlendColor(deepRed, red, fade);
    uint32_t dimWhite = BlendColor(dark, white, fade);
    DrawCenteredText(49, "YOUR LIFE IS OVER", 2, deepRed);
    DrawCenteredText(47, "YOUR LIFE IS OVER", 2, titleRed);

    char stageText[40];
    wsprintfA(stageText, "%s BEST: %dM",
              CurrentChapterConfig().name,
              gBestStageMeters[gChapter]);
    DrawCenteredText(82, stageText, 1, dimWhite);

    char reachedText[32];
    wsprintfA(reachedText, "REACHED: %dM", gDeathStageMeters);
    DrawCenteredText(96, reachedText, 1, Rgb(162, 108, 112));

    bool inputReady = gPlayerDeathTimer >= 0.25f;
    DrawMenuButton(56, 132, 92, 20, "RESTART", inputReady, white, red, dark);
    DrawMenuButton(172, 132, 92, 20, "MAIN MENU", inputReady, white, red, dark);
}

static void RenderChapterTransitionOverlay(uint32_t white, uint32_t red, uint32_t dark)
{
    float elapsed = kChapterTransitionSeconds - gChapterTransitionTimer;
    float fade = SmoothStep(elapsed / 0.28f);
    DarkenScreen(0.78f * fade);
    float slam = 1.0f - SmoothStep(AbsFloat(elapsed - 0.58f) / 0.16f);
    TintScreen(Rgb(255, 40, 52), 0.16f * slam);

    uint32_t sealRed = BlendColor(dark, red, 0.62f * fade);
    for (int i = 0; i < 5; ++i)
    {
        int y = 28 + i * 24;
        int inset = (int)(SmoothStep(elapsed / 0.70f) * (float)(52 + i * 10));
        DrawLine(0, y, inset, y + 10, sealRed);
        DrawLine(kBufferW - 1, y, kBufferW - 1 - inset, y + 10, sealRed);
    }

    if (elapsed > 0.14f)
    {
        uint32_t textColor = BlendColor(dark, white, fade);
        uint32_t sealColor = BlendColor(dark, red, fade);
        DrawCenteredText(66, "PIT SEALED", 2, sealColor);
        if (gChapterTransitionWaitingConfirm)
        {
            DrawCenteredText(91, "RING CLEARED", 1, textColor);
            DrawMenuButton(112, 132, 96, 20, "CONTINUE", true, white, red, dark);
            DrawCenteredText(158, "ENTER / CLICK", 1, Rgb(150, 112, 126));
        }
        else
        {
            DrawCenteredText(91, "DESCENDING TO NEXT RING", 1, textColor);
        }
    }
}

static void RenderEndingOverlay(uint32_t white, uint32_t red, uint32_t dark)
{
    float flash = 1.0f - SmoothStep(gEndingTimer / 0.20f);
    float light = SmoothStep((gEndingTimer - 0.12f) / 0.58f);
    float textFade = SmoothStep((gEndingTimer - 0.24f) / 0.38f);
    DarkenScreen(0.78f * textFade);
    RenderEndingLightLeak(light, white, red, dark);
    if (flash > 0.0f)
        TintScreen(Rgb(255, 246, 218), 0.82f * flash);
    uint32_t titleColor = BlendColor(dark, Rgb(255, 74, 80), textFade);
    uint32_t textColor = BlendColor(dark, white, textFade);
    DrawCenteredText(56, "YOU ESCAPED", 2, titleColor);
    DrawCenteredText(84, "THE PIT CLOSES BELOW", 1, textColor);
    DrawCenteredText(103, "FINAL HEIGHT: 8000M", 1,
                     BlendColor(dark, Rgb(196, 140, 126), textFade));
    DrawMenuButton(112, 132, 96, 20, "MAIN MENU", gEndingTimer > 0.52f,
                   white, red, dark);
}

static void RenderPerkRouletteOverlay(uint32_t white, uint32_t red, uint32_t dark)
{
    if (!gPerkRouletteOpen || gPerkRouletteFinal == PerkNone) return;
    float elapsed = kPerkRouletteSeconds - gPerkRouletteTimer;
    float fade = SmoothStep(elapsed / 0.18f);
    if (!gPerkRouletteWaitingConfirm && gPerkRouletteTimer < 0.25f)
        fade *= SmoothStep(gPerkRouletteTimer / 0.25f);
    DarkenScreen(0.72f * fade);

    PerkType shown = gPerkRouletteFinal;
    if (!gPerkRouletteWaitingConfirm && gPerkRouletteTimer > 0.55f)
    {
        int cycle = ((int)(elapsed * 18.0f)) % (PerkCount - 1);
        shown = (PerkType)(cycle + 1);
    }

    uint32_t edge = (!gPerkRouletteWaitingConfirm && gPerkRouletteTimer > 0.55f)
        ? (((int)(elapsed * 24.0f) & 1) ? red : Rgb(220, 166, 74))
        : Rgb(238, 66, 82);
    FillRect(82, 34, 156, 104, Rgb(8, 2, 10));
    DrawLine(82, 34, 237, 34, edge);
    DrawLine(82, 137, 237, 137, edge);
    DrawLine(82, 34, 82, 137, edge);
    DrawLine(237, 34, 237, 137, edge);

    DrawCenteredText(46, "NEW PERK", 2, red);
    FillRect(136, 70, 48, 42, dark);
    DrawLine(136, 70, 183, 70, edge);
    DrawLine(136, 111, 183, 111, edge);
    DrawLine(136, 70, 136, 111, edge);
    DrawLine(183, 70, 183, 111, edge);
    DrawPerkIcon(152, 82, shown, white, red, dark);
    DrawCenteredText(119, PerkName(shown), 1, white);
    if (gPerkRouletteWaitingConfirm || gPerkRouletteTimer <= 0.55f)
        DrawCenteredText(130, PerkRouletteDescription(shown), 1, Rgb(220, 166, 74));
    if (gPerkRouletteWaitingConfirm)
    {
        DrawMenuButton(112, 148, 96, 18, "CONFIRM", true, white, red, dark);
        DrawCenteredText(169, "ENTER / CLICK", 1, Rgb(150, 112, 126));
    }
}

static void RenderAchievementPopup(uint32_t white, uint32_t red, uint32_t dark)
{
    if (gAchievementPopup == AchievementCount) return;
    float visible = gAchievementPopupTimer;
    float fade = 1.0f;
    if (visible > 3.0f) fade = SmoothStep((3.4f - visible) / 0.4f);
    else if (visible < 0.45f) fade = SmoothStep(visible / 0.45f);
    float punch = visible > 3.0f ? (1.0f - SmoothStep((3.4f - visible) / 0.22f)) : 0.0f;
    int slide = (int)((1.0f - fade) * -104.0f);
    int x = 10 + slide;
    int y = 130 - (int)(punch * 2.0f);
    uint32_t panel = BlendColor(Rgb(0, 0, 0), dark, 0.45f);
    uint32_t edge = BlendColor(dark, red, fade);
    BlendCircleRaw(x + 26, y + 18, 26, red, 0.12f * fade + 0.08f * punch);
    FillRect(x + 2, y + 2, 148, 38, Rgb(2, 0, 4));
    FillRect(x, y, 148, 38, panel);
    DrawLine(x, y, x + 147, y, edge);
    DrawLine(x, y + 37, x + 147, y + 37, edge);
    DrawLine(x, y, x, y + 37, edge);
    DrawLine(x + 147, y, x + 147, y + 37, edge);
    DrawLine(x + 4, y + 3, x + 58, y + 3, BlendColor(edge, white, 0.18f));
    DrawAchievementIcon(x + 8, y + 9, gAchievementPopup, true, white, red, dark);
    DrawItalicText(x + 34, y + 7, "ACHIEVEMENT", 1,
                   BlendColor(dark, Rgb(220, 166, 74), fade));
    DrawItalicText(x + 34, y + 18, AchievementName(gAchievementPopup), 1,
                   BlendColor(dark, white, fade));
    DrawItalicText(x + 34, y + 28, AchievementDescription(gAchievementPopup), 1,
                   BlendColor(dark, Rgb(164, 118, 128), fade));
}

static void DrawCrosshairAt(int x, int y, int style, bool cooldown,
                            uint32_t white, uint32_t red, uint32_t dark)
{
    uint32_t mainColor = cooldown ? Rgb(112, 82, 92) : white;
    uint32_t accent = cooldown ? Rgb(112, 24, 38) : red;
    uint32_t shadow = BlendColor(dark, Rgb(0, 0, 0), 0.45f);

    if (style == 1)
    {
        PutPixel(x - 1, y, shadow);
        PutPixel(x + 1, y, shadow);
        PutPixel(x, y - 1, shadow);
        PutPixel(x, y + 1, shadow);
        PutPixel(x, y, mainColor);
        PutPixel(x - 1, y - 1, accent);
        PutPixel(x + 1, y + 1, accent);
    }
    else if (style == 2)
    {
        PutPixel(x, y - 5, shadow);
        PutPixel(x + 5, y, shadow);
        PutPixel(x, y + 5, shadow);
        PutPixel(x - 5, y, shadow);
        PutPixel(x, y - 4, mainColor);
        PutPixel(x + 1, y - 3, mainColor);
        PutPixel(x + 2, y - 2, mainColor);
        PutPixel(x + 3, y - 1, mainColor);
        PutPixel(x + 4, y, mainColor);
        PutPixel(x + 3, y + 1, mainColor);
        PutPixel(x + 2, y + 2, mainColor);
        PutPixel(x + 1, y + 3, mainColor);
        PutPixel(x, y + 4, mainColor);
        PutPixel(x - 1, y + 3, mainColor);
        PutPixel(x - 2, y + 2, mainColor);
        PutPixel(x - 3, y + 1, mainColor);
        PutPixel(x - 4, y, mainColor);
        PutPixel(x - 3, y - 1, mainColor);
        PutPixel(x - 2, y - 2, mainColor);
        PutPixel(x - 1, y - 3, mainColor);
        PutPixel(x, y, accent);
    }
    else if (style == 3)
    {
        DrawLine(x - 6, y - 5, x - 4, y - 5, shadow);
        DrawLine(x - 5, y - 6, x - 5, y - 4, shadow);
        DrawLine(x + 4, y - 5, x + 6, y - 5, shadow);
        DrawLine(x + 5, y - 6, x + 5, y - 4, shadow);
        DrawLine(x - 6, y + 5, x - 4, y + 5, shadow);
        DrawLine(x - 5, y + 4, x - 5, y + 6, shadow);
        DrawLine(x + 4, y + 5, x + 6, y + 5, shadow);
        DrawLine(x + 5, y + 4, x + 5, y + 6, shadow);
        DrawLine(x - 5, y - 4, x - 3, y - 4, mainColor);
        DrawLine(x - 4, y - 5, x - 4, y - 3, mainColor);
        DrawLine(x + 3, y - 4, x + 5, y - 4, mainColor);
        DrawLine(x + 4, y - 5, x + 4, y - 3, mainColor);
        DrawLine(x - 5, y + 4, x - 3, y + 4, mainColor);
        DrawLine(x - 4, y + 3, x - 4, y + 5, mainColor);
        DrawLine(x + 3, y + 4, x + 5, y + 4, mainColor);
        DrawLine(x + 4, y + 3, x + 4, y + 5, mainColor);
        PutPixel(x, y, accent);
    }
    else
    {
        DrawLine(x - 6, y, x - 3, y, shadow);
        DrawLine(x + 3, y, x + 6, y, shadow);
        DrawLine(x, y - 6, x, y - 3, shadow);
        DrawLine(x, y + 3, x, y + 6, shadow);
        DrawLine(x - 5, y, x - 3, y, mainColor);
        DrawLine(x + 3, y, x + 5, y, mainColor);
        DrawLine(x, y - 5, x, y - 3, mainColor);
        DrawLine(x, y + 3, x, y + 5, mainColor);
        PutPixel(x - 2, y, accent);
        PutPixel(x + 2, y, accent);
        PutPixel(x, y - 2, accent);
        PutPixel(x, y + 2, accent);
        PutPixel(x, y, cooldown ? shadow : Rgb(255, 216, 176));
    }
}

static void DrawCustomCursor(uint32_t white, uint32_t red, uint32_t dark)
{
    int x = gInput.mouseX;
    int y = gInput.mouseY;
    if (x < 0 || x >= kBufferW || y < 0 || y >= kBufferH) return;

    bool cooldown = gScreen == ScreenGameplay && HookOnCooldown();
    DrawCrosshairAt(x, y, gCrosshairStyle, cooldown, white, red, dark);
}

static void Render()
{
    if (gScreen != ScreenGameplay)
    {
        RenderMenuScreen();
        return;
    }

    if (gCameraShakeTimer > 0.0f)
    {
        int phase = (int)(gCameraShakeTimer * 1000.0f);
        gRenderOffsetX = ((phase * 3) % 5) - 2;
        gRenderOffsetY = ((phase * 7 + 2) % 5) - 2;
    }
    else
    {
        gRenderOffsetX = 0;
        gRenderOffsetY = 0;
    }

    uint32_t bg = Rgb(5, 1, 7);
    uint32_t dark = Rgb(20, 6, 17);
    uint32_t white = Rgb(235, 220, 205);
    uint32_t red = Rgb(210, 18, 32);
    uint32_t blue = Rgb(125, 16, 38);
    uint32_t ground = Rgb(82, 65, 84);
    uint32_t groundEdge = Rgb(126, 101, 128);
    uint32_t pipeVein = Rgb(104, 36, 68);
    uint32_t enemyColor = Rgb(130, 9, 20);
    uint32_t walkerColor = Rgb(185, 38, 28);
    uint32_t heartEnemyColor = Rgb(235, 72, 128);
    uint32_t wormColor = Rgb(154, 18, 55);
    uint32_t projectileColor = Rgb(245, 175, 70);
    uint32_t orbColor = Rgb(182, 84, 245);
    uint32_t respawnColor = Rgb(190, 54, 92);
    uint32_t trailDark = Rgb(54, 20, 58);
    uint32_t trailBright = Rgb(178, 44, 96);
    uint32_t slashEdge = Rgb(125, 0, 8);
    uint32_t slashFill = Rgb(220, 8, 18);
    uint32_t slashCore = Rgb(255, 82, 64);
    uint32_t playerReady = Rgb(228, 76, 64);
    uint32_t playerSpent = Rgb(86, 54, 76);
    uint32_t playerColor = gPlayer.jumpAvailable ? playerReady : playerSpent;
    if (gSuperJumpTimer > 0.0f)
        playerColor = Rgb(218, 132, 255);
    uint32_t monsterDark = Rgb(24, 0, 13);
    uint32_t monsterFlesh = Rgb(78, 4, 30);
    uint32_t monsterEdge = Rgb(142, 10, 36);
    uint32_t particleBright = Rgb(255, 54, 42);
    uint32_t particleDark = Rgb(112, 5, 20);

    switch (gChapter)
    {
    case 1:
        bg = Rgb(3, 5, 10);
        dark = Rgb(8, 18, 26);
        red = Rgb(222, 42, 30);
        blue = Rgb(58, 86, 128);
        ground = Rgb(76, 82, 96);
        groundEdge = Rgb(118, 126, 142);
        pipeVein = Rgb(76, 74, 112);
        enemyColor = Rgb(145, 18, 24);
        walkerColor = Rgb(205, 62, 32);
        wormColor = Rgb(126, 28, 74);
        projectileColor = Rgb(245, 145, 58);
        monsterDark = Rgb(15, 5, 22);
        monsterFlesh = Rgb(66, 18, 48);
        monsterEdge = Rgb(150, 30, 48);
        break;
    case 2:
        bg = Rgb(2, 8, 10);
        dark = Rgb(4, 22, 24);
        red = Rgb(164, 52, 82);
        blue = Rgb(36, 112, 124);
        ground = Rgb(50, 92, 88);
        groundEdge = Rgb(86, 144, 132);
        pipeVein = Rgb(36, 124, 112);
        enemyColor = Rgb(112, 24, 70);
        walkerColor = Rgb(188, 52, 86);
        wormColor = Rgb(94, 32, 86);
        projectileColor = Rgb(204, 198, 88);
        orbColor = Rgb(150, 108, 255);
        monsterDark = Rgb(3, 18, 18);
        monsterFlesh = Rgb(24, 78, 72);
        monsterEdge = Rgb(154, 64, 102);
        break;
    case 3:
        bg = Rgb(8, 2, 3);
        dark = Rgb(25, 5, 8);
        red = Rgb(238, 18, 22);
        blue = Rgb(118, 24, 42);
        ground = Rgb(92, 42, 48);
        groundEdge = Rgb(154, 70, 74);
        pipeVein = Rgb(178, 28, 42);
        enemyColor = Rgb(180, 8, 16);
        walkerColor = Rgb(232, 64, 28);
        wormColor = Rgb(180, 20, 58);
        projectileColor = Rgb(255, 102, 48);
        orbColor = Rgb(210, 74, 255);
        monsterDark = Rgb(24, 0, 2);
        monsterFlesh = Rgb(96, 4, 18);
        monsterEdge = Rgb(205, 20, 34);
        break;
    default:
        break;
    }

    Clear(bg);
    DrawPerspectivePipeBackdrop(bg, dark, pipeVein, red);

    int gridOffsetY = (int)gViewHeightPixels & 15;
    for (int y = gridOffsetY - 16; y < kBufferH; y += 16)
    {
        float screenDepth = Clamp((float)y / (float)kBufferH, 0.0f, 1.0f);
        float bulge = 6.0f + screenDepth * 17.0f;
        int previousX = 0;
        int previousY = y;
        for (int x = 6; x < kBufferW; x += 6)
        {
            float u = ((float)x / (float)(kBufferW - 1)) * 2.0f - 1.0f;
            int curveY = y - (int)(bulge * (1.0f - u * u));
            DrawLine(previousX, previousY, x, curveY, dark);
            previousX = x;
            previousY = curveY;
        }
        DrawLine(previousX, previousY, kBufferW - 1, y, dark);
    }
    for (int x = 0; x < kBufferW; x += 16)
        DrawLine(x, 0, x, kBufferH - 1, dark);

    for (int i = 0; i < (int)(sizeof(gPlatforms) / sizeof(gPlatforms[0])); ++i)
    {
        if (!gPlatforms[i].active) continue;
        DrawPlatformShape(gPlatforms[i], ground, groundEdge);
    }

    if (CurrentChapterConfig().pipeWalls)
    {
        DrawPipeWalls(ground, red, pipeVein);
    }
    else
    {
        for (int i = 0; i < (int)(sizeof(gSideWalls) / sizeof(gSideWalls[0])); ++i)
        {
            const RectF& wall = gSideWalls[i];
            FillRect((int)wall.x, (int)wall.y, (int)wall.w, (int)wall.h, ground);
            DrawLine((int)(wall.x + (i == 0 ? wall.w - 1.0f : 0.0f)), 0,
                     (int)(wall.x + (i == 0 ? wall.w - 1.0f : 0.0f)),
                     kBufferH - 1, red);
        }
    }

    if (gChapterTransitionActive)
    {
        int left = (int)PlayfieldLeft();
        int width = (int)(PlayfieldRight() - PlayfieldLeft());
        FillRect(left, 158, width, 22, ground);
        DrawLine(left, 158, left + width - 1, 158, Rgb(162, 126, 148));
        float elapsed = kChapterTransitionSeconds - gChapterTransitionTimer;
        float close = SmoothStep((elapsed - kChapterTransitionMoveSeconds * 0.55f) /
                                 (kChapterTransitionSeconds -
                                  kChapterTransitionMoveSeconds * 0.55f));
        int doorWidth = (int)((float)width * 0.5f * close);
        FillRect(left, 160, doorWidth, 20, monsterFlesh);
        FillRect(left + width - doorWidth, 160, doorWidth, 20, monsterFlesh);
        DrawLine(left + doorWidth, 160, left + doorWidth, kBufferH - 1, red);
        DrawLine(left + width - doorWidth, 160, left + width - doorWidth, kBufferH - 1, red);
        for (int rib = 0; rib < 7; ++rib)
        {
            int y = 162 + rib * 3;
            DrawLine(left, y, left + doorWidth - 2, y + (rib & 1), monsterEdge);
            DrawLine(left + width - doorWidth + 2, y + (rib & 1),
                     left + width - 1, y, monsterEdge);
        }
        float slam = 1.0f - SmoothStep(AbsFloat(elapsed - 0.58f) / 0.14f);
        if (slam > 0.01f)
        {
            int seam = left + width / 2;
            DrawLine(seam - 2, 158, seam - 2, kBufferH - 1, Rgb(255, 74, 76));
            DrawLine(seam + 2, 158, seam + 2, kBufferH - 1, Rgb(255, 74, 76));
            BlendCircleRaw(seam, 160, 28, Rgb(255, 78, 66), 0.25f * slam);
        }
    }

    for (int i = 0; i < (int)(sizeof(gEnemies) / sizeof(gEnemies[0])); ++i)
    {
        if (!gEnemies[i].active) continue;
        RectF enemy = gEnemies[i].bounds;
        if (gEnemies[i].alive && gEnemies[i].type != EnemyWalker)
            enemy.y += FloatBob(gEnemies[i].floatSeed);
        if (!gEnemies[i].alive)
        {
            if (gEnemies[i].type != EnemyWalker)
            {
                int cx = (int)(enemy.x + enemy.w * 0.5f);
                int cy = (int)(enemy.y + enemy.h * 0.5f);
                int radius = ((int)(gEnemies[i].respawnTimer * 12.0f) & 1) ? 5 : 4;
                DrawLine(cx - radius, cy, cx + radius, cy, respawnColor);
                DrawLine(cx, cy - radius, cx, cy + radius, respawnColor);
                PutPixel(cx - radius, cy - radius, white);
                PutPixel(cx + radius, cy + radius, white);
            }
            continue;
        }
        if (gEnemies[i].type == EnemyWalker)
        {
            DrawCrawlerEnemy(enemy, walkerColor, BlendColor(walkerColor, Rgb(0, 0, 0), 0.35f),
                             Rgb(255, 214, 156), gEnemies[i].velocityX >= 0.0f);
        }
        else if (gEnemies[i].type == EnemyHeartFlying)
        {
            DrawPureAngelEnemy(enemy, Rgb(232, 170, 190), Rgb(232, 224, 210),
                               Rgb(255, 80, 132));
        }
        else
        {
            DrawFallenAngelEnemy(enemy, enemyColor, Rgb(92, 18, 34),
                                 Rgb(252, 230, 172));
        }
    }

    for (int i = 0; i < (int)(sizeof(gWorms) / sizeof(gWorms[0])); ++i)
    {
        const Worm& worm = gWorms[i];
        if (!worm.active) continue;
        for (int point = worm.pointCount - 1; point > 0; --point)
        {
            DrawLine((int)worm.points[point].x, (int)worm.points[point].y,
                     (int)worm.points[point - 1].x, (int)worm.points[point - 1].y,
                     BlendColor(wormColor, Rgb(0, 0, 0), 0.28f));
        }
        for (int point = worm.pointCount - 1; point >= 0; --point)
        {
            int px = (int)worm.points[point].x;
            int py = (int)worm.points[point].y;
            int radius = point == 0 ? 7 : 5;
            uint32_t segmentColor = point == 0 ? walkerColor : wormColor;
            FillCircle(px, py, radius, segmentColor);
            FillCircle(px - 1, py - 2, radius > 5 ? 2 : 1,
                       BlendColor(segmentColor, white, 0.24f));
            int limb = (point & 1) ? 1 : -1;
            uint32_t limbColor = BlendColor(wormColor, Rgb(25, 0, 10), 0.22f);
            DrawLine(px - 3, py, px - 8, py + limb * 3, limbColor);
            DrawLine(px + 3, py, px + 8, py - limb * 3, limbColor);
            DrawLine(px - 8, py + limb * 3, px - 10, py + limb * 5, limbColor);
            DrawLine(px + 8, py - limb * 3, px + 10, py - limb * 5, limbColor);
            PutPixel(px - 11, py + limb * 5, white);
            PutPixel(px + 11, py - limb * 5, white);
        }
        Vec2 head = worm.points[0];
        Vec2 direction = worm.pointCount > 1 ? Normalize(Sub(head, worm.points[1])) :
                         Normalize(Sub(gPlayer.position, head));
        Vec2 normal = { -direction.y, direction.x };
        Vec2 mouthCenter = { head.x + direction.x * 5.0f, head.y + direction.y * 5.0f };
        FillTriangle({ mouthCenter.x + normal.x * 4.0f, mouthCenter.y + normal.y * 4.0f },
                     { mouthCenter.x - normal.x * 4.0f, mouthCenter.y - normal.y * 4.0f },
                     { mouthCenter.x + direction.x * 6.0f, mouthCenter.y + direction.y * 6.0f },
                     Rgb(24, 0, 6));
        for (int tooth = -2; tooth <= 2; ++tooth)
        {
            Vec2 base =
            {
                mouthCenter.x + normal.x * (float)(tooth * 2),
                mouthCenter.y + normal.y * (float)(tooth * 2)
            };
            Vec2 tip =
            {
                base.x + direction.x * 4.0f,
                base.y + direction.y * 4.0f
            };
            DrawLine((int)base.x, (int)base.y, (int)tip.x, (int)tip.y, white);
        }
        PutPixel((int)head.x - 2, (int)head.y - 2, white);
        PutPixel((int)head.x + 2, (int)head.y - 1, white);
    }

    for (int i = 0; i < (int)(sizeof(gTraps) / sizeof(gTraps[0])); ++i)
    {
        const WallTrap& trap = gTraps[i];
        if (!trap.active) continue;
        DrawWallMawTrap(trap, BlendColor(enemyColor, Rgb(30, 0, 8), 0.18f),
                        monsterEdge, Rgb(255, 216, 158), projectileColor);
    }

    for (int i = 0; i < (int)(sizeof(gProjectiles) / sizeof(gProjectiles[0])); ++i)
    {
        const Projectile& projectile = gProjectiles[i];
        if (!projectile.active) continue;
        uint32_t bulletColor = projectile.hookable ? projectileColor : Rgb(190, 44, 54);
        uint32_t bulletCore = projectile.hookable ? white : Rgb(52, 8, 12);
        Vec2 flight = Normalize(projectile.velocity);
        int sx = (int)projectile.position.x + gRenderOffsetX;
        int sy = (int)projectile.position.y + gRenderOffsetY;
        BlendCircleRaw(sx, sy, projectile.hookable ? 9 : 8,
                       projectile.hookable ? Rgb(238, 142, 54) : Rgb(204, 34, 44),
                       projectile.hookable ? 0.36f : 0.28f);
        for (int trail = 1; trail <= 6; ++trail)
        {
            Vec2 normal = { -flight.y, flight.x };
            float wobble = (float)((trail & 1) ? 1 : -1) *
                           (float)(1 + (trail % 3));
            int tx = sx - (int)(flight.x * (float)(trail * 5)) +
                     (int)(normal.x * wobble);
            int ty = sy - (int)(flight.y * (float)(trail * 5)) +
                     (int)(normal.y * wobble);
            int radius = trail < 3 ? 4 : 2;
            uint32_t trailColor = projectile.hookable ? Rgb(216, 68, 22) : Rgb(152, 12, 26);
            BlendCircleRaw(tx, ty, radius, trailColor, 0.22f / (float)trail);
            if ((trail & 1) == 0)
                BlendPixelRaw(tx, ty, projectile.hookable ? Rgb(255, 186, 74) : Rgb(224, 58, 62),
                              0.32f / (float)trail);
        }
        FillCircle((int)projectile.position.x, (int)projectile.position.y,
                   projectile.hookable ? 5 : 4, bulletColor);
        FillCircle((int)projectile.position.x, (int)projectile.position.y,
                   projectile.hookable ? 2 : 1, bulletCore);
    }

    for (int i = 0; i < (int)(sizeof(gSawBlades) / sizeof(gSawBlades[0])); ++i)
    {
        const SawBlade& blade = gSawBlades[i];
        if (!blade.active) continue;
        int x = (int)blade.position.x;
        int y = (int)blade.position.y;
        uint32_t metal = Rgb(178, 188, 196);
        uint32_t core = Rgb(42, 38, 48);
        FillRect(x - 5, y - 5, 11, 11, metal);
        FillRect(x - 2, y - 2, 5, 5, core);
        int frame = ((int)(blade.spin * 2.0f)) & 3;
        if ((frame & 1) == 0)
        {
            PutPixel(x, y - 8, white);
            PutPixel(x, y + 8, white);
            PutPixel(x - 8, y, white);
            PutPixel(x + 8, y, white);
            DrawLine(x - 7, y - 7, x - 4, y - 4, metal);
            DrawLine(x + 7, y - 7, x + 4, y - 4, metal);
            DrawLine(x - 7, y + 7, x - 4, y + 4, metal);
            DrawLine(x + 7, y + 7, x + 4, y + 4, metal);
        }
        else
        {
            PutPixel(x - 6, y - 6, white);
            PutPixel(x + 6, y - 6, white);
            PutPixel(x - 6, y + 6, white);
            PutPixel(x + 6, y + 6, white);
            DrawLine(x, y - 8, x, y - 5, metal);
            DrawLine(x, y + 8, x, y + 5, metal);
            DrawLine(x - 8, y, x - 5, y, metal);
            DrawLine(x + 8, y, x + 5, y, metal);
        }
    }

    for (int i = 0; i < (int)(sizeof(gSuperOrbs) / sizeof(gSuperOrbs[0])); ++i)
    {
        const SuperOrb& orb = gSuperOrbs[i];
        if (!orb.active) continue;
        int x = (int)orb.position.x;
        int pulse = ((int)(gMapTime * 12.0f + (float)i * 3.0f)) & 3;
        int y = (int)orb.position.y + (pulse == 1 ? -1 : (pulse == 3 ? 1 : 0));
        int radiusPulse = (pulse == 0 || pulse == 3) ? 1 : 0;
        int sx = x + gRenderOffsetX;
        int sy = y + gRenderOffsetY;
        BlendCircleRaw(sx, sy, (int)kSuperOrbHitRadius + radiusPulse, orbColor, 0.24f);
        FillCircle(x, y, 6 + radiusPulse, Rgb(62, 10, 92));
        FillTriangle({ (float)x, (float)y - 9.0f - (float)radiusPulse },
                     { (float)x - 7.0f - (float)radiusPulse, (float)y },
                     { (float)x, (float)y + 9.0f + (float)radiusPulse }, Rgb(132, 48, 210));
        FillTriangle({ (float)x, (float)y - 9.0f - (float)radiusPulse },
                     { (float)x + 7.0f + (float)radiusPulse, (float)y },
                     { (float)x, (float)y + 9.0f + (float)radiusPulse }, orbColor);
        FillCircle(x, y, 3, Rgb(230, 176, 255));
        PutPixel(x, y, white);
        if ((pulse & 1) == 0)
        {
            DrawLine(x - (int)kSuperOrbHitRadius - radiusPulse, y, x - 7, y, Rgb(212, 128, 255));
            DrawLine(x + 7, y, x + (int)kSuperOrbHitRadius + radiusPulse, y, Rgb(212, 128, 255));
            DrawLine(x, y - (int)kSuperOrbHitRadius - radiusPulse, x, y - 7, Rgb(212, 128, 255));
        }
        else
        {
            DrawLine(x - 8, y - 8, x - 5, y - 5, Rgb(212, 128, 255));
            DrawLine(x + 8, y - 8, x + 5, y - 5, Rgb(212, 128, 255));
            DrawLine(x, y + 8, x, y + (int)kSuperOrbHitRadius + radiusPulse, Rgb(212, 128, 255));
        }
    }

    Vec2 portal;
    bool freePortal = false;
    bool startPortal = false;
    if (ShopPortalPosition(&portal, &freePortal, &startPortal))
    {
        int x = (int)portal.x;
        int y = (int)portal.y;
        int portalPulse = ((int)(gMapTime * 10.0f)) & 3;
        int portalWobble = portalPulse == 0 ? 1 : (portalPulse == 2 ? -1 : 0);
        uint32_t portalOuter = Rgb(58, 10, 72);
        uint32_t portalInner = orbColor;
        int portalTick = (int)(gMapTime * 30.0f);
        for (int p = 0; p < 16; ++p)
        {
            uint32_t h = Hash2D(p, portalTick / 2, 617);
            float progress = (float)(h & 255u) / 255.0f +
                             gMapTime * (0.34f + (float)(p % 5) * 0.025f);
            progress -= (float)(int)progress;
            Vec2 direction = Normalize(
                {
                    ((float)((h >> 8) & 255u) / 255.0f) * 2.0f - 1.0f,
                    ((float)((h >> 16) & 255u) / 255.0f) * 2.0f - 1.0f
                });
            float radius = 31.0f - progress * 25.0f;
            int px = x + (int)(direction.x * radius);
            int py = y + (int)(direction.y * radius);
            int nx = x + (int)(direction.x * (radius - 5.0f));
            int ny = y + (int)(direction.y * (radius - 5.0f));
            uint32_t particleColor = BlendColor(portalInner, white, progress * 0.35f);
            DrawLine(px, py, nx, ny, particleColor);
            BlendCircleRaw(px + gRenderOffsetX, py + gRenderOffsetY,
                           progress > 0.75f ? 2 : 1, particleColor, 0.22f + progress * 0.22f);
        }
        BlendCircleRaw(x + gRenderOffsetX, y + gRenderOffsetY, 18 + portalWobble, portalInner, 0.22f);
        FillCircle(x, y, 13 + portalWobble, portalOuter);
        FillRect(x - 10 - portalWobble, y - 14, 21 + portalWobble * 2, 29, portalOuter);
        FillCircle(x, y, 10 + portalWobble, portalInner);
        FillRect(x - 7 - portalWobble, y - 11, 15 + portalWobble * 2, 23, portalInner);
        FillCircle(x, y, 6, Rgb(12, 1, 18));
        FillRect(x - 4, y - 8, 9, 17, Rgb(12, 1, 18));
        DrawLine(x - 11, y - 10, x - 15, y - 15, portalInner);
        DrawLine(x + 11, y - 10, x + 15, y - 15, portalInner);
        DrawLine(x - 12, y + 11, x - 16, y + 15, portalOuter);
        DrawLine(x + 12, y + 11, x + 16, y + 15, portalOuter);
        DrawLine(x - 8, y - 12, x + 8, y - 12, white);
        PutPixel(x, y + 13, white);
    }

    int devourY = (int)gDevourerY;
    if (devourY < kBufferH)
        FillRect(6, devourY, kBufferW - 12, kBufferH - devourY, monsterDark);
    int backMotion = (int)(gMapTime * 13.0f);
    for (int i = 0; i < 8; ++i)
    {
        float baseX = 4.0f + (float)i * 40.0f;
        float sway = (float)(((backMotion + i * 9) % 19) - 9);
        float height = 18.0f + (float)((i * 17) % 28);
        Vec2 left = { baseX - 14.0f, gDevourerY + 16.0f };
        Vec2 right = { baseX + 14.0f, gDevourerY + 16.0f };
        Vec2 tip = { baseX + sway, gDevourerY - height };
        FillTriangle(left, right, tip, BlendColor(monsterFlesh, Rgb(0, 0, 0), 0.28f));
        DrawLine((int)left.x, (int)left.y, (int)tip.x, (int)tip.y,
                 BlendColor(monsterEdge, Rgb(0, 0, 0), 0.18f));
    }
    int frontMotion = (int)(-gMapTime * 19.0f);
    for (int i = 0; i < 7; ++i)
    {
        float baseX = 24.0f + (float)i * 45.0f;
        float sway = (float)(((frontMotion + i * 11) % 21) - 10);
        float height = 25.0f + (float)((i * 13) % 32);
        Vec2 left = { baseX - 12.0f, gDevourerY + 12.0f };
        Vec2 right = { baseX + 12.0f, gDevourerY + 12.0f };
        Vec2 tip = { baseX + sway, gDevourerY - height };
        FillTriangle(left, right, tip, monsterFlesh);
        DrawLine((int)left.x, (int)left.y, (int)tip.x, (int)tip.y, monsterEdge);
        DrawLine((int)tip.x, (int)tip.y, (int)right.x, (int)right.y, monsterEdge);
    }

    if (gDevourerY < (float)kBufferH - 8.0f)
    {
        for (int eye = 0; eye < 18; ++eye)
        {
            int x = 18 + (eye * 37 + (int)(Hash2D(eye, 2, 99) % 18u)) % (kBufferW - 36);
            int y = devourY + 8 + (int)(Hash2D(eye, 3, 101) % 42u);
            if (y >= kBufferH - 2) continue;
            int blink = ((int)(gMapTime * 6.0f) + eye * 3) % 17;
            uint32_t sclera = blink == 0 ? monsterEdge : Rgb(238, 190, 154);
            FillCircle(x, y, blink == 0 ? 2 : 3, red);
            FillRect(x - 3, y - 1, 7, 3, sclera);
            PutPixel(x, y, Rgb(18, 0, 4));
            if ((eye & 2) == 0)
                DrawLine(x - 5, y + 1, x - 9, y + 4, monsterEdge);
        }
    }

    DrawRisingBottomMist(red, particleBright, particleDark);

    for (int i = 0; i < (int)(sizeof(gParticles) / sizeof(gParticles[0])); ++i)
    {
        const Particle& particle = gParticles[i];
        if (particle.life <= 0.0f) continue;
        uint32_t color = particle.color;
        if (particle.life > 0.34f)
            FillRect((int)particle.position.x - 1, (int)particle.position.y - 1, 3, 3, color);
        else
            PutPixel((int)particle.position.x, (int)particle.position.y, color);
    }

    for (int i = 0; i < (int)(sizeof(gAfterImages) / sizeof(gAfterImages[0])); ++i)
    {
        if (gAfterImages[i].life <= 0.0f) continue;
        uint32_t ghostColor = gAfterImages[i].life > 0.08f ? trailBright : trailDark;
        FillRect(
            (int)(gAfterImages[i].position.x - gPlayer.halfW),
            (int)(gAfterImages[i].position.y - gPlayer.halfH),
            (int)(gPlayer.halfW * 2.0f + 1.0f),
            (int)(gPlayer.halfH * 2.0f + 1.0f),
            ghostColor);
    }

    if (gSlashTrail.life > 0.0f)
    {
        Vec2 slashDirection = Normalize(Sub(gSlashTrail.end, gSlashTrail.start));
        Vec2 normal = { -slashDirection.y, slashDirection.x };
        Vec2 delta = Sub(gSlashTrail.end, gSlashTrail.start);
        Vec2 middle =
        {
            gSlashTrail.start.x + delta.x * 0.5f,
            gSlashTrail.start.y + delta.y * 0.5f
        };
        float halfWidth = gSlashTrail.life > 0.07f ? 2.25f : 1.5f;
        Vec2 upper = { middle.x + normal.x * halfWidth,
                       middle.y + normal.y * halfWidth };
        Vec2 lower = { middle.x - normal.x * halfWidth,
                       middle.y - normal.y * halfWidth };

        FillTriangle(gSlashTrail.start, upper, lower, slashFill);
        FillTriangle(gSlashTrail.end, lower, upper, slashFill);
        DrawLine((int)gSlashTrail.start.x, (int)gSlashTrail.start.y,
                 (int)upper.x, (int)upper.y, slashEdge);
        DrawLine((int)upper.x, (int)upper.y,
                 (int)gSlashTrail.end.x, (int)gSlashTrail.end.y, slashEdge);
        DrawLine((int)gSlashTrail.end.x, (int)gSlashTrail.end.y,
                 (int)lower.x, (int)lower.y, slashEdge);
        DrawLine((int)lower.x, (int)lower.y,
                 (int)gSlashTrail.start.x, (int)gSlashTrail.start.y, slashEdge);
        DrawLine((int)gSlashTrail.start.x, (int)gSlashTrail.start.y,
                 (int)gSlashTrail.end.x, (int)gSlashTrail.end.y, slashCore);
    }

    if (gCrossReviveEffectTimer > 0.0f)
    {
        float t = Clamp(gCrossReviveEffectTimer / 0.62f, 0.0f, 1.0f);
        int cx = (int)gCrossReviveEffectPosition.x;
        int cy = (int)gCrossReviveEffectPosition.y - 8;
        int vertical = 13 + (int)((1.0f - t) * 10.0f);
        int horizontal = 6 + (int)((1.0f - t) * 5.0f);
        uint32_t halo = BlendColor(Rgb(238, 226, 184), white, t);
        DrawLine(cx - 1, cy - vertical, cx - 1, cy + vertical, halo);
        DrawLine(cx + 1, cy - vertical, cx + 1, cy + vertical, halo);
        DrawLine(cx - horizontal, cy - 4, cx + horizontal, cy - 4, halo);
        DrawLine(cx - horizontal, cy - 2, cx + horizontal, cy - 2, halo);
        DrawLine(cx, cy - vertical - 2, cx, cy + vertical + 2, white);
        DrawLine(cx - horizontal - 2, cy - 3, cx + horizontal + 2, cy - 3, white);
        BlendCircleRaw(cx + gRenderOffsetX, cy + gRenderOffsetY, 13,
                       Rgb(255, 246, 218), 0.18f * t);
    }

    if (!gPlayerDead)
    {
        Vec2 mouse = { (float)gInput.mouseX, (float)gInput.mouseY };
        bool drawFacingRight = gPlayerFacingRight;
        if (gPlayer.velocity.x > 8.0f)
            drawFacingRight = true;
        else if (gPlayer.velocity.x < -8.0f)
            drawFacingRight = false;
        else if (gHook.mode == HookIdle)
            drawFacingRight = mouse.x >= gPlayer.position.x;

        Vec2 hookAnchor =
        {
            gPlayer.position.x,
            gPlayer.position.y - 7.0f
        };
        Vec2 aim = Normalize(Sub(mouse, hookAnchor));
        if (gSuperJumpTimer > 0.0f)
            aim = { 0.0f, -1.0f };
        Vec2 bladeEnd;
        if (gHook.mode == HookIdle)
        {
            bladeEnd =
            {
                hookAnchor.x + aim.x * 24.0f,
                hookAnchor.y + aim.y * 24.0f
            };
            if (!HookOnCooldown())
                DrawLine((int)hookAnchor.x, (int)hookAnchor.y, (int)bladeEnd.x, (int)bladeEnd.y, blue);
        }
        else
        {
            bladeEnd = gHook.position;
            DrawLine(
                (int)hookAnchor.x,
                (int)hookAnchor.y,
                (int)bladeEnd.x,
                (int)bladeEnd.y,
                gHook.mode == HookPullEnemy ? red : blue);
        }

        if (gSuperJumpTimer > 0.0f)
        {
            int auraX = (int)hookAnchor.x;
            int auraY = (int)hookAnchor.y;
            BlendCircleRaw(auraX + gRenderOffsetX, auraY + gRenderOffsetY,
                           15, Rgb(162, 54, 255), 0.42f);
            BlendCircleRaw(auraX + gRenderOffsetX, auraY + gRenderOffsetY,
                           8, Rgb(246, 172, 255), 0.28f);
            int pulse = (int)(gMapTime * 48.0f);
            for (int wisp = 0; wisp < 7; ++wisp)
            {
                uint32_t h = Hash2D(wisp, pulse / 3, 271);
                float side = ((float)(h & 255u) / 255.0f) * 2.0f - 1.0f;
                int baseX = auraX + (int)(side * 10.0f);
                int baseY = auraY - 7 + (int)((h >> 8) % 18u);
                int tipX = auraX + (int)(side * 15.0f);
                int tipY = baseY - 9 - (int)((h >> 16) % 9u);
                uint32_t flameColor = (wisp & 1)
                    ? Rgb(216, 82, 255)
                    : Rgb(112, 26, 220);
                FillTriangle({ (float)baseX - 2.0f, (float)baseY + 4.0f },
                             { (float)baseX + 2.0f, (float)baseY + 4.0f },
                             { (float)tipX, (float)tipY },
                             flameColor);
            }
        }

        DrawPlayerHeroSprite(playerColor, aim, gSuperJumpTimer > 0.0f || !gPlayer.grounded);
        if (HookOnCooldown())
        {
            float ratio = gHookCooldownTimer / kHookMissCooldownSeconds;
            DrawCooldownRing((int)hookAnchor.x, (int)hookAnchor.y, 13, ratio,
                             Rgb(238, 44, 56), Rgb(64, 42, 58));
        }
        uint32_t hookColor =
            (gHook.mode == HookPullEnemy && !gHook.dashing) ? white :
            (gHook.mode == HookPullEnemy ? red : blue);
        if (!HookOnCooldown() || gHook.mode != HookIdle)
            DrawHookSprite(bladeEnd, Normalize(Sub(bladeEnd, hookAnchor)), hookColor);
    }

    int savedOffsetX = gRenderOffsetX;
    int savedOffsetY = gRenderOffsetY;
    ApplyVerticalMapLighting(BlendColor(bg, white, 0.13f));
    DrawTopLightLeaks(BlendColor(bg, white, 0.72f));
    gRenderOffsetX = 0;
    gRenderOffsetY = 0;
    DrawHeightMarkers(Rgb(138, 104, 128));
    DrawHud(white, red, dark, bg);
    DrawShopOverlay(white, red, dark);
    if (gChapterTransitionActive)
        RenderChapterTransitionOverlay(white, red, dark);
    if (gEndingActive)
        RenderEndingOverlay(white, red, dark);
    if (gPerkRouletteOpen)
        RenderPerkRouletteOverlay(white, red, dark);
    if (gPlayerDead)
        RenderDeathOverlay(white, red, dark);
    RenderAchievementPopup(white, red, dark);
    DrawCustomCursor(white, red, dark);
    ApplyCrtEffect();
    gRenderOffsetX = savedOffsetX;
    gRenderOffsetY = savedOffsetY;
}

static void Present(HDC dc, RECT client)
{
    StretchDIBits(
        dc,
        0,
        0,
        client.right - client.left,
        client.bottom - client.top,
        0,
        0,
        kBufferW,
        kBufferH,
        gPixels,
        &gBitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
}

static void UpdateMouseFromClient(HWND window, LPARAM lParam)
{
    int mx = (int)(short)LOWORD(lParam);
    int my = (int)(short)HIWORD(lParam);

    RECT client;
    GetClientRect(window, &client);

    int cw = client.right - client.left;
    int ch = client.bottom - client.top;
    if (cw <= 0 || ch <= 0) return;

    gInput.mouseX = (int)((float)mx * (float)kBufferW / (float)cw);
    gInput.mouseY = (int)((float)my * (float)kBufferH / (float)ch);
}

static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        gRunning = false;
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wParam < 256)
        {
            if (!gInput.keys[wParam]) gInput.pressed[wParam] = true;
            gInput.keys[wParam] = true;
        }
        return 0;

    case WM_KEYUP:
        if (wParam < 256) gInput.keys[wParam] = false;
        return 0;

    case WM_KILLFOCUS:
        for (int i = 0; i < 256; ++i)
        {
            gInput.keys[i] = false;
            gInput.pressed[i] = false;
        }
        gInput.mouseLeft = false;
        gInput.mousePressed = false;
        return 0;

    case WM_MOUSEMOVE:
        UpdateMouseFromClient(window, lParam);
        return 0;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(0);
            return TRUE;
        }
        break;

    case WM_LBUTTONDOWN:
        UpdateMouseFromClient(window, lParam);
        if (!gInput.mouseLeft)
        {
            gInput.mousePressed = true;
            gInput.mousePressedX = gInput.mouseX;
            gInput.mousePressedY = gInput.mouseY;
        }
        gInput.mouseLeft = true;
        return 0;

    case WM_LBUTTONUP:
        gInput.mouseLeft = false;
        UpdateMouseFromClient(window, lParam);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        RECT client;
        GetClientRect(window, &client);
        Present(dc, client);
        EndPaint(window, &paint);
        return 0;
    }
    }

    return DefWindowProcA(window, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    gBitmapInfo.bmiHeader.biSize = sizeof(gBitmapInfo.bmiHeader);
    gBitmapInfo.bmiHeader.biWidth = kBufferW;
    gBitmapInfo.bmiHeader.biHeight = -kBufferH;
    gBitmapInfo.bmiHeader.biPlanes = 1;
    gBitmapInfo.bmiHeader.biBitCount = 32;
    gBitmapInfo.bmiHeader.biCompression = BI_RGB;

    WNDCLASSA wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = "HookRiseWindowClass";
    wc.hCursor = 0;
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCEA(kAppIconResourceId));

    if (!RegisterClassA(&wc)) return 1;

    RECT desired = { 0, 0, kBufferW * kWindowScale, kBufferH * kWindowScale };
    AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, FALSE);

    HWND window = CreateWindowExA(
        0,
        wc.lpszClassName,
        "HELL PIT",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        desired.right - desired.left,
        desired.bottom - desired.top,
        0,
        0,
        instance,
        0);

    if (!window) return 1;

    HDC dc = GetDC(window);
    InitializeSaveFilePath();
    LoadPersistentData();
    SavePersistentData();
    RestartGame();
    StartAudio();

    LARGE_INTEGER frequency;
    LARGE_INTEGER previousCounter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previousCounter);
    double accumulator = 0.0;
    const double fixedDt = 1.0 / 120.0;

    while (gRunning)
    {
        MSG message;
        while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT) gRunning = false;
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        double frameTime = (double)(counter.QuadPart - previousCounter.QuadPart) /
                           (double)frequency.QuadPart;
        previousCounter = counter;
        if (frameTime > 0.1) frameTime = 0.1;
        accumulator += frameTime;

        while (accumulator >= fixedDt)
        {
            if (gScreen != ScreenGameplay)
            {
                UpdateMenu((float)fixedDt);
            }
            else if (gHitStopTimer > 0.0f)
            {
                gHitStopTimer -= (float)fixedDt;
                if (gHitStopTimer < 0.0f) gHitStopTimer = 0.0f;
            }
            else
            {
                UpdatePhysics((float)fixedDt);
                if (gHitStopTimer <= 0.0f)
                    UpdateCameraAndDevourer((float)fixedDt);
            }
            accumulator -= fixedDt;
        }

        Render();
        UpdateAudio();

        RECT client;
        GetClientRect(window, &client);
        Present(dc, client);

        Sleep(1);
    }

    ReleaseDC(window, dc);
    StopAudio();
    return 0;
}
