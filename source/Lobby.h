/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#pragma once
#include <Windows.h>

struct LobbyData {
	bool bEnabled;
	bool bTincatDebug;
	bool bNetworkPatch;
	bool bDebugMode;
	const char* serverIP;
	int patchlevel;
};

extern PROCESS_INFORMATION bridgeProcessInfo;

DWORD WINAPI LobbyPatchThread(LPVOID param);
