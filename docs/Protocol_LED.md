# LED microcontroller communication protocol
The Host and LED controller are connected with a pair of wires that are on I2C-capable pins for each. They do not exclusively use I2C.

During the LED bootloader, PA10 and PA11 are read as digital inputs. If both are low, the LED bootloader enters programming mode. When the Host firmware starts, it reads FN2. If the button is depressed, the two lines are pulled low. Otherwise the Host begins operation as an I2C slave, pulling both lines high.

The Host operates as the slave, as the LED microcontroller writing to the LEDs is a timing-critical that cannot be interrupted.

## General comments
The TASOLLER has 48 LEDs on the ground slider (on two 24-LED PCBs) and 24 LEDs in each tower.

The 48 ground LEDs, while individually controllable by the LED firmware, are not exposed to the Host by this protocol. Instead, 31 "logical" LEDs are exposed. The 48th LED is obscured by the housing of the controller, and as such is always left unlit. Each cell has two LEDs and each divider has one, giving 16*2+15 = 31.

All RGB and HSV colours are transmitted as in the following structures:

```c
typedef struct grb {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} grb_t;

typedef struct hsv {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} hsv_t;
```

## Stock protocol
In the stock protocol, the Host exposes 256 single-byte registers. The LED microcontroller reads register 0 to get the command byte, then subsequently reads the appropriate number of registers for the given command.

Stock firmware implements two commands. `A5` is a basic LED write based on a number of parameters. `5A` writes raw RGB data to all LEDs. Factory-stock and mainland "custom" firmware implement slight variants of both.

### Basic write (factory) [`A5`]
```c
{
    uint8_t u8Cmd = 0xA5;
    uint32_t u32Ground;
    uint8_t u8TowerFill;

    uint8_t u8HueLeft;
    uint8_t u8HueRight;
    uint8_t u8HueGround;
    uint8_t u8HueGroundActive;

    struct {
        uint8_t bTowersOff: 1;
        uint8_t bGroundOff: 1;
        uint8_t bSeparators: 2;
        uint8_t Rsv: 1;
        uint8_t bKeyMode: 3;
    };
    uint8_t u8LedSpecial;
}
```

| Parameter           | Value                                                      |
| ------------------- | ---------------------------------------------------------- |
| `u32Ground`         | 32 bits indicating boolean state of every pad              |
| `u8TowerFill`       | Bits 0 through 5 indicate the state of the air sensors     |
| `u8HueLeft`         | Hue of the left tower                                      |
| `u8HueRight`        | Hue of the right tower                                     |
| `u8HueGround`       | Hue of the slider cells                                    |
| `u8HueGroundActive` | Hue of the slider cells when active, and dividers          |
| `bTowersOff`        | Disable tower LEDs                                         |
| `bGroundOff`        | Disable ground LEDs                                        |
| `bSeparators`       | Number of separating dividers                              |
| `bKeyMode`          | How to group cells when lighting them based on `u32Ground` |
| `u8LedSpecial`      | Performs specific functions as described below             |

`bSeparators` enumeration:
- `0`: Divider every 4 cells (creates 4 sections)
- `1`: Divider every 2 cells (creates 8 sections)
- `2`: Divider every 1 cell (creates 16 sections)
- `3`: No dividers

`u8LedSpecial` enumeration:
- `0x00`: Normal operation (all other values ignore ground data)
- `0x01`: [Stock] Uses colours. Bar 1 full (from right)
- `0x02`: [Stock] Uses colours. Bar 2 full (from right)
- `0x03`: [Stock] Uses colours. Bar 3 full (from right)
- `0x04`: [Stock] Uses colours. Bar 4 full (from right)
- `0x1X`: [Stock] All white with black gaps. X bars black (from right)
- `0x80`: [Stock] Flashes three times

All hues are divided by 5, such that 360° = `72`

