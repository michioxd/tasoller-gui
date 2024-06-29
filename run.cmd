@..\..\openocd-0.12.0-3\bin\openocd.exe -f interface/stlink-v2.cfg -f ..\..\nucxxx.cfg ^
    -c "SysReset halt" ^
    -c "SysReset aprom run" ^
    -c "exit"
