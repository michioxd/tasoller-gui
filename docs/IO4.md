# SEGA IO4 Protocol
IO4 is SEGA's latest (as of 2024) iteration of their JAMMA IO boards. As this board can be populated with either a USB port or a JVS 2 port, or both, multiple versions of it exist. This document focuses exclusively on the USB implementation.

The IO4 exposes itself as a USB HID device. It has a single pair of HID endpoints, with an 8ms interval for IN and 1ms interval for OUT. The complete descriptors are included at the end of this document.

The protocol described here is not explicitly unique to IO4; it is SEGA's `UsbIO` protocol.

## USB Product Strings
UsbIO devices identify themselves using the USB product string. There are 8 fields are separated by `;`:

| Field              | Content                                             |
| ------------------ | --------------------------------------------------- |
| Board type         | Up to 14 characters                                 |
| Board number       | Up to 8 characters, padded with spaces on the right |
| Mode               | Hexadecimal value up to FF                          |
| Firmware revision  | Hexadecimal value up to FF                          |
| Firmware checksum  | Hexadecimal value up to FFFF                        |
| Custom chip number | Up to 5 characters, padded with spaces on the right |
| Config             | Hexadecimal value up to FF                          |
| Features report    | _See below_                                         |

The features report contains multiple entries, separated by `_`. Each entry is of the form `NAME=value` where `value` is a comma-separated list of one or more items. All numeric values are in hexadecimal unless otherwise specified.

| Feature name | Usage                      | Arguments                                        |
| ------------ | -------------------------- | ------------------------------------------------ |
| `GOUT`       | General purpose output     | Number of digital outputs                        |
| `PWMOUT`     | PWM output                 | Number of PWM outputs                            |
| `ADIN`       | ADC inputs                 | Number of ADC inputs; bit depth (eg 14)          |
| `ROTIN`      | Rotary input               | Number of rotary inputs                          |
| `COININ`     | Coin counter               | Number of coin counters                          |
| `SWIN`       | Switch input               | Number of players; Number of switches per player |
| `UQ<n>`      | Unique IO function n (1-9) | Command ID; 0 to 3 additional arguments          |

## IO4 -> Host (report ID 1)
Every 8ms, the Host initiates an interrupt transfer for HID. This is always responded to with report ID 1.

The structure of this report is described in the descriptor, and matches the below structure:

```c
struct {
    uint8_t bReportId = 1;
    uint16_t wADC[8];
    uint16_t wRotary[4];
    uint16_t wCoin[2];
    uint16_t wButtons[2];
    struct {
        uint8_t bResetReason : 4;
        uint8_t bTimeoutSet : 1;
        uint8_t bSampleCountSet : 1;
        uint8_t Rsv : 2;
    } BoardStatus;
    struct {
        uint8_t Rsv : 2;
        uint8_t bTimeoutOcurred : 1;
        uint8_t Rsv : 5;
    } UsbStatus;
    uint8_t bUnique[29];
}
```

`bUnique` is always written as 29 null bytes.

It is critical that `bTimeoutSet` and `bSampleCountSet` are updated correctly, as amdaemon validates these bits.

When a timeout occurs, `bTimeoutOcurred` is set. While the first two bits of `UsbStatus` are used internally on the IO4 board, they are masked out before transmission.

`bResetReason` is a bit field:
- `1`: Power-on reset
- `2`: Pin reset
- `4`: Watchdog timer reset
- `8`: Voltage monitoring reset (either Vdet1 or Vdet2)

## Host -> IO4 (report ID 16)
Packets to IO4 are always 64 bytes, the first of which is a command byte.

```c
enum {
    IO4_Command_SetCommTimeout = 1,
    IO4_Command_SetSamplingCount = 2,
    IO4_Command_ClearBoardStatus = 3,
    IO4_Command_SetGeneralOutput = 4,
    IO4_Command_SetPwmOutput = 5,
    IO4_Command_SetUniqueOutput = 0x41,

    IO4_Command_84 = 0x84,
    IO4_Command_UpdateFirmware = 0x85,
    IO4_Command_88 = 0x88,
};
```

### Set Communication Timeout [`01`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 1;
    uint8_t bTimeout;

    uint8_t Rsv[61];
}
```

Sets the timeout value for the IO board. I'm not totally sure of the unit currently, but work on the assumption it's multiples of 200us.

Set `bTimeout` to `0` to disable the timeout.

When timeout occurs, the IO board performs a soft reset.

### Set Switch Sampling Count [`02`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 2;
    uint8_t bSamplingCount;

    uint8_t Rsv[61];
}
```

Set the number of samples required in the debouncer for an input pin to be counted. The easiest way to understand this value is to study the IO4's debouncing functionality below.

