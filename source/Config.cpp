/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 */

#pragma once
#include "Config.h"

EngineData* loadEngineSettings(CSimpleIni& ini) {
	auto* eData = new EngineData;

	eData->bHardwareCursor = !ini.GetBoolValue("Game", "CursorFix");
	eData->fpsLimit = ini.GetLongValue("Game", "FPSLimit");
	eData->bVSync = ini.GetBoolValue("Game", "ForceVSync", true);
	eData->MSAA = ini.GetLongValue("Game", "ForceMSAA", 4);
	eData->Anisotropy = ini.GetLongValue("Game", "ForceAnisotropy", 16);
	eData->bNativeDX = ini.GetBoolValue("Game", "ForceNativeDX");
	eData->refreshRate = ini.GetLongValue("Game", "RefreshRate");

	eData->bDebugMode = ini.GetBoolValue("Misc", "DebugMode");

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
	/*
	lData->serverIP = new char[100];
	const char* _sip = ini.GetValue("Lobby", "ServerIP", "www.diesiedler2lobby.de:8777");
	strncpy_s(lData->serverIP, 100, _sip, 100);
	*/
	lData->serverIP = ini.GetValue("Lobby", "ServerIP", "www.diesiedler2lobby.de:8777");
	lData->patchlevel = ini.GetLongValue("Lobby", "PatchLevel", 9212);
	lData->bDebugMode = ini.GetBoolValue("Lobby", "DebugMode");

	return lData;
}


void setEngineData(char* iniPath, EngineData* eData) {
	CSimpleIni ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);

	ini.SetLongValue("Engine", "hardwareCursor", (long)eData->bHardwareCursor);
	ini.SetLongValue("Engine", "vsync", (long)eData->bVSync);
	ini.SetLongValue("Engine", "refreshRate", (long)eData->refreshRate);

	ini.SaveFile(iniPath);
}


void setNetworkData(char* iniPath, NetworkData* nData) {
	CSimpleIni ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);

	std::stringstream ss;
	ss << nData->serverAddr.IP << ":" << nData->serverAddr.Port;

	ini.SetLongValue("Basics", "gamePort", (long)nData->gamePort);
	ini.SetValue("Lobby", "url", ss.str().c_str());
	ini.SetLongValue("Lobby", "patchlevel", (long)nData->patchLevel);

	ini.SaveFile(iniPath);
}

void initDXconfig(char* path, EngineData* eData) {
	// remove in case it already exists
	remove(path);

	std::ofstream ofstr(path);

	ofstr << "d3d9.maxFrameRate = " << eData->refreshRate << std::endl;
	ofstr << "d3d9.forceSwapchainMSAA = " << eData->MSAA << std::endl;
	ofstr << "d3d9.samplerAnisotropy = " << eData->Anisotropy << std::endl;
	ofstr << "d3d9.presentInterval = " << (eData->bVSync ? 1 : 0) << std::endl;

	ofstr.close();
}
