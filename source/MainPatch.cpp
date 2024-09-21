/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include <sstream>
#include <iostream>
#include <thread>
#include <fstream>

#include "utilities/Helper/Logger.h"

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

    // TODO file load decrypt patch
    patchFileLoader();

    logger.info("Waiting for application startup...");

    unsigned int time = 0;
    while (isWorldObject() == false) {
        Sleep(1);
        time += 1;
    }
    logger.info() << "Game init in " << time << "ms" << std::endl;
    logger.info("MainPatch started");

    //patchLobbyFilter();
    //patchFogDisable();

    for (;; Sleep(1000)) {
        worldObj = (float*)tracePointer(&patchData.worldObject);

        patchCamera();

        if (settings->engineData->bDebugMode)
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

bool MainPatch::isWorldObject() {
    DWORD* tmp = calcAddress(patchData.worldObject.base_address);
    return *tmp > 0;
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

void MainPatch::patchLobbyFilter() {
    if (patchData.lobbyVersionFilterAddr == 0)
        return;

    logger.debug("patching lobby version filter");

    short jne = 0x1E75;
    short* filter = (short*)calcAddress(patchData.lobbyVersionFilterAddr);
    writeBytes(filter, &jne, 2);
}

float fogStart = 1000;
DWORD ret1;
void _declspec(naked) jumperFogPatch1() {
    __asm {
        fld dword ptr [fogStart]
        fstp dword ptr [esi+0x6C]
        
        fstp st(0)
        fld st(0)
        jmp [ret1]
    }
}
float fogEnd = 1000;
DWORD ret2;
void _declspec(naked) jumperFogPatch2() {
    __asm {
        fld dword ptr [fogEnd]
        fstp dword ptr [esi+0x70]

        fstp st(0)
        fmul dword ptr [eax+0x40]
        jmp[ret2]
    }
}

void MainPatch::patchFogDisable() {
    logger.debug("fog patch");

    DWORD* fogStart = calcAddress(0x13CECA);
    DWORD* fogEnd = calcAddress(0x13CEDD);

    functionInjectorReturn(fogStart, jumperFogPatch1, ret1, 5);
    functionInjectorReturn(fogEnd, jumperFogPatch2, ret2, 6);
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


struct Stringbla {
    DWORD unknown;
    char* filename;
    DWORD unknown2;
    DWORD unknown3;
    DWORD unknown4;
    int length;
    int bufferSize;
};

DWORD FilenamePtr = 0;
DWORD FilebufferPtr = 0;
DWORD FilebufferSizePtr = 0;
DWORD ret3;

static bool IsEncrypted = false;

void lolol2() {
    Stringbla* ptr = (Stringbla*)FilenamePtr;
    char* filebuffer = *(char**)FilebufferPtr;

    IsEncrypted = strncmp(filebuffer + 0x4, "rc00", 4) == 0;

    logger.debug() << "-- loading: " 
        << IsEncrypted << " | "
        << ptr->filename 
        << std::endl;

    /*
    logger.info(" -------- new file -------- ");
    logger.info() << "EBX content: \n"
        << "filename: " << ptr->filename << "\n"
        << "string length: " << ptr->length << "\n"
        << "string buffer size: " << ptr->bufferSize << "\n"
        << "first file bytes: " << **(int**)FilebufferPtr << "\n"
        << "file buffer size: " << *(int*)FilebufferSizePtr << "\n"
        << "file encrypted: ";

    if (IsEncrypted) {
        logger.naked("YES");
    }
    else {
        logger.naked("NO");
    }

    logger.naked() << std::endl;
    */
}

void _declspec(naked) nakedFileLoadTest() {
    __asm {
        push eax
        
        mov eax, [esp+0x28+0x4]
        mov [FilenamePtr], eax

        mov eax, [esp + 0x28 + 0x4 + 0x4]
        mov[FilebufferPtr], eax

        mov eax, [esp + 0x28 + 0x4 + 0x4 + 0x4]
        mov[FilebufferSizePtr], eax

        pop eax

        push ebx
        push ebp
        push esi
        mov [esp+0x2C], eax

        pushad
        call [lolol2]
        popad
    }

    if (IsEncrypted) {
        __asm {
            jmp [ret3]
        }
    }
    else {
        // cleanup and RET
        __asm {
            pop esi
            pop ebp
            pop ebx

            add esp, 0x24
            ret
        }
    }
}

static void storeDecryptedData() {
    Stringbla* ptr = (Stringbla*)FilenamePtr;
    char* filebuffer = (char*)FilebufferPtr;
    int filebuffersize = (int)FilebufferSizePtr;

    char suffix[9] = ".decrypt";
    int newStringSize = ptr->length + sizeof(suffix);
    
    char* newFilename = (char*)malloc(newStringSize);

    strncpy_s(newFilename, newStringSize, ptr->filename, ptr->length);
    strncat_s(newFilename, newStringSize, suffix, sizeof(suffix));

    /*
    logger.debug() << "NEW FILENAME: " << newFilename << "\n"
        << "filebuffersize: " << filebuffersize
        << std::endl;
    */

    std::ofstream fileOutput;
    fileOutput.open(newFilename, std::ios::binary | std::ios::trunc);
    fileOutput.write(filebuffer, filebuffersize);
    fileOutput.close();

    free(newFilename);
}

DWORD ret4;
void _declspec(naked) storeEncryptedData() {
    __asm {
        push edx
        push ecx

        mov edx, [esp+0x10+0x8]
        mov [FilebufferSizePtr], edx

        mov ecx, [esp+0x14+0x8]
        mov [FilebufferPtr], ecx

        pop ecx
        pop edx

        pushad
        call [storeDecryptedData]
        popad

        mov esi, [esp + 0x3C]
        mov eax, [esi]

        jmp [ret4]
    }
}

void MainPatch::patchFileLoader() {
    DWORD* fileloadFktAddr = calcAddress(0x193B78);

    int time = 0;

    while (*fileloadFktAddr == 0) {
        Sleep(1);
        time += 1;
    }
    logger.info() << "Function might be loaded; took:" << time << "ms" << std::endl;

    if (functionInjectorReturn(fileloadFktAddr, nakedFileLoadTest, ret3, 7)) {
        logger.info("data decrypt function inject");
    }

    DWORD* fileloadFktEndAddr = calcAddress(0x193D35);

    if (functionInjectorReturn(fileloadFktEndAddr, storeEncryptedData, ret4, 6)) {
        logger.info("data decrypt store function inject");
    }
}

static int prepareMain(PatchSettings* settings) {
    /* check if gameVersion is supported */
    switch (settings->gameVersion) {
    case V_BASE_GOG:
        logger.info("Found DNG GOG version");
        return MainPatch(Base_GOG::patchData, settings).run();
        break;

    case V_ADDON_NOCD:
        logger.info("Found Wikinger Retail noCD version");
        return MainPatch(Addon::patchData, settings).run();
        break;

    case V_BASE_GOLD:
        logger.info("Found DNG Gold Edition");
        return MainPatch(Base_Gold::patchData, settings).run();
        break;
    case V_ADDON_GOLD:
        logger.info("Found Wikinger Gold Edition");
        return MainPatch(Addon_Gold::patchData, settings).run();
        break;

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
