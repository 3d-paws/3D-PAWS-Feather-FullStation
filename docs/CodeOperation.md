# Code Operation Notes
[←Top](../README.md)<BR>
### Timing
The main loop runs then calls the background function. This function provide a 1 a second delay for the main loop and performs the needed background work.

- Takes the 1 second readings
  - Distance
  - Wind
  - Air
- Makes sure we are WiFi connected 
- Does a Watchdog heartbeat
- Checks for LoRaWAN callbacks
- Turns off the LED if on from a rain gauge tip

In the main loop. Based on what you have configured for observation timing. Observations are performed and then transmitted. Other tasks performed are:

- ### Time Management
  A valid time source is required for normal operation and for observations to be made. The RTC clock is used for all date and time. The system clock is not used.

  If at startup the RTC is found at initialization and has a year greater equal to 2026 and less than or equal to 2035. We assume time on the RTC is valid.

  If the RTC is not found. The code will hang at startup.

  If the RTC is found and does not have valid date (Invalid RTC mode). The code will wait for user input. Prompting on the serial console for date and time.

     SET RTC w/GMT, ENTER: 
     YYYY:MM:DD:HH:MM:SS

  While waiting for user to enter a valid time and date on the serial console, The WiFi network and GPS and asked for date and time.  

  After setting the RTC from one of the above methods, the board will reboot.

- ### Realtime Clock (RTC) periodic refresh.
  The main code loop will update the RTC from WiFi network time or from a GPS if attached. If this update by some odd chance invalidates the RTC; the code will revert back to the invalid RTC mode and try to obtain date and ime from the WiFi network or GPS unit. It will stay in this mode until a valid date and time or it hits the 22 hour daily reboot.

- ### Daily Reboot
  A loop counter is maintained and set so around every 22 hours the system reboots itself. This is done to try and bring back online any missing sensors.  If a WatchDog board is not connected a System reset is performed. The reboot will generate a INFO to be sent.

### Transmittion Failure Handling
If it detected that there was a transmission failure. The failed message is appended to the Need to Send (N2S) file located on the SD card at the top level and called N2SOBS.TXT. If the file does not exist, it is created then appended to. These information and observation messages will later be transmitted.

Caveat: At the time of appending the observation to the N2S file, if the file is greater than 512 * 60 * 48 bytes. ~2 days of observations. The file is deleted. Recreated and the observation is appended.

### Sending N2S Messages
After successful completion of transmitting current observation. If N2S file exist, entries are read from the file and transmitted.  If the entry is a INFO message and board is a WiFi, the INFO message will be sent to the information server.

If the eeprom is connected to the i2c bus. The position in the N2S file of what has been transmitted is maintained in nvram / eeprom.  So rebooting does not cause the retransmission of N2S observations. If the eeprom does not exist. Rebooting will cause the N2S observations to be resent.

On a N2S observation transmit failure, N2S processing stops until the next observation window.  

While sending N2S entries, 1 second tasks and 1 minute observations still occur.

We stop sending N2S entries 45 seconds before the next observation is do. Unless a Feather LoRa board is in use. We then have to delay 30 seconds between LoRaWAN transmissions. So we forget about stopping before the next observation period and just send N2S entries for 5 minutes

When storage space for 1 minute observations becomes full. N2S processing stops and the main work loop takes over.  At which point current 1 minute observations will be transmitted. Freeing up the observation storage.