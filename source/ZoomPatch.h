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

struct patchData {
    memoryPTR WorldObject;
    memoryPTR MaxZoom;
    memoryPTR CurrZoom;
    DWORD FilterPatch;
};

const int version_maj = 1;
const int version_min = 4;

DWORD WINAPI ZoomPatchThread(LPVOID param);
