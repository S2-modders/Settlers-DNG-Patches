/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include "Helper.h"

 // reading and writing stuff / helper functions and other crap

 /* update memory protection and read with memcpy */
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
/* read from address into read buffer of length len */
bool readBytes(void* read_addr, void* read_buffer, int len) {
    // compile with "/EHa" to make this work
    // see https://stackoverflow.com/questions/16612444/catch-a-memory-access-violation-in-c
    try {
        protectedRead(read_buffer, read_addr, len);
        return true;
    }
    catch (...) {
        return false;
    }
}
/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedRead(dest_addr, patch, len);
}

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}
HMODULE getMatchmakingAddress() {
    return GetModuleHandle("matchmaking.dll");
}

DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}
DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int n : patch->offsets) {
        location = (DWORD*)(*location + n);
    }

    return location;
}

void nopper(void* startAddr, int len) {
    BYTE nop = 0x90;
    for (int i = 0; i < len; i++)
        writeBytes((DWORD*)((DWORD)startAddr + i), &nop, 1);
}

bool functionInjector(void* hookAddr, void* function, int len) {
    if (len < 5)
        return false;

    BYTE jmp = 0xE9;
    DWORD relAddr = ((DWORD)function - (DWORD)hookAddr) - 5;

    /* NOP needed area */
    nopper(hookAddr, len);

    writeBytes(hookAddr, &jmp, 1);
    writeBytes((DWORD*)((DWORD)hookAddr + 1), &relAddr, 4);

    return true;
}
bool functionInjectorReturn(void* hookAddr, void* function, DWORD& returnAddr, int len) {
    if (len < 5)
        return false;

    BYTE jmp = 0xE9;
    DWORD relAddr = ((DWORD)function - (DWORD)hookAddr) - 5;
    returnAddr = (DWORD)hookAddr + len;

    nopper(hookAddr, len);

    writeBytes(hookAddr, &jmp, 1);
    writeBytes((DWORD*)((DWORD)hookAddr + 1), &relAddr, 4);

    return true;
}

/* logging stuff */
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
void showMessage(short val) {
    std::cout << "DEBUG: " << val << "\n";
    return;
}
void showMessage(char* val) {
    std::cout << "DEBUG: " << val << "\n";
}
void showMessage(LPCSTR val) {
    std::cout << "DEBUG: " << val << "\n";
    //MessageBoxA(NULL, val, "ZoomPatch by zocker_160", MB_OK);
}

/* resolution stuff */

void getDesktopResolution(int& horizontal, int& vertical)
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
void getDesktopResolution2(int& hor, int& vert) {
    hor = GetSystemMetrics(SM_CXSCREEN);
    vert = GetSystemMetrics(SM_CYSCREEN);
}

int getDesktopRefreshRate() {
    DEVMODE lpDevMode;
    memset(&lpDevMode, 0, sizeof(lpDevMode));
    lpDevMode.dmSize = sizeof(DEVMODE);
    lpDevMode.dmDriverExtra = 0;

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
        return lpDevMode.dmDisplayFrequency;
    else
        return 0;
}

float calcAspectRatio(int horizontal, int vertical) {
    if (horizontal > 0 && vertical > 0)
        return (float)horizontal / (float)vertical;
    else
        return -1.0f;
}

float calcAspectRatio() {
    int horizontal, vertical;

    getDesktopResolution2(horizontal, vertical);
    return calcAspectRatio(horizontal, vertical);
}

/* other helper functions and stuff */
bool isKeyPressed(int vKey) {
    /* some bitmask trickery because why not */
    return GetAsyncKeyState(vKey) & 0x8000;
}

bool isWine() {
    HMODULE ntdllMod = GetModuleHandle("ntdll.dll");

    return ntdllMod && GetProcAddress(ntdllMod, "wine_get_version");
}

bool isVulkanSupported() {
    HMODULE vk = LoadLibraryA("vulkan-1.dll");
    if (!vk)
        return false; // Vulkan drivers not installed

    typedef VOID*(WINAPI* VK_GETINSTANCE_PROC)(void*, const char*);
    typedef INT(WINAPI* VK_CREATE_INSTANCE)(int*, void*, void**);

    auto vkGetInstanceProcAddr = (VK_GETINSTANCE_PROC)GetProcAddress(vk, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr)
        return false; // Vulkan is malfunctioning (vkGetInstanceProcAddr)

    auto vkCreateInstance = (VK_CREATE_INSTANCE)vkGetInstanceProcAddr(0, "vkCreateInstance");
    if (!vkCreateInstance)
        return false; // Vulkan is malfunctioning (vkCreateInstance)

    void* instance = 0;
    int dummyInt[16] = { 1 };
    int result = vkCreateInstance(dummyInt, 0, &instance);

    if (!instance || result != 0)
        return false; // Vulkan drivers installed and functioning, but are incompatible. An instance could not be created

    return true;
}

void getGameDirectory(HMODULE hm, char* path, int size, char* loc, int levels) {
    GetModuleFileName(hm, path, size);

    for (int i = 0; i <= levels; i++) {
        *strrchr(path, '\\') = '\0';
    }

    strcat_s(path, size, loc);
}
