#!/bin/bash
# ./configure --help
CUR_PREFIX=`pwd`
if [ "$1" = "pc" ];then
export PREFIXLIB=$CUR_PREFIX/../i386-linux/
else
export PREFIXLIB=$CUR_PREFIX/../arm-linux/
fi	
#export CROSS=csky-elf
export CROSS=arm-linux-uclibcgnueabihf
#export CROSS_EXTRAL_FLAG=-mlittle-endian
export LIB_PREFIX_DIR=${PREFIXLIB}/
#export INCLUDE_PATH=${PREFIXLIB}/include
#export LIBS_PATH=${PREFIXLIB}/lib
export INCLUDE_PATH=${LIB_PREFIX_DIR}/include
export LIBS_PATH=${LIB_PREFIX_DIR}/lib
 
if [ "$1" = "pc" ];then
./configure --prefix=${LIB_PREFIX_DIR}
echo "pc"
else
CC=${CROSS}-gcc ./configure --prefix=${LIB_PREFIX_DIR} --static;
echo "arm"
fi
echo "${PREFIXLIB}"
make clean;make;make install
