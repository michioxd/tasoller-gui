@..\..\openocd-0.12.0-3\bin\openocd.exe -f interface/stlink-v2.cfg -f ..\..\nucxxx.cfg ^
    -c "SysReset halt" ^
    -c "flash read_bank 1 dataflash.bin" ^
    -c "ChipErase" ^
    -c "exit"

@..\..\openocd-0.12.0-3\bin\openocd.exe -f interface/stlink-v2.cfg -f ..\..\nucxxx.cfg ^
    -c "SysReset halt" ^
    -c "WriteConfigRegs 0xFFFFFF7F 0xFFFFFFFF" ^
    -c "ReadConfigRegs" ^
    -c "program ../bootloader/host_bl.bin 0x100000" ^
    -c "program host_aprom.bin 0" ^
    -c "program dataflash.bin 0x1F000" ^
    -c "SysReset aprom run" ^
    -c "exit"

@del dataflash.bin
