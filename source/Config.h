/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */
#pragma once

#include <SimpleIni.h>

const int startupDelay = 4000; // in ms

enum GameVersion {
    V_BASE_GOG, // base game GOG (11757)

    V_BASE_NOCD, // base game Retail noCD (11757)
    V_ADDON_NOCD, // addon Retail noCD (11758)

    V_BASE_GOLD, // base game Gold Edition (11757)
    V_ADDON_GOLD, // addon Gold Edition (11758)

    V_UNSUPPORTED,
    V_UNKNOWN
};

struct ServerAddr {
    const char* IP;
    unsigned int Port;
};

struct GameSettings {
    bool bHardwareCursor;
    bool bVSync;
    bool bVulkan;
    bool bDebugMode;
    bool bDebugWindow;
    bool bFileLoadPatch;
    bool bFileStorePatch;
    int fpsLimit;
    int MSAA;
    int Anisotropy;

    GameSettings(CSimpleIniA& ini);
    void writeEngineConfig(char* path);
    void writeDXconfig(char* path);
};

struct CameraData {
    bool bEnabled;
    bool bWideView;
    float fZoomIncrement;
    int customZoom;
    char VkConfigPath[MAX_PATH];

    CameraData(CSimpleIniA& ini);
};

struct LobbyData {
    bool bEnabled;
    bool bTincatDebug;
    //bool bNetworkPatch;
    unsigned int patchLevel;
    unsigned int gamePort;
    ServerAddr serverAddr;
    unsigned int apiPort;

    LobbyData(CSimpleIniA& ini);
    void writeNetworkConfig(char* path);
};

struct PatchSettings {
    HCURSOR cursor;
    GameVersion gameVersion;
    GameSettings* gameSettings;
    CameraData* cameraData;
    LobbyData* lobbyData;

    PatchSettings(GameSettings* eg, CameraData* cd, LobbyData* ld);
};

GameVersion getGameVersion(char* exePath);
