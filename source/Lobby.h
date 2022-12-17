/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */
#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <sstream>
#include <thread>

#include "Config.h"
#include "Helper/Logger.h"

#include "httplib/httplib.h"
#include "SimpleIni/SimpleIni.h"


extern PROCESS_INFORMATION bridgeProcessInfo;

DWORD WINAPI LobbyPatchThread(LPVOID param);

class LobbyPatch {
public:
	explicit LobbyPatch(PatchSettings* settings);

	int run();
private:
	PatchSettings* settings;

	void hookCreateGameServerPayload();
	void setTincatDebugMode();
	void patchLobbyFilter();
};
