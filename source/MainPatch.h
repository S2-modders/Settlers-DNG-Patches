/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once

#include <Helper.h>

#include "Config.h"

const int version_maj = 2;
const int version_min = 2;


struct PatchData {
    memoryPTR worldObject;
    memoryPTR maxZoom;
    memoryPTR currZoom;
    DWORD zoomIncrAddr;
    DWORD zoomDecrAddr;
    DWORD fileLoadAddr;
    DWORD fileLoadEndAddr;
    DWORD lobbyVersionFilterAddr1;
    DWORD lobbyVersionFilterAddr2;
};

DWORD WINAPI MainPatchThread(LPVOID param);

class MainPatch {
public:
    static void startupMessage();
    static int calcMaxFramerate(int maxFrameRate = 0, bool vSync = true);

    explicit MainPatch(PatchData& patchData, PatchSettings* settings);
    int run();

private:
    PatchData& patchData;
    PatchSettings* settings;

    float* worldObj;
    float* maxZoom;
    float* zoomStep_p;

    float newZoomValue = 4.0f; // 4 is the default zoom value

    void waitGameLoad();
    long long waitGameInit();

    void patchFileLoad();
    void patchFileStore();

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
