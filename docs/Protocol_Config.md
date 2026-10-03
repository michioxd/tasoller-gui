# HOST configuration API 1.3

CDC framing is `FF command length payload checksum`; the unescaped sum is zero
modulo 256. After sync, escape `FF` as `FD FE` and `FD` as `FD FC`.
All multibyte values are little-endian. Requests start with API major `01`.
Responses echo the command and start with `[01, status]`; errors have no data.
Statuses: 0 OK, 1 length, 2 version, 3 value, 4 physical UI busy, 5 storage failure.
Length/version/complete value validation precedes the busy check.

| Command | Request payload | Successful response payload |
| --- | --- | --- |
| F2 GET_INFO | `[01]` | `[01,00]` + 16-byte identification |
| F3 GET_CONFIG | `[01]` | `[01,00]` + unchanged 14-byte config |
| F4 SET_CONFIG | `[01]` + 14-byte config | `[01,00]` |
| F5 SAVE_CONFIG | `[01]` | `[01,00]` |
| F6 RESET_CONFIG | `[01]` | `[01,00]` |
| F7 GET_KEYMAP | `[01]` | `[01,00]` + 40 HID usages |
| F8 SET_KEYMAP | `[01]` + 40 HID usages | `[01,00]` |
| F8 SET_KEYMAP (grouped) | `[01, mode, divider]` + 40 HID usages | `[01,00]` |
| F9 GET_INPUT | `[01]` | `[01,00]` + 32 processed pad bytes + buttons byte |
| FA GET_MODES | `[01]` | `[01,00,mode,divider]` |
| FB GET_PROFILE | `[01, mode]` | `[01,00]` + 32 pad usages |
| FC SET_PROFILE | `[01, mode]` + 32 pad usages | `[01,00]` |

Identification: API minor `03`, firmware `01 03 00` (1.3.0), capabilities
`7F 00 00 00` (bits 0 config, 1 verified save, 2 reset, 3 keymap, 4 input preview,
5 grouped keyboard/divider modes, 6 independent persistent profiles),
then eight ASCII bytes `TAS-HOST`.

The 14-byte config remains: schema `01`, flags (bit 0 keyboard, bit 1 rainbow),
sensitivity 1–16, reserved zero, four u16 hues 0–359 (tower left/right,
ground/active), ground brightness and tower brightness (0–255).

Keymap slots 0–31 follow `gu8GroundData`/`gu32PSoCDigital` order, not a visual
reordering. Slots 32 FN2, 33 FN1, 34–39 AIR1–6 correspond to buttons bits 0–7.
Usages must be zero (unassigned) or ordinary keyboard usages `04`–`A4` inclusive.
Error usages `01`–`03`, reserved `A5`–`DF`, modifiers `E0`–`E7`, and higher values
are rejected. Duplicates are allowed and deduplicated in HID reports.
Internal FN actions are unchanged. Keyboard/map changes send release-all followed
by a forced refresh of held inputs when enabled; USB reconnection is unnecessary.

Mode values: `0=32k, 1=16k, 2=9k, 3=8k, 4=4k, 5=2k`.
Divider values: `0=none, 1=single, 2=all, 3=2k, 4=4k, 5=8k, 6=9k`.
The legacy 41-byte F8 request sets mode 0 and preserves the divider. The 43-byte
request validates the complete map and both enums before updating map/mode/divider
together in the main loop. Every pad in a group must have the same usage (including
zero); mode 0 has independent pads. AIR/FN slots remain independent. Invalid lengths,
usages, enums or inconsistent groups apply nothing. F7 still returns only 40 usages;
FB reads any bank without switching modes. FC validates length, mode, usages and
all groups before changing that bank; it refreshes the active map only when the
selected bank is active. FC never changes shared keys, divider or active mode.
F8 saves the outgoing active pads into their bank, loads the target bank, then
replaces only that target's pads and the shared eight keys with its payload.
GET_MODES and FB remain available during the physical UI; FC returns busy.

