/*
 * Widescreen patch for The Settlers: 10th anniversary by zocker_160
 *
 * This source code is licensed under GPL-v3
 *
 */

#include <Logger.h>
#include <Helper.h>

#include "MainPatchASM.h"

namespace MainPatchASM_Logger {
    Logging::Logger logger("ASM");
}
using MainPatchASM_Logger::logger;


namespace DecryptPatch {

    struct Stringbla {
        DWORD unknown;
        char* filename;
        DWORD unknown2;
        DWORD unknown3;
        DWORD unknown4;
        int length;
        int bufferSize;
    };
    
    DWORD FilenamePtr = 0;
    DWORD FilebufferPtr = 0;
    DWORD FilebufferSizePtr = 0;
    DWORD ret3;
    
    static bool IsEncrypted = false;
    
    void lolol2() {
        Stringbla* ptr = (Stringbla*)FilenamePtr;
        char* filebuffer = *(char**)FilebufferPtr;
    
        IsEncrypted = strncmp(filebuffer + 0x4, "rc00", 4) == 0;
    
        logger.debug() << "-- loading: " 
            << (IsEncrypted ? "encrypted" : "raw")
            << " | "
            << ptr->filename 
            << std::endl;

#if 0
        logger.info(" -------- new file -------- ");
        logger.info() << "EBX content: \n"
            << "filename: " << ptr->filename << "\n"
            << "string length: " << ptr->length << "\n"
            << "string buffer size: " << ptr->bufferSize << "\n"
            << "first file bytes: " << **(int**)FilebufferPtr << "\n"
            << "file buffer size: " << *(int*)FilebufferSizePtr << "\n"
            << "file encrypted: ";
    
        if (IsEncrypted) {
            logger.naked("YES");
        }
        else {
            logger.naked("NO");
        }
    
        logger.naked() << std::endl;
#endif
    }
    
    void _declspec(naked) nakedFileLoadTest() {
        __asm {
            push eax
            
            mov eax, [esp + 0x28 + 0x4]
            mov [FilenamePtr], eax
    
            mov eax, [esp + 0x28 + 0x4 + 0x4]
            mov [FilebufferPtr], eax
    
            mov eax, [esp + 0x28 + 0x4 + 0x4 + 0x4]
            mov [FilebufferSizePtr], eax
    
            pop eax
    
            push ebx
            push ebp
            push esi
            mov [esp+0x2C], eax
    
            pushad
            call [lolol2]
            popad
        }
    
        if (IsEncrypted) {
            __asm {
                jmp [ret3]
            }
        }
        else {
            // cleanup and RET
            __asm {
                pop esi
                pop ebp
                pop ebx
    
                add esp, 0x24
                ret
            }
        }
    }
    
    void storeDecryptedData() {
        Stringbla* ptr = (Stringbla*)FilenamePtr;
        char* filebuffer = (char*)FilebufferPtr;
        int filebuffersize = (int)FilebufferSizePtr;
    
        const char suffix[9] = ".decrypt";
        int newStringSize = ptr->length + sizeof(suffix);
        
        char* newFilename = new char[newStringSize];
    
        strncpy_s(newFilename, newStringSize, ptr->filename, ptr->length);
        strncat_s(newFilename, newStringSize, suffix, sizeof(suffix));
    
        /*
        logger.debug() << "NEW FILENAME: " << newFilename << "\n"
            << "filebuffersize: " << filebuffersize
            << std::endl;
        */
    
        std::ofstream fileOutput;
        fileOutput.open(newFilename, std::ios::binary | std::ios::trunc);
        fileOutput.write(filebuffer, filebuffersize);
        fileOutput.close();
    
        delete[] newFilename;
    }
    
    DWORD ret4;
    void _declspec(naked) storeEncryptedData() {
        __asm {
            push edx
            push ecx
    
            mov edx, [esp+0x10+0x8]
            mov [FilebufferSizePtr], edx
    
            mov ecx, [esp+0x14+0x8]
            mov [FilebufferPtr], ecx
    
            pop ecx
            pop edx
    
            pushad
            call [storeDecryptedData]
            popad
    
            mov esi, [esp + 0x3C]
            mov eax, [esi]
    
            jmp [ret4]
        }
    }

    bool injectFileLoad(DWORD* fileloadFktAddr) {
        bool res = functionInjectorReturn(fileloadFktAddr, nakedFileLoadTest, ret3, 7);
    
        if (res) {
            logger.info("Data decrypt function inject");
        }
    
        return res;
    }
    
    bool injectFileStore(DWORD* fileloadEndFktAddr) {
        bool res = functionInjectorReturn(fileloadEndFktAddr, storeEncryptedData, ret4, 6);
    
        if (res) {
            logger.info("Data decrypt store function inject");
        }
    
        return res;
    }
}

namespace FogPatch {

    float fogStart = 1000;
    DWORD ret1;
    void _declspec(naked) jumperFogPatch1() {
        __asm {
            fld dword ptr [fogStart]
            fstp dword ptr [esi+0x6C]
            
            fstp st(0)
            fld st(0)
            jmp [ret1]
        }
    }
    float fogEnd = 1000;
    DWORD ret2;
    void _declspec(naked) jumperFogPatch2() {
        __asm {
            fld dword ptr [fogEnd]
            fstp dword ptr [esi+0x70]
    
            fstp st(0)
            fmul dword ptr [eax+0x40]
            jmp[ret2]
        }
    }

    bool injectJumperFog(DWORD* fogStart, DWORD* fogEnd) {
        bool res1 = functionInjectorReturn(fogStart, jumperFogPatch1, ret1, 5);
        bool res2 = functionInjectorReturn(fogEnd, jumperFogPatch2, ret2, 6);

        return res1 && res2;
    }

}
