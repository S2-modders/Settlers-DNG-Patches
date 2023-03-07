/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include "MainPatch.h"

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

    memoryPTR maxZoom = {
        0x002BD4E8,
        { 0x4C, 0x1A8 }
    };

    memoryPTR currZoom = {
        0x002BD4E8,
        { 0x4C, 0x1A4 }
    };

    memoryPTR worldObject = {
        0x002BD4E8,
        { 0x4C }
    };

    DWORD lobbyVersionFilterAddr = 0x107055;
    DWORD gameVersionAddr = 0x2C5A30;

    DWORD zoomIncrAddr = 0x20D00E + 0x2;
    DWORD zoomDecrAddr = 0x20CFE4 + 0x2;

    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr,
        zoomIncrAddr,
        zoomDecrAddr
    };

}

/* DNG Base game (11757) Gold Edition */
namespace Base_Gold {

    memoryPTR maxZoom = {
        0x002B5B48,
        { 0x4C, 0x1A8 }
    };

    memoryPTR currZoom = {
        0x002B5B48,
        { 0x4C, 0x1A4 }
    };

    memoryPTR worldObject = {
        0x002B5B48,
        { 0x4C }
    };

    DWORD lobbyVersionFilterAddr = 0x00; // unknown
    DWORD gameVersionAddr = 0x2BE090;

    DWORD zoomIncrAddr = 0x20CCDE + 0x2;
    DWORD zoomDecrAddr = 0x20CCB4 + 0x2;

    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr,
        zoomIncrAddr,
        zoomDecrAddr
    };

}


/* DNG Wikinger Addon (11758) */
namespace Addon {

    memoryPTR maxZoom = {
        0x002CA528,
        { 0x4C, 0x1BC }
    };

    memoryPTR currZoom = {
        0x002CA528,
        { 0x4C, 0x1B8 }
    };

    memoryPTR worldObject = {
        0x002CA528,
        { 0x4C }
    };

    DWORD lobbyVersionFilterAddr = 0x00; // unknown
    DWORD gameVersionAddr = 0x2D2DB8;

    DWORD zoomIncrAddr = 0x218568 + 0x2;
    DWORD zoomDecrAddr = 0x21853E + 0x2;

    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr,
        zoomIncrAddr,
        zoomDecrAddr
    };
}

/* DNG Wikinger Addon (11758) Gold Edition */
namespace Addon_Gold {

    memoryPTR maxZoom = {
        0x002C2BA8,
        { 0x4C, 0x1BC }
    };

    memoryPTR currZoom = {
        0x002C2BA8,
        { 0x4C, 0x1B8 }
    };

    memoryPTR worldObject = {
        0x002C2BA8,
        { 0x4C }
    };

    DWORD lobbyVersionFilterAddr = 0x00; // unknown
    DWORD gameVersionAddr = 0x2CB438;

    DWORD zoomIncrAddr = 0x2182C8 + 0x2;
    DWORD zoomDecrAddr = 0x21829E + 0x2;

    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr,
        zoomIncrAddr,
        zoomDecrAddr
    };
}

/*###################################*/




MainPatch::MainPatch(PatchData& patchData, PatchSettings* settings) : patchData(patchData) {
    this->settings = settings;
    this->zoomStep_p = &settings->cameraData->fZoomIncrement;
}

void MainPatch::startupMessage() {
    std::cout << "ZoomPatch & LobbyPatch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n"
        << "Waiting for application startup... \n"
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
    if (!isWorldObject())
        return 0;
    
    logger.info("MainPatch started");
    //patchLobbyFilter();

    for (;; Sleep(1000)) {
        worldObj = (float*)tracePointer(&patchData.worldObject);

        patchCamera();

        doDebug();
    }
}


bool MainPatch::isWorldObject() {
    float* tmp = (float*)calcAddress(patchData.worldObject.base_address);

    if (*tmp < 0) {
        logger.error("WorldObject does not exist!");
        return false;
    }
    else
        return true;
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
        logger.debug() << "New MaxZoom: " << newZoomValue << std::endl;

        patchZoomIncrement();
    }
}

void MainPatch::patchLobbyFilter() {
    if (patchData.lobbyVersionFilterAddr == 0)
        return;

    logger.debug("patching lobby version filter");

    short jne = 0x1E75;
    short* filter = (short*)calcAddress(patchData.lobbyVersionFilterAddr);
    writeBytes(filter, &jne, 2);
}

