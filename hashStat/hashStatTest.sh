#/bin/bash

export CXX=gcc

if [ "$1" == "--debug" ]; then
  export CFLAGS="-g3 -O0"
fi

$CXX $CFLAGS -dynamiclib -Wall -Werror -o libhashStat.dylib hashStat.c
$CXX $CFLAGS hashStatTest.c -o hashStatTest -L ./ -I ./ -l hashStat -Wl,-rpath,.
