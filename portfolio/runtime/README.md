# Win32 게임 루프와 소프트웨어 렌더링

**담당자:** 최성원 — 개인 프로젝트 전체 구현, Codex를 구현·개선 보조 도구로 활용

## 코드

**WinMain:** 메시지를 처리하고 120Hz 고정 간격으로 게임을 갱신합니다. **PutPixel / Present:** 내부 픽셀 버퍼에 그린 결과를 창 크기로 확대합니다. **UpdateMouseFromClient:** 창 좌표를 내부 해상도로 변환합니다.

```cpp
static const int kBufferW = 320;
static const int kBufferH = 180;
static uint32_t gPixels[kBufferW * kBufferH];
static BITMAPINFO gBitmapInfo;

// WinMain의 비트맵 초기화: 음수 높이는 위에서 아래로 읽는 DIB
static void InitializeBitmap()
{
    gBitmapInfo.bmiHeader.biSize = sizeof(gBitmapInfo.bmiHeader);
    gBitmapInfo.bmiHeader.biWidth = kBufferW;
    gBitmapInfo.bmiHeader.biHeight = -kBufferH;
    gBitmapInfo.bmiHeader.biPlanes = 1;
    gBitmapInfo.bmiHeader.biBitCount = 32;
    gBitmapInfo.bmiHeader.biCompression = BI_RGB;
}

// 화면 흔들림 오프셋을 적용하고 버퍼 바깥 쓰기를 차단
static void PutPixel(int x, int y, uint32_t color)
{
    x += gRenderOffsetX;
    y += gRenderOffsetY;
    if (x < 0 || x >= kBufferW || y < 0 || y >= kBufferH) return;
    gPixels[y * kBufferW + x] = color;
}

// GPU 렌더러 대신 CPU에서 만든 버퍼를 Win32 창에 출력
static void Present(HDC dc, RECT client)
{
    StretchDIBits(dc, 0, 0, client.right - client.left,
        client.bottom - client.top, 0, 0, kBufferW, kBufferH,
        gPixels, &gBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}
```

```cpp
// WinMain의 루프 발췌: 창·오디오·게임 상태 초기화 이후 실행
static void RunLoop(HWND window, HDC dc)
{
    LARGE_INTEGER frequency, previous;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previous);
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
        double frameTime = double(counter.QuadPart - previous.QuadPart)
            / double(frequency.QuadPart);
        previous = counter;
        // 긴 프레임 이후 누적 갱신 횟수가 급증하지 않도록 제한
        if (frameTime > 0.1) frameTime = 0.1;
        accumulator += frameTime;

        // 렌더링 한 번 사이에 누적된 시간만큼 0회 이상 갱신해 이동 속도를 맞춘다.
        while (accumulator >= fixedDt)
        {
            if (gScreen != ScreenGameplay) UpdateMenu(float(fixedDt));
            // 타격 정지 중에는 물리 갱신을 잠시 쉬지만 아래 렌더링·오디오는 계속한다.
            else if (gHitStopTimer > 0.0f)
            {
                gHitStopTimer -= float(fixedDt);
                if (gHitStopTimer < 0.0f) gHitStopTimer = 0.0f;
            }
            else
            {
                UpdatePhysics(float(fixedDt));
                if (gHitStopTimer <= 0.0f)
                    UpdateCameraAndDevourer(float(fixedDt));
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
}
```

```cpp
// 창 크기가 바뀌어도 조준 좌표는 320×180 기준으로 통일
static void UpdateMouseFromClient(HWND window, LPARAM lParam)
{
    RECT client;
    GetClientRect(window, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;
    if (width <= 0 || height <= 0) return;

    // 입력 좌표를 부호 있는 값으로 읽어 창 경계 밖의 음수 좌표도 보존한다.
    int x = (int)(short)LOWORD(lParam);
    int y = (int)(short)HIWORD(lParam);
    gInput.mouseX = int(float(x) * kBufferW / width);
    gInput.mouseY = int(float(y) * kBufferH / height);
}
```

## 생략 및 재구성한 내용

- 원본: `1Seong/hell-pit`의 `src/main.cpp` (WinMain, PutPixel, Present, UpdateMouseFromClient). 기준 커밋: `fdc7490a6dd0e2962980e1ea2a88db999846affe`.
- WinMain의 초기화와 루프를 설명용 InitializeBitmap·RunLoop 함수로 분리했습니다. 원본 파일을 실제로 여러 모듈로 리팩터링한 것은 아닙니다.
- 창 등록·생성, WindowProc의 키/마우스 상태 기록과 포커스 해제 처리, 도형·스프라이트·CRT 효과, 오디오 합성을 생략했습니다.
- 프레임률 고정이나 수직 동기화를 보장하지 않습니다. 고정되는 것은 업데이트 시간 간격입니다.
- windows.h·stdint.h, 입력·화면·타이머 전역 상태와 업데이트 함수가 필요합니다. 발췌만으로 게임이 실행되지는 않습니다.
