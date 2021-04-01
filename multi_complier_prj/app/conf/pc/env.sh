#/bin/bash

unset CHIP
unset DEMOD
unset ARCH
unset OS
unset CROSS_PATH
unset GXLIB_PATH
unset GXSRC_PATH
unset GX_KERNEL_PATH
unset COMMON_LIB_PATH
unset COMMON_PRJ_PATH
unset CHIP_TYPE
unset BUILD_PRJ_NAME


if [ "$1" = "csky-ecos" ] ; then
	echo ARCH=csky OS=ecos Configuration  !
	export ARCH=csky
	export OS=ecos
	export CROSS_PATH=csky-ecos
fi

if [ "$1" = "csky-linux" ] ; then
	echo ARCH=csky OS=linux Configuration  !
	export ARCH=csky
	export OS=linux
	export CROSS_PATH=csky-linux
fi

if [ "$1" = "i386-linux" ] ; then
	echo ARCH=i386 OS=linux Configuration  !
	export ARCH=i386
	export OS=linux
	export CROSS_PATH=common_lib
fi

if [ -z "$CROSS_PATH" ] ; then
	echo -e "\033[31m Config Errror!!! ,please check  ARCH_OS \033[0m"
	echo "               "              
	echo eg: source env.sh csky-ecos
	echo "    "source env.sh csky-linux
fi

# goxceed库版本路径
export COMMON_PRJ_PATH=`pwd`
export COMMON_LIB_PATH=$COMMON_PRJ_PATH/../$CROSS_PATH
# linux方案，内核代码路径
export CHIP=gx3201h
export CHIP_TYPE=i386
export CHIP_ON_PC=yes
#export BUILD_PRJ_NAME=test
export BUILD_PRJ_NAME=pc_test
#export BUILD_PRJ_NAME=ui_test

# echo export path
if [ -z "$CROSS_PATH" ] ; then
	echo "CROSS_PATH is NULL"
else

	echo ARCH=$ARCH
 	echo OS=$OS
 	echo CROSS_PATH=$CROSS_PATH
	echo CHIP=$CHIP
	echo GXLIB_PATH=$GXLIB_PATH
	echo COMMON_PRJ_PATH=$COMMON_PRJ_PATH
	echo BUILD_PRJ_NAME=$BUILD_PRJ_NAME
	echo COMMON_LIB_PATH=$COMMON_LIB_PATH
	echo
	echo
fi

