#!/bin/bash

mkdir -p site
mkdir -p build

flags="-mthumb -mcpu=arm968e-s -march=armv5te -nostartfiles -Wl,-Ttext,0x120000 -Wl,--entry,main -std=c99 -Wno-builtin-declaration-mismatch -Os -g"

for dir in payload-proper/*; do
	name=${dir##*/}
	arm-none-eabi-gcc ${flags} ${dir}/*.c -I${dir} -o build/${name}.o
	arm-none-eabi-objcopy -O binary build/${name}.o build/${name}.bin
	arm-none-eabi-objdump -dS build/${name}.o > site/${name}.lst
	ltchiptool soc bkpackager encrypt 510fb093a3cbeadc5993a17ec7adeb03 build/${name}.bin site/${name}.bin 0x120000 -c
done
