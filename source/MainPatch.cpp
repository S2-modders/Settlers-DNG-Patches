/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include "pch.h"

#include <chrono>

#include <Logger.h>

#include "MainPatch.h"
#include "MainPatchASM.h"

namespace MainPatch_Logger {
    Logging::Logger logger("MAIN");
}
using MainPatch_Logger::logger;

const int retryCount = 4;
const int retryTimeout = 2000;


/* 
* memory values 
*/

/* DNG Base game (11757) GOG */
namespace Base_GOG { 
    PatchData offsets = {
        .worldObject = {
            0x002BD4E8,
            { 0x4C }
        },
        .maxZoom = {
            0x002BD4E8,
            { 0x4C, 0x1A8 }
        },
        .currZoom = {
            0x002BD4E8,
            { 0x4C, 0x1A4 }
        },
        .zoomIncrAddr = 0x20D00E + 0x2,
        .zoomDecrAddr = 0x20CFE4 + 0x2,
        .fileLoadAddr = 0x193C28,           // 5355568944242C8B442438
        .fileLoadEndAddr = 0x193DE5,        // 8B74243C8B0685C0740F
        .lobbyVersionFilterAddr = 0x107055, // 741E8B8C24BC0000008B9424B80000005653555152
    };
}

/* DNG Base game (11757) Gold Edition */
namespace Base_Gold {
    PatchData offsets = {
        .worldObject = {
            0x002B5B48,
            { 0x4C }
        },
        .maxZoom = {
            0x002B5B48,
            { 0x4C, 0x1A8 }
        },
        .currZoom = {
            0x002B5B48,
            { 0x4C, 0x1A4 }
        },
        .zoomIncrAddr = 0x20CCDE + 0x2,
        .zoomDecrAddr = 0x20CCB4 + 0x2,
        .fileLoadAddr = 0x193B78,
        .fileLoadEndAddr = 0x193D35,
        .lobbyVersionFilterAddr = 0x107365,
    };
}

/* DNG Wikinger Addon (11758) Gold Edition */
namespace Addon_Gold {
    PatchData offsets = {
        .worldObject = {
            0x002C2BA8,
            { 0x4C }
        },
        .maxZoom = {
            0x002C2BA8,
            { 0x4C, 0x1BC }
        },
        .currZoom = {
            0x002C2BA8,
            { 0x4C, 0x1B8 }
        },
        .zoomIncrAddr = 0x2182C8 + 0x2,
        .zoomDecrAddr = 0x21829E + 0x2,
        .fileLoadAddr = 0x19D788,
        .fileLoadEndAddr = 0x19D945,
        .lobbyVersionFilterAddr = 0,
    };
}

/* DNG Wikinger Addon (11758) */
namespace Addon {
    PatchData offsets = {
        .worldObject = {
            0x002CA528,
            { 0x4C }
        },
        .maxZoom = {
            0x002CA528,
            { 0x4C, 0x1BC }
        },
        .currZoom = {
            0x002CA528,
            { 0x4C, 0x1B8 }
        },
        .zoomIncrAddr = 0x218568 + 0x2,
        .zoomDecrAddr = 0x21853E + 0x2,
        .fileLoadAddr = 0,
        .fileLoadEndAddr = 0,
        .lobbyVersionFilterAddr = 0,
    };
}

/*###################################*/




MainPatch::MainPatch(PatchData& patchData, PatchSettings* settings) : patchData(patchData) {
    this->settings = settings;
    this->zoomStep_p = &settings->cameraData->fZoomIncrement;
}

void MainPatch::startupMessage() {
    std::cout << "ZoomPatch & LobbyPatch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n"
        << std::endl;
}

int MainPatch::calcMaxFramerate(int maxFrameRate, bool vSync) {
    // game is limited to 200 fps and causes issues above
    // we need to calculate a forced fps limiter to prevent issues
    int rr = getDesktopRefreshRate();
    int newrr = min(maxFrameRate, 200);

    // detection failed if rr = 0
    if (rr == 0)
        return max(0, newrr);

    // if user specified fps limit, then we use that
    // unless it is higher than the refresh rate
    if (newrr > 0 && newrr < rr)
        return newrr;

    // anything under 200Hz will get limited to (Hz - 1) fps
    // anything over 200Hz will get limited to (Hz / 2) fps
    if (rr > 200)
        return rr / 2;
    else
        return max(vSync ? rr - 1 : rr, 60); // do not go lower than 60fps

}

