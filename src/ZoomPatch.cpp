/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include <Windows.h>
#include <sstream>
#include <iostream>
#include "ZoomPatch.h"

const int version_maj = 1;
const int version_min = 2;

/* memory values */

/* Base game */
memoryPTR MaxZoomPTR_base = {
    0x002BD4E8,
    2,
    { 0x4C, 0x1A8 }
};
memoryPTR CurrZoomPTR_base = {
    0x002BD4E8,
    2,
    { 0x4C, 0x1A4 }
};
memoryPTR WorldObjectPTR_base = {
    0x002BD4E8,
    1,
    { 0x4C }
};

/* Wikinger Addon */
memoryPTR MaxZoomPTR_addon = {
    0x002CA528,
    2,
    { 0x4C, 0x1BC }
};
memoryPTR CurrZoomPTR_addon = {
    0x002CA528,
    2,
    { 0x4C, 0x1B8 }
};
memoryPTR WorldObjectPTR_addon = {
    0x002CA528,
    1,
    { 0x4C }
};

DWORD BaseGameVersionAddr = 0x2C5A30;
DWORD AddonGameVersionAddr = 0x2D2DB8;

// reading and writing stuff / helper functions and other crap

/* update memory protection and read with memcpy */
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
/* read from address into read buffer of length len */
void readBytes(void* read_addr, void* read_buffer, int len) {
    protectedRead(read_buffer, read_addr, len);
}
/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedRead(dest_addr, patch, len);
}

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}
DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}
DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int i = 0; i < patch->total_offsets; i++) {
        location = (DWORD*)(*location + patch->offsets[i]);
    }
    return location;
}

float calcAspectRatio(int horizontal, int vertical) {
    if (horizontal != 0 && vertical != 0) {
        return (float)horizontal / (float)vertical;
    }
    else {
        return -1.0f;
    }
}

