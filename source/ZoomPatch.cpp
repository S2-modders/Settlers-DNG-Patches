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


    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr
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


    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr
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


    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr
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


    PatchData patchData = {
        worldObject,
        maxZoom,
        currZoom,
        lobbyVersionFilterAddr,
        gameVersionAddr
    };
}

/*###################################*/


class ZoomPatch {

public:

    explicit ZoomPatch(PatchData& patchData, CameraData* cameraData) : patchData(patchData) {
        this->cameraData = cameraData;
    }

    int run() {
        if (!isWorldObject())
            return 0;

        //patchLobbyFilter();

        for (;; Sleep(1000)) {
            worldObj = (float*)tracePointer(&patchData.worldObject);
            patchZoom();
            //setZoomIncrement();

            doDebug();
        }
    }

    static void startupMessage() {
        std::cout << "ZoomPatch & LobbyPatch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n";
        std::cout << "Debug mode enabled!\n";
        std::cout << "Waiting for application startup...\n";
    }

private:

    PatchData& patchData;
    CameraData* cameraData;
    
    float* worldObj;
    float* maxZoom;

    int hor, ver;
    float newZoomValue = 4.0f; // 4 is the default zoom value


    bool isWorldObject() {
        float* tmp = (float*)calcAddress(patchData.worldObject.base_address);

        if (*tmp < 0) {
            showMessage("WorldObject does not exist!");
            return false;
        }
        else
            return true;
    }

    void patchZoom() {
        if (*worldObj == 0)
            return;

        maxZoom = (float*)tracePointer(&patchData.maxZoom);
        if (calcZoomValue() && *maxZoom != newZoomValue) {
            if (cameraData->bWideView)
                showMessage("WideView enabled");
            else
                showMessage("WideView disabled");

            *maxZoom = newZoomValue;
        }
    }

    void patchLobbyFilter() {
        if (patchData.lobbyVersionFilterAddr == 0)
            return;

        showMessage("patching lobby version filter");

        short jne = 0x1E75;
        short* filter = (short*)calcAddress(patchData.lobbyVersionFilterAddr);
        writeBytes(filter, &jne, 2);
    }

    void setZoomIncrement() {
        //float* zoomStep_p = &tData->fZoomIncrement;

        /*
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
        */
    }

    bool calcZoomValue() {
        GetDesktopResolution2(hor, ver);
        float aspr = calcAspectRatio(hor, ver);

        if (aspr > 0.0f && aspr <= 20.0f) {
            /* maxZoomValue will be set depending on the aspect ratio of the screen */
            if (cameraData->bWideView) {
                if (aspr < 1.5f) {
                    newZoomValue = 6.0f;
                    return true;
                }
                else if (aspr < 1.9f) {
                    newZoomValue = 7.0f;
                    return true;
                }
                else if (aspr < 2.6f) {
                    newZoomValue = 8.0f;
                    return true;
                }
                else if (aspr >= 2.6f) {
                    newZoomValue = 9.0f;
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                if (aspr < 1.5f) {
                    newZoomValue = 4.0f;
                    return true;
                }
                else if (aspr < 1.9f) {
                    newZoomValue = 5.0f;
                    return true;
                }
                else if (aspr < 2.2f) {
                    newZoomValue = 6.0f;
                    return true;
                }
                else if (aspr < 2.6f) {
                    newZoomValue = 7.0f;
                    return true;
                }
                else if (aspr >= 2.6f) {
                    newZoomValue = 7.0f;
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

    void doDebug() {
        if (!cameraData->bDebugMode)
            return;

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
                showMessage(*(float*)(tracePointer(&patchData.maxZoom)));
            }
        }
        if (IsKeyPressed(VK_F7)) {
            showMessage(*(float*)(tracePointer(&patchData.currZoom)));
        }
        if (IsKeyPressed(VK_F8)) {
            std::cout << "World address: " << worldObj << " World value: " << *worldObj << "\n";
        }
    }

};


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
    char versionBase[15] = "Version: 11757"; // GOG version & patched CD version
    char versionAddon[15] = "Version: 11758"; // Wikings Addon
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


int prepare(CameraData* cData) {
    FILE* f;

    if (cData->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        ZoomPatch::startupMessage();
    }

    /* wait a bit for the application to start up (might crash otherwise) */
    Sleep(4000);

    /* check if gameVersion is supported */
    char* sBase = (char*)calcAddress(Base_GOG::gameVersionAddr);
    char* sBaseGold = (char*)calcAddress(Base_Gold::gameVersionAddr);
    char* sAddon = (char*)calcAddress(Addon::gameVersionAddr);
    char* sAddonGold = (char*)calcAddress(Addon_Gold::gameVersionAddr);
    bool bSupported = false;

    for (int i = 0; i < retryCount; i++) {

        if (checkSettlersII(sBase) || checkSettlersII(sBaseGold) || checkSettlersII(sAddon) || checkSettlersII(sAddonGold)) {
            if (checkSettlersVersion(sBase)) {
                showMessage("Found Base GOG version.");
                bSupported = true;

                return ZoomPatch(Base_GOG::patchData, cData).run();
            }
            else if (checkSettlersVersion(sBaseGold)) {
                showMessage("Found Base Gold Edition version.");
                bSupported = true;

                return ZoomPatch(Base_Gold::patchData, cData).run();
            }
            else if (checkSettlersVersion(sAddon)) {
                showMessage("Found Addon version.");
                bSupported = true;

                return ZoomPatch(Addon::patchData, cData).run();
            }
            else if (checkSettlersVersion(sAddonGold)) {
                showMessage("Found Addon Gold Edition.");
                bSupported = true;

                return ZoomPatch(Addon_Gold::patchData, cData).run();
            }
        }

        showMessage("retrying...");
        Sleep(retryTimeout);
    }

    if (!bSupported) {
        showMessage("This game version is not supported!");
        MessageBoxA(NULL, "This game version is not supported!", "Widescreen Fix ERROR", MB_OK);
    }

    return 0;
}

DWORD WINAPI ZoomPatchThread(LPVOID param) {
    return prepare(reinterpret_cast<CameraData*>(param));
}
