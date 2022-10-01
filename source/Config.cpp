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
	eData->bVSync = ini.GetBoolValue("Game", "VSync", true);
	eData->refreshRate = ini.GetLongValue("Game", "RefreshRate");
	eData->bWindowed = ini.GetBoolValue("Game", "ForceWindowedMode");

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
	cData->bDebugMode = ini.GetBoolValue("Camera", "DebugMode");

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
