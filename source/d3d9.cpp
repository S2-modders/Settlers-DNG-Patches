/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */

#include <Helper.h>
#include <Logger.h>
#include <SimpleIni.h>

#include "f_d3d9.h"

#include "Config.h"
#include "Lobby.h"
#include "MainPatch.h"

/*************************
Edit Values
*************************/

bool bFPSLimit, bForceWindowedMode;
float fFPSLimit;

HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
    if (bFPSLimit)
    {
        static LARGE_INTEGER PerformanceCount1;
        static LARGE_INTEGER PerformanceCount2;
        static bool bOnce1 = false;
        static double targetFrameTime = 1000.0 / fFPSLimit;
        static double t = 0.0;
        static DWORD i = 0;

        if (!bOnce1)
        {
            bOnce1 = true;
            QueryPerformanceCounter(&PerformanceCount1);
            PerformanceCount1.QuadPart = PerformanceCount1.QuadPart >> i;
        }

        while (true)
        {
            QueryPerformanceCounter(&PerformanceCount2);
            if (t == 0.0)
            {
                LARGE_INTEGER PerformanceCount3;
                static bool bOnce2 = false;

                if (!bOnce2)
                {
                    bOnce2 = true;
                    QueryPerformanceFrequency(&PerformanceCount3);
                    i = 0;
                    t = 1000.0 / (double)PerformanceCount3.QuadPart;
                    auto v = t * 2147483648.0;
                    if (60000.0 > v)
                    {
                        while (true)
                        {
                            ++i;
                            v *= 2.0;
                            t *= 2.0;
                            if (60000.0 <= v)
                                break;
                        }
                    }
                }
                SleepEx(0, 1);
                break;
            }

            if (((double)((PerformanceCount2.QuadPart >> i) - PerformanceCount1.QuadPart) * t) >= targetFrameTime)
                break;

            SleepEx(0, 1);
        }
        QueryPerformanceCounter(&PerformanceCount2);
        PerformanceCount1.QuadPart = PerformanceCount2.QuadPart >> i;
        //return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
    return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

void ForceWindowed(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
    HMONITOR monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    int DesktopResX = info.rcMonitor.right - info.rcMonitor.left;
    int DesktopResY = info.rcMonitor.bottom - info.rcMonitor.top;

    int left = (int)(((float)DesktopResX / 2.0f) - ((float)pPresentationParameters->BackBufferWidth / 2.0f));
    int top = (int)(((float)DesktopResY / 2.0f) - ((float)pPresentationParameters->BackBufferHeight / 2.0f));

    pPresentationParameters->Windowed = true;

    SetWindowPos(pPresentationParameters->hDeviceWindow, HWND_NOTOPMOST, left, top, pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, SWP_SHOWWINDOW);
}

HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
    LPDIRECT3DDEVICE9 *temp = ppReturnedDeviceInterface;

    *temp = new f_IDirect3DDevice9(*ppReturnedDeviceInterface, &ppReturnedDeviceInterface);
    *ppReturnedDeviceInterface = *temp;

    if (bForceWindowedMode)
        ForceWindowed(pPresentationParameters);

    HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    // NOTE: initialize your custom D3D components here

    return hr;
}

HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
    if (bForceWindowedMode)
        ForceWindowed(pPresentationParameters);

    // NOTE: call onLostDevice for custom D3D components here.

    HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);

    // NOTE: call onResetDevice for custom D3D components here.

    return hr;
}

