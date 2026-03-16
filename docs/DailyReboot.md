# Daily Reboot
[←Top](../README.md)<BR>
The weather station will perform a daily reboot. This will be a hard reboot if a Watch Dog board is connected. The reboot will hopefully clear any issues with the station. The reboot occurs based on the setting from the configuration file.

```
# Number of hours between daily reboots
# A value of 0 disables this feature
daily_reboot=22
```

We chose 22 hours from station power on. From station power on provides a random time so all stations deployed are not rebooting at the same time.  The 22 hours will move the reboot around the clock dial to not reboot the same time each day.