```c
#define NUM_INPUT 32

uint8_t counter[NUM_INPUT] = { 0 };
uint8_t samplingCount[NUM_INPUT] = { 0 };
uint8_t debounced[NUM_INPUT / 8] = { 0 };

void Debouncer_Tick(uint8_t* sampledData) {
    uint8_t bitMask = 0;
    uint8_t bitsDiffer = 0;
    for (int i = 0; i < NUM_INPUT; i++) {
        if (i % 7 == 0) {
            bitsDiffer = sampledData[i / 8] ^ debounced[i / 8];
            bitMask = 0x01;
        }

        if (!(bitsDiffer & bitMask)) {
            // If the input doesn't differ, reset our counter
            counter[i] = 0;
        } else {
            if (counter[i] < samplingCount[i]) {
                counter[i]++;
            } else {
                // Toggle the bit in the output once we've seen it differ for enough counts
                counter[i] = 0;
                debounced[i / 8] ^= bitMask;
            }
        }

        bitMask <<= 1;
    }
}
```

### Clear Board Status [`03`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 3;

    uint8_t Rsv[62];
}
```

Unsets the timeout and sampling count values.

`bSystemStatus` and `bTimeoutOcurred` are reset to 0.

### Set General Output [`04`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 4;

    uint8_t bData[3];

    uint8_t Rsv[59];
}
```

Write 20 digital output values. 4 bits of padding are included.

### Set PWM Output [`05`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 5;

    uint8_t Rsv[62];
}
```

Write PWM duty cycles. As IO4 has no PWM outputs this packet is empty.

### Set Unique Output [`41`]
```c
struct {
    uint8_t bReportId = 16;
    uint8_t bCmd = 0x41;

