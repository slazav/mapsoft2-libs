JNX files for Garmin GPS

- Here I'm working with v3 jnx files, same as Demo.JNX on me Garmin etrex 22x device.
Not with v4 which is produced by nakarte.me (they are different).

- I believe that I can understand file structure. There is read_jnx program for
printing file structure and extracting all jpeg files from jnx. It will
not work with v4. (TODO: some safety checks to prevent reading/printing
random amount of random data).

- There is a program for converting mbtiles to jnx, mbtiles2jnx.
It's not a user-friendly version, but just a playground for me.

- Firmware patch is needed for the gps to allow using custom jnx files.
Otherwise "invalid jnx file" message is shown on startup. I do not have
the patch yet.

- If gps device contains broken jnx file, it can fail to boot and
connect to usb. Device reset will not help in this case. The useful fix
for etrex 22x: switch off device; press joystick to up position, connect
usb. The device will be available as mass storage and the bad map can be
removed. The better way is to use a separate sd card for suspicious maps.
