##############################################################################
SUBDIRS=$(BUILD_PRJ_NAME)
#SUBDIRS= work

##############################################################################
-include  $(COMMON_PRJ_PATH)/scripts/inc.Makefile
##############################################################################


ifeq ($(OS), ecos)
all:	subdirs
endif


ifeq ($(OS), linux)

#USERDIR=$(shell whoami)
USERDIR=gx
NFSROOT=/home/gx/nfs
all:subdirs

endif
