/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once
#include <Windows.h>

struct memoryPTR {
    DWORD base_address;
    int total_offsets;
    int offsets[];
};

struct threadData {
    bool bDebugMode;
    bool bWideView;
    float ZoomIncrement;
};

const int version_maj = 1;
const int version_min = 3;

DWORD WINAPI ZoomPatchThread(LPVOID param);
