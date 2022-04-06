Q & A
#####

My device is no going in to sleep after fully charged
-----------------------------------------------------
- Calibrate the device in the web interface
- Check the angle/tilt. If the device is reporting 90 degress then its not going into sleep. Tilt the device and check if sleep works.
- Check in reported voltage of the battery in the web interface. If this is higher than 4.15V the device belives its beeing charged. In that case adjust the voltage factor under hardware so the voltage drops below 4.15V.
- Check if the `always on` option is activated in the web interface.

My device reports a temperature of -273C or -491F
-------------------------------------------------
- The DS18B20 temperature sensor cannot be found and this is the default value reported in this case.
- Check the orienation of the sensor and soldering.
