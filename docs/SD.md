# SD Card Information
[←Top](../README.md)<BR>
## Supported Card Sizes
- **Size:** 32 GB or smaller  
  *Note: Larger SD cards will not be detected.*

---

## Formatting Tool
- **Recommended Tool:** SD Memory Card Formatter by Tuxera [https://www.sdcard.org/downloads/formatter/](https://www.sdcard.org/downloads/formatter/)  
- **Format Type:** MS-DOS (FAT32)

## File and Directory Overview

| File/Directory    | Purpose                                                                                 |
|-------------------|-----------------------------------------------------------------------------------------|
| `/OBS/`           | Directory containing observation files.                                                 |
| `/OBS/20231024.LOG` | Daily observation file in JSON format (one file per day).                             |
| `/N2SOBS.TXT`     | "Need to Send" file storing unsent observations. Resets if larger than specified size.  |
| `/CONFIG.TXT`     | [Configuration file.](Configuration.md)                                                 |
| `/INFO.TXT`       | [Station info file.](INFO.md) Created / overwritten with every boot and rewritten every 6 hours from boot.             |
| `/ELEV.TXT`       | Station elevation. File contents is elevation in meters.                                |
| `/CRT.TXT`        | Clear rain totals from EEPROM at boot. Then file is deleted from SD card                |