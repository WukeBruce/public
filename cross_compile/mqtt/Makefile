##############################################################################
SUBDIRS=

##############################################################################
lib_dir=$(BUILD_DIR_NAME)

#SRC += $(shell ls ./$(lib_dir)/magick/*.c)
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
CFLAGS += -I./include/openssl
CFLAGS += -I./$(lib_dir) 
CFLAGS += -I$(COMMON_LIB_PATH)/include/
#CFLAGS += -I./$(lib_dir)/www

##############################################################################
-include  $(COMMON_PRJ_PATH)/scripts/inc.Makefile
##############################################################################
#LDFLAGS += -L$(COMMON_LIB_PATH)/lib -ljpeg -lpng -lz -ldns_sd -lplist-2.0  -lcrypto -ldl
LDFLAGS += -L$(COMMON_LIB_PATH)/lib -lz -ldl -lssl -lcrypto -lssl -lcrypto -ldl 

