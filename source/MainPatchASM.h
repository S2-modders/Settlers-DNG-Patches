#pragma once

#include "pch.h"

namespace DecryptPatch {
    bool injectFileLoad(DWORD* fileloadFktAddr);
    bool injectFileStore(DWORD* fileloadEndFktAddr);
}

namespace FogPatch {
    bool injectJumperFog(DWORD* fogStart, DWORD* fogEnd);
}
