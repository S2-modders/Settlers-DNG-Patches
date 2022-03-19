/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include <Windows.h>
#include <sstream>
#include <iostream>
#include "ZoomPatch.h"
#include "Helper.h"

/* memory values */

/* DnG Base game (11757) */
memoryPTR MaxZoomPTR_base = {
    0x002BD4E8,
    { 0x4C, 0x1A8 }
};
memoryPTR CurrZoomPTR_base = {
    0x002BD4E8,
    { 0x4C, 0x1A4 }
};
memoryPTR WorldObjectPTR_base = {
    0x002BD4E8,
    { 0x4C }
};
DWORD LobbyVersionFilter_base = 0x107055;

/* DnG Wikinger Addon (11758) */
memoryPTR MaxZoomPTR_addon = {
    0x002CA528,
    { 0x4C, 0x1BC }
};
memoryPTR CurrZoomPTR_addon = {
    0x002CA528,
    { 0x4C, 0x1B8 }
};
memoryPTR WorldObjectPTR_addon = {
    0x002CA528,
    { 0x4C }
};
DWORD LobbyVersionFilter_addon = 0x00; // unknown

patchData patchBase = {
    WorldObjectPTR_base,
    MaxZoomPTR_base,
    CurrZoomPTR_base,
    LobbyVersionFilter_base
};

patchData patchAddon = {
    WorldObjectPTR_addon,
    MaxZoomPTR_addon,
    CurrZoomPTR_addon,
    LobbyVersionFilter_addon
};

DWORD BaseGameVersionAddr = 0x2C5A30;
DWORD AddonGameVersionAddr = 0x2D2DB8;

/*###################################*/

void startupMessage() {
    std::cout << "ZoomPatch & LobbyPatch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n";
    std::cout << "Debug mode enabled!\n";
    std::cout << "Waiting for application startup...\n";
}

bool checkSettlersII(char* versionString) {
    char versionSettlers[9] = "Version:"; // All Settlers II remakes have this
    char tVersion[9];

    if (!readBytes(versionString, tVersion, 8))
        return false;
    //memcpy(tVersion, versionString, 8);
    tVersion[8] = 0;

    showMessage(tVersion);

    if (strcmp(tVersion, versionSettlers) != 0)
        return false;
    else
        return true;
}

bool checkSettlersVersion(char* versionString) {
    char versionBase[15] = "Version: 11757"; // GOG version & patched CD version + NOCD
    char versionAddon[15] = "Version: 11758"; // Wikings Addon + NOCD
    char gameVersion[15];

    if (!readBytes(versionString, gameVersion, 14))
        return false;
    //memcpy(gameVersion, versionString, 14);
    gameVersion[14] = 0;

    showMessage(gameVersion);

    if (strcmp(gameVersion, versionBase) != 0
            && strcmp(gameVersion, versionAddon) != 0)
        return false;
    else
        return true;
}

bool calcNewZoomValue(int& hor, int& vert, float& zoom_value, bool wideview) {
    GetDesktopResolution2(hor, vert);
    float aspr = calcAspectRatio(hor, vert);
    if (aspr > 0.0f && aspr <= 20.0f) {
        /* maxZoomValue will be set depending on the Aspect Ratio of the screen */
        if (wideview) {
            if (aspr < 1.5f) {
                zoom_value = 6.0f;
                return true;
            }
            else if (aspr < 1.9f) {
                zoom_value = 7.0f;
                return true;
            }
            else if (aspr < 2.6f) {
                zoom_value = 8.0f;
                return true;
            }
            else if (aspr >= 2.6f) {
                zoom_value = 9.0f;
                return true;
            }
            else {
                return false;
            }
        }
        else {
            if (aspr < 1.5f) {
                zoom_value = 4.0f;
                return true;
            }
            else if (aspr < 1.9f) {
                zoom_value = 5.0f;
                return true;
            }
            else if (aspr < 2.2f) {
                zoom_value = 6.0f;
                return true;
            }
            else if (aspr < 2.6f) {
                zoom_value = 7.0f;
                return true;
            }
            else if (aspr >= 2.6f) {
                zoom_value = 7.0f;
                return true;
            }
            else {
                return false;
            }
        }

        return true;
    }
    else {
        return false;
    }
}

