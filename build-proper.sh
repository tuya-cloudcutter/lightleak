#!/bin/bash

mkdir -p site
mkdir -p build

flags="-mthumb -mcpu=arm968e-s -march=armv5te -nostartfiles -Wl,-Ttext,0x120000 -Wl,--entry,main -std=c99 -Wno-builtin-declaration-mismatch -Os -g"

SOURCE=$1
OUTPUT=$2
DEFINE=$3

arm-none-eabi-gcc ${flags} payload-proper/${SOURCE}/*.c -D${DEFINE} -Ipayload-proper/${SOURCE} -o build/${OUTPUT}.o
arm-none-eabi-objcopy -O binary build/${OUTPUT}.o build/${OUTPUT}.bin
arm-none-eabi-objdump -dS build/${OUTPUT}.o > site/${OUTPUT}.lst
ltchiptool soc bkpackager encrypt 510fb093a3cbeadc5993a17ec7adeb03 build/${OUTPUT}.bin site/${OUTPUT}.bin 0x120000 -c
