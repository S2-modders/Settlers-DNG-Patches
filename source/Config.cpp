/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */
#pragma once

#include <sstream>
#include <fstream>

#include "utilities/Helper/Helper.h"

#include "Config.h"

EngineData* loadEngineSettings(CSimpleIni& ini) {
    auto* eData = new EngineData;

    eData->bHardwareCursor = !ini.GetBoolValue("Game", "CursorFix");
    eData->fpsLimit = ini.GetLongValue("Game", "FPSLimit");
    eData->bVSync = ini.GetBoolValue("Game", "ForceVSync", true);
    eData->MSAA = ini.GetLongValue("Game", "ForceMSAA", 4);
    eData->Anisotropy = ini.GetLongValue("Game", "ForceAnisotropy", 16);
    eData->bVulkan = ini.GetBoolValue("Game", "UseVulkan");

    eData->bDebugMode = ini.GetBoolValue("Misc", "DebugMode");
    eData->bDebugWindow = ini.GetBoolValue("Misc", "DebugWindow");

    return eData;
}

CameraData* loadCameraSettings(CSimpleIni& ini) {
    auto* cData = new CameraData;

    cData->bEnabled = ini.GetBoolValue("Camera", "enabled", true);

    int zoomIncr = ini.GetLongValue("Camera", "ZoomPatchStep", 1);
    switch (zoomIncr) {
    case 2:
        cData->fZoomIncrement = 0.5f;
        break;
    case 3:
        cData->fZoomIncrement = 0.25f;
        break;
    default:
        cData->fZoomIncrement = 1.0f;
    }

    cData->bWideView = ini.GetBoolValue("Camera", "WideViewMode", true);

    return cData;
}

LobbyData* loadLobbySettings(CSimpleIni& ini) {
    auto* lData = new LobbyData;

    lData->bEnabled = ini.GetBoolValue("Lobby", "enabled", false);
    ServerAddr addr = {
        ini.GetValue("Lobby", "ServerIP", "www.diesiedler2lobby.de"),
        (unsigned int)ini.GetLongValue("Lobby", "ServerPort", 8777)
    };
    lData->serverAddr = addr;
    lData->patchLevel = ini.GetLongValue("Lobby", "PatchLevel", 9212);
    lData->bTincatDebug = ini.GetBoolValue("Lobby", "DebugMode");
    lData->gamePort = 5479; // config option possible but IMO not needed

    return lData;
}


void setEngineData(char* iniPath, EngineData* eData) {
    CSimpleIni ini;
    ini.SetUnicode(false);

    ini.SetLongValue("Engine", "hardwareCursor", (long)eData->bHardwareCursor);
    ini.SetLongValue("Engine", "refreshRate", 0); // this setting causes more problems than it solves
    ini.SetLongValue("Engine", "useMeshCache", 1);

    if (eData->bVulkan)
        ini.SetLongValue("Engine", "vsync", 0);
    else 
        ini.SetLongValue("Engine", "vsync", eData->bVSync ? 1 : 0);

    remove(iniPath);
    ini.SaveFile(iniPath);
}


void setNetworkData(char* iniPath, LobbyData* lData) {
    CSimpleIni ini;
    ini.SetUnicode(false);

    ini.SetLongValue("Basics", "gamePort", (long)lData->gamePort);
    std::stringstream ss;
    ss << lData->serverAddr.IP << ":" << lData->serverAddr.Port;
    ini.SetValue("Lobby", "url", ss.str().c_str());
    ini.SetLongValue("Lobby", "patchlevel", (long)lData->patchLevel);

    // set all the remaining default values
    ini.SetLongValue("Broadcast", "broadcastPort", 6582);
    ini.SetLongValue("Broadcast", "broadcastTimeout", 10000);

    ini.SetDoubleValue("Ping", "pingFrequencyMenu", 10.0f);
    ini.SetDoubleValue("Ping", "pingFrequencyGame", 2.5f);
    
    ini.SetDoubleValue("Reconnector", "timeHostWaitForClients", 30.0f);
    ini.SetDoubleValue("Reconnector", "timeClientWaits", 3.0f);
    ini.SetLongValue("Reconnector", "timesClientRetries", 5);
    ini.SetDoubleValue("Reconnector", "timeClientWaitsFailed", 4.0f);

    remove(iniPath);
    ini.SaveFile(iniPath);
}

void initDXconfig(char* path, EngineData* eData) {
    // remove in case it already exists
    remove(path);

    std::ofstream ofstr(path);

    ofstr << "d3d9.maxFrameRate = " << eData->fpsLimit << std::endl;
    ofstr << "d3d9.forceSwapchainMSAA = " << eData->MSAA << std::endl;
    ofstr << "d3d9.samplerAnisotropy = " << eData->Anisotropy << std::endl;
    ofstr << "d3d9.presentInterval = " << (eData->bVSync ? 1 : 0) << std::endl;

    ofstr.close();
}
