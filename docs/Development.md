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

To pull in additional BSP drivers, if required, the `Makefile` should be modified.

## CONFIG Flags

Stock TASOLLERs ship with Config0=FFFFFF7D, Config1=FFFFFFFF. The breakdown of Config0 is as follows:

| Bit   | Function   | Stock Value | Description                         |
| ----- | ---------- | ----------- | ----------------------------------- |
| 31    | `CWDTEN`   | 1           | Watchdog inactive                   |
| 30    | `CWDTPDEN` | 1           | N/A                                 |
| 29:28 | Reserved   | 11          |                                     |
| 27    | `CGPFMFP`  | 1           | PF.0/PF.1 used for external crystal |
| 26:24 | `CFOSC`    | 111         | 22.11284 MHz internal HIRC          |
| 23    | `CBODEN`   | 1           | Brown-out detection disabled        |
| 22:21 | `CBOV`     | 11          | Brown-out voltage is 4.5V           |
| 20    | `CBORST`   | 1           | Brown-out reset disabled            |
| 19:11 | Reserved   | 111111111   |                                     |
| 10    | `CIOINI`   | 1           | GPIO defaults to quasi-directional  |
| 9:8   | Reserved   | 11          |                                     |
| 7:6   | `CBS`      | 01          | Boot from LDROM without IAP         |
| 5:3   | Reserved   | 111         |                                     |
| 2     | `DFVSEN`   | 1           | Data flash fixed at 4kB             |
| 1     | `LOCK`     | 0           | Flash memory is locked              |
| 0     | `DFEN`     | 1           | N/A                                 |

For debugging, `LOCK` being enabled can be problematic. Unlocking the processor involves performing a complete chip erase, at which point we must provision out bootloader. We can then set config0 to `FFFFFF7F`, allowing access to flash memory for debugging. `flash.cmd` does this.

Config1 is just used for `DFBADR` and because `DFVSEN=1` this is ignored anyway.

Of note, neither the watchdog nor brown-out detection is enabled. This custom firmware makes no attempt to alter this, as it would present complications when reverting to stock firmware. I would much prefer to have the watchdog enabled, trust me.

## USB
This firmware is compliant with the USB specification. The following USB3CV tests are used to verify compliance:

- `Chapter 9 Tests [USB 2 devices]`
- `Connector Type Tests`
- `HID Tests`

If you make changes to the USB code, it is recommended to re-run these tests to ensure continued compliance.

Additionally, the following tests may be run for diagnostics:

- `Device Summary`
- `Current Measurement Test [USB 2 devices]`

xHSETT can be used to directly control the USB device, such as sending SUSPEND signals.

## Airs
See https://www.shinkoh-elecs.jp/wp-content/uploads/2024/05/C_KB1281_1581_24A.pdf

The loop of wire between the two wings is for connecting emitter pin 3 to detector pin 1.

The right bottom is an emitter, and then it alternates up from there.