void MainPatch::patchZoomIncrement() {
    writeBytes(calcAddress(patchData.zoomIncrAddr), &zoomStep_p, 4);
    writeBytes(calcAddress(patchData.zoomDecrAddr), &zoomStep_p, 4);

    logger.debug("Zoom increment set");
}

bool MainPatch::calcZoomValue() {
    /* maxZoomValue will be set depending on the aspect ratio of the screen */
    float aspr = calcAspectRatio();

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

void MainPatch::doDebug() {
    if (!settings->engineData->bDebugMode)
        return;

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


bool isSupportedVersion(char* versionString) {
    char versionSettlers[9] = "Version:"; // All Settlers II remakes have this
    char tVersion[9];

    if (!readBytes(versionString, tVersion, 8))
        return false;

    //memcpy(tVersion, versionString, 8);
    tVersion[8] = 0;

    //showMessage(tVersion);

    return strcmp(tVersion, versionSettlers) == 0;
}

bool isSettlersVersion(char* versionString) {
    char versionBase[15] = "Version: 11757"; // GOG version & patched CD version
    char versionAddon[15] = "Version: 11758"; // Wikings Addon
    char gameVersion[15];

    if (!readBytes(versionString, gameVersion, 14))
        return false;

    //memcpy(gameVersion, versionString, 14);
    gameVersion[14] = 0;

    //showMessage(gameVersion);

    return strcmp(gameVersion, versionBase) == 0
        || strcmp(gameVersion, versionAddon) == 0;
}


int prepareMain(PatchSettings* settings) {
    /* wait a bit for the application to start up (might crash otherwise) */
    Sleep(startupDelay);

    // delete temporary VK config file after a few seconds
    std::thread rmThread([](char* VkPath, int sleep = 1e4) {
        Sleep(sleep);
        remove(VkPath);
    }, settings->cameraData->VkConfigPath);
    rmThread.detach();

    /* check if gameVersion is supported */
    char* sBase = (char*)calcAddress(Base_GOG::gameVersionAddr);
    char* sBaseGold = (char*)calcAddress(Base_Gold::gameVersionAddr);
    char* sAddon = (char*)calcAddress(Addon::gameVersionAddr);
    char* sAddonGold = (char*)calcAddress(Addon_Gold::gameVersionAddr);
    std::vector<char*> sVersions = { sBase, sBaseGold, sAddon, sAddonGold };

    bool bSupported = false;

    for (int i = 0; i < retryCount; i++) {
        if (std::any_of(sVersions.begin(), sVersions.end(), isSupportedVersion)) {
            bSupported = true;

            if (isSettlersVersion(sBase)) {
                logger.info("Found Base GOG version");
                settings->gameVersion = V_BASE_GOG;
                return MainPatch(Base_GOG::patchData, settings).run();
            }
            else if (isSettlersVersion(sBaseGold)) {
                logger.info("Found Base Gold Edition version");
                settings->gameVersion = V_BASE_GOLD;
                return MainPatch(Base_Gold::patchData, settings).run();
            }
            else if (isSettlersVersion(sAddon)) {
                logger.info("Found Addon version");
                settings->gameVersion = V_ADDON;
                return MainPatch(Addon::patchData, settings).run();
            }
            else if (isSettlersVersion(sAddonGold)) {
                logger.info("Found Addon Gold Edition");
                settings->gameVersion = V_ADDON_GOLD;
                return MainPatch(Addon_Gold::patchData, settings).run();
            }
        }

        logger.info("Version check retrying...");
        Sleep(retryTimeout);
    }

    if (!bSupported) {
        logger.error() << "This game version or editor is not supported! \n"
            << "Supported game versions are: \n"
            << "- GOG (11757) \n"
            << "- Base Game Gold Edition (11757) \n"
            << "- Wikings Addon (11758) \n"
            << "- Wikings Addon Gold Edition (11758)"
            << std::endl;
        //MessageBoxA(NULL, "This game version is not supported!", "Widescreen Fix ERROR", MB_OK);
    }

    return 0;
}

DWORD WINAPI MainPatchThread(LPVOID param) {
    return prepareMain(reinterpret_cast<PatchSettings*>(param));
}
