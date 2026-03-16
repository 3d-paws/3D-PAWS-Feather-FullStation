# Voltaic Battery Voltage
[←Top](../README.md)<BR>
The Voltaic Battery provides a battery cell voltage breakout on the USB-C port.

Newer V25/V50/V75 firmware (post-2023) moved this cell voltage signal to SBU pins (A8/B8) on USB-C for better USB
compliance. Older units used the D+ pin. Which conflicted with data transfer. Tying A8/B8 SBU pins together ensures compatibility regardless of USB-C cable orientation (flipped or not).

### Voltaic Battery Voltage Information
[Updated USB-C PD and Always On for V25, V50, V75 Batteries](https://blog.voltaicsystems.com/updated-usb-c-pd-and-always-on-for-v25-v50-v75-batteries/)

[Reading Charge Level of Voltaic USB Battery Packs](https://blog.voltaicsystems.com/reading-charge-level-of-voltaic-usb-battery-packs/)

### USB-C Breakout Parts
[Adafruit USB Type C Breakout Board - Downstream Connection](https://www.adafruit.com/product/4090)

[USB C Male to Male Cable](https://www.amazon.com/dp/B0DL698HQZ)

[USB C Male to Male Adapter](https://www.amazon.com/AreMe-Adapter-Coupler-Extender-Thunderbolt/dp/B0BN3LKKPQ)

### Notes
- Voltaic reports half the cell voltage.
- The expected voltage range on Voltaic V25's monitor pins (D+ on older firmware, SBU1 (A8) and SBU2 (B8) on newer) is 1.6V to 2.1V

### Enabling Feature
To enable reading and reporting of Voltaic voltage, configure OP2 pin. Use DoAction OP2VBV which creates file OP2VBV.TXT.
Voltaic voltage and percent charge will be reported as (vbv and vpc).

### Voltaic Cell Percent Charge
```
Full charge:  4.2V cell →  2.1V  on SBU (100%)  
75% charge:  ~3.9V cell → ~1.95V on SBU  
50% charge:  ~3.7V cell → ~1.85V on SBU  
25% charge:  ~3.4V cell → ~1.7V  on SBU  
Empty:        3.2V cell →  1.6V  on SBU (0%)
```


