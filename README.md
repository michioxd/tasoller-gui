# TASOLLER Custom Firmware (Host MCU - Application ROM)

See [Development](./Development.md) for development information.

Like what you see? [Buy me a coffee](https://ko-fi.com/bottersnike)

## Features
- Superior slider processing; no more dropped holds or missed notes!
- Native arcade IO support
- Air sensor anti-jitter (fixes dodgy sensors)
- Intuitive menus
- Arcade-accurate colours

And of course, it's all open source :).

## Setup
### Installing Firmware
- Download both `host_aprom.bin` and `led_aprom.bin`
- Run `TASOLLER-FirmwareUpdater.exe`
- Follow the instructions

### Configuring segatools
This firmware emulates arcade IO. As such, segatools' emulation should be disabled, by adding the following lines:

```ini
[slider]
enable=0
[io4]
enable=0
```

Ensure no other `[slider]` section exists in the file (or if it does, set `enable=0` there instead).

The slider **MUST** be assigned to port `COM1`. To do this:

- Go to `Devices and Printers` in control panel
- Double click `TASOLLER`
- In the `Hardware` tab, double click `USB Serial Device`
  - If this already reads `USB Serial Device (COM1)` nothing needs done
- Click `Change settings`
- Go to `Port Settings` -> `Advanced...`
- Under the dropdown for `COM Port Number:` select `COM1`

If `COM1` is already in use, check what device it is assigned to in Device Manager under `Ports (COM & LPT)`. You may need to enable `View` -> `Show hidden devices`.

Pre-chusan Chunithm uses IO3. This firmware does not (and unfortunately cannot) support IO3. It is recommended to enable the HID keyboard mode, and continue to use keyboard input for IRs.

## Controls
### General
Tap FN1 to insert a coin. FN2 is currently unbound.

### Configuration
While holding FN1, the following configuration options are available:

![](./docs/img/Graphic_Slider_Settings.png)

※ "Insert card" will hold the Enter key for 5 seconds

### HID keyboard settings

Double-tapping FN1 will bring you to the HID keyboard settings, where the following configuration options are available:

![](./docs/img/Graphic_Slider_Keyboard.png)

### Test menu

To enter the system test menu, double-tap FN2. This will mirror the on-screen controls, and additionally adds controls for the TEST and SERVICE buttons on a real cabinet. Double-tap FN2 again to leave this mode on the controller (note that this is not synced with the game exiting the menu!).

![](./docs/img/Graphic_Slider_Test.png)

To access the TEST and SERVICE buttons without entering the test menu, hold FN2 instead.

![](./docs/img/Graphic_Slider_TestReduced.png)

## Keyboard Mapping

When the HID keyboard mode is enabled, these mappings are used for the various configuration options, with the airs mapped as `0OLP,.`:

### 32k (UMIGURI default)

![](./docs/img/Graphic_Slider_Binds32k.png)

### 16k

![](./docs/img/Graphic_Slider_Binds16k.png)

### 8k

![](./docs/img/Graphic_Slider_Binds8k.png)

### 4k

![](./docs/img/Graphic_Slider_Binds4k.png)

### 9k

![](./docs/img/Graphic_Slider_Binds9k.png)


## Divider Options

When the HID keyboard mode is enabled, these are the options for the dividers:

### No divider

![](./docs/img/Graphic_Slider_DividerNone.png)

### Single divider

![](./docs/img/Graphic_Slider_DividerSingle.png)

### 4k divider

![](./docs/img/Graphic_Slider_Divider4k.png)

### 8k divider

![](./docs/img/Graphic_Slider_Divider8k.png)

### All dividers

![](./docs/img/Graphic_Slider_DividerAll.png)

### 9k divider

![](./docs/img/Graphic_Slider_Divider9k.png)
