/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include "Lobby.h"

namespace Lobby_Logger {
    Logging::Logger logger("LOBBY");
}
using Lobby_Logger::logger;


STARTUPINFO si;
PROCESS_INFORMATION bridgeProcessInfo;

DWORD LoggerAddr1 = 0x7538;
DWORD LoggerAddr2 = 0x751E;

DWORD CreateGamePayloadPortHook = 0x60FA;

const int retryCount = 10;
const int retryTimeout = 1000;

LobbyData* lobbyData;
unsigned int bridgePort;

void requestRemotePort(const char* ip, int controllerPort) {
    logger.debug("Requesting remote Port");

    std::stringstream url;
    url << "http://" << ip << ":" << controllerPort;

    httplib::Client cli(url.str());

    if (auto res = cli.Get("/api/request")) {
        if (res->status == 200) {
            bridgePort = std::atoi(res->body.c_str());

            logger.debug() << "port received: " << bridgePort << std::endl;
        } else {
            logger.error() << "API request failed with " << res->status << std::endl;
            bridgePort = 0;
        }
    }
}

void createTCPBridge() {
    logger.debug("CreateGameServerPayload triggered!");

    logger.debug() << "Bridge process ID: " << bridgeProcessInfo.dwProcessId << std::endl;
    if (bridgeProcessInfo.dwProcessId > 0) {
        logger.warn("Bridge seems to be already running...");
        return;
    }

    char iniPath[MAX_PATH];
    char exePath[MAX_PATH];

    GetCurrentDirectoryA(MAX_PATH, iniPath);

    strncpy_s(exePath, iniPath, MAX_PATH);
    strcat_s(iniPath, "\\bin\\frpc.ini");
    strcat_s(exePath, "\\bin\\frpc.exe");

    logger.debug() << "frpc ini path: " << iniPath << std::endl;

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(iniPath);

    const char* serverIP = ini.GetValue("common", "server_addr", "0.0.0.0");
    long controllerPort = ini.GetLongValue("common", "server_controller_port", 5480);

    logger.info() << "server IP: " << serverIP
        << ", controller port: " << controllerPort << std::endl;

    logger.debug("starting TCP bridge...");

    std::stringstream command;
    command << "\"" << exePath << "\" -c .\\bin\\frpc.ini";

    requestRemotePort(serverIP, controllerPort);

    if (bridgePort == 0) {
        logger.error("failed to get bridge port, aborting");
        return;
    }

    ini.SetLongValue("s2lobby", "remote_port", (long)bridgePort);
    ini.SaveFile(iniPath);

    logger.debug("Creating new process...");

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);;
    ZeroMemory(&bridgeProcessInfo, sizeof(bridgeProcessInfo));
        
    CreateProcessA(NULL, (LPSTR)command.str().c_str(), NULL, NULL, false, 0, NULL, NULL, &si, &bridgeProcessInfo);

    logger.debug("End injected function");
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

LobbyPatch::LobbyPatch(PatchSettings* settings) {
    this->settings = settings;
}

int LobbyPatch::run() {
    logger.info("LobbyPatch started");

    if (settings->lobbyData->bTincatDebug)
        setTincatDebugMode();

    //hookCreateGameServerPayload();
    //patchLobbyFilter();

    return 0;
}

void LobbyPatch::setTincatDebugMode() {
    logger.debug("Patching Lobby Debug mode");
    short lobbyDebug = 0x11EB;

    short* lobby1 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr1);
    short* lobby2 = (short*)((DWORD)getMatchmakingAddress() + LoggerAddr2);

    writeBytes(lobby1, &lobbyDebug, 2);
    writeBytes(lobby2, &lobbyDebug, 2);
}

void LobbyPatch::hookCreateGameServerPayload() {
    DWORD hookAddr = (DWORD)getMatchmakingAddress() + CreateGamePayloadPortHook;
    functionInjectorReturn((DWORD*)hookAddr, jumperFunction, jmpBackAddr, 9);

    /*
     * old inject CreateGameServerPayload
     * 
    int hookLength = 9;
    DWORD hookAddr = (DWORD)getMatchmakingAddress() + CreateGamePayloadPortHook;
    jmpBackAddr = hookAddr + hookLength;

    functionInjector((DWORD*)hookAddr, jumperFunction, hookLength);
    */
}


int prepareLobby(PatchSettings* settings) {
    Sleep(startupDelay+500);

    while (settings->gameVersion == V_UNKNOWN) {
        logger.info("Version check retrying...");
        Sleep(retryTimeout);
    }

    switch (settings->gameVersion) {
    case V_BASE_GOG:
        return LobbyPatch(settings).run();
        break;

    default:
        logger.info("Lobby patch disabled for this version");
        break;
    }

    return 0;
}

DWORD WINAPI LobbyPatchThread(LPVOID param) {
    return prepareLobby(reinterpret_cast<PatchSettings*>(param));
}
