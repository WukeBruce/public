
-include $(COMMON_PRJ_PATH)/scripts/$(ARCH)-$(OS)-compiler.mak

ifeq ($(ENABLE_MEMWATCH), yes)
	CFLAGS += -DMEMWATCH
endif


ifeq ($(debug),yes)
CFLAGS += -pipe -O0 -g -D_DEBUG -DMEMORY_DEBUG
else
CFLAGS += -pipe -O0 -g
endif

CFLAGS += -ffunction-sections -fdata-sections

MAKE    += -s
MAKEFLAGS += -s

ifeq ($(OS), linux)
CFLAGS  += -Wall
CFLAGS  += -I$(COMMON_LIB_PATH)/include -I$(COMMON_LIB_PATH)/include/bus  $(TARGET_DEFS) -DARCH=$(ARCH) $(WARNING) -I.
CFLAGS += -I$(COMMON_LIB_PATH)/include/kernel_include/
#LIBS += -lgxcore -lz -lrt -Wl --whole-archive -lpthread -Wl --no-whole-archive
CFLAGS += -DOPENSSL
LIBS += -lpthread -W
LIBS += -Wl,--gc-sections
LDFLAGS += -static
endif


LDFLAGS += -L$(COMMON_LIB_PATH)/lib 

OBJS=$(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

all: env $(BEFORE) deps objects $(OBJS) $(LIB) $(BIN) 

env:

ifndef COMMON_LIB_PATH
	$(error Error: you must set the COMMON_LIB_PATH environment variable to point to your gxsoft Path.)
endif

	#@cp $(COMMON_LIB_PATH)/include/test.bin    $(COMMON_PRJ_PATH)/

	#@-rm -f *.elf;	
	#@sh $(COMMON_PRJ_PATH)/scripts/create_signal_connect.sh
	#@cd $(COMMON_PRJ_PATH)/theme/$(DVB_THEME)/image && sh create_image.sh && cd -
	#@cd $(COMMON_PRJ_PATH)/theme/$(DVB_THEME)/widget && sh create_widget.sh && cd -
	#@cd $(COMMON_PRJ_PATH) && sh create_var.sh && cd -

	#@cd $(COMMON_PRJ_PATH)/theme/$(DVB_THEME)/language && sh create_language.sh && cd -

#	@echo "env END...";
	
# automatic generation of all the rules written by vincent by hand.	
deps: $(SRC) Makefile

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

    #@-rm -r $(COMMON_LIB_PATH)/include/app;
	@-rm  $(COMMON_LIB_PATH)/lib/$(LIB);
	@-rm  -r $(COMMON_LIB_PATH)/include/$(BUILD_DIR_NAME)/*;
 
	@echo "install $(LIB)" "->" "$(INSTALL)$(COMMON_LIB_PATH)/lib" 
	@install -d "$(INSTALL)$(COMMON_LIB_PATH)/include"
	@install -d "$(INSTALL)$(COMMON_LIB_PATH)/include/$(BUILD_DIR_NAME)"
	@install -d "$(INSTALL)$(COMMON_LIB_PATH)/lib"
	@install -m 644 $(LIB) "$(INSTALL)$(COMMON_LIB_PATH)/lib"
	@install -m 644 ./$(BUILD_DIR_NAME)/*.h "$(INSTALL)$(COMMON_LIB_PATH)/include/$(BUILD_DIR_NAME)/"

ifeq ($(OS), linux)
$(BIN): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	rm -rf output
	mkdir -p output
	mv out.elf output/out.elf
	@echo "[execute file] [$(COMMON_PRJ_PATH)/$(BUILD_DIR_NAME)/output/out.elf ok]"
#cth	
	#$(STRIP) output/out.elf
	#cp -ar ../theme/$(DVB_THEME) output/theme
	#find output/ -type d -name ".svn" | xargs rm -rf
#	fakeroot mkfs.cramfs output user.bin
	#mksquashfs output user.bin -noappend -no-duplicates > /dev/null
	#@echo "make user.bin ok"
#	@echo "LDFLAGS"=$(LDFLAGS)
#	@echo "LIBS"=$(LIBS)
endif

ifeq ($(OS), linux)

bin:
ifeq  ($GENROOTFS_PATH),)
else
	@rm -f $(GENROOTFS_PATH)/flashrom.bin
	@rm -f $(GENROOTFS_PATH)/flash_ts.bin
	#拷贝目标文件到根文件系统
	cp $(COMMON_PRJ_PATH)/app/user.bin $(GENROOTFS_PATH)/flash_image/
##cth	
	cp $(COMMON_PRJ_PATH)/flash/loader-sflash.bin $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash/causer.bin $(GENROOTFS_PATH)/flash_image/
#	cp $(COMMON_PRJ_PATH)/flash/logo.bin $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash/logo.jpg $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash/ota.img $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash/invariable_oem.ini $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash/variable_oem.ini $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash_linux/flash.conf $(GENROOTFS_PATH)/flash_image/
	cp $(COMMON_PRJ_PATH)/flash_linux/flash_ts.conf $(GENROOTFS_PATH)/flash_image/
	cp $(GX_KERNEL_PATH)/arch/csky/boot/uImage $(GENROOTFS_PATH)/flash_image/
	
#	cp $(COMMON_LIB_PATH)/include/app/linux/S04frontend $(GENROOTFS_PATH)/gx3201_rootfs/etc/rcS.d/
	cp $(COMMON_LIB_PATH)/include/app/linux/S05panel $(GENROOTFS_PATH)/gx3201_rootfs/etc/rcS.d/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ohci-hcd.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ehci-hcd.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	cp $(COMMON_LIB_PATH)/include/app/linux/panel.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
#	cp $(GXAVKO_PATH) $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/av.ko
#	cp $(GXFEKO_PATH) $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	mkfs.cramfs $(GENROOTFS_PATH)/gx3201_rootfs/ $(GENROOTFS_PATH)/flash_image/rootfs.bin
	chmod 777 $(GENROOTFS_PATH)/scripts/*
	@echo "create bin begin..."
	$(GENROOTFS_PATH)/scripts/genflash mkflash $(GENROOTFS_PATH)/flash_image/flash.conf $(GENROOTFS_PATH)/flashrom.bin	
	$(GENROOTFS_PATH)/scripts/genflash mkflash $(GENROOTFS_PATH)/flash_image/flash_ts.conf $(GENROOTFS_PATH)/flash_ts.bin
	cp $(GENROOTFS_PATH)/flashrom.bin $(GENROOTFS_PATH)/flash_ts.bin $(COMMON_PRJ_PATH)/flash_linux/
	cp $(GENROOTFS_PATH)/flashrom.bin $(COMMON_PRJ_PATH)/flash_linux/flashrom_GX3201_"$(DVB_CA_1_NAME)"_"$(DVB_AD_NAME)"_`date +%Y%m%d_%H%M`.bin
	cp $(GENROOTFS_PATH)/flash_ts.bin $(COMMON_PRJ_PATH)/flash_linux/flash_ts_GX3201_"$(DVB_CA_1_NAME)"_"$(DVB_AD_NAME)"_`date +%Y%m%d_%H%M`.bin
	
endif

ifeq  ($NFSROOTFS_PATH),)
else
	cp $(COMMON_LIB_PATH)/include/app/linux/S05panel $(NFSROOTFS_PATH)/etc/rcS.d/
#	cp $(COMMON_LIB_PATH)/include/app/linux/S04frontend $(NFSROOTFS_PATH)/etc/rcS.d/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ohci-hcd.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ehci-hcd.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	cp $(COMMON_LIB_PATH)/include/app/linux/panel.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
#	cp $(GXAVKO_PATH) $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/av.ko
#	cp $(GXFEKO_PATH) $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	@echo "start cp file to nfs"
	rm -rf $(NFSROOTFS_PATH)/dvb/theme
	cp -ar $(COMMON_PRJ_PATH)/theme/$(DVB_THEME) $(NFSROOTFS_PATH)/dvb/theme
	find $(NFSROOTFS_PATH)/dvb/ -type d -name ".svn" | xargs rm -rf
	cp $(COMMON_PRJ_PATH)/app/out.elf $(NFSROOTFS_PATH)
endif

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
	rm -rf output
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
