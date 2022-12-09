/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once

#include <Windows.h>
#include <sstream>
#include <iostream>
#include <thread>

#include "Config.h"
#include "Helper/Logger.h"

const int version_maj = 1;
const int version_min = 7;

const int retryCount = 4;
const int retryTimeout = 2000;

DWORD WINAPI MainPatchThread(LPVOID param);

class MainPatch {
public:
	explicit MainPatch(PatchData& patchData, CameraData* cameraData);

	int run();
	static void startupMessage();

private:
    PatchData& patchData;
    CameraData* cameraData;

    float* worldObj;
    float* maxZoom;
    float* zoomStep_p;

    float newZoomValue = 4.0f; // 4 is the default zoom value

    bool isWorldObject();
    
    void patchZoom();
    void patchZoomIncrement();
    bool calcZoomValue();

    // TODO: this should be in Lobby.h
    void patchLobbyFilter();

    void doDebug();
};
