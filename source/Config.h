/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */

#pragma once
#include "Helper.h"
#include "SimpleIni/SimpleIni.h"

struct EngineData {
	bool bHardwareCursor;
	bool bVSync;
	bool bWindowed;
	int refreshRate;
	int fpsLimit;
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
    DWORD zoomIncrAddr;
    DWORD zoomDecrAddr;
};

EngineData* loadEngineSettings(CSimpleIni& ini);
CameraData* loadCameraSettings(CSimpleIni& ini);
LobbyData* loadLobbySettings(CSimpleIni& ini);

void setEngineData(char* iniPath, EngineData* eData);
