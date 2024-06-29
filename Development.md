# Development
## Requirements
- Windows (Strongly recommend. Compiling on anything else is not officially supported.)
- [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm)
  - On the path, please
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (Currently using 13.2 Rel1)
  - To make your life easier, ensure this is located at `C:\Program Files (x86)\GNU Arm Embedded Toolchain\13.2 Rel1`
- [picolibc 1.8.6](https://github.com/picolibc/picolibc/releases/download/1.8.6/picolibc-1.8.6-13.2.Rel1.zip)
  - Place the contents of this zip file in `picolibc`

VSCode is recommended for development, and all recommended extensions should be installed.

## Compiling and Uploading
Run `make -j` to compile the firmware.

For rapid development, the HID bootloader is quite annoying. `flash.cmd` is provided which reprograms the chip using an ST-Link V2. This requires an OpenOCD installation in the root project folder, along with an existing compiled host bootloader binary.

For debugging in VSCode, install the recommended extensions then hit F5. This requires an OpenOCD installation in the root project folder, and an ST-Link V2.

## NUC123 BSP
The `NUC123` folder is a heavily reduced form of the complete BSP provided by Nuvoton. Modifications should not be made to any files within the `inc` or `StdDriver` folder.

`startup_NUC123.s` is the program entrypoint, and may require modification.

`NUC123.ld` is a modified version of the GCC linker script provided by Nuvoton, and may require modification.

To pull in additional BSP library modules, if required, the `Makefile` should be modified.