/*************************
Exports
*************************/
INT WINAPI f_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
    return d3d9.D3DPERF_BeginEvent(col, wszName);
}
INT WINAPI f_D3DPERF_EndEvent()
{
    return d3d9.D3DPERF_EndEvent();
}
DWORD WINAPI f_D3DPERF_GetStatus()
{
    return d3d9.D3DPERF_GetStatus();
}
BOOL WINAPI f_D3DPERF_QueryRepeatFrame()
{
    return d3d9.D3DPERF_QueryRepeatFrame();
}
VOID WINAPI f_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
    return d3d9.D3DPERF_SetMarker(col, wszName);
}
VOID WINAPI f_D3DPERF_SetOptions(DWORD dwOptions)
{
    return d3d9.D3DPERF_SetOptions(dwOptions);
}
VOID WINAPI f_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
    return d3d9.D3DPERF_SetRegion(col, wszName);
}
VOID WINAPIV f_DebugSetLevel()
{
    return d3d9.DebugSetLevel();
}
VOID WINAPIV f_DebugSetMute()
{
    return d3d9.DebugSetMute();
}
INT WINAPI f_Direct3D9EnableMaximizedWindowedModeShim(INT a)
{
    return d3d9.Direct3D9EnableMaximizedWindowedModeShim(a);
}
IDirect3D9* WINAPI f_Direct3DCreate9(UINT SDKVersion)
{
    return new f_iD3D9(d3d9.Direct3DCreate9(SDKVersion));
}
HRESULT WINAPI f_Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D)
{
    return d3d9.Direct3DCreate9Ex(SDKVersion, ppD3D);
}
struct IDirect3DShaderValidator9* WINAPI f_Direct3DShaderValidatorCreate9()
{
    return d3d9.Direct3DShaderValidatorCreate9();
}
VOID WINAPI f_PSGPError(class D3DFE_PROCESSVERTICES* a, enum PSGPERRORID b, unsigned int c)
{
    return d3d9.PSGPError(a, b, c);
}
VOID WINAPI f_PSGPSampleTexture(class D3DFE_PROCESSVERTICES* a, unsigned int b, float(*const c)[4], unsigned int d, float(*const e)[4])
{
    return d3d9.PSGPSampleTexture(a, b, c, d, e);
}

/*************************
Testing Ground
*************************/

HCURSOR test(HMODULE hm) {
    auto handle = (HCURSOR)LoadImage(
        hm, MAKEINTRESOURCE(207), IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR);

    //auto handle = LoadCursorFromFile("Z:\\GitHub\\Settlers-DNG-Patches\\data\\cursor.cur");
    //auto handle = LoadCursor(hm, MAKEINTRESOURCE(207));

    if (!handle) {
        DWORD error = GetLastError();
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);

        std::cout << "FUCK OFF: " << messageBuffer;

        OutputDebugStringA(messageBuffer);
        LocalFree(messageBuffer);

        return NULL;
    }
    else {
        std::cout << "WORKED!";

        SetCursor(handle);

        return handle;
    }
}

/*************************
DllMain
*************************/


static char VkConfigPath[MAX_PATH];

bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        HMODULE baseModule = getBaseModule();

        char mainExecutable[MAX_PATH];
        GetModuleFileNameA(baseModule, mainExecutable, MAX_PATH);

        char configPath[MAX_PATH];
        getGameDirectory(baseModule, configPath, MAX_PATH, "\\bin\\d3d9.ini", 1);
        
        char engineINI[MAX_PATH];
        getGameDirectory(baseModule, engineINI, MAX_PATH, "\\data\\settings\\engine.ini", 1);

        char networkINI[MAX_PATH];
        getGameDirectory(baseModule, networkINI, MAX_PATH, "\\data\\settings\\network.ini", 1);

        char logFile[MAX_PATH];
        getGameDirectory(baseModule, logFile, MAX_PATH, "\\bin\\d3d9.log", 1);
        remove(logFile);

        CSimpleIniA config;

        config.SetUnicode();
        config.LoadFile(configPath);

        auto* gameSettings = new GameSettings(config);
        auto* cameraData = new CameraData(config);
        auto* lobbySettings = new LobbySettings(config);
        auto* settings = new PatchSettings(gameSettings, cameraData, lobbySettings);
        settings->gameVersion = getGameVersion(mainExecutable);

        Logging::Logger logger("DX9", logFile, gameSettings->bDebugWindow);
        MainPatch::startupMessage();

        logger.debug() << "launching from: " << mainExecutable << std::endl;

        if (gameSettings->bDebugWindow)
            logger.info("logging to DebugWindow");
        else
            logger.info() << "logging to " << logFile << std::endl;

        int refreshRate = getDesktopRefreshRate();
        gameSettings->fpsLimit = MainPatch::calcMaxFramerate(gameSettings->fpsLimit, gameSettings->bVSync);

        logger.debug() << "Detected refresh rate: " << refreshRate << "Hz | "
            << "Enforced fps limit: " << gameSettings->fpsLimit << "fps"
            << std::endl;

        if (gameSettings->bVulkan) {
            if (isVulkanSupported()) {
                logger.debug("Vulkan supported!");
            }
            else {
                logger.debug("Vulkan NOT supported!");
                gameSettings->bVulkan = false;
            }
        }
        else {
            logger.debug("Using native DX");
        }

        logger.debug("Setting engine INI");
        gameSettings->writeEngineConfig(engineINI);

        if (lobbySettings->bEnabled) {
            logger.debug("Setting network INI");
            lobbySettings->writeNetworkConfig(networkINI);
        }

        char path[MAX_PATH];

        if (!isWine() && gameSettings->bVulkan) {
            getGameDirectory(baseModule, path, MAX_PATH, "\\bin\\__config_cache", 1);
            memcpy_s(cameraData->VkConfigPath, MAX_PATH, path, MAX_PATH);
            memcpy_s(VkConfigPath, MAX_PATH, path, MAX_PATH);
            //logger.debug() << "Vk config cache location: " << VkConfigPath << std::endl;

            logger.info() << "Using shipped DX9: ";
            getGameDirectory(baseModule, path, MAX_PATH, "\\bin\\d3d9vk.dll", 1);

            bFPSLimit = false;
        }
        else {
            // use d3d9 provided by the system
            logger.info() << "Using system DX9: ";

            GetSystemDirectoryA(path, MAX_PATH);
            strcat_s(path, "\\d3d9.dll");

            if (gameSettings->fpsLimit > 0) {
                bFPSLimit = true;
                fFPSLimit = (float)gameSettings->fpsLimit;
            }
            else
                bFPSLimit = false;
        }

        logger.naked(path);

        if (gameSettings->bVulkan || isWine()) {
            if (!gameSettings->bDebugMode)
                SetEnvironmentVariableA("DXVK_LOG_LEVEL", "none");
            SetEnvironmentVariableA("DXVK_CONFIG_FILE", cameraData->VkConfigPath);

            logger.debug("Writing Vk config cache");
            gameSettings->writeDXconfig(cameraData->VkConfigPath);
        }


        CreateThread(0, 0, MainPatchThread, settings, 0, 0);

        if (lobbySettings->bEnabled)
            CreateThread(0, 0, LobbyPatchThread, settings, 0, 0);

        d3d9.dll = LoadLibraryA(path);
        d3d9.D3DPERF_BeginEvent = (LPD3DPERF_BEGINEVENT)GetProcAddress(d3d9.dll, "D3DPERF_BeginEvent");
        d3d9.D3DPERF_EndEvent = (LPD3DPERF_ENDEVENT)GetProcAddress(d3d9.dll, "D3DPERF_EndEvent");
        d3d9.D3DPERF_GetStatus = (LPD3DPERF_GETSTATUS)GetProcAddress(d3d9.dll, "D3DPERF_GetStatus");
        d3d9.D3DPERF_QueryRepeatFrame = (LPD3DPERF_QUERYREPEATFRAME)GetProcAddress(d3d9.dll, "D3DPERF_QueryRepeatFrame");
        d3d9.D3DPERF_SetMarker = (LPD3DPERF_SETMARKER)GetProcAddress(d3d9.dll, "D3DPERF_SetMarker");
        d3d9.D3DPERF_SetOptions = (LPD3DPERF_SETOPTIONS)GetProcAddress(d3d9.dll, "D3DPERF_SetOptions");
        d3d9.D3DPERF_SetRegion = (LPD3DPERF_SETREGION)GetProcAddress(d3d9.dll, "D3DPERF_SetRegion");
        d3d9.DebugSetLevel = (LPDEBUGSETLEVEL)GetProcAddress(d3d9.dll, "DebugSetLevel");
        d3d9.DebugSetMute = (LPDEBUGSETMUTE)GetProcAddress(d3d9.dll, "DebugSetMute");
        d3d9.Direct3D9EnableMaximizedWindowedModeShim = (LPDIRECT3D9ENABLEMAXIMIZEDWINDOWEDMODESHIM)GetProcAddress(d3d9.dll, "Direct3D9EnableMaximizedWindowedModeShim");
        d3d9.Direct3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress(d3d9.dll, "Direct3DCreate9");
        d3d9.Direct3DCreate9Ex = (LPDIRECT3DCREATE9EX)GetProcAddress(d3d9.dll, "Direct3DCreate9Ex");
        d3d9.Direct3DShaderValidatorCreate9 = (LPDIRECT3DSHADERVALIDATORCREATE9)GetProcAddress(d3d9.dll, "Direct3DShaderValidatorCreate9");
        d3d9.PSGPError = (LPPSGPERROR)GetProcAddress(d3d9.dll, "PSGPError");
        d3d9.PSGPSampleTexture = (LPPSGPSAMPLETEXTURE)GetProcAddress(d3d9.dll, "PSGPSampleTexture");
        break;
    }
    case DLL_PROCESS_DETACH:
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, bridgeProcessInfo.dwProcessId);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }

        remove(VkConfigPath);

        FreeLibrary(hModule);
        break;
    }
    return true;
}

