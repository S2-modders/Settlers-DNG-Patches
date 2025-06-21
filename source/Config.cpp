/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */

#include "pch.h"

#include <Helper.h>
#include "Config.h"

GameSettings::GameSettings(CSimpleIniA& ini) {
    bHardwareCursor = !ini.GetBoolValue("Game", "CursorFix");
    fpsLimit = ini.GetLongValue("Game", "FPSLimit");
    bVSync = ini.GetBoolValue("Game", "ForceVSync", true);
    MSAA = ini.GetLongValue("Game", "ForceMSAA", 4);
    Anisotropy = ini.GetLongValue("Game", "ForceAnisotropy", 16);
    bVulkan = ini.GetBoolValue("Game", "UseVulkan");

    bDebugMode = ini.GetBoolValue("Misc", "DebugMode");
    bDebugWindow = ini.GetBoolValue("Misc", "DebugWindow");

    bFileLoadPatch = ini.GetBoolValue("Misc", "FileLoadPatch", true);
    bFileStorePatch = ini.GetBoolValue("Misc", "FileStorePatch");
}

CameraData::CameraData(CSimpleIniA& ini) {
    bEnabled = ini.GetBoolValue("Camera", "enabled", true);

    int zoomIncr = ini.GetLongValue("Camera", "ZoomPatchStep", 1);
    switch (zoomIncr) {
    case 2:
        fZoomIncrement = 0.5f;
        break;
    case 3:
        fZoomIncrement = 0.25f;
        break;
    default:
        fZoomIncrement = 1.0f;
    }

    bWideView = ini.GetBoolValue("Camera", "WideViewMode", true);
    customZoom = ini.GetLongValue("Camera", "DebugZoomOverwrite", -1);
}

LobbySettings::LobbySettings(CSimpleIniA& ini) {
    bEnabled = ini.GetBoolValue("Lobby", "enabled", false);
    ServerAddr addr = {
        ini.GetValue("Lobby", "ServerIP", "www.diesiedler2lobby.de"),
        (unsigned int)ini.GetLongValue("Lobby", "ServerPort", 8777)
    };
    serverAddr = addr;
    patchLevel = ini.GetLongValue("Lobby", "PatchLevel", 9212);
    bTincatDebug = ini.GetBoolValue("Lobby", "DebugMode");
    gamePort = 5479; // config option possible but IMO not needed
    apiPort = (unsigned int)ini.GetLongValue("Lobby", "ApiPort", 6801);

#if 0
    bCreateBridge = ini.GetBoolValue("Lobby", "Bridge");
#else
    bCreateBridge = false;
#endif
}


PatchSettings::PatchSettings(GameSettings* eg, CameraData* cd, LobbySettings* ld) 
    : gameSettings(eg), cameraData(cd), lobbySettings(ld) {
    cursor = NULL;
    gameVersion = V_UNKNOWN;
}


void GameSettings::writeEngineConfig(char* path) {
    CSimpleIniA ini;
    ini.SetUnicode(false);

    ini.SetLongValue("Engine", "hardwareCursor", (long)bHardwareCursor);
    ini.SetLongValue("Engine", "refreshRate", 0); // this setting causes more problems than it solves
    ini.SetLongValue("Engine", "useMeshCache", 1);

    if (bVulkan)
        ini.SetLongValue("Engine", "vsync", 0);
    else
        ini.SetLongValue("Engine", "vsync", bVSync ? 1 : 0);

    remove(path);
    ini.SaveFile(path);
}

void LobbySettings::writeNetworkConfig(char* path) {
    CSimpleIniA ini;
    ini.SetUnicode(false);

    ini.SetLongValue("Basics", "gamePort", (long)gamePort);
    std::stringstream ss;
    ss << serverAddr.IP << ":" << serverAddr.Port;
    ini.SetValue("Lobby", "url", ss.str().c_str());
    ini.SetLongValue("Lobby", "patchlevel", (long)patchLevel);

    // set all the remaining default values
    ini.SetLongValue("Broadcast", "broadcastPort", 6582);
    ini.SetLongValue("Broadcast", "broadcastTimeout", 10000);

    ini.SetDoubleValue("Ping", "pingFrequencyMenu", 10.0f);
    ini.SetDoubleValue("Ping", "pingFrequencyGame", 2.5f);

    ini.SetDoubleValue("Reconnector", "timeHostWaitForClients", 30.0f);
    ini.SetDoubleValue("Reconnector", "timeClientWaits", 3.0f);
    ini.SetLongValue("Reconnector", "timesClientRetries", 5);
    ini.SetDoubleValue("Reconnector", "timeClientWaitsFailed", 4.0f);

    remove(path);
    ini.SaveFile(path);
}

void GameSettings::writeDXconfig(char* path) {
    std::ofstream ofstr(path, std::ios::trunc);

    ofstr << "d3d9.maxFrameRate = " << fpsLimit << std::endl;
    ofstr << "d3d9.forceSwapchainMSAA = " << MSAA << std::endl;
    ofstr << "d3d9.samplerAnisotropy = " << Anisotropy << std::endl;
    ofstr << "d3d9.presentInterval = " << (bVSync ? 1 : 0) << std::endl;

    ofstr.close();
}

GameVersion getGameVersion(char* exePath) {
    std::string checksum;

    if (!getFileChecksum(exePath, checksum)) {
        return V_UNKNOWN;
    }

    if (checksum == "484a8afc396df4ae0e9429b604993dcb5e238f2c0f0fe6d5085ab4620af548b7") {
        return V_BASE_GOG;
    }
    else if (checksum == "3bc844be08d3c15ccf40c0b3dc1ff19a858c01ac48738a441033b352cb1217f1") {
        return V_BASE_GOLD;
    }
    else if (checksum == "091944faf932c30825fb4920d71247c427e75a497ce01fe2540ad8595f655ef9") {
        return V_ADDON_GOLD;
    }
#if 0
    else if (checksum == "178a4299a2dc441aa8ed2f32e050f9a99ef40150f42e2c1acd8b00933db1285e") {
        return V_BASE_NOCD;
    }
    else if (checksum == "f461cbaa27e523a0ae8eb6fd6e318c5c21df5e2eddda9f6a4d424aa08b8c17e8") {
        return V_ADDON_NOCD;
    }
#endif
    else {
        return V_UNSUPPORTED;
    }
}
