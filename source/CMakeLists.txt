cmake_minimum_required(VERSION 3.10)
project(SettlersDNGPatches LANGUAGES CXX)

#add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DCMAKE_GENERATOR_PLATFORM=Win32)

add_subdirectory(utilities)

add_library(${PROJECT_NAME} SHARED
	d3d9.cpp
	Config.cpp
	Lobby.cpp
	MainPatch.cpp
	MainPatchASM.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE CppUtils)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/d3d9.def"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    
    OUTPUT_NAME "d3d9"
	SUFFIX ".dll"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
