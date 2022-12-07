/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */
#pragma once
#define WIN32_LEAN_AND_MEAN

#include "Config.h"

#include "httplib/httplib.h"
#include "SimpleIni/SimpleIni.h"

#include <Windows.h>
#include <stdlib.h>
#include <sstream>
#include <thread>

extern PROCESS_INFORMATION bridgeProcessInfo;

DWORD WINAPI LobbyPatchThread(LPVOID param);
