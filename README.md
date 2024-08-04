# TASOLLER Custom Firmware (Host MCU - Application ROM)

See [Development](./Development.md) for development information.

Yes, the code is a mess. I just wanted to get something onto git y'know :).

## Setup
### Installing Firmware
- Disconnect the PC cable from the TASOLLER
- Hold down the FN2 button, while reconnecting the PC cable
- `HOSTMCU` should show as a device
- Use `Update V1.1.exe` from official firmware updates to load the firmware

Currently, Dao CFW is required on the LED board. This is controlled by `LED_FIRMWARE_CFW` in `src/led.h`.

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
Hold FN1 for configuration:

- Pad 1/2 (cell 0): Left tower colour
- Pad 3/4 (cell 1): Ground colour
- Pad 5/6 (cell 2): Ground colour when pressed, and separator colour
- Pad 7/8 (cell 3): Right tower colour
- Pad 9/10 (cell 4): No function
- Pad 11/12 (cell 5): Toggle rainbow effect on/off
- Pad 13/14 (cell 6): Increase/decrease ground brightness
- Pad 15/16 (cell 7): Increase/decrease tower brightness
- Pad 17/18 (cell 8): No function
- Pad 19/20 (cell 9): System volume up/down
- Pad 21/22 (cell 10): Holds Enter for 5 seconds (insert Aime card)
- Pad 23/24 (cell 11): No function
- Pad 25/26 (cell 12): Increase/decrease sensitivity
- Pad 27/28 (cell 13): No function
- Pad 29/30 (cell 14): No function
- Pad 31/32 (cell 15): Toggle HID keyboard mode

### Test menu
To enter the system test menu, double-tap FN2. This will mirror the on-screen controls, and additionally adds controls for the TEST and SERVICE buttons on a real cabinet. Double-tap FN2 again to leave this mode on the controller (note that this is not synced with the game exiting the menu!).

Cell 6 and 7 are the TEST button, and cell 8 and 9 are the SERVICE button.

To access the TEST and SERVICE buttons without entering the test menu, hold FN2 instead.

## Keyboard Mapping
When the HID keyboard is enabled, the following mapping (UMIGURI defaults) is used:

```
I8U7Y6T5R4E3W2Q1
9KMJNHBGVFCDXSZA
```

with the airs mapped as `0OLP,.`.

Please note that this is slightly different to stock firmware!
