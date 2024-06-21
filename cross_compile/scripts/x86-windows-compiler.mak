#i686-w64-mingw32-gcc
CC     = x86_64-w64-mingw32-gcc
CPP    = x86_64-w64-mingw32-g++
LD     = x86_64-w64-mingw32-gcc
AR     = x86_64-w64-mingw32-ar
RANLIB = x86_64-w64-mingw32-ranlib

TARGET_DEFS=-DWIN32
LDFLAGS =
LIBS    += -lmingw32 -lwsock32 -D__MINGW -static

