/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 * 
 * This source code is licensed under GPL-v3
 * 
 */
#pragma once
#include <Windows.h>

const int version_maj = 1;
const int version_min = 6;

const int retryCount = 4;
const int retryTimeout = 2000;

DWORD WINAPI ZoomPatchThread(LPVOID param);
