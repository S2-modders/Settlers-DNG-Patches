/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once
#include <Windows.h>
#include <vector>

struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

struct CameraData {
    bool bEnabled;
    bool bDebugMode;
    bool bWideView;
    float fZoomIncrement;
};

struct PatchData {
    memoryPTR worldObject;
    memoryPTR maxZoom;
    memoryPTR currZoom;
    DWORD lobbyVersionFilterAddr;
    DWORD gameVersionAddr;
};

const int version_maj = 1;
const int version_min = 6;

const int retryCount = 4;
const int retryTimeout = 2000;

DWORD WINAPI ZoomPatchThread(LPVOID param);