/*************************
Augmented Callbacks
*************************/

//HRESULT f_iD3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) 
//{
//   LPDIRECT3DDEVICE9 *temp = ppReturnedDeviceInterface;
//
//   *temp = new f_IDirect3DDevice9(*ppReturnedDeviceInterface, &ppReturnedDeviceInterface);
//   *ppReturnedDeviceInterface = *temp;
//   delete temp;
//
//   HRESULT hr = f_pD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
//
//   // NOTE: initialize your custom D3D components here.
//
//   return hr;
//}

//HRESULT f_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters) 
//{
//   // NOTE: call onLostDevice for custom D3D components here.
//
//   HRESULT hr = f_pD3DDevice->Reset(pPresentationParameters);
//
//   // NOTE: call onResetDevice for custom D3D components here.
//
//   return hr;
//}

HRESULT f_IDirect3DDevice9::EndScene()
{

    // NOTE: draw your custom D3D components here.

    return f_pD3DDevice->EndScene();
}

/*************************
Bare D3D Callbacks
*************************/
ULONG f_iD3D9::AddRef()
{
    return f_pD3D->AddRef();
}

HRESULT f_iD3D9::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = NULL;
    HRESULT hr = f_pD3D->QueryInterface(riid, ppvObj);
    if (hr == D3D_OK)
        *ppvObj = this;
    return hr;
}

ULONG f_iD3D9::Release()
{
    ULONG count = f_pD3D->Release();
    if (count == 0) {
        delete(this);
    }
    return(count);
}

HRESULT f_iD3D9::EnumAdapterModes(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
    return f_pD3D->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

UINT f_iD3D9::GetAdapterCount()
{
    return f_pD3D->GetAdapterCount();
}

HRESULT f_iD3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode)
{
    return f_pD3D->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT f_iD3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier)
{
    return f_pD3D->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT f_iD3D9::GetAdapterModeCount(THIS_ UINT Adapter, D3DFORMAT Format)
{
    return f_pD3D->GetAdapterModeCount(Adapter, Format);
}

HMONITOR f_iD3D9::GetAdapterMonitor(UINT Adapter)
{
    return f_pD3D->GetAdapterMonitor(Adapter);
}

HRESULT f_iD3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps)
{
    return f_pD3D->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HRESULT f_iD3D9::RegisterSoftwareDevice(void *pInitializeFunction)
{
    return f_pD3D->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT f_iD3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
    return f_pD3D->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT f_iD3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
    return f_pD3D->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT f_iD3D9::CheckDeviceMultiSampleType(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
    return f_pD3D->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT f_iD3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
    return f_pD3D->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}


HRESULT f_iD3D9::CheckDeviceFormatConversion(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
    return f_pD3D->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

/*************************
Bare Device Callbacks
*************************/
f_IDirect3DDevice9::f_IDirect3DDevice9(LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DDEVICE9 **ppDevice)
{
    f_pD3DDevice = pDevice;
    *ppDevice = &f_pD3DDevice;
}

ULONG f_IDirect3DDevice9::AddRef()
{
    return f_pD3DDevice->AddRef();
}

HRESULT f_IDirect3DDevice9::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = NULL;
    HRESULT hr = f_pD3DDevice->QueryInterface(riid, ppvObj);
    if (hr == D3D_OK)
        *ppvObj = this;
    return hr;
}

ULONG f_IDirect3DDevice9::Release()
{
    ULONG count = f_pD3DDevice->Release();
    if (count == 0) {
        delete(this);
    }
    return (count);
}

void f_IDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
    f_pD3DDevice->SetCursorPosition(X, Y, Flags);
}

HRESULT f_IDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
    return f_pD3DDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL f_IDirect3DDevice9::ShowCursor(BOOL bShow)
{
    return f_pD3DDevice->ShowCursor(bShow);
}

HRESULT f_IDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
    return f_pD3DDevice->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);
}

