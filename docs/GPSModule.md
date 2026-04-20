# GPS Module
[←Top](../README.md)<BR>
A GPS module can be added but not necessary. If partnered with a Feather M0 LoRa board, The GPS will be used to set the RTC clock at power on. Which means you do not need to enter a date and time on the the serial console to set the RTC. The GPS can be partnered with the Fether M0 WiFi also. The WiFi will first try to get Time from the Network. If that fails, it then will try the GPS.

Every 4 hours from power on, the RTC clock is updated. That is, if a valid time souce like GPS is available. 

Use the below GPS module
- [Adafruit Mini GPS PA1010D - UART and I2C - STEMMA QT](https://www.adafruit.com/product/4415)

## Wiring
The GPS module connects to the i2c buss. A command will put the GPS to sleep. A pulse to the GPS's Reset  pin is required to wake the GPS up.  Connect the GPS Reset pin to the Feather D5 pin.