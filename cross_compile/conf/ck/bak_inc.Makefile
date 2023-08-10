-include $(GXSRC_PATH)/scripts/$(ARCH)-$(OS)-compiler.mak

ifeq ($(ENABLE_MEMWATCH), yes)
	CFLAGS += -DMEMWATCH
endif

STB_INFO_XML=win_stb_info.xml
STB_INFO_2=$(wildcard $(GXLIB_PATH)/include/app/customer/$(STB_INFO_XML))
STB_INFO_EXIT=$(notdir $(STB_INFO_2))

ifeq ($(debug),yes)
CFLAGS += -pipe -O0 -g -D_DEBUG -DMEMORY_DEBUG
else
CFLAGS += -pipe -O0 -g
endif

MAKE    += -s
MAKEFLAGS += -s
ifeq ($(OS), ecos)
WARNING +=-Werror
WARNING += -Wundef -Wall

CFLAGS  += -I$(GXLIB_PATH)/include -I$(GXLIB_PATH)/include/bus \
	   $(TARGET_DEFS) -DARCH=$(ARCH) -DCHIP=$(CHIP) $(WARNING) -I. -I./common/include
	   
#$(TARGET_DEFS) -DARCH=$(ARCH) $(WARNING) -I. -I./cdcas/include
LIBS += -lgxcore
endif

ifeq ($(OS), linux)
CFLAGS  += -Wall
CFLAGS  += -I$(GXLIB_PATH)/include -I$(GXLIB_PATH)/include/bus  $(TARGET_DEFS) -DARCH=$(ARCH) $(WARNING) -I.
CFLAGS += -I$(GXLIB_PATH)/include/kernel_include/
LIBS += -lgxcore -lz -lrt -Wl --whole-archive -lpthread -Wl --no-whole-archive
LDFLAGS += -static

endif

LDFLAGS += -L$(GXLIB_PATH)/lib 