HRESULT f_IDirect3DDevice9::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::BeginStateBlock()
{
    return f_pD3DDevice->BeginStateBlock();
}

HRESULT f_IDirect3DDevice9::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
    return f_pD3DDevice->CreateStateBlock(Type, ppSB);
}

HRESULT f_IDirect3DDevice9::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
    return f_pD3DDevice->EndStateBlock(ppSB);
}

HRESULT f_IDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
    return f_pD3DDevice->GetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9::GetDisplayMode(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
    return f_pD3DDevice->GetDisplayMode(iSwapChain, pMode);
}

HRESULT f_IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
    return f_pD3DDevice->GetRenderState(State, pValue);
}

HRESULT f_IDirect3DDevice9::GetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
    return f_pD3DDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT f_IDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
    return f_pD3DDevice->GetTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus)
{
    return f_pD3DDevice->SetClipStatus(pClipStatus);
}

HRESULT f_IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
    return f_pD3DDevice->SetRenderState(State, Value);
}

HRESULT f_IDirect3DDevice9::SetRenderTarget(THIS_ DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
    return f_pD3DDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT f_IDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
    //return D3D_OK;
    return f_pD3DDevice->SetTransform(State, pMatrix);

    if (State == D3DTS_PROJECTION) {
        return D3D_OK;

        //std::cout << "Set Transform D3DTS_PROJECTION" << std::endl;

        D3DMATRIX tmat;

        HRESULT res = f_pD3DDevice->GetTransform(D3DTS_PROJECTION, &tmat);
        if (res != D3D_OK)
            std::cout << "shit!" << std::endl;

        tmat._11 = 1 / std::tanf(2.0f * 0.5f);

        std::cout << "11: " << tmat._11 << " 22: " << tmat._22 << std::endl;

        return f_pD3DDevice->SetTransform(State, &tmat);

        float w = pMatrix->_11;
        float fov_w = std::atanf(1 / w) * 2;
        
        float h = pMatrix->_22;
        float fov_h = std::atanf(1 / h) * 2;

        //std::cout << "w: " << fov_w << " h: " << fov_h << std::endl;

        // value is usually 0,872
        if (fov_w > 0.85f && fov_w < 0.88f && false) {
            //D3DMATRIX tmat(*pMatrix);
            //ZeroMemory(&tmat, sizeof(tmat));

            tmat._11 = 1 / std::tanf(1.91f * 0.5f);

            float tt = std::atanf(1 / tmat._11) * 2;
            float vv = std::atanf(1 / tmat._22) * 2;

            std::cout << "w: " << fov_w << " -> " << tt
                << " ||  h: " << fov_h << " -> " << vv << std::endl;

            return f_pD3DDevice->SetTransform(State, &tmat);
        }

        if (fov_w > 1.0f) {
            //D3DMATRIX tmat(*pMatrix);

            //tmat._11 = 1 / std::tanf(1.91f * 0.5f);
            tmat._11 = tmat._22 / (16 / 9);

            float tt = std::atanf(1 / tmat._11) * 2;
            float vv = std::atanf(1 / tmat._22) * 2;

            std::cout << "w: " << fov_w << " -> " << tt
                << " ||  h: " << fov_h << " -> " << vv << std::endl;

            std::cout << "w: " << fov_w << " h: " << fov_h << std::endl;

            //ZeroMemory(&tmat, sizeof(tmat));

            return f_pD3DDevice->SetTransform(State, &tmat);
        }


        //std::cout << "h: " << pMatrix->_22 << std::endl;
    }

    return f_pD3DDevice->SetTransform(State, pMatrix);
}