Visual cells below are numbered **1–16 left to right**, unlike native input order:
cell 1 contains pads 30/31, cell 16 pads 0/1. Group layouts:

| Mode | Visual groups |
| --- | --- |
| 32k | Each of the 32 pads independently |
| 16k | Each cell (two pads) |
| 9k | 1; 2–3; 4–5; 6–7; 8–9; 10–11; 12–13; 14–15; 16 |
| 8k | 1–2; 3–4; 5–6; 7–8; 9–10; 11–12; 13–14; 15–16 |
| 4k | 1–4; 5–8; 9–12; 13–16 |
| 2k | 1–8; 9–16 |

Local keyboard lighting uses the divider selection: single and 2k light logical
index 15; all lights every odd index 1–29; 4k lights 7,15,23; 8k lights
3,7,11,15,19,23,27; 9k lights 1,5,9,13,17,21,25,29. The latter separates
after visual cells 1,3,5,7,9,11,13,15 (the layout is reflection-symmetric).
None disables persistent separators, retaining reactive lighting. These predicates
replace only the local persistent-divider rule when keyboard output is enabled;
game-controlled and menu LEDs remain unchanged, as does non-keyboard 4k lighting.

## Physical FN1 settings

Holding FN1 keeps the original settings menu. Double-tapping FN1 opens the keyboard
subpage directly. The original menu's previously unused visual **cell 14** (blue)
returns to that subpage. Existing colour, lighting, brightness, consumer, sensitivity and keyboard
toggles remain in their original cells. On the keyboard subpage:

| Visual cells (1-based) | Assignment |
| --- | --- |
| 1–6 (blue) | 32k, 16k, 9k, 8k, 4k, 2k respectively |
| 7 | Unused |
| 8–14 (green) | none, single, all, 2k, 4k, 8k, 9k respectively |
| 15 (white) | Return to original settings |
| 16 (red/green) | Existing keyboard disable/enable toggle |

Selected choices are bright; others are dim. Divider choices are dark and ignore
touches when keyboard output is disabled. Touch rising edges select choices,
using native cell mask `1 << (16 - visual_cell)`. Physical mode selection stores
the outgoing RAM pads and loads the selected independent bank (`Keymap_Switch`).
It never normalizes an existing bank; AIR/FN are shared and unchanged. A saved
4k ADHI mapping returns exactly after switching away and back.
Double-tap FN1 again to exit and persist using the existing physical-save path;
holding FN1 always renders the original menu. Internal FN actions are unchanged.

SET commands and RESET affect RAM only; RESET restores all six default banks, mode 0
and divider 4. SAVE includes map and modes and verifies flash readback. Mutations/SAVE return busy
while the physical UI is open; reads remain available. GET_INPUT returns the
current processed values even during that UI, without changing legacy reports.

Storage magic `54617305` retains the active 40-byte map and appends six 32-byte banks:
boot flag offset 144, map offset 145, mode offset 185, divider offset 186;
profiles offset 187; the packed, 4-byte-aligned struct is **380 bytes**, statically
asserted to fit the existing 512-byte page. AIR/FN remain in active slots 32–39.
Magic `54617304` preserves its custom active bank, mode/divider, shared keys,
calibration and boot flags. Other banks start from the original default map
normalized separately for each mode, never from the custom active map.
Magic `54617302` retains
settings, calibration and boot flags, supplying the original default map;
magic `54617303` preserves its map. Both supply mode 0/divider 4 in RAM only.
Invalid maps reset to the original map; invalid modes/grouping fall back to mode 0
and divider 4. Migration does not request an automatic write;
explicit SAVE or the existing physical-save path persists it. Invalid/uninitialized
storage retains the existing defaults-initialization behavior. Persistence remains
confined to the existing data-flash page at `1F000`; LDROM and CONFIG are untouched.
Saving is not power-loss atomic. Never automatically retry a write with unknown outcome.