    uint8_t bUnique[62];
}
```

Write 62 bytes of "unique" data. IO4 uses the first 8 bytes of these as follows:

| Byte | Usage                                        |
| ---- | -------------------------------------------- |
| 0    | Enable mask (bit 7=PWM1, 2=PWM6, 0~1 unused) |
| 1    | Brightness scaler (1~256, with 0=256)        |
| 2    | PWM1 brightness (pin CN3.55)                 |
| 3    | PWM2 brightness (pin CN3.56)                 |
| 4    | PWM3 brightness (pin CN9.5 )                 |
| 5    | PWM4 brightness (pin CN9.6 )                 |
| 6    | PWM5 brightness (pin CN9.9 )                 |
| 7    | PWM6 brightness (pin CN9.10)                 |

### Unknown 84 [`84`]
### Update Firmware [`85`]
### Unknown 88 [`88`]

## Hardware Pinout
This is all wrong lol

- [0], P60: 1P START
- [1], P61: 1P RIGHT
- [2], P62: 1P LEFT
- [3], P63: 1P UP
- [4], P64: 1P DOWN
- [5], P65: 1P PUSH1
- [6], P66 [NC] Service
- [7], P67 [NC]
- [8], P70 [NC]
- [9], P71 [NC] Test
- [10], P72: 1P PUSH2
- [11], P73: 1P PUSH3
- [12], P74: 1P PUSH4
- [13], P75: 1P PUSH5
- [14], P76: 1P PUSH6
- [15], P77: 1P PUSH7
-
- [16], PC0: 2P START
- [17], PC1: 2P RIGHT
- [18], PC2: 2P LEFT
- [19], PC3: 2P UP
- [20], PC4: 2P DOWN
- [21], PC5: 2P PUSH1
- [22], PC6 [NC]
- [23], PC7 [NC]
- [24], PE0 [NC]
- [25], PE1 [NC]
- [26], PE2: 2P PUSH2
- [27], PE3: 2P PUSH3
- [28], PE4: 2P PUSH4
- [29], PE5: 2P PUSH5
- [30], PE6: 2P PUSH6
- [31], PE7: 2P PUSH7

- P02: ? (GetPort0ButtonsB)
- P03: ? (GetPort0ButtonsB)


- PD?: ? (GetPortDButtons)

Used for coins:
- P00: ? (GetPort0ButtonsA)
- P01: ? (GetPort0ButtonsA)
- PE0: ? (inverted, GetPortEButtons)
- PE1: ? (inverted, GetPortEButtons)

```c
uint8_t GetPortEButtons(void) {
    uint8_t buttons = dat_portE >> 1 & 0xfd  // PE1
    buttons |= 0xfc
    buttons |= (dat_portE & 1) << 1;  // PE0

    return ~buttons;
}
```



## USB Descriptors
### USB Strings
| Index | Use          | String                                                                                             |
| ----- | ------------ | -------------------------------------------------------------------------------------------------- |
| 1     | Manufacturer | `SEGA INTERACTIVE`                                                                                 |
| 2     | Product      | `I/O CONTROL BD;BDNUMBER;MD;RV;SUM ;CPNUM;CF;GOUT=14_ADIN=8,E_ROTIN=4_COININ=2_SWIN=2,E_UQ1=41,6;` |

### Device Descriptor
`12 01 00 02 00 00 00 40 a3 0c 21 00 00 01 01 02 00 01`

| Field              | Value                                              |
| ------------------ | -------------------------------------------------- |
| bLength            | 18                                                 |
| bDescriptorType    | Device                                             |
| bcdUSB             | 2.00                                               |
| bDeviceClass       | Use class information in the Interface Descriptors |
| bDeviceSubClass    | 0                                                  |
| bDeviceProtocol    | 0                                                  |
| bMaxPacketSize0    | 64                                                 |
| idVendor           | 0x0CA3                                             |
| idProduct          | 0x0021                                             |
| bcdDevice          | 2.00                                               |
| iManufacturer      | 1                                                  |
| iProduct           | 2                                                  |
| iSerialNumber      | 0                                                  |
| bNumConfigurations | 1                                                  |

### Configuration Descriptor
`09 02 29 00 01 01 00 c0 32`
| Field               | Value         |
| ------------------- | ------------- |
| bLength             | 9             |
| bDescriptorType     | Configuration |
| wTotalLength        | 41            |
| bNumInterfaces      | 1             |
| bConfigurationValue | 1             |
| iConfiguration      | 0             |
| bmAttributes        | Self powered  |
| bMaxPower           | 100mA         |

`09 04 00 00 02 03 00 00 00`
| Field              | Value     |
| ------------------ | --------- |
| bLength            | 9         |
| bDescriptorType    | Interface |
| bInterfaceNumber   | 0         |
| bAlternateSetting  | 0         |
| bNumEndpoints      | 2         |
| bInterfaceClass    | HID       |
| bInterfaceSubClass | 0         |
| bInterfaceProtocol | 0         |
| iInterface         | 0         |

`09 21 11 01 00 01 22 71 00`
| Field                | Value |
| -------------------- | ----- |
| bLength              | 9     |
| bDescriptorType      | HID   |
| bcdHID               | 1.11  |
| bCountryCode         | 0     |
| bNumDescriptors      | 1     |
| bDescriptorType[0]   | HID   |
| wDescriptorLength[0] | 113   |

`07 05 81 03 40 00 08`
| Field            | Value     |
| ---------------- | --------- |
| bLength          | 7         |
| bDescriptorType  | Endpoint  |
| bEndpointAddress | IN/D2H    |
| bmAttributes     | Interrupt |
| wMaxPacketSize   | 64        |
| bInterval        | 8ms       |

`07 05 01 03 40 00 01`
| Field            | Value     |
| ---------------- | --------- |
| bLength          | 7         |
| bDescriptorType  | Endpoint  |
| bEndpointAddress | OUT/H2D   |
| bmAttributes     | Interrupt |
| wMaxPacketSize   | 64        |
| bInterval        | 1ms       |

### HID Descriptor
`05 01 09 04 a1 01 85 01 09 01 a1 00 09 30 09 31 09 30 09 31 09 30 09 31 09 30 09 31 09 33 09 34 09 33 09 34 09 36 09 36 15 00 27 ff ff 00 00 35 00 47 ff ff 00 00 95 0e 75 10 81 02 c0 05 02 05 09 19 01 29 30 15 00 25 01 45 01 75 01 95 30 81 02 09 00 75 08 95 1d 81 01 06 a0 ff 09 00 85 10 a1 01 09 00 15 00 26 ff 00 75 08 95 3f 91 02 c0 c0`

```
Usage Page (Generic Desktop Ctrls)
Usage (Joystick)
Collection (Application)
  Report ID (1)

  Usage (Pointer)
  Collection (Physical)
    Usage (X)
    Usage (Y)
    Usage (X)
    Usage (Y)
    Usage (X)
    Usage (Y)
    Usage (X)
    Usage (Y)
    Usage (Rx)
    Usage (Ry)
    Usage (Rx)
    Usage (Ry)
    Usage (Slider)
    Usage (Slider)
    Logical Minimum (0)
    Logical Maximum (65534)
    Physical Minimum (0)
    Physical Maximum (65534)
    Report Count (14)
    Report Size (16)
    Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  End Collection

  Usage Page (Sim Ctrls)
  Usage Page (Button)
  Usage Minimum (0x01)
  Usage Maximum (0x30)
  Logical Minimum (0)
  Logical Maximum (1)
  Physical Maximum (1)
  Report Size (1)
  Report Count (48)
  Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

  Usage (0x00)
  Report Size (8)
  Report Count (29)
  Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  Usage Page (Vendor Defined 0xFFA0)
  Usage (0x00)


  Report ID (16)

  Collection (Application)
    Usage (0x00)
    Logical Minimum (0)
    Logical Maximum (255)
    Report Size (8)
    Report Count (63)
    Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  End Collection
End Collection
```