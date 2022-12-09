/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */
#pragma once

#include "Helper/Helper.h"
#include "SimpleIni/SimpleIni.h"

#include <sstream>
#include <fstream>

struct EngineData {
	bool bHardwareCursor;
	bool bVSync;
    bool bNativeDX;
    bool bDebugMode;
    bool bDebugWindow;
	int refreshRate;
	int fpsLimit;
    int MSAA;
    int Anisotropy;
};

struct ServerAddr {
    char* IP;
    int Port;
};

struct NetworkData {
    int gamePort;
    ServerAddr serverAddr;
    int patchLevel;
};

struct CameraData {
    bool bEnabled;
    bool bWideView;
    bool bDebugMode;
    bool bDebugWindow;
    float fZoomIncrement;
    char VkConfigPath[MAX_PATH];
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

struct LobbyData {
    bool bEnabled;
    bool bTincatDebug;
    bool bNetworkPatch;
    bool bDebugMode;
    const char* serverIP;
    int patchlevel;
};


EngineData* loadEngineSettings(CSimpleIni& ini);
CameraData* loadCameraSettings(CSimpleIni& ini);
LobbyData* loadLobbySettings(CSimpleIni& ini);

void setEngineData(char* iniPath, EngineData* eData);
void setNetworkData(char* iniPath, NetworkData* nData);

void initDXconfig(char* path, EngineData* eData);
