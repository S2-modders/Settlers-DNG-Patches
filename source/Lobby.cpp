/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include <Windows.h>
#include <stdlib.h>
#include <sstream>
#include <thread>
#include "Helper.h"
#include "Lobby.h"
#include "SimpleIni/SimpleIni.h"

STARTUPINFO si;
PROCESS_INFORMATION bridgeProcessInfo;

DWORD LoggerAddr1 = 0x7538;
DWORD LoggerAddr2 = 0x751E;

DWORD CreateGamePayloadPortHook = 0x60FA;

unsigned int bridgePort = 10001;

lobbyThreadData* lobbyData;

void createTCPBridge() {
	char iniPath[MAX_PATH];
	char exePath[MAX_PATH];

	showMessage("CreateGameServerPayload triggered!");

	GetCurrentDirectoryA(MAX_PATH, iniPath);

	strncpy(exePath, iniPath, MAX_PATH);
	strcat_s(iniPath, "\\bin\\frpc.ini");
	strcat_s(exePath, "\\bin\\frpc.exe");

	showMessage(iniPath);

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);

	const char* serverIP = ini.GetValue("common", "server_addr", "0.0.0.0");
	showMessage(serverIP);

	ini.SetLongValue("s2lobby", "remote_port", (long) bridgePort); // TODO request port from server
	ini.SaveFile(iniPath);

	showMessage("starting TCP bridge...");

	std::stringstream command;
	command << "\"" << exePath << "\" -c .\\bin\\frpc.ini";

	showMessage((int) bridgeProcessInfo.dwProcessId);

	if (bridgeProcessInfo.dwProcessId == 0) {
		showMessage("Creating new process...");

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);;
		ZeroMemory(&bridgeProcessInfo, sizeof(bridgeProcessInfo));
		
		CreateProcessA(NULL, (LPSTR)command.str().c_str(), NULL, NULL, false, 0, NULL, NULL, &si, &bridgeProcessInfo);
	} else {
		showMessage("Bridge seems to be already running...");
	}
	
	showMessage("End injected function");
}

DWORD jmpBackAddr;
DWORD portStrAddr = (DWORD)getMatchmakingAddress() + 0xA7EC;
void __declspec(naked) jumperFunction() {
	createTCPBridge();
	__asm {
		//mov edx, [eax+0x2C]
		mov edx, [bridgePort]
		push edx
		push [portStrAddr]
		jmp [jmpBackAddr]
	}
}

int LobbyPatch(lobbyThreadData* tData) {
	FILE* f;
	lobbyData = tData;

	if (tData->bDebugMode) {
		AllocConsole();
		freopen_s(&f, "CONOUT$", "w", stdout);
	}

	{
		/* patch lobby / tincat debug mode */
		if (tData->bDebugMode) {
			showMessage("Patching Lobby Debug mode");
			short lobbyDebug = 0x11EB;

			short* lobby1 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr1);
			short* lobby2 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr2);

			writeBytes(lobby1, &lobbyDebug, 2);
			writeBytes(lobby2, &lobbyDebug, 2);
		}
	}

	/* inject CreateGameServerPayload */
	int hookLength = 9;
	DWORD hookAddr = (DWORD)getMatchmakingAddress() + CreateGamePayloadPortHook;
	jmpBackAddr = hookAddr + hookLength;

	functionInjector((DWORD*)hookAddr, jumperFunction, hookLength);

	return 0;
};

DWORD WINAPI LobbyPatchThread(LPVOID param) {
	return LobbyPatch(reinterpret_cast<lobbyThreadData*>(param));
}