int MainPatch::run() {
    // delete temporary VK config file after a few seconds
    std::thread rmThread([](char* VkPath, int sleep = 1e4) {
        Sleep(sleep);
        remove(VkPath);
        }, settings->cameraData->VkConfigPath);
    rmThread.detach();

    logger.info("Waiting for application init...");
    waitGameLoad();

    logger.info("MainPatch started");

    if (settings->gameSettings->bFileLoadPatch) {
        patchFileLoad();
    }
    if (settings->gameSettings->bFileStorePatch) {
        patchFileStore();
    }

    auto time = waitGameInit();
    logger.info() << "Game init in " << time << "ms" << std::endl;

    //patchLobbyFilter();
    //patchFogDisable(); # FIXME causes crash, something is fucked up

    for (;; Sleep(1000)) {
        worldObj = (float*)tracePointer(&patchData.worldObject);

        patchCamera();

        if (settings->gameSettings->bDebugMode)
            doDebug();

        //setCursor();
    }
}

void MainPatch::setCursor() {
    //auto handle = LoadImageA(hm, MAKEINTRESOURCEA(101), IMAGE_BITMAP, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR);
    //auto hlol = FindResource(hm, MAKEINTRESOURCE(101), "PNG");
    //auto handle = LoadResource(hm, hlol);
    //auto handle = LoadCursor(hm, MAKEINTRESOURCE(101));

    /*
    ICONINFO fuck;
    fuck.fIcon = false;
    fuck.xHotspot = 0;
    fuck.yHotspot = 0;
    fuck.hbmMask = settings->bitmap;
    fuck.hbmColor = settings->bitmap;

    HCURSOR cursor = CreateIconIndirect(&fuck);
    if (!cursor) {
        logger.debug(" FUCK IconIndirect failed!");
    }
    else {
        logger.debug("SetCursor()");
        SetCursor(cursor);
    }
    */

    logger.debug("SetCursor()");
    SetCursor(settings->cursor);
}

void MainPatch::waitGameLoad() {
    while (true) {
        DWORD wb = *calcAddress(patchData.worldObject.base_address);
        //std::cout << wb << std::endl;

        if (wb == 0) {
            return;
        }

        Sleep(1);
    }
}

long long MainPatch::waitGameInit() {
    auto t0 = std::chrono::steady_clock::now();

    while (true) {
        void* worldBase = (void*)*calcAddress(patchData.worldObject.base_address);
        //std::cout << "worldBase " << worldBase << std::endl;

        if (isMemoryReadable(worldBase)) {
            auto t1 = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        } else {
            Sleep(1);
        }
    }
}

void MainPatch::patchCamera() {
    if (*worldObj == 0 || !settings->cameraData->bEnabled)
        return;

    maxZoom = (float*)tracePointer(&patchData.maxZoom);

    if (calcZoomValue() && *maxZoom != newZoomValue) {
        if (settings->cameraData->bWideView)
            logger.debug("WideView enabled");
        else
            logger.debug("WideView disabled");

        *maxZoom = newZoomValue;
        logger.debug() << "New MaxZoom: " << newZoomValue;
        if (isZoomOverride())
            logger.naked() << " (override)";
        logger.naked() << std::endl;

        patchZoomIncrement();
    }
}

void MainPatch::patchZoomIncrement() {
    float** incr = (float**)calcAddress(patchData.zoomIncrAddr);
    logger.debug() << "Incr: " << *zoomStep_p << " " << **incr << std::endl;

    writeBytes(calcAddress(patchData.zoomIncrAddr), &zoomStep_p, 4);
    writeBytes(calcAddress(patchData.zoomDecrAddr), &zoomStep_p, 4);

    logger.debug("Zoom increment set");
}

bool MainPatch::calcZoomValue() {
    if (isZoomOverride()) {
        newZoomValue = (float)settings->cameraData->customZoom;
        return true;
    }

    /* maxZoomValue will be set depending on the aspect ratio of the screen */
    float aspr = getAspectRatio();

    if (aspr <= 0.0f || aspr > 20.0f) {
        logger.error() << "ASPR is ridiculous (" << aspr << ")" << std::endl;
        return false;
    }

    if (settings->cameraData->bWideView) {
        if (aspr < 1.5f)
            newZoomValue = 5.0f;
        else if (aspr < 1.9f)
            newZoomValue = 6.0f;
        else if (aspr < 2.2f)
            newZoomValue = 7.0f;
        else if (aspr < 2.5f)
            newZoomValue = 8.0f;
        else if (aspr >= 2.5f)
            newZoomValue = 9.0f;
        else
            return false;

        return true;
    }

    if (aspr < 1.5f)
        newZoomValue = 4.0f;
    else if (aspr < 1.9f)
        newZoomValue = 5.0f;
    else if (aspr < 2.2f)
        newZoomValue = 6.0f;
    else if (aspr < 2.5f)
        newZoomValue = 7.0f;
    else if (aspr < 3.2f)
        newZoomValue = 8.0f;
    else if (aspr >= 3.2f)
        newZoomValue = 9.0f;
    else
        return false;

    return true;
}