OBJS=$(addprefix objects/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

all: env $(BEFORE) deps objects $(OBJS) $(LIB) $(BIN) 

env:
ifndef GXLIB_PATH
	$(error Error: you must set the GXLIB_PATH environment variable to point to your gxsoft Path.)
endif
	@-rm -f $(GXSRC_PATH)/theme/$(DVB_THEME)/keymap.xml	
	@cp $(GXLIB_PATH)/include/app/key_xml/$(DVB_KEY_TYPE).xml $(GXSRC_PATH)/theme/$(DVB_THEME)/keymap.xml
	@cp $(GXLIB_PATH)/include/app/customer/logo.bin $(GXSRC_PATH)/theme/$(DVB_THEME)/logo.bin
	@cp $(GXLIB_PATH)/include/app/customer/bootlogo.bin $(GXSRC_PATH)/theme/$(DVB_THEME)/bootlogo.bin
	@cp $(GXLIB_PATH)/include/app/customer/watermark.png $(GXSRC_PATH)/theme/$(DVB_THEME)/image/image_india_fta/fullscreen/watermark.png	
	@cp $(GXLIB_PATH)/include/app/customer/watermark1.png $(GXSRC_PATH)/theme/$(DVB_THEME)/image/image_india_fta/fullscreen/watermark1.png	
	@cp $(GXLIB_PATH)/include/app/customer/variable_oem.ini $(GXSRC_PATH)/flash/variable_oem.ini
	@cp $(GXLIB_PATH)/include/app/customer/*.conf $(GXSRC_PATH)/flash/
	@cp $(GXLIB_PATH)/include/app/customer/*.ini $(GXSRC_PATH)/flash/
	@cp $(GXLIB_PATH)/include/app/customer/ota.img $(GXSRC_PATH)/flash/ota.img
	@cp $(GXLIB_PATH)/include/app/customer/usb_oem.ini $(GXSRC_PATH)/flash/usb_oem.ini
	@cp $(GXLIB_PATH)/include/app/customer/loader-sflash.bin $(GXSRC_PATH)/flash/loader-sflash.bin
	@cp $(GXLIB_PATH)/include/app/customer/flash_boot.bin    $(GXSRC_PATH)/flash_boot.bin
ifeq ($(STB_INFO_XML),$(STB_INFO_EXIT))
	@cp $(GXLIB_PATH)/include/app/customer/win_stb_info.xml $(GXSRC_PATH)/theme/$(DVB_THEME)/widget/win_stb_info.xml
endif
ifeq ($(DVB_LOGO_JPG), )
else
	@-rm -f $(GXSRC_PATH)/flash/logo.jpg
	cp $(GXLIB_PATH)/include/app/logo-jpg/$(DVB_LOGO_JPG) $(GXSRC_PATH)/flash/logo.jpg
endif
	find $(GXSRC_PATH)/theme/$(DVB_THEME)/widget/ -name "*_cas*.xml"|xargs rm -rf
ifeq ($(DVB_CA_1_NAME), )
else
	cp $(GXSRC_PATH)/app/cas/$(DVB_CA_1_NAME)/xml/*.xml $(GXSRC_PATH)/theme/$(DVB_THEME)/widget
endif
ifeq ($(DVB_CA_2_NAME), )
else
	cp $(GXSRC_PATH)/app/cas/$(DVB_CA_2_NAME)/xml/*.xml $(GXSRC_PATH)/theme/$(DVB_THEME)/widget
endif
	@-rm -f *.elf;	
	@sh $(GXSRC_PATH)/scripts/create_signal_connect.sh
	@cd $(GXSRC_PATH)/theme/$(DVB_THEME)/image && sh create_image.sh && cd -
	@cd $(GXSRC_PATH)/theme/$(DVB_THEME)/widget && sh create_widget.sh && cd -
	@cd $(GXSRC_PATH) && sh create_var.sh && cd -
ifeq ($(DVB_CA_1_NAME), )
else
	@cp $(GXSRC_PATH)/app/cas/$(DVB_CA_1_NAME)/i18n.xls $(GXSRC_PATH)/theme/$(DVB_THEME)/language/ca.xls
endif
ifeq ($(DVB_CA_2_NAME), )
else
	@cp $(GXSRC_PATH)/app/cas/$(DVB_CA_2_NAME)/i18n.xls $(GXSRC_PATH)/theme/$(DVB_THEME)/language/ca2.xls
endif

	@cd $(GXSRC_PATH)/theme/$(DVB_THEME)/language && sh create_language.sh && cd -

ifeq ($(DVB_CA_1_NAME), )
else
	@rm -rf $(GXSRC_PATH)/theme/$(DVB_THEME)/language/ca.xls
endif
ifeq ($(DVB_CA_2_NAME), )
else
	@rm -rf $(GXSRC_PATH)/theme/$(DVB_THEME)/language/ca2.xls
endif
#	@echo "env END...";
	
# automatic generation of all the rules written by vincent by hand.	
deps: $(SRC) Makefile
	echo "#ifndef _DEVELOPMENT_VERSION_H_"> $(GXSRC_PATH)/app/include/development_version.h
	echo "#define _DEVELOPMENT_VERSION_H_">> $(GXSRC_PATH)/app/include/development_version.h
	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h
	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h

	if [ -f .svn/entries ] ; then \
	cd $(GXSRC_PATH); \
	echo "#define DEVELOPMENT_SVN `sed -n -e 11p .svn/entries`"  >> $(GXSRC_PATH)/app/include/development_version.h; \
        echo "#define DEVELOPMENT_SVN_URL \"`sed -n -e 5p .svn/entries`\"" >> $(GXSRC_PATH)/app/include/development_version.h; \
	cd -; \
	fi;

	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h
	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h

	if [ -f $(GXSRC_PATH)/../.git/HEAD ] ; then \
		echo "#define DEVELOPMENT_GIT `git rev-list HEAD | wc -l | awk '{print $1}'`" >> $(GXSRC_PATH)/app/include/development_version.h; \
		echo "#define DEVELOPMENT_GIT_VER \"`git rev-list HEAD -n 1 | cut -c 1-10`\"" >> $(GXSRC_PATH)/app/include/development_version.h; \
	fi;

	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h
	echo "  ">> $(GXSRC_PATH)/app/include/development_version.h
	echo "#endif">> $(GXSRC_PATH)/app/include/development_version.h



	@echo "Generating new dependency file...";
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

ifeq ($(OS), ecos)
$(BIN): $(OBJS) 
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	@$(OBJCOPY) $@ -S -g -O binary ecos.bin
#	@rm -f ecos.bin.gz
	@rm -f ecos.bin.lzma
#	@gzip ecos.bin
	@lzma -z ecos.bin
	@rm -rf ecos
	@mkdir ecos
#	@mv ecos.bin.gz ecos
	@mv -f ecos.bin.lzma ecos
	@genromfs -f ecos.img -d ecos/
	@rm  ../flash/ecos.img -rf
	@mv ecos.img  ../flash
#cth    	
	@bash ../flash/mkimg
	@rm -rf ecos
endif

ifeq ($(OS), linux)
$(BIN): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	rm -rf output
	mkdir -p output
	cp out.elf output/out.elf
#cth	
	$(STRIP) output/out.elf
	cp -ar ../theme/$(DVB_THEME) output/theme
	find output/ -type d -name ".svn" | xargs rm -rf
#	fakeroot mkfs.cramfs output user.bin
	mksquashfs output user.bin -noappend -no-duplicates > /dev/null
	@echo "make user.bin ok"
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
	cp $(GXSRC_PATH)/app/user.bin $(GENROOTFS_PATH)/flash_image/
##cth	
	cp $(GXSRC_PATH)/flash/loader-sflash.bin $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash/causer.bin $(GENROOTFS_PATH)/flash_image/
#	cp $(GXSRC_PATH)/flash/logo.bin $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash/logo.jpg $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash/ota.img $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash/invariable_oem.ini $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash/variable_oem.ini $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash_linux/flash.conf $(GENROOTFS_PATH)/flash_image/
	cp $(GXSRC_PATH)/flash_linux/flash_ts.conf $(GENROOTFS_PATH)/flash_image/
	cp $(GX_KERNEL_PATH)/arch/csky/boot/uImage $(GENROOTFS_PATH)/flash_image/
	
#	cp $(GXLIB_PATH)/include/app/linux/S04frontend $(GENROOTFS_PATH)/gx3201_rootfs/etc/rcS.d/
	cp $(GXLIB_PATH)/include/app/linux/S05panel $(GENROOTFS_PATH)/gx3201_rootfs/etc/rcS.d/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ohci-hcd.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ehci-hcd.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	cp $(GXLIB_PATH)/include/app/linux/panel.ko $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
#	cp $(GXAVKO_PATH) $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/av.ko
#	cp $(GXFEKO_PATH) $(GENROOTFS_PATH)/gx3201_rootfs/lib/modules/2.6.27.55/
	mkfs.cramfs $(GENROOTFS_PATH)/gx3201_rootfs/ $(GENROOTFS_PATH)/flash_image/rootfs.bin
	chmod 777 $(GENROOTFS_PATH)/scripts/*
	@echo "create bin begin..."
	$(GENROOTFS_PATH)/scripts/genflash mkflash $(GENROOTFS_PATH)/flash_image/flash.conf $(GENROOTFS_PATH)/flashrom.bin	
	$(GENROOTFS_PATH)/scripts/genflash mkflash $(GENROOTFS_PATH)/flash_image/flash_ts.conf $(GENROOTFS_PATH)/flash_ts.bin
	cp $(GENROOTFS_PATH)/flashrom.bin $(GENROOTFS_PATH)/flash_ts.bin $(GXSRC_PATH)/flash_linux/
	cp $(GENROOTFS_PATH)/flashrom.bin $(GXSRC_PATH)/flash_linux/flashrom_GX3201_"$(DVB_CA_1_NAME)"_"$(DVB_AD_NAME)"_`date +%Y%m%d_%H%M`.bin
	cp $(GENROOTFS_PATH)/flash_ts.bin $(GXSRC_PATH)/flash_linux/flash_ts_GX3201_"$(DVB_CA_1_NAME)"_"$(DVB_AD_NAME)"_`date +%Y%m%d_%H%M`.bin

	
endif
ifeq  ($NFSROOTFS_PATH),)
else
	cp $(GXLIB_PATH)/include/app/linux/S05panel $(NFSROOTFS_PATH)/etc/rcS.d/
#	cp $(GXLIB_PATH)/include/app/linux/S04frontend $(NFSROOTFS_PATH)/etc/rcS.d/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ohci-hcd.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	cp $(GX_KERNEL_PATH)/drivers/usb/host/ehci-hcd.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	cp $(GXLIB_PATH)/include/app/linux/panel.ko $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
#	cp $(GXAVKO_PATH) $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/av.ko
#	cp $(GXFEKO_PATH) $(NFSROOTFS_PATH)/lib/modules/2.6.27.55/
	@echo "start cp file to nfs"
	rm -rf $(NFSROOTFS_PATH)/dvb/theme
	cp -ar $(GXSRC_PATH)/theme/$(DVB_THEME) $(NFSROOTFS_PATH)/dvb/theme
	find $(NFSROOTFS_PATH)/dvb/ -type d -name ".svn" | xargs rm -rf
	cp $(GXSRC_PATH)/app/out.elf $(NFSROOTFS_PATH)
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
	install -d "$(GXLIB_PATH)/include"	
	install -d "$(GXLIB_PATH)/lib"

clean: subdirsclean
	@rm -rf $(OBJS) *.o .*swp objects deps $(CLEANFILE) $(BIN) *.log $(LIB) cscope.* tags *.img *.gz
ifeq ($(OS), ecos)
	@rm -f ../flash/flash.dat
	@rm -f ../flash/ecos.img	
	@rm -f ../flash/datafs.jffs2	
	@rm -f ../flash/flash.bin
	@rm -f ../flash/flash_logo.bin
	@rm -f ../flash/flash_ts.dat
	@rm -f ../flash/flash_usb.bin
	@rm -f ../flash/root_cramfs.img	
endif
	@rm -f ../app/out.elf
#	@rm -f ../app/signal_connect.c
	@rm -f ../signal_connect.c
	@rm -f ../theme/$(DVB_THEME)/image/image.xml
	@rm -f ../theme/$(DVB_THEME)/keymap.xml
	@rm -f ../theme/$(DVB_THEME)//widget/widget.xml
ifeq ($(OS), linux)
	@rm -rf ../app/output
	@rm -rf ../app/user.bin
	@rm -f ../flash_linux/flashrom.bin
	@rm -f ../flash_linux/flash_ts.bin
#	@rm -f $(GENROOTFS_PATH)/flashrom.bin
#	@rm -f $(GENROOTFS_PATH)/flash_ts.bin
endif
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