### Basic write (mainland) [`A5`]
```c
{
    uint8_t u8Cmd = 0xA5;
    uint32_t u32Ground;
    uint8_t u8TowerFill;

    struct {
        uint8_t bInvertRainbow: 1;
        uint8_t bRainbowSeparator: 7;
    };

    uint8_t Rsv07[3];

    struct {
        bTowersOff: 1;
        bGroundOff: 1;
        Rsv: 6;
    };
    uint8_t u8LedSpecial;
}
```

| Parameter           | Value                                          |
| ------------------- | ---------------------------------------------- |
| `bDisplayRainbow `  | Displays a rainbow across the ground LEDs      |
| `bRainbowSeparator` | The number of separator LEDs to light          |
| `u8LedSpecial`      | Performs specific functions as described below |

Other parameters function as in factory firmware.

`bRainbowSeparator` will light separators from 1 through (x+1). Maximum value 14.

`u8LedSpecial` enumeration:
- `0x00`: Normal operation (all other values ignore ground data)
- `0x1X`: Rainbow with black gaps. X bars black (from right)
- `0x20`: Flashes three times

### RGB write (factory) [`5A`]
```c
struct {
    uint8_t u8Cmd = 0x5A;

    grb_t aGround[31];
    grb_t aTowerLeft[24];
    grb_t aTowerRight[24];
}
```

Writes raw RGB data to all LEDs. Colours are transmitted in GRB format.

### RGB write (mainland) [`5A`]
```c
struct {
    uint8_t u8Cmd = 0x5A;
    struct {
        bTowersOff: 1;
        bGroundOff: 1;
        u8LedSpecial: 6;
    } u8Config;

    grb_t aGround[31];
    grb_t aTowerLeft[24];
    grb_t aTowerRight[24];
}
```

Writes raw RGB data to all LEDs. Colours are transmitted in GRB format.

See "Basic write (mainland)" for documentation of `u8LedSpecial`.

## Custom firmware protocol
In our custom protocol packets larger than 256 bytes need transmitted. To retain compatibility with the stock protocol, the Host still only exposes registers with an 8-bit address. Register `FF` however is reserved. Reading from register `FF` will begin a multiple read starting at address `0` but continuing until the complete buffer has been transferred, which may be more than 256 bytes.

The RGB write packet is implemented as in stock factory firmware. The Basic write packet is not implemented.

The LED microcontroller polls the Host for a packet once approximately every millisecond. This should not be relied upon as a guarantee.

The following additional packets are implemented:

### RGB write (custom) [`5B`]

```c
struct {
    uint8_t u8Cmd = 0x5B;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    grb_t aGround[31];
    grb_t aTowerLeft[24];
    grb_t aTowerRight[24];
}
```

### HSV write [`5C`]

```c
struct {
    uint8_t u8Cmd = 0x5C;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    hsv_t aGround[31];
    hsv_t aTowerLeft[24];
    hsv_t aTowerRight[24];
};
```

### Mixed RGB and HSV write [`5D`]

This packet transmits RGB data for the ground slider, but HSV for the towers. The rationale here is that the game sends data in RGB for the slider, which we use, but for the towers we wish to continue using our custom lighting.

```c
struct {
    uint8_t u8Cmd = 0x5C;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    grb_t aGround[31];
    hsv_t aTowerLeft[24];
    hsv_t aTowerRight[24];
};
```

### Flash read [`5E`]

Read 32 bytes from flash at the provided offset. The LED microcontroller will respond to this packet by writing four bytes, starting at address 0 (in a single transfer).

```c
struct {
    uint8_t u8Cmd = 0x5E;
    uint32_t u32Offset;
};
```

### Enter LDROM [`5F`]

Instruct the LED microcontroller to enter LDROM. After transmission of this packet the Host microcontroller should transition to driving both I2C lines low if it wishes the LED bootloader to enter programming mode.

Alternatively, the Host may drive both I2C low at any time outside of an I2C transmission, and the LED microcontroller will enter LDROM. This check is performed before the LED microcontroller begins its I2C read, and as such occurs at the same frequency.

```c
struct {
    uint8_t u8Cmd = 0x5F;
};
```