void MainPatch::patchLobbyFilter() {
    if (patchData.lobbyVersionFilterAddr == 0)
        return;

    logger.debug("patching lobby version filter");

    short jne = 0x1E75;
    short* filter = (short*)calcAddress(patchData.lobbyVersionFilterAddr);
    writeBytes(filter, &jne, 2);
}

void MainPatch::patchFogDisable() {
    logger.debug("fog patch");

    DWORD* fogStart = calcAddress(0x13CECA);
    DWORD* fogEnd = calcAddress(0x13CEDD);

    FogPatch::injectJumperFog(fogStart, fogEnd);
}

void MainPatch::doDebug() {
    if (isKeyPressed(VK_F3)) {
        int t_hor, t_ver;
        getDesktopResolution2(t_hor, t_ver);
        std::stringstream ss;
        ss << "xRes: " << t_hor << " yRes: " << t_ver;
        ss << " ASPR: " << calcAspectRatio(t_hor, t_ver);
        ss << " MaxZoomValue: " << newZoomValue;
        logger.debug() << ss.str() << std::endl;
        //MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
    }
    if (isKeyPressed(VK_F4)) {
        int h, v;
        getDesktopResolution(h, v);
        std::stringstream ss;
        ss << "xRes: " << h << " yRes: " << v;
        ss << " ASPR: " << calcAspectRatio(h, v);
        ss << " MaxZoomValue: " << newZoomValue;
        logger.debug() << ss.str() << std::endl;
    }
    if (isKeyPressed(VK_F6)) {
        if (*worldObj != 0) {
            //*(float*)(tracePointer(&CurrZoomPTR)) += 1.0f;
            logger.debug() << "maxZoom: " << *(float*)(tracePointer(&patchData.maxZoom)) << std::endl;
        }
    }
    if (isKeyPressed(VK_F7)) {
        logger.debug() << "currZoom: " << *(float*)(tracePointer(&patchData.currZoom)) << std::endl;
    }
    if (isKeyPressed(VK_F8)) {
        logger.debug() << "World address: " << worldObj << " World value: " << *worldObj << std::endl;
    }
}

bool MainPatch::isZoomOverride() {
    return settings->cameraData->customZoom > 0;
}

void MainPatch::patchFileLoad() {
    DWORD* fileloadFktAddr = calcAddress(patchData.fileLoadAddr);

#if 0
    int time = 0;
    while (*fileloadFktAddr == 0) {
        Sleep(1);
        time += 1;
    }
    logger.info() << "Function might be loaded; took:" << time << "ms" << std::endl;
#endif

    DecryptPatch::injectFileLoad(fileloadFktAddr);
}

void MainPatch::patchFileStore() {
    DWORD* fileloadFktEndAddr = calcAddress(patchData.fileLoadEndAddr);
    DecryptPatch::injectFileStore(fileloadFktEndAddr);
}

static int prepareMain(PatchSettings* settings) {
    /* check if gameVersion is supported */
    switch (settings->gameVersion) {
    case V_BASE_GOG:
        logger.info("Found DNG GOG version");
        return MainPatch(Base_GOG::offsets, settings).run();
        break;

    case V_BASE_GOLD:
        logger.info("Found DNG Gold Edition");
        return MainPatch(Base_Gold::offsets, settings).run();
        break;
    case V_ADDON_GOLD:
        logger.info("Found Wikinger Gold Edition");
        return MainPatch(Addon_Gold::offsets, settings).run();
        break;

    /*
    case V_ADDON_NOCD:
        logger.info("Found Wikinger Retail noCD version");
        return MainPatch(Addon::patchData, settings).run();
        break;
    */

    default:
        logger.error() << "This game version is not supported! \n"
            << "Supported game versions are: \n"
            << "- GOG (11757) \n"
            << "- DNG Gold Edition (11757) \n"
            << "- Wikings Addon Gold Edition (11758)"
            << std::endl;
        //MessageBoxA(NULL, "This game version is not supported!", "Widescreen Fix ERROR", MB_OK);
        break;
    }

    return 0;
}

DWORD WINAPI MainPatchThread(LPVOID param) {
    return prepareMain(reinterpret_cast<PatchSettings*>(param));
}
