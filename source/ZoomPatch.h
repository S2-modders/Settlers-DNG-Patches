/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once
#include "Config.h"

#include <Windows.h>
#include <sstream>
#include <iostream>

const int version_maj = 1;
const int version_min = 6;

const int retryCount = 4;
const int retryTimeout = 2000;

DWORD WINAPI ZoomPatchThread(LPVOID param);

class ZoomPatch {
public:
	explicit ZoomPatch(PatchData& patchData, CameraData* cameraData);

	int run();
	static void startupMessage();

private:
    PatchData& patchData;
    CameraData* cameraData;

    float* worldObj;
    float* maxZoom;
    float* zoomStep_p;

    int hor, ver;
    float newZoomValue = 4.0f; // 4 is the default zoom value

    bool isWorldObject();
    
    void patchZoom();
    void patchLobbyFilter();
    void patchZoomIncrement();
    bool calcZoomValue();

    void doDebug();
};
