/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */
#pragma once

#include <sstream>
#include <fstream>

#include "Helper/Helper.h"
#include "SimpleIni/SimpleIni.h"

const int startupDelay = 4000; // in ms

enum GameVersion {
    V_BASE_GOG,
    V_BASE_GOLD,
    V_ADDON,
    V_ADDON_GOLD,
    V_UNKNOWN,
    V_UNSUPPORTED
};

struct ServerAddr {
    const char* IP;
    unsigned int Port;
};

struct EngineData {
    bool bHardwareCursor;
    bool bVSync;
    bool bNativeDX;
    bool bDebugMode;
    bool bDebugWindow;
    int fpsLimit;
    int MSAA;
    int Anisotropy;
};

struct CameraData {
    bool bEnabled;
    bool bWideView;
    float fZoomIncrement;
    char VkConfigPath[MAX_PATH];
};

struct LobbyData {
    bool bEnabled;
    bool bTincatDebug;
    //bool bNetworkPatch;
    unsigned int patchLevel;
    unsigned int gamePort;
    ServerAddr serverAddr;
};

struct PatchSettings {
    HCURSOR cursor;
    GameVersion gameVersion;
    EngineData* engineData;
    CameraData* cameraData;
    LobbyData* lobbyData;
};

EngineData* loadEngineSettings(CSimpleIni& ini);
CameraData* loadCameraSettings(CSimpleIni& ini);
LobbyData* loadLobbySettings(CSimpleIni& ini);

void setEngineData(char* iniPath, EngineData* eData);
void setNetworkData(char* iniPath, LobbyData* lData);

void initDXconfig(char* path, EngineData* eData);
