/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */ 

#include "pch.h"

// this is here correctly, DO NOT TOUCH
#include <httplib.h>

#include <Helper.h>
#include <Logger.h>
#include <SimpleIni.h>

#include "Config.h"
#include "Lobby.h"


namespace Lobby_Logger {
    Logging::Logger logger("LOBBY");
}
using Lobby_Logger::logger;

DWORD LoggerAddr1 = 0x7538;
DWORD LoggerAddr2 = 0x751E;

DWORD CreateGamePayloadPortHook = 0x60FA;

const int retryCount = 10;
const int retryTimeout = 1000;

LobbyData* lobbyData;

LPSTARTUPINFOA si;
PROCESS_INFORMATION bridgeProcessInfo;


HMODULE getMatchmakingAddress() {
    return getModuleAddress("matchmaking.dll");
}

bool checkPortForward(std::string& hostIP, const char* serverIP, int apiPort) {
    std::stringstream url;
    url << "http://" << serverIP << ":" << apiPort;

    httplib::Client client(url.str());

    logger.info() << "Port forward check: ";
    if (auto res = client.Get("/port/check")) {
        switch (res->status) {
        case 200:
            logger.naked("ok");
            hostIP = std::string(res->body);
            return true;
            break;
        case 900:
            logger.naked("no forward");
            hostIP = std::string(serverIP);
            return false;
            break;
        default:
            logger.naked() << "failed with: " << res->status << std::endl;
            return false;
        }
    }
    else {
        logger.error("Failed to connect to Controller server");
        return false;
    }
}

bool requestControllerPort(unsigned int& port, const char* serverIP, int apiPort) {
    std::stringstream url;
    url << "http://" << serverIP << ":" << apiPort;

    httplib::Client httpClient(url.str());
    if (auto res = httpClient.Get("/port/controller")) {
        if (res->status == 200) {
            port = std::atoi(res->body.c_str());
            logger.debug() << "Using controller port: " << port << std::endl;
            return true;
        }
    }

    logger.error("Failed to request controller port");
    return false;
}

bool requestNetworkBridge(unsigned int& hostPort, const char* serverIP, int apiPort) {
    std::stringstream url;
    url << "http://" << serverIP << ":" << apiPort;

    httplib::Client httpClient(url.str());

    logger.info() << "Requesting host port - ";
    if (auto res = httpClient.Get("/request/port")) {
        switch (res->status) {
        case 200:
            hostPort = std::atoi(res->body.c_str());
            logger.naked() << "ok (" << hostPort << ")" << std::endl;
            break;
        case 500:
            logger.naked() << "failed with: " << res->body << std::endl;
            return false;
            break;
        default:
            logger.naked() << "failed with: " << res->status << std::endl;
            return false;
        }
    }
    else {
        logger.naked() << "Failed to connect to Controller server";
        return false;
    }

    if (bridgeProcessInfo.dwProcessId > 0) {
        logger.debug() << "Bridge process seems to be running: " << bridgeProcessInfo.dwProcessId << std::endl;

        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, bridgeProcessInfo.dwProcessId);
        if (hProcess != NULL) {
            if (!TerminateProcess(hProcess, 0))
                logger.error("TerminateProcess failed");
            else
                logger.debug("Process stopped");

            CloseHandle(hProcess);
        }
        else {
            logger.error("hProcess is NULL");
        }
    }

    unsigned int controllerPort;
    if (!requestControllerPort(controllerPort, serverIP, apiPort)) {
        return false;
    }

    char exePath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, exePath);
    strcat_s(exePath, "\\bin\\tincat3bridge.dll");
    logger.debug() << "bridge DLL: " << exePath << std::endl;

    std::stringstream command;
    command << "\"" << exePath << "\" tcp"
        << " -l " << lobbyData->gamePort
        << " -r " << hostPort
        << " -s " << serverIP
        << " -P " << controllerPort
        << " -u " << serverIP << ":" << hostPort
        << " --tls_enable false";

    logger.info() << "Starting bridge process (" << serverIP << ":" << hostPort << ")" << std::endl;

    ZeroMemory(si, sizeof(si));
    si->cb = sizeof(si);
    ZeroMemory(&bridgeProcessInfo, sizeof(bridgeProcessInfo));

    CreateProcessA(NULL, (LPSTR)command.str().c_str(), NULL, NULL, false, 0, NULL, NULL, si, &bridgeProcessInfo);

    logger.debug("End injected function");
    return true;
}


std::string hostIP; // public IP of this client
unsigned int hostPort = 0;

DWORD jmpBackAddr;
DWORD portStrAddr = (DWORD)getMatchmakingAddress() + 0xA7EC;

void createNetBridge() {
    logger.debug("CreateGameServerPayload triggered");

    auto serverIP = lobbyData->serverAddr.IP;
    auto serverPort = lobbyData->apiPort;

    if (checkPortForward(hostIP, serverIP, serverPort)) {
        hostPort = lobbyData->gamePort;
        return;
    }

    if ( ! requestNetworkBridge(hostPort, serverIP, serverPort)) {
        hostPort = 9999; // misused as error code for the lobby server
    }
}
void __declspec(naked) jumperFunction() {
    __asm {
        pushad
        call [createNetBridge]
        popad

        //mov edx, [eax+0x2C]
        mov edx, [hostPort]
        push edx
        push [portStrAddr]
        jmp [jmpBackAddr]
    }
}

LobbyPatch::LobbyPatch(PatchSettings* settings) {
    this->settings = settings;

    lobbyData = this->settings->lobbyData;
}

int LobbyPatch::run() {
    logger.info("LobbyPatch started");

//    if (settings->lobbyData->bTincatDebug)
//        setTincatDebugMode();

    hookCreateGameServerPayload();
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
    logger.debug("Installing hook for ServerPayload");
    DWORD hookAddr = (DWORD)getMatchmakingAddress() + CreateGamePayloadPortHook;
    functionInjectorReturn((DWORD*)hookAddr, jumperFunction, jmpBackAddr, 9);
}


int prepareLobby(PatchSettings* settings) {
    Sleep(startupDelay+500);

    while (settings->gameVersion == V_UNKNOWN) {
        logger.info("Version check retrying...");
        Sleep(retryTimeout);
    }

    switch (settings->gameVersion) {
    case V_BASE_GOG:
    case V_BASE_GOLD:
        return LobbyPatch(settings).run();
        break;

    default:
        logger.info("Lobby patch not supported for this version");
        break;
    }

    return 0;
}

DWORD WINAPI LobbyPatchThread(LPVOID param) {
    return prepareLobby(reinterpret_cast<PatchSettings*>(param));
}
