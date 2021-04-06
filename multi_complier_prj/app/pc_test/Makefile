##############################################################################
SUBDIRS=

##############################################################################

SRC += $(shell ls ./sdl2/*.c)
SRC += $(shell find ./com_private -iname "*.c")
SRC += $(shell ls *.c)

#ifeq  ($(DVB_MEDIA_FLAG), yes)
#SRC += $(shell find ./media -iname "*.c")
#endif
#SRC += bsp.cpp

##############################################################################
LIB = libapp.a
BIN = out.elf

##############################################################################
CFLAGS += -I./include 
CFLAGS += -I./sdl2/include 
CFLAGS += -I./com_private/app_list/include 
CFLAGS += -I./com_private/include 
CFLAGS += -I$(COMMON_LIB_PATH)/SDL2/include/SDL2
CFLAGS += -I$(COMMON_LIB_PATH)/SDL2/ffmepg/include
CFLAGS += -I$(COMMON_LIB_PATH)/SDL2/ffmepg/include/libavutil

#libavcodec  libavdevice  libavfilter  libavformat  libavutil  libswresample  libswscale
#-lavcodec -lavformat -lavutil -lswscale -lavdevice -lavfilter -lswresample
#-lavcodec -lavformat -lavutil -lswscale
#-lswscale -lavutil -lavformat -lavcodec 

#CFLAGS  += -I$(GXLIB_PATH)/include/gx_gui
CFLAGS += -g -O2 -D_REENTRANT -I$(COMMON_LIB_PATH)/SDL2/include/SDL2   -DHAVE_OPENGL -g
##############################################################################
-include  $(COMMON_PRJ_PATH)/scripts/inc.Makefile
##############################################################################
LDFLAGS += -L /usr/linclude -lz
LDFLAGS += -L$(COMMON_LIB_PATH)/SDL2/lib
LDFLAGS += -L$(COMMON_LIB_PATH)/ffmepg/lib
LDFLAGS += -lreadline

ifeq ($(OS), linux)
#LDFLAGS	+= -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL2 -lpthread -Wl,--no-undefined -lm -ldl -lpthread -lrt
LDFLAGS	+= -L$(COMMON_LIB_PATH)/SDL2/lib -Wl,-rpath,$(COMMON_LIB_PATH)/SDL2/lib -lSDL2 -lavcodec -lavformat -lavutil -lswscale -lswresample -lavutil -lavformat -lavcodec -lz -llzma -lpthread -Wl,--no-undefined -lm -ldl -lpthread -lrt
endif


#	@echo $(LDFLAGS)
#test: 
#	@echo making test begin...	
#	$(CC)  main.c  -o test \
#	$(CFLAGS)                       \
#	-L$(GXLIB_PATH)/lib -lservice -L./ -lapp $(LDFLAGS) -lui   
