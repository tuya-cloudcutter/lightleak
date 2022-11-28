@echo off
set PATH=%PATH%;c:\Users\Kuba\.platformio\packages\toolchain-gccarmnoneeabi@1.70201.0\bin\
@REM bash ./build-stager.sh bk7231t 0x1C5AC0 standard
@REM bash ./build-stager.sh bk7231n 0x1B5AC0 ip
bash ./build-stager.sh bk7231n 0x1B5AC0 xor
@REM bash ./build-proper.sh bk7231-proper-v1 bk7231t-proper-v1 BK7231T
bash ./build-proper.sh bk7231-proper-v1 bk7231n-proper-v1 BK7231N
@rem pause
