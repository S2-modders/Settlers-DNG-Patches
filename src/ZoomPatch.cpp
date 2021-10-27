/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
*/

#include <Windows.h>
#include <sstream>
#include "ZoomPatch.h"

/* memory values */
struct memoryPTR {
    DWORD base_address;
    int total_offsets;
    int offsets[];
};


memoryPTR MaxZoomPTR = {
    0x002BD4E8,
    2,
    { 0x4C, 0x1A8 }
};

memoryPTR CurrZoomPTR = {
    0x002BD4E8,
    2,
    { 0x4C, 0x1A4 }
};

memoryPTR WorldObjectPTR = {
    0x002BD4E8,
    1,
    { 0x4C }
};

DWORD GameVersionAddr = 0x2C5A30;

// reading and writing stuff / helper functions and other crap
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
void readBytes(void* read_addr, void* read_buffer, int len) {
    protectedRead(read_buffer, read_addr, len);
}
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
    //DWORD* location = (DWORD*)((DWORD)getBaseAddress() + patch->base_address);
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
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
void GetDesktopResolution2(int& hor, int& vert) {
    hor = GetSystemMetrics(SM_CXSCREEN);
    vert = GetSystemMetrics(SM_CYSCREEN);
}

void showMessage(float val) {
    std::stringstream ss;
    ss << "Debug: " << val;
    MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
}
void showMessage(int val) {
    std::stringstream ss;
    ss << "Debug: " << val;
    MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
}
void showMessage(LPCSTR val) {
    MessageBoxA(NULL, val, "ZoomPatch by zocker_160", MB_OK);
}

bool calcNewZoomValue(int& hor, int& vert, float& zoom_value) {
    GetDesktopResolution2(hor, vert);
    float aspr = calcAspectRatio(hor, vert);
    if (aspr > 0.0f && aspr <= 20.0f) {
        /* maxZoomValue will be set depending on the aspect ratio of the screen */

        if (aspr < 1.4f) {
            zoom_value = 4.0f;
            return true;
        }
        else if (aspr < 1.7f) {
            zoom_value = 5.0f;
            return true;
        }
        else if (aspr < 2.0f) {
            zoom_value = 6.0f;
            return true;
        }
        else if (aspr < 2.5f) {
            zoom_value = 7.0f;
            return true;
        }
        else if (aspr >= 2.5f) {
            zoom_value = 8.0f;
            return true;
        }
        else {
            return false;
        }

        return true;
    }
    else {
        return false;
    }
}

int MainEntry(bool debug) {
    /* wait a bit for the application to start up (might crash otherwise) */
    Sleep(5000);

    char versionGOG[15] = "Version: 11757"; // GOG version & patched CD version + NOCD
    char gameVersion[15];

    float* worldObj;
    float* maxZoom;

    int hor;
    int ver;
    float newZoomValue = 4.0f;

    /* check if WorldObject does exist */
    {
        float* tmp = (float*)calcAddress(WorldObjectPTR.base_address);

        if (*tmp < 0)
            return 0;
    }

    /* check if gameVersion is "11757" */
    {
        char* tversion = (char*)((DWORD)getBaseAddress() + GameVersionAddr);
        memcpy(gameVersion, tversion, 14);
        gameVersion[14] = 0;
        if (strcmp(gameVersion, versionGOG) != 0) {
            if (debug)
                showMessage("Game version not supported!");
            return 0;
        }
    }

    while (true) {
        worldObj = (float*)(tracePointer(&WorldObjectPTR));

        if (debug) {
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
                char* gameVersionSTR = (char*)((DWORD)getBaseAddress() + GameVersionAddr);
                showMessage(gameVersionSTR);
            }
            if (IsKeyPressed(VK_F3)) {
                int t_hor = 0;
                int t_ver = 0;
                GetDesktopResolution2(t_hor, t_ver);
                std::stringstream ss;
                ss << "xRes: " << t_hor << " yRes: " << t_ver;
                ss << " ASPR: " << calcAspectRatio(t_hor, t_ver);
                ss << " MaxZoomValue: " << newZoomValue;
                MessageBoxA(NULL, (LPCSTR)ss.str().c_str(), "ZoomPatch by zocker_160", MB_OK);
            }
        }

        /* Zoom hack */
        if (*worldObj != 0) {
            maxZoom = (float*)(tracePointer(&MaxZoomPTR));
            if (calcNewZoomValue(hor, ver, newZoomValue) && *maxZoom != newZoomValue)
                *maxZoom = newZoomValue;
        }

        Sleep(1000);
    }
    return 0;
}

DWORD WINAPI ZoomPatchThread(LPVOID param) {
    return MainEntry(false);
}
DWORD WINAPI ZoomPatchThreadDebug(LPVOID param) {
    return MainEntry(true);
}

// rename to "DllMain" if you want to use this directly
bool APIENTRY DllMain_alt(  HMODULE hModule,
                            DWORD  ul_reason_for_call,
                            LPVOID lpReserved
                          ) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //SetProcessDPIAware();
        CreateThread(0, 0, ZoomPatchThread, hModule, 0, 0);
        return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        return TRUE;
    }
    return TRUE;
}
