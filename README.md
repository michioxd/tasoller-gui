# TASOLLER Custom Firmware (Host MCU - Application ROM)

See [Development](./Development.md) for development information.

Yes, the code is a mess. I just wanted to get something onto git y'know :).

## Configuration
Hold FN2 for configuration. It is not the same as stock DAO.

- Pad 1/2 (cell 0): Left wing colour
- Pad 3/4 (cell 1): Ground colour
- Pad 5/6 (cell 2): Ground colour when pressed, and separator colour
- Pad 7/8 (cell 3): Right wing colour
- Pad 9/10 (cell 4): No function
- Pad 11/12 (cell 5): Toggle rainbow effect on/off
- Pad 13/14 (cell 6): Increase/decrease ground brightness
- Pad 15/16 (cell 7): Increase/decrease wing brightness
- Pad 17/18 (cell 8): No function
- Pad 19/20 (cell 9): No function
- Pad 21/22 (cell 10): No function
- Pad 23/24 (cell 11): No function
- Pad 25/26 (cell 12): Increase/decrease sensitivity
- Pad 27/28 (cell 13): No function
- Pad 29/30 (cell 14): Toggle HID keyboard mode
- Pad 31/32 (cell 15): Toggle IO4 emulation mode

## Calibration
Make sure no hands or objects are near the slider before starting calibration.

Hold FN1 for two seconds; the slider will flash red for a few seconds, then fill up with a blue bar.

The slider will then briefly flash green. After this, begin to rub your hands across the slider as much as possible! The cells will turn increasingly green; the greener you can get them the better. Once the dividers have turned green you can save your calibration by pressing FN2.

**Note:** Calibration is a separate process to adjusting the sensor sensitivity. It is recommended to re-calibrate after adjustment of sensitivity. For the best performance, run a re-calibration before every play session to account for changes in your room's temperature and humidity.
