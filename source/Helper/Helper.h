#pragma once

#include <Windows.h>
#include <vector>
#include <sstream>
#include <iostream>

struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

void protectedRead(void* dest, void* src, int n);
bool readBytes(void* read_addr, void* read_buffer, int len);
void writeBytes(void* dest_addr, void* patch, int len);

HMODULE getBaseAddress();
HMODULE getMatchmakingAddress();

DWORD* calcAddress(DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);

void nopper(void* startAddr, int len);
bool functionInjector(void* toHook, void* function, int len);
bool functionInjectorReturn(void* hookAddr, void* function, DWORD& returnAddr, int len);

void getDesktopResolution(int& horizontal, int& vertical);
void getDesktopResolution2(int& hor, int& vert);

int getDesktopRefreshRate();

void getGameDirectory(HMODULE hm, char* path, int size, char* location, int levels = 0);

bool isKeyPressed(int vKey);

bool isWine();
bool isVulkanSupported();

float calcAspectRatio();
float calcAspectRatio(int horizontal, int vertical);