void f_IDirect3DDevice9::GetGammaRamp(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
    f_pD3DDevice->GetGammaRamp(iSwapChain, pRamp);
}

void f_IDirect3DDevice9::SetGammaRamp(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
    f_pD3DDevice->SetGammaRamp(iSwapChain, Flags, pRamp);
}

HRESULT f_IDirect3DDevice9::DeletePatch(UINT Handle)
{
    return f_pD3DDevice->DeletePatch(Handle);
}

HRESULT f_IDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
    return f_pD3DDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT f_IDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
    return f_pD3DDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT f_IDirect3DDevice9::GetIndices(THIS_ IDirect3DIndexBuffer9** ppIndexData)
{
    return f_pD3DDevice->GetIndices(ppIndexData);
}

HRESULT f_IDirect3DDevice9::SetIndices(THIS_ IDirect3DIndexBuffer9* pIndexData)
{
    return f_pD3DDevice->SetIndices(pIndexData);
}

UINT f_IDirect3DDevice9::GetAvailableTextureMem()
{
    return f_pD3DDevice->GetAvailableTextureMem();
}

HRESULT f_IDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return f_pD3DDevice->GetCreationParameters(pParameters);
}

HRESULT f_IDirect3DDevice9::GetDeviceCaps(D3DCAPS9 *pCaps)
{
    return f_pD3DDevice->GetDeviceCaps(pCaps);
}

HRESULT f_IDirect3DDevice9::GetDirect3D(IDirect3D9 **ppD3D9)
{
    return f_pD3DDevice->GetDirect3D(ppD3D9);
}

HRESULT f_IDirect3DDevice9::GetRasterStatus(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
    return f_pD3DDevice->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT f_IDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
    return f_pD3DDevice->GetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable)
{
    return f_pD3DDevice->GetLightEnable(Index, pEnable);
}

HRESULT f_IDirect3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial)
{
    return f_pD3DDevice->GetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9::LightEnable(DWORD LightIndex, BOOL bEnable)
{
    return f_pD3DDevice->LightEnable(LightIndex, bEnable);
}

HRESULT f_IDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
{

    return f_pD3DDevice->SetLight(Index, pLight);
}

HRESULT f_IDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
    return f_pD3DDevice->SetMaterial(pMaterial);
}

HRESULT f_IDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
    return f_pD3DDevice->MultiplyTransform(State, pMatrix);
}

HRESULT f_IDirect3DDevice9::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
    return f_pD3DDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT f_IDirect3DDevice9::TestCooperativeLevel()
{
    return f_pD3DDevice->TestCooperativeLevel();
}

HRESULT f_IDirect3DDevice9::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
    return f_pD3DDevice->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT f_IDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
    return f_pD3DDevice->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
    return f_pD3DDevice->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT f_IDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
    return f_pD3DDevice->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT f_IDirect3DDevice9::CreatePixelShader(THIS_ CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
    return f_pD3DDevice->CreatePixelShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9::GetPixelShader(THIS_ IDirect3DPixelShader9** ppShader)
{
    return f_pD3DDevice->GetPixelShader(ppShader);
}

HRESULT f_IDirect3DDevice9::SetPixelShader(THIS_ IDirect3DPixelShader9* pShader)
{
    return f_pD3DDevice->SetPixelShader(pShader);
}

//HRESULT f_IDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion) 
//{
//   return f_pD3DDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
//}

HRESULT f_IDirect3DDevice9::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    return f_pD3DDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT f_IDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    return f_pD3DDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    return f_pD3DDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT f_IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    return f_pD3DDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT f_IDirect3DDevice9::BeginScene()
{
    return f_pD3DDevice->BeginScene();
}

HRESULT f_IDirect3DDevice9::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
    return f_pD3DDevice->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT f_IDirect3DDevice9::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
    return f_pD3DDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT f_IDirect3DDevice9::GetBackBuffer(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
    return f_pD3DDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT f_IDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
    return f_pD3DDevice->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT f_IDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
    return f_pD3DDevice->GetTexture(Stage, ppTexture);
}

HRESULT f_IDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
    return f_pD3DDevice->GetTextureStageState(Stage, Type, pValue);
}

