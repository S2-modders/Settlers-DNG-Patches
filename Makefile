#files := $(wildcard */*.cpp)
#files += $(wildcard */*.c)

files := $(shell find source -type f -name "*.cpp")
files += $(shell find source -type f -name "*.c")
files += source/d3d9.def

CXX_FLAGS := -s -static -Os -Wl,-subsystem,windows -fPIC -shared -std=c++11

OUTPUT := build/d3d9.dll

dll:
	i686-w64-mingw32-g++ $(CXX_FLAGS) -o $(OUTPUT) $(files)

test:
	@echo $(CXX_FLAGS)
	@echo $(files)

