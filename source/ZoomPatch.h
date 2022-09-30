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

struct zoomThreadData {
    bool bDebugMode;
    bool bWideView;
    float ZoomIncrement;
};

struct patchData {
    memoryPTR WorldObject;
    memoryPTR MaxZoom;
    memoryPTR CurrZoom;
    DWORD FilterPatch;
};

const int version_maj = 2;
const int version_min = 0;

DWORD WINAPI ZoomPatchThread(LPVOID param);
