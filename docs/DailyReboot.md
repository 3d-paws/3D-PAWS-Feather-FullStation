# Daily Reboot
[←Top](../README.md)<BR>
The weather station will perform a daily reboot. This will be a hard reboot if a Watch Dog board is connected. The reboot occurs some time after 22 hours from station power on. The reboot will hopefully clear any issues with the station.
- A count down timer is set to 79200 at boot. The main Loop() decrements this once per second / loop cycle.
- An exact time after power on can not be determined. Because overhead of initialization, taking and transmitting observations adds time. \
  So sometime after 22 hours from power on is the best guess.
- The solution varies when a station reboots. If many stations are deployed this avoids them all rebooting at same time. 
- The daily reboot will result in a INFO message being sent.