
-include $(COMMON_PRJ_PATH)/scripts/$(ARCH)-$(OS)-compiler.mak

ifeq ($(ENABLE_MEMWATCH), yes)
	CFLAGS += -DMEMWATCH
endif


ifeq ($(debug),yes)
CFLAGS += -pipe -O0 -g -D_DEBUG -DMEMORY_DEBUG
else
CFLAGS += -pipe -O0 -g
endif


MAKE    += -s
MAKEFLAGS += -s

ifeq ($(OS), linux)
CFLAGS  += -Wall
CFLAGS  += -I$(COMMON_LIB_PATH)/include -I$(COMMON_LIB_PATH)/include/bus  $(TARGET_DEFS) -DARCH=$(ARCH) $(WARNING) -I.
#LIBS += -lgxcore -lz -lrt -Wl --whole-archive -lpthread -Wl --no-whole-archive
LIBS += -lpthread
endif


LDFLAGS += -L$(COMMON_LIB_PATH)/lib 

OBJS=$(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

all: env $(BEFORE) deps objects $(OBJS) $(LIB) $(BIN) 
conf:
	echo "#ifndef _GXAPP_SYS_CONFIG_H_"> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "#define _GXAPP_SYS_CONFIG_H_">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "#endif">> $(COMMON_PRJ_PATH)/include/app_common.h
	echo "#endif">> $(COMMON_PRJ_PATH)/include/app_common.h

env:

ifndef COMMON_LIB_PATH
	$(error Error: you must set the COMMON_LIB_PATH environment variable to point to your gxsoft Path.)
endif

#	@echo "env END...";
	
# automatic generation of all the rules written by vincent by hand.	
deps: $(SRC) Makefile
	echo "#ifndef _DEVELOPMENT_VERSION_H_"> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "#define _DEVELOPMENT_VERSION_H_">> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h

	if [ -f .svn/entries ] ; then \
	cd $(COMMON_PRJ_PATH); \
	echo "#define DEVELOPMENT_SVN `sed -n -e 11p .svn/entries`"  >> $(COMMON_PRJ_PATH)/include/app_version.h; \
        echo "#define DEVELOPMENT_SVN_URL \"`sed -n -e 5p .svn/entries`\"" >> $(COMMON_PRJ_PATH)/include/app_version.h; \
	cd -; \
	fi;

	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h

	if [ -f $(COMMON_PRJ_PATH)/../.git/HEAD ] ; then \
		echo "#define DEVELOPMENT_GIT `git rev-list HEAD | wc -l | awk '{print $1}'`" >> $(COMMON_PRJ_PATH)/include/app_version.h; \
		echo "#define DEVELOPMENT_GIT_VER \"`git rev-list HEAD -n 1 | cut -c 1-10`\"" >> $(COMMON_PRJ_PATH)/include/app_version.h; \
	fi;

	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "  ">> $(COMMON_PRJ_PATH)/include/app_version.h
	echo "#endif">> $(COMMON_PRJ_PATH)/include/app_version.h


	echo "#ifndef _APP_COMMON_H_"> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "#define _APP_COMMON_H_">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h

ifeq ($(CHIP_TYPE),i386)
	echo "#define CHIP_TYPE  i386">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
endif

ifeq ($(CHIP_ON_PC),yes)
	echo "#define CHIP_ON_PC_CODE">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
endif

	echo "#define GUI_TTF">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h

	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "  ">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h
	echo "#endif">> $(COMMON_PRJ_PATH)/$(SUBDIRS)/include/app_common.h


	@echo "Generating new dependency file...";	
	@echo "$(COMMON_PRJ_PATH)/scripts/$(ARCH)-$(OS)-compiler.mak";
	@-rm -f deps;
	@for f in $(SRC); do \
		OBJ=objects/`basename $$f|sed -e 's/\.cpp/\.o/' -e 's/\.cxx/\.o/' -e 's/\.cc/\.o/' -e 's/\.c/\.o/'`; \
		echo $$OBJ: $$f>> deps; \
		echo '	@echo -e "compiling \033[032m[$(CC)]\033[0m": ' $$f >> deps; \
		echo '	@$(CC) $$(CFLAGS) -c -o $$@ $$^'>> deps; \
	done
	
-include ./deps
#-include ./$(ARCH)-$(OS)-compiler.mak

objects:
	@mkdir objects
.PHONY: madlib

$(LIB): objects $(OBJS)
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@


ifeq ($(OS), linux)
$(BIN): $(OBJS)
	@echo "$(LDFLAGS)"
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	rm -rf output
	mkdir -p output
	cp out.elf output/out.elf
#cth	
	@echo "make user.bin ok"
endif


subdirs:
	@list='$(SUBDIRS)'; \
		for subdir in $$list; do \
			echo "Making $$target in $$subdir"; \
			cd $$subdir && $(MAKE); \
			cd ..; \
		done;

subdirsclean:
	@list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		echo "Making $$target in $$subdir"; \
		cd $$subdir && $(MAKE) clean; \
		cd ..; \
	done;

subdirinstall:
	@list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		echo "Making $$target in $$subdir"; \
		cd $$subdir && $(MAKE) install; \
		cd ..; \
	done;

install-dir:env subdirinstall
	install -d "$(COMMON_LIB_PATH)/include"	
	install -d "$(COMMON_LIB_PATH)/lib"

clean: subdirsclean
	@rm -rf $(OBJS) *.o .*swp objects deps $(CLEANFILE) $(BIN) *.log $(LIB) cscope.* tags *.img *.gz
	@rm -f ../app/out.elf
	@rm -f ../signal_connect.c
	@find -name "*~" -exec rm {} \;

#format:
#	@echo "Makeing format...";
#	@find -name "*.c" -exec dos2unix -qU 2>d2utmp1 {} \;
#	@find -name "*.h" -exec dos2unix -qU 2>d2utmp1 {} \; 
#	@find -name "*.c" -exec indent -npro -kr -i8 -sob -l120 -ss -ncs  {} \;
#	@find -name "*~" -exec rm {} \;
#	@find -name "d2utmp*" -exec rm {} \;
#	@find -name "deps*" -exec rm {} \;

format:
	@echo "Makeing format...";
	@find -name "*.[chCH]" -print |xargs dos2unix -U
