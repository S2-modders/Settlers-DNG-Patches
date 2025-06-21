/*
 * Lobby patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */
#pragma once

extern PROCESS_INFORMATION bridgeProcessInfo;

DWORD WINAPI LobbyPatchThread(LPVOID param);

class LobbyPatch {
public:
    PatchSettings* settings;

    explicit LobbyPatch(PatchSettings* settings);

    int run();
private:
    void hookCreateGameServerPayload();
    void setTincatDebugMode();
    void patchLobbyFilter();
};
