# WatchDog Board
[←Top](../README.md)<BR>
The WatchDog is a external device that can turn off power to your weather station's microcontroller and sensors. Power remains off for 10 seconds. Ways the WatchDog can be triggered to turn off power
- If the weather station microcontroller's heartbeat pulse stops for 5 minutes.
- If a trigger pulse is sent from the weather station's microcontroller to the WatchDog.
- Weather station software does a daily reboot and will send a trigger pulse.

For more information see GutHub Site [3d-paws/3D-PAWS-WatchDog](https://github.com/3d-paws/3D-PAWS-WatchDog/blob/master/README.md).