#!/bin/bash

#compile the whole of project for pc
./build pc
source env.sh i386-linux #set env
make clean; #clean all middle file
make #
#compile the whole of project for arm cross_compile
./build arm
source env.sh arm-linux #set env
make clean; #clean all middle file
make #
#compile the whole of project for ck cross_compile
./build ck
