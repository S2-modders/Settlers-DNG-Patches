/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once

#include <Windows.h>

#include "utilities/Helper/Helper.h"

#include "Config.h"

const int version_maj = 1;
const int version_min = 11;


struct PatchData {
    memoryPTR worldObject;
    memoryPTR maxZoom;
    memoryPTR currZoom;
    DWORD lobbyVersionFilterAddr;
    DWORD gameVersionAddr;
    DWORD zoomIncrAddr;
    DWORD zoomDecrAddr;
};

DWORD WINAPI MainPatchThread(LPVOID param);

class MainPatch {
public:
    explicit MainPatch(PatchData& patchData, PatchSettings* settings);

    int run();
    static void startupMessage();

    static int calcMaxFramerate(int maxFrameRate = 0, bool vSync = true);

private:
    PatchData& patchData;
    PatchSettings* settings;

    float* worldObj;
    float* maxZoom;
    float* zoomStep_p;

    float newZoomValue = 4.0f; // 4 is the default zoom value

    bool isWorldObject();
    
    void patchCamera();
    void patchZoomIncrement();

    void patchFogDisable();

    // TODO: this should be in Lobby.h
    void patchLobbyFilter();

    bool isZoomOverride();
    bool calcZoomValue();

    void doDebug();
    void setCursor();
};
