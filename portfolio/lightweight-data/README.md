# 고정 배열 재사용과 비트마스크 저장

**담당자:** 최성원 — 개인 프로젝트 전체 구현, Codex를 구현·개선 보조 도구로 활용

## 코드

**Platform / AddPlatform:** 고정 배열의 빈 슬롯에 지형을 생성합니다. **ReclaimOffscreenTraversal:** 화면 아래로 벗어난 슬롯을 재사용 가능하게 바꿉니다. **BoolArrayToMask / MaskToBoolArray:** 여러 상태를 정수로 변환해 INI 저장에 사용합니다.

```cpp
struct Platform
{
    RectF bounds;
    PlatformCategory category;
    PlatformShape shape;
    int goreSeed;
    bool active; // false면 다음 생성에서 재사용할 수 있는 슬롯
};

static const int kMaxPlatforms = 64;
static Platform gPlatforms[kMaxPlatforms];

// 빈 슬롯을 재사용하고, 가득 찼으면 생성 실패를 반환
static Platform* AddPlatform(RectF bounds)
{
    for (auto& platform : gPlatforms)
    {
        if (platform.active) continue;
        platform.bounds = bounds;
        platform.category = PlatformSolid;
        platform.shape = (PlatformShape)((int)(Random01() * 5.0f) % 5);
        platform.goreSeed = (int)(Random01() * 100000.0f);
        platform.active = true;
        return &platform;
    }
    return nullptr;
}

// 원본 ReclaimOffscreenTraversal 중 지형 부분
static void ReclaimPlatforms()
{
    // 화면 경계보다 여유를 둔 회수선 아래에서만 슬롯을 비운다. 메모리를 해제하진 않는다.
    const float reclaimY = float(kBufferH) + 420.0f;
    for (auto& platform : gPlatforms)
        if (platform.active && platform.bounds.y > reclaimY)
            platform.active = false;
}
```

```cpp
// 인덱스마다 한 비트 사용: 현재 호출부의 4개 챕터 상태 등에 사용
// 예: 챕터 1·3 방문은 [true, false, true, false] → 이진수 0101 → 정수 5
static int BoolArrayToMask(const bool* values, int count)
{
    int mask = 0;
    for (int i = 0; i < count; ++i)
        if (values[i]) mask |= (1 << i);
    return mask;
}

// 저장된 정수의 i번째 비트를 검사해 챕터별 방문 여부로 되돌린다.
static void MaskToBoolArray(int mask, bool* values, int count)
{
    for (int i = 0; i < count; ++i)
        values[i] = (mask & (1 << i)) != 0;
}

// SavePersistentData / LoadPersistentData에서 상점 방문 상태만 발췌
static void SaveShopVisits()
{
    WriteIniInt("save", "mid_shops",
        BoolArrayToMask(gCheckpointShopUsed, kChapterCount));
}

static void LoadShopVisits()
{
    int mask = GetPrivateProfileIntA("save", "mid_shops", 0, gSaveFileName);
    MaskToBoolArray(mask, gCheckpointShopUsed, kChapterCount);
}
```

## 생략 및 재구성한 내용

- 원본: `1Seong/hell-pit`의 `src/main.cpp` (Platform, AddPlatform, ReclaimOffscreenTraversal, BoolArrayToMask, MaskToBoolArray, SavePersistentData, LoadPersistentData). 기준 커밋: `fdc7490a6dd0e2962980e1ea2a88db999846affe`.
- AddPlatform의 인덱스 순회를 범위 기반 순회로 바꿨습니다. 적·함정·오브 등의 유사한 배열 관리와 회수 반복은 생략했습니다. 
- 비트마스크는 원본과 같은 int 기반이며, 현재 작은 개수의 상태에 한정합니다. 임의 길이의 bool 배열을 직렬화하는 범용 함수가 아닙니다.
- INI 경로 초기화, WriteIniInt의 Win32 호출, 버전 검사, 옵션·업적·체크포인트의 나머지 필드를 생략했습니다. 전역 상태·타입·Win32 API에 의존하는 발췌입니다.
