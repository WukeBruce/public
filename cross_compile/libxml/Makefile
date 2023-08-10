##############################################################################
SUBDIRS=

##############################################################################
lib_dir=$(BUILD_DIR_NAME)
common_api_dir=common_api

SRC += $(shell ls ./$(common_api_dir)/*.c)
SRC += $(shell ls ./$(lib_dir)/*.c)
SRC += $(shell ls ./*.c)

#ifeq  ($(DVB_MEDIA_FLAG), yes)
#SRC += $(shell find ./media -iname "*.c")
#endif
#SRC += bsp.cpp

##############################################################################
#CFLAGS  += -I$(GXLIB_PATH)/include/gx_gui
LIB = $(lib_dir).a
BIN = out.elf

##############################################################################
CFLAGS += -I./include
CFLAGS += -I./include/$(common_api_dir)
CFLAGS += -I./$(lib_dir) 
CFLAGS += -I$(COMMON_LIB_PATH)/include/

##############################################################################
-include  $(COMMON_PRJ_PATH)/scripts/inc.Makefile
##############################################################################
LDFLAGS += -L$(COMMON_LIB_PATH)/lib

