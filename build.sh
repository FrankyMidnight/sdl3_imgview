#!/bin/bash

CC="gcc -std=c23"

CFLAGS_STRICT="-Wstrict-aliasing=2 -Wall -Wextra -Werror -Wpedantic -Wwrite-strings -Wconversion -Wmissing-declarations \
-Wmissing-include-dirs -Wfloat-equal -Wsign-compare -Wundef -Wcast-align -Wswitch-default -Wimplicit-fallthrough \
-Wempty-body -Wuninitialized -Wmisleading-indentation -Wshadow -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition "


CFLAGS_RELEASE="-O3 -march=native -flto=auto -fno-plt -fomit-frame-pointer "

CFLAGS_DEBUG="-O0 -g3 -ggdb3 -fno-strict-aliasing -fstack-protector-strong \
-DDEBUG -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -ftrapv "

LD_LIBS="-lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer "

# CHECK ARGUMENTS
if [ "$#" -lt 2 ]
then
    echo "./build.sh FILE.c release | debug | strict" 
    exit 1
fi
# COMPILE
if  [ "$2" = "strict" ]
then

echo $CC $CFLAGS_STRICT $LD_LIBS $1   
$CC $CFLAGS_STRICT $LD_LIBS $1  

elif  [ "$2" = "debug" ]
then

echo $CC $CFLAGS_DEBUG $LD_LIBS $1   
$CC $CFLAGS_DEBUG $LD_LIBS $1  

elif  [ "$2" = "release" ]
then

echo $CC $CFLAGS_RELEASE $LD_LIBS $1   
$CC $CFLAGS_RELEASE $LD_LIBS $1  

fi