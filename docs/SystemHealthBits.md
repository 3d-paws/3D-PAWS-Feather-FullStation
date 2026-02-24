### System Health Bits
[←Top](../README.md)<BR>
A register is maintained where its individual bits correspond to various state information and sensor state. This register is reported in each observation as a decimal number with tag id "hth".<BR>
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 120px; border: 1px solid black; padding: 10px;">
<pre>
Name       Bit   HEX  Decimal
PWRON      00001 0x1        1  Set at power on, but cleared after first observation
SD         00010 0x2        2  Set if SD missing at boot or other SD related issues
N2S        00100 0x4        4  Set when Need to Send observations exist
FROM_N2S   01000 0x8        8  Set in transmitted N2S observation when finally transmitted
RTC        10000 0x10      16  Set if RTC missing at boot
</pre>
</div><BR>
Example "hth" values reported. This is often what people actually need when decoding logs:
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 120px; border: 1px solid black; padding: 10px;">
<pre>
Decimal Meaning
1       Just powered on
3       Power on + SD issue (1 + 2)
5       Power on + N2S pending (1 + 4)
17      Power on + RTC missing (1 + 16)
20      RTC missing + N2S pending (16 + 4)
</pre>
</div>
<br>