int MainLoop(patchData& patchData, zoomThreadData* tData) {
    float* worldObj;
    float* maxZoom;

    int hor;
    int ver;
    float newZoomValue = 4.0f; // 4 is the default zoom value
    float* zoomStep_p = &tData->ZoomIncrement;


    /*
    {
        memoryPTR zoomIncr = {
            0x20D00E + 0x02,
            { 0x0 }
        };
        memoryPTR zoomDecr = {
            0x20CFE4 + 0x02,
            { 0x0 }
        };

        DWORD tmp[4];
        readBytes(tracePointer(&zoomIncr), tmp, 4);
        std::cout << "ZoomStep " << tData->ZoomIncrement << "\n";
        std::cout << "ZoomStep " << &tData->ZoomIncrement << "\n";
        writeBytes(tracePointer(&zoomIncr), &zoomStep_p, 4);
        writeBytes(tracePointer(&zoomDecr), &zoomStep_p, 4);
    }
    */

    /* lobby version filter patch */
    {
        if (patchData.FilterPatch != 0) {
            showMessage("patching lobby version filter");

            short jne = 0x1E75;
            short* filter = (short*)calcAddress(patchData.FilterPatch);
            writeBytes(filter, &jne, 2);
        }
    }

    /* check if WorldObject does exist */
    {
        float* tmp = (float*)calcAddress(patchData.WorldObject.base_address);
        if (*tmp < 0)
            return 0;
    }

    for (;; Sleep(1000)) {
        worldObj = (float*)(tracePointer(&patchData.WorldObject));

        if (tData->bDebugMode) {
            if (IsKeyPressed(VK_F3)) {
                int t_hor = 0;
                int t_ver = 0;
                GetDesktopResolution2(t_hor, t_ver);
                std::stringstream ss;
                ss << "DEBUG:  xRes: " << t_hor << " yRes: " << t_ver;
                ss << " ASPR: " << calcAspectRatio(t_hor, t_ver);
                ss << " MaxZoomValue: " << newZoomValue;
                std::cout << ss.str() << "\n";
                //MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
            }
            if (IsKeyPressed(VK_F6)) {
                if (*worldObj != 0) {
                    //*(float*)(tracePointer(&CurrZoomPTR)) += 1.0f;
                    showMessage(*(float*)(tracePointer(&patchData.MaxZoom)));
                }
            }
            if (IsKeyPressed(VK_F7)) {
                showMessage(*(float*)(tracePointer(&patchData.CurrZoom)));
            }
            if (IsKeyPressed(VK_F8)) {
                std::cout << "World address: " << worldObj << "\n";
            }
        }

        /* Zoom hack */
        if (*worldObj != 0) {
            maxZoom = (float*)(tracePointer(&patchData.MaxZoom));
            if (calcNewZoomValue(hor, ver, newZoomValue, tData->bWideView) && *maxZoom != newZoomValue) {
                if (tData->bWideView)
                    showMessage("WideViewMode enabled");
                else
                    showMessage("WideViewMode disabled");
                *maxZoom = newZoomValue;
            }
        }
    }
}

int ZoomPatch(zoomThreadData* tData) {
    //Sleep(1000);
    FILE* f;

    if (tData->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        startupMessage();
    }

    /* wait a bit for the application to start up (might crash otherwise) */
    Sleep(4000);

    /* check if gameVersion is supported */
    char* sBase = (char*)calcAddress(BaseGameVersionAddr);
    char* sAddon = (char*)calcAddress(AddonGameVersionAddr);
    bool bSupported = false;

    for (int i = 0; i < 4; i++) {
        if (checkSettlersII(sBase) || checkSettlersII(sAddon)) {
            if (checkSettlersVersion(sBase)) {
                showMessage("Found Base version.");
                bSupported = true;
                return MainLoop(patchBase, tData);
            }
            else if (checkSettlersVersion(sAddon)) {
                showMessage("Found Addon version.");
                bSupported = true;
                return MainLoop(patchAddon, tData);
            }
        }
        showMessage("retrying...");
        Sleep(2000);
    }
    if (!bSupported)
        showMessage("Game version not supported!");
    return 0;
}

DWORD WINAPI ZoomPatchThread(LPVOID param) {
    return ZoomPatch(reinterpret_cast<zoomThreadData*>(param));
}

// rename to "DllMain" if you want to use this
bool APIENTRY DllMain_alt(  HMODULE hModule,
                            DWORD  ul_reason_for_call,
                            LPVOID lpReserved
                          ) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //SetProcessDPIAware();
        CreateThread(0, 0, ZoomPatchThread, hModule, 0, 0);
        return 0;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }
    return 0;
}
