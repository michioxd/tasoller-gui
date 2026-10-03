@echo off
setlocal
cd /d "%~dp0"
set "PATH=C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin;C:\Program Files (x86)\GnuWin32\bin;%PATH%"
where arm-none-eabi-gcc >nul 2>&1
if errorlevel 1 (
    echo ERROR: ARM GNU toolchain not found.
    exit /b 1
)
where make >nul 2>&1
if errorlevel 1 (
    echo ERROR: GNU Make not found.
    exit /b 1
)
make -j all
exit /b %errorlevel%