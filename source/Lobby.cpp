/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <sstream>
#include <thread>
#include "Helper.h"
#include "Lobby.h"
#include "SimpleIni/SimpleIni.h"
#include "httplib/httplib.h"

STARTUPINFO si;
PROCESS_INFORMATION bridgeProcessInfo;

DWORD LoggerAddr1 = 0x7538;
DWORD LoggerAddr2 = 0x751E;

DWORD CreateGamePayloadPortHook = 0x60FA;

lobbyThreadData* lobbyData;
unsigned int bridgePort;

void requestRemotePort(const char* ip, int controllerPort) {
	showMessage("Requesting remote Port");

	std::stringstream url;
	url << "http://" << ip << ":" << controllerPort;

	httplib::Client cli(url.str());

	if (auto res = cli.Get("/api/request")) {
		if (res->status == 200) {
			bridgePort = std::atoi(res->body.c_str());

			showMessage("port received:");
			showMessage((int)bridgePort);

		} else {
			showMessage("fail");
			showMessage(res->status);
			bridgePort = 0;
		}
	}
}

void createTCPBridge() {
	showMessage("CreateGameServerPayload triggered!");

	showMessage((int)bridgeProcessInfo.dwProcessId);
	if (bridgeProcessInfo.dwProcessId > 0) {
		showMessage("Bridge seems to be already running...");
		return;
	}

	char iniPath[MAX_PATH];
	char exePath[MAX_PATH];

	GetCurrentDirectoryA(MAX_PATH, iniPath);

	strncpy(exePath, iniPath, MAX_PATH);
	strcat_s(iniPath, "\\bin\\frpc.ini");
	strcat_s(exePath, "\\bin\\frpc.exe");

	showMessage(iniPath);

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);

	const char* serverIP = ini.GetValue("common", "server_addr", "0.0.0.0");
	long controllerPort = ini.GetLongValue("common", "server_controller_port", 5480);
	showMessage(serverIP);
	showMessage(controllerPort);

	showMessage("starting TCP bridge...");

	std::stringstream command;
	command << "\"" << exePath << "\" -c .\\bin\\frpc.ini";

	requestRemotePort(serverIP, controllerPort);

	if (bridgePort == 0) {
		showMessage("failed to get bridge port, aborting");
		return;
	}

	ini.SetLongValue("s2lobby", "remote_port", (long)bridgePort);
	ini.SaveFile(iniPath);

	showMessage("Creating new process...");

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);;
	ZeroMemory(&bridgeProcessInfo, sizeof(bridgeProcessInfo));
		
	CreateProcessA(NULL, (LPSTR)command.str().c_str(), NULL, NULL, false, 0, NULL, NULL, &si, &bridgeProcessInfo);

	
	showMessage("End injected function");
}

// uhh https://gcc.gnu.org/onlinedocs/gcc/x86-Function-Attributes.html

DWORD jmpBackAddr;
DWORD portStrAddr = (DWORD)getMatchmakingAddress() + 0xA7EC;
void __declspec(naked) jumperFunction() {
	//createTCPBridge();
	__asm (
		//mov edx, [eax+0x2C]
		"mov edx, [bridgePort];"
        "push edx;"
        "push [portStrAddr];"
        "jmp [jmpBackAddr];"
    );
}

void setTincatDebugMode() {
	showMessage("Patching Lobby Debug mode");
	short lobbyDebug = 0x11EB;

	short* lobby1 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr1);
	short* lobby2 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr2);

	writeBytes(lobby1, &lobbyDebug, 2);
	writeBytes(lobby2, &lobbyDebug, 2);
}

void setNetworking(lobbyThreadData* tData) {
	showMessage("Patching network.ini");
	char iniPath[MAX_PATH];

	GetCurrentDirectoryA(MAX_PATH, iniPath);
	strcat_s(iniPath, "\\data\\settings\\network.ini");

	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);
	ini.SetValue("Lobby", "url", (const char*)tData->serverIP);
	ini.SetLongValue("Lobby", "patchlevel", (long)tData->patchlevel);
	ini.SaveFile(iniPath);
}

int LobbyPatch(lobbyThreadData* tData) {
	Sleep(500);
	FILE* f;
	lobbyData = tData;

	if (tData->bDebugMode) {
		AllocConsole();
		freopen_s(&f, "CONOUT$", "w", stdout);
	}

	/* patch network.ini */
	if (tData->bNetworkPatch)
		setNetworking(tData);

	/* patch lobby / tincat debug mode */
	if (tData->bDebugMode)
		setTincatDebugMode();

	/* inject CreateGameServerPayload */
	{
		int hookLength = 9;
		DWORD hookAddr = (DWORD)getMatchmakingAddress() + CreateGamePayloadPortHook;
		jmpBackAddr = hookAddr + hookLength;

		functionInjector((DWORD*)hookAddr, (void*)jumperFunction, hookLength);
	}

	return 0;
};

DWORD WINAPI LobbyPatchThread(LPVOID param) {
	return LobbyPatch(reinterpret_cast<lobbyThreadData*>(param));
}
