@ECHO off

FOR /F "tokens=* USEBACKQ" %%g IN (`
    ..\..\openocd-0.12.0-3\bin\openocd.exe -f ..\..\nucxxx.cfg ^
        -c "init; PrintPart; exit" 2^>NUL
`) DO (SET "PART_NUM=%%g")

IF NOT "%PART_NUM%"=="NUC123SD4AN0" (
    ECHO Invalid part number %PART_NUM%. Make sure you're connected to HOST not LED!
    EXIT /B 1
)

REM SET LDROM_PATH=../bootloader/host_bl.bin
SET LDROM_PATH=../../../TASOLLER_HOST_LDROM.bin
SET APROM_PATH=host_aprom.bin

..\..\openocd-0.12.0-3\bin\openocd.exe -f ..\..\nucxxx.cfg ^
    -c "SysReset halt" ^
    -c "flash read_bank 1 dataflash.bin" ^
    -c "ChipErase" ^
    -c "WriteConfigRegs 0xFFFFFF7F 0xFFFFFFFF" ^
    -c "ReadConfigRegs" ^
    -c "program %LDROM_PATH% 0x100000" ^
    -c "program %APROM_PATH% 0" ^
    -c "program dataflash.bin 0x1F000" ^
    -c "ReadConfigRegs" ^
    -c "SysReset aprom run" ^
    -c "exit" -d0

REM Clean up after ourself
del dataflash.bin
