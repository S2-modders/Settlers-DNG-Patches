/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */ 
#define WIN32_LEAN_AND_MEAN

// this is here correctly, DO NOT TOUCH
#include "utilities/httplib/httplib.h"

#include <Windows.h>

#include <sstream>
#include <thread>

#include "Config.h"
#include "Lobby.h"

#include "utilities/Helper/Helper.h"
#include "utilities/Helper/Logger.h"
#include "utilities/SimpleIni/SimpleIni.h"

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


HMODULE getMatchmakingAddress() {
    return getModuleAddress("matchmaking.dll");
}

bool checkPortForward(std::string& hostIP, const char* serverIP, int controllerPort) {
    std::stringstream url;
    url << "http://" << serverIP << ":" << controllerPort;

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

bool requestNetworkBridge(unsigned int& hostPort, unsigned int& clientPort, const char* serverIP, int controllerPort) {
    std::stringstream url;
    url << "http://" << serverIP << ":" << controllerPort;

    httplib::Client httpClient(url.str());
    std::string hostClientPair;

    logger.info() << "Requesting network bridge - ";
    if (auto res = httpClient.Get("/request/bridge")) {
        switch (res->status) {
        case 200:
            logger.naked("ok");
            hostClientPair = res->body;
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
        logger.error("Failed to connect to Controller server");
        return false;
    }

    auto nPos = hostClientPair.find(":");
    std::string host = hostClientPair.substr(0, nPos);
    std::string client = hostClientPair.substr(nPos+1);

    hostPort = std::atoi(host.c_str());
    clientPort = std::atoi(client.c_str());

    logger.debug() << "HostPort: " << hostPort << " ClientPort: " << clientPort << std::endl;

    // TODO create network bridge

    return true;
}

/*
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
*/


std::string hostIP; // public IP of this client
unsigned int hostPort = 0;
unsigned int clientPort = 0;

DWORD jmpBackAddr;
DWORD portStrAddr = (DWORD)getMatchmakingAddress() + 0xA7EC;

void createNetBridge() {
    logger.debug("CreateGameServerPayload triggered");

    auto serverIP = lobbyData->serverAddr.IP;
    auto serverPort = lobbyData->apiPort;

    if (checkPortForward(hostIP, serverIP, serverPort)) {
        clientPort = lobbyData->gamePort;
        return;
    }

    if ( ! requestNetworkBridge(hostPort, clientPort, serverIP, serverPort)) {
        clientPort = 9999; // misused as error code for the lobby server
    }
}
void __declspec(naked) jumperFunction() {
    __asm {
        pushad
    }
    createNetBridge();
    __asm {
        popad

        //mov edx, [eax+0x2C]
        mov edx, [clientPort]
        push edx
        push [portStrAddr]
        jmp [jmpBackAddr]
    }
}

LobbyPatch::LobbyPatch(PatchSettings* settings) {
    this->settings = settings;

    lobbyData = settings->lobbyData;
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