HRESULT f_IDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
    return f_pD3DDevice->SetTexture(Stage, pTexture);
}

HRESULT f_IDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    return f_pD3DDevice->SetTextureStageState(Stage, Type, Value);
}

HRESULT f_IDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
    return f_pD3DDevice->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT f_IDirect3DDevice9::ValidateDevice(DWORD *pNumPasses)
{
    return f_pD3DDevice->ValidateDevice(pNumPasses);
}

HRESULT f_IDirect3DDevice9::GetClipPlane(DWORD Index, float *pPlane)
{
    return f_pD3DDevice->GetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9::SetClipPlane(DWORD Index, CONST float *pPlane)
{
    return f_pD3DDevice->SetClipPlane(Index, pPlane);
}

HRESULT f_IDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
    return f_pD3DDevice->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT f_IDirect3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport)
{
    return f_pD3DDevice->GetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9 *pViewport)
{
    return f_pD3DDevice->SetViewport(pViewport);
}

HRESULT f_IDirect3DDevice9::CreateVertexShader(THIS_ CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
    return f_pD3DDevice->CreateVertexShader(pFunction, ppShader);
}

HRESULT f_IDirect3DDevice9::GetVertexShader(THIS_ IDirect3DVertexShader9** ppShader)
{
    return f_pD3DDevice->GetVertexShader(ppShader);
}

HRESULT f_IDirect3DDevice9::SetVertexShader(THIS_ IDirect3DVertexShader9* pShader)
{
    return f_pD3DDevice->SetVertexShader(pShader);
}

