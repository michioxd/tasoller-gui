# PSoC communication protocol
Commands to and from the PSoC follow the structure

```c
{
    uint8_t bCommand;
    uint8_t bLength;
    uint8_t bData0;
    uint8_t bData1;
    uint8_t bChecksum;
}
```

`bChecksum` is the sum of the preceding four bytes.

For requests, `bLength` is always `2`, and this is validated aggressively on the PSoC.

**There is no synchronisation mechanism** and the existing PSoC implementation is brittle. If sync is lost between the PSoC and Host this is unrecoverable without a reset.

If the appropriate debug flags have been set on the PSoC, it will transmit `00` before calling `CapSense_Start(); CapSense_InitializeAllBaselines()`, and `FF` afterwards. These bytes do not follow the standard packet structure.

The PSoC transmits `C1` once when it has completed its first initialisation. This does not follow the standard packet structure. There is no mechanism to hard reset the PSoC, so if the Host resets for any reason other than a hardware power cycle, this byte will not be observed.
