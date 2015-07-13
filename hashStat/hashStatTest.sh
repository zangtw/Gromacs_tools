#/bin/bash

export CC=gcc

if [ "$1" == "--debug" ]; then
  export CFLAGS="-g3 -O0"
fi

if [ "$(uname)" == 'Linux' ]; then
  $CC $CFLAGS -c -Wall -Werror -fpic hashStat.c
  $CC $CFLAGS -shared -o libhashStat.so hashStat.o
  rm hashStat.o
elif [ "$(uname)" == 'Darwin' ]; then
  $CC $CFLAGS -dynamiclib -Wall -Werror -install_name @rpath/libhashStat.dylib -o libhashStat.dylib hashStat.c
else
  echo platform not supported!
  exit 1
fi

$CC $CFLAGS hashStatTest.c -o hashStatTest -L ./ -I ./ -l hashStat -l m -Wl,-rpath,.
