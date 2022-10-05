#!/bin/bash

cpu=$1
addr=$2

mkdir -p site
mkdir -p build

flags="-mthumb -mcpu=arm968e-s -march=armv5te -nostartfiles -Wl,-Ttext,$2 -Os -g"

for file in payload-stager/$1-*.s; do
	name=${file##*/}
	base=${name%.s}
	arm-none-eabi-gcc ${flags} ${file} -o build/${base}.o
	arm-none-eabi-objcopy -O binary build/${base}.o build/${base}.bin
	arm-none-eabi-objdump -d build/${base}.o > site/${base}.lst
	python build-stager-encrypt.py ${addr} build/${base}.bin site/${base}.bin
done