/* other helper functions and stuff */
bool IsKeyPressed(int vKey) {
    /* some bitmask trickery because why not */
    return GetAsyncKeyState(vKey) & 0x8000;
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
void GetDesktopResolution2(int& hor, int& vert) {
    hor = GetSystemMetrics(SM_CXSCREEN);
    vert = GetSystemMetrics(SM_CYSCREEN);
}

void showMessage(float val) {
    std::cout << "DEBUG: " << val << "\n";
    return;
    std::stringstream ss;
    ss << "Debug: " << val;
    MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
}
void showMessage(int val) {
    std::cout << "DEBUG: " << val << "\n";
    return;
    std::stringstream ss;
    ss << "Debug: " << val;
    MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
}
void showMessage(LPCSTR val) {
    std::cout << "DEBUG: " << val << "\n";
    //MessageBoxA(NULL, val, "ZoomPatch by zocker_160", MB_OK);
}
void startupMessage() {
    std::cout << "ZoomPatch by zocker_160 - Version: v" << version_maj << "." << version_min << "\n";
    std::cout << "Debug mode enabled!\n";
    std::cout << "Waiting for application startup...\n";
}

bool checkSupport(char* versionString) {
    char versionBase[15] = "Version: 11757"; // GOG version & patched CD version + NOCD
    char versionAddon[15] = "Version: 11758"; // Wikings Addon + NOCD
    char gameVersion[15];

    memcpy(gameVersion, versionString, 14);
    gameVersion[14] = 0;

    showMessage(gameVersion);

    if (strcmp(gameVersion, versionBase) != 0 && strcmp(gameVersion, versionAddon) != 0)
        return false;
    else
        return true;
}

bool calcNewZoomValue(int& hor, int& vert, float& zoom_value, bool wideview) {
    GetDesktopResolution2(hor, vert);
    float aspr = calcAspectRatio(hor, vert);
    if (aspr > 0.0f && aspr <= 20.0f) {
        /* maxZoomValue will be set depending on the Aspect Ratio of the screen */
        if (wideview) {
            if (aspr < 1.5f) {
                zoom_value = 6.0f;
                return true;
            }
            else if (aspr < 1.9f) {
                zoom_value = 7.0f;
                return true;
            }
            else if (aspr < 2.6f) {
                zoom_value = 8.0f;
                return true;
            }
            else if (aspr >= 2.6f) {
                zoom_value = 9.0f;
                return true;
            }
            else {
                return false;
            }
        }
        else {
            if (aspr < 1.5f) {
                zoom_value = 4.0f;
                return true;
            }
            else if (aspr < 1.9f) {
                zoom_value = 5.0f;
                return true;
            }
            else if (aspr < 2.2f) {
                zoom_value = 6.0f;
                return true;
            }
            else if (aspr < 2.6f) {
                zoom_value = 7.0f;
                return true;
            }
            else if (aspr >= 2.6f) {
                zoom_value = 7.0f;
                return true;
            }
            else {
                return false;
            }
        }

        return true;
    }
    else {
        return false;
    }
}

int MainLoop(memoryPTR& WorldObjectPTR,
    memoryPTR& MaxZoomPTR,
    memoryPTR& CurrZoomPTR,
    threadData* tData
    ) {
    float* worldObj;
    float* maxZoom;

    int hor;
    int ver;
    float newZoomValue = 4.0f; // 4 is the default zoom value
    float* zoomStep_p = &tData->ZoomIncrement;

    memoryPTR zoomIncr = {
        0x20D00E + 0x02,
        0,
        { 0x0 }
    };
    memoryPTR zoomDecr = {
        0x20CFE4 + 0x02,
        0,
        { 0x0 }
    };

    /*
    {
        DWORD tmp[4];
        readBytes(tracePointer(&zoomIncr), tmp, 4);
        std::cout << "ZoomStep " << tData->ZoomIncrement << "\n";
        std::cout << "ZoomStep " << &tData->ZoomIncrement << "\n";
        writeBytes(tracePointer(&zoomIncr), &zoomStep_p, 4);
        writeBytes(tracePointer(&zoomDecr), &zoomStep_p, 4);
    }
    */

    /* check if WorldObject does exist */
    {
        float* tmp = (float*)calcAddress(WorldObjectPTR.base_address);
        if (*tmp < 0)
            return 0;
    }

    while (true) {
        worldObj = (float*)(tracePointer(&WorldObjectPTR));

        if (tData->bDebugMode) {
            if (IsKeyPressed(VK_F3)) {
                int t_hor = 0;
                int t_ver = 0;
                GetDesktopResolution2(t_hor, t_ver);
                std::stringstream ss;
                ss << "DEBUG:  xRes: " << t_hor << " yRes: " << t_ver;
                ss << " ASPR: " << calcAspectRatio(t_hor, t_ver);
                ss << " MaxZoomValue: " << newZoomValue;
                std::cout << ss.str() << "\n";
                //MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
            }
            if (IsKeyPressed(VK_F6)) {
                if (*worldObj != 0) {
                    //*(float*)(tracePointer(&CurrZoomPTR)) += 1.0f;
                    showMessage(*(float*)(tracePointer(&MaxZoomPTR)));
                }
            }
            if (IsKeyPressed(VK_F7)) {
                showMessage(*(float*)(tracePointer(&CurrZoomPTR)));
            }
            if (IsKeyPressed(VK_F8)) {
                std::cout << "World address: " << worldObj << "\n";
            }
        }

        /* Zoom hack */
        if (*worldObj != 0) {
            maxZoom = (float*)(tracePointer(&MaxZoomPTR));
            if (calcNewZoomValue(hor, ver, newZoomValue, tData->bWideView) && *maxZoom != newZoomValue) {
                if (tData->bWideView)
                    showMessage("WideViewMode enabled");
                else
                    showMessage("WideViewMode disabled");
                *maxZoom = newZoomValue;
            }
        }

        Sleep(1000);
    }
}

int MainEntry(threadData* tData) {
    Sleep(1000);
    FILE* f;

    if (tData->bDebugMode) {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        startupMessage();
    }
    /* wait a bit for the application to start up (might crash otherwise) */
    Sleep(4000);

    /* check if gameVersion is supported */
    char* sBase = (char*)((DWORD)getBaseAddress() + BaseGameVersionAddr);
    char* sAddon = (char*)((DWORD)getBaseAddress() + AddonGameVersionAddr);

    if (checkSupport(sBase)) {
        showMessage("Found Base version.");
        return MainLoop(WorldObjectPTR_base, MaxZoomPTR_base, CurrZoomPTR_base, tData);
    }
    else if (checkSupport(sAddon)) {
        showMessage("Found Addon version.");
        return MainLoop(WorldObjectPTR_addon, MaxZoomPTR_addon, CurrZoomPTR_addon, tData);
    } else {
        showMessage("Game version not supported!");
        return 0;
    }
    return 0;
}

DWORD WINAPI ZoomPatchThread(LPVOID param) {
    return MainEntry(reinterpret_cast<threadData*>(param));
}

// rename to "DllMain" if you want to use this
bool APIENTRY DllMain_alt(  HMODULE hModule,
                            DWORD  ul_reason_for_call,
                            LPVOID lpReserved
                          ) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //SetProcessDPIAware();
        CreateThread(0, 0, ZoomPatchThread, hModule, 0, 0);
        return 0;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }
    return 0;
}
