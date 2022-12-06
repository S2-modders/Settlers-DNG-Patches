cmake_minimum_required(VERSION 3.22)
project(d3d9)

set(CMAKE_CXX_STANDARD 11)


# see https://gist.github.com/peterspackman/8cf73f7f12ba270aa8192d6911972fe8
set(CMAKE_SYSTEM_NAME Windows)
#set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
set(TOOLCHAIN_PREFIX i686-w64-mingw32)

# cross compilers to use for C, C++ and Fortran
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_Fortran_COMPILER ${TOOLCHAIN_PREFIX}-gfortran)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# see https://www.transmissionzero.co.uk/computing/building-dlls-with-mingw/
set(SHARED_OPT "-fPIC -shared")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -static -Os -Wl,-subsystem,windows ${SHARED_OPT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -static -Os -Wl,-subsystem,windows ${SHARED_OPT}")

add_library(${PROJECT_NAME} SHARED
        source/d3d9.cpp source/d3d9.def
        source/Helper.cpp
        source/Lobby.cpp
        source/ZoomPatch.cpp
        source/SimpleIni/ConvertUTF.c)