HRESULT f_IDirect3DDevice9::CreateQuery(THIS_ D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
    return f_pD3DDevice->CreateQuery(Type, ppQuery);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
    return f_pD3DDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
    return f_pD3DDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
    return f_pD3DDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
    return f_pD3DDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::SetPixelShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
    return f_pD3DDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::GetPixelShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
    return f_pD3DDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT Divider)
{
    return f_pD3DDevice->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9::GetStreamSourceFreq(THIS_ UINT StreamNumber, UINT* Divider)
{
    return f_pD3DDevice->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantB(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
    return f_pD3DDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantB(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
    return f_pD3DDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantF(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
    return f_pD3DDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantF(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
    return f_pD3DDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT f_IDirect3DDevice9::SetVertexShaderConstantI(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
    return f_pD3DDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::GetVertexShaderConstantI(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
    return f_pD3DDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT f_IDirect3DDevice9::SetFVF(THIS_ DWORD FVF)
{
    return f_pD3DDevice->SetFVF(FVF);
}

HRESULT f_IDirect3DDevice9::GetFVF(THIS_ DWORD* pFVF)
{
    return f_pD3DDevice->GetFVF(pFVF);
}

HRESULT f_IDirect3DDevice9::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
    return f_pD3DDevice->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT f_IDirect3DDevice9::SetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9* pDecl)
{
    return f_pD3DDevice->SetVertexDeclaration(pDecl);
}

HRESULT f_IDirect3DDevice9::GetVertexDeclaration(THIS_ IDirect3DVertexDeclaration9** ppDecl)
{
    return f_pD3DDevice->GetVertexDeclaration(ppDecl);
}

HRESULT f_IDirect3DDevice9::SetNPatchMode(THIS_ float nSegments)
{
    return f_pD3DDevice->SetNPatchMode(nSegments);
}

float f_IDirect3DDevice9::GetNPatchMode(THIS)
{
    return f_pD3DDevice->GetNPatchMode();
}

int f_IDirect3DDevice9::GetSoftwareVertexProcessing(THIS)
{
    return f_pD3DDevice->GetSoftwareVertexProcessing();
}

unsigned int f_IDirect3DDevice9::GetNumberOfSwapChains(THIS)
{
    return f_pD3DDevice->GetNumberOfSwapChains();
}

HRESULT f_IDirect3DDevice9::EvictManagedResources(THIS)
{
    return f_pD3DDevice->EvictManagedResources();
}

HRESULT f_IDirect3DDevice9::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
    return f_pD3DDevice->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT f_IDirect3DDevice9::SetScissorRect(THIS_ CONST RECT* pRect)
{
    return f_pD3DDevice->SetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9::GetScissorRect(THIS_ RECT* pRect)
{
    return f_pD3DDevice->GetScissorRect(pRect);
}

HRESULT f_IDirect3DDevice9::GetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
    return f_pD3DDevice->GetSamplerState(Sampler, Type, pValue);
}

HRESULT f_IDirect3DDevice9::SetSamplerState(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
    const int maxAnisotropy = 16;

    if (Type == D3DSAMP_MAXANISOTROPY)
        return f_pD3DDevice->SetSamplerState(Sampler, Type, maxAnisotropy);
    else
        return f_pD3DDevice->SetSamplerState(Sampler, Type, Value);

    /*
    if (Type == D3DSAMP_MAXANISOTROPY) {
        return f_pD3DDevice->SetSamplerState(Sampler, Type, maxAnisotropy);

    } else if (Value == D3DTEXF_LINEAR) {
        f_pD3DDevice->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, maxAnisotropy);

        return f_pD3DDevice->SetSamplerState(Sampler, Type, D3DTEXF_ANISOTROPIC);
    }

    return f_pD3DDevice->SetSamplerState(Sampler, Type, Value);
    */
    /*
    // Setup Anisotropy Filtering
    if (isoTropyFlag && (Type == D3DSAMP_MAXANISOTROPY || ((Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER) && Value == D3DTEXF_LINEAR))) {
        isoTropyFlag = false;
        f_pD3DDevice->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, maxAnisotropy);
    }

    // Enable Anisotropic Filtering
    if (Type == D3DSAMP_MAXANISOTROPY) {
        if (f_pD3DDevice->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, maxAnisotropy) == D3D_OK) {
            return D3D_OK;
        }
    } else if ((Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER) && Value == D3DTEXF_LINEAR) {
        if (f_pD3DDevice->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, maxAnisotropy) == D3D_OK
            && f_pD3DDevice->SetSamplerState(Sampler, Type, D3DTEXF_ANISOTROPIC) == D3D_OK) {
            return D3D_OK;
        }
    }
    

    if (Type == D3DSAMP_MAXANISOTROPY || ((Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MAGFILTER) && Value == D3DTEXF_LINEAR)) {
    }
    */
}

HRESULT f_IDirect3DDevice9::SetDepthStencilSurface(THIS_ IDirect3DSurface9* pNewZStencil)
{
    return f_pD3DDevice->SetDepthStencilSurface(pNewZStencil);
}

HRESULT f_IDirect3DDevice9::CreateOffscreenPlainSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
    return f_pD3DDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT f_IDirect3DDevice9::ColorFill(THIS_ IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
    return f_pD3DDevice->ColorFill(pSurface, pRect, color);
}

HRESULT f_IDirect3DDevice9::StretchRect(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
    return f_pD3DDevice->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT f_IDirect3DDevice9::GetFrontBufferData(THIS_ UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
    return f_pD3DDevice->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT f_IDirect3DDevice9::GetRenderTargetData(THIS_ IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
    return f_pD3DDevice->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT f_IDirect3DDevice9::UpdateSurface(THIS_ IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
    return f_pD3DDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT f_IDirect3DDevice9::SetDialogBoxMode(THIS_ BOOL bEnableDialogs)
{
    return f_pD3DDevice->SetDialogBoxMode(bEnableDialogs);
}

HRESULT f_IDirect3DDevice9::GetSwapChain(THIS_ UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
    return f_pD3DDevice->GetSwapChain(iSwapChain, pSwapChain);
}
