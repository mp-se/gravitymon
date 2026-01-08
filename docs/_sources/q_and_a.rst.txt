Q & A
#####

I cant see any sensor values
----------------------------
When in WIFI setup mode the device will not initialize any sensors so no data will be available when in this mode.

If you are running in configuration mode there is a hardware scan feature under support that can be used to see what sensors are connected to the device.


My device is no going in to sleep after fully charged
-----------------------------------------------------
- Calibrate the device in the web interface
- Check the angle/tilt. If the device is reporting 90 degrees then its not going into sleep. Tilt the device and check if sleep works.
- Check in reported voltage of the battery in the web interface. If this is higher than 4.15V the device believes its being charged. In that case adjust the voltage factor under hardware so the voltage drops below 4.15V.
- Check if the `always on` option is activated in the web interface.

What does the flashing led mean?
--------------------------------

On an ESP32c3 or ESP32s3 there is a multicolor LED and the colors means the following:

- Blue = Configuration mode
- White = WiFi setup mode

On other devices there is only a single color led.

- Slow Blinking = Config Mode
- Solid = WiFi setup mode  

My device reports an invalid temperature
----------------------------------------
- The DS18B20 temperature sensor cannot be found and this is the default value reported in this case.
- Check the orientation of the sensor and the soldering.

Calibration error (unable to find a valid formula)
--------------------------------------------------
If you have issues to get a calibration formula. When a formula has been created the device always tries to validate the formula against the supplied values and if there is a to high deviation on any of the values then the formula will be rejected.

The image here shows such a case, one of the values is out of bounds. 

.. image:: images/qa_1.png
  :width: 500
  :alt: Calibration Error

To fix these this you can;

- remove the value from the list (setting the angle to zero will do that)
- change the advanced setting (Formula max deviation) to a higher value and save the values again. 

In the case above this parameter was changed from 1.6 SG to 4 SG and the formula was accepted. The deviation on this point was just above 3 SG. 

How can I filter data on influxdb without needing to know the time range
------------------------------------------------------------------------

You can use any of the available fields to enter your custom data. An option is to use the token fields to 
add some custom information to identify your brew. This can then be used to filter your data in influxdb.

When you switch brews you need to go in and change the token to identify the brewing session.

Change the format template for the influx target to include the token field. Now you will have an 
field called event that you can filter on in influx.

.. code-block::

  gravity-format=${gravity-unit} gravity=${gravity},corr-gravity=${corr-gravity},
  angle=${angle},temp=${temp},battery=${battery},rssi=${rssi}

to

.. code-block::

  gravity-format=${gravity-unit} gravity=${gravity},corr-gravity=${corr-gravity},
  angle=${angle},temp=${temp},battery=${battery},rssi=${rssi},event=${token}


Device only reporting data once
-------------------------------

If the device does not wake up from a deep sleep, this is most likely a hardware problem,
check the resistor / diode between D0 and RST. Wrong value can stop the device from wakeing up.


What does the messages in the error log mean
--------------------------------------------

* Not enough values for deriving formula

  To create a formula its required to have at least 3 measurements. 

* Error validating created formula. Deviation to large, formula rejected

  The device will try to create formulas with different complexities. It will try to 
  validate the formula using the supplied values. If the difference is more than 1.6 SG on any point
  the formula will be rejected. Check the entered values if they seams to be reasonable.

* No valid calibration values, please calibrate the device.

  The gyro needs to be calibrated at 90 degrees (flat). This is done on the configuration page.

* Low on memory, skipping push

  The arduino libraries sometimes leak memory, this only occurs when running in configuration mode. To avoid
  crashes the device will skip pushing data if the memory drops to much. Network connections seams to be connected
  to memory leaks. 

* Unable to set header, invalid value

  Check the format for your custom header. This means it has not a correct format.

HTTP error codes
----------------

All these errors are standard http error codes. This are the common ones;

*  400 - Bad request. Probably an issue with the post format. Check format in the format editor.
*  401 - Unauthorized. The service needs an token or other means to authenticate the device. 
*  403 - Forbidden. Could be an issue with token or URL. 
*  404 - Not found. Probably a wrong URL.

MQTT error codes
----------------

MQTT connection errors:

* -1 - Connection refused
* -2 - Send header failed
* -3 - Send payload failed
* -4 - Not connected
* -5 - Connection lost
* -6 - No stream
* -7 - No HTTP server
* -8 - Too little RAM available
* -9 - Error encoding
* -10 - Error writing to stream
* -11 - Read timeout
* -100 - Endpoint skipped since its SSL and the device is in gravity mode

MQTT push on topic errors:

* -1 - Buffer to short
* -2 - Overflow
* -3 - Network failed connected
* -4 - Network timeout
* -5 - Network read failed
* -6 - Network write failed
* -10 - Connection denied
* -11 - Failed subscription

WIFI error codes
----------------

* 1 - No SSID found.
* 4 - Connection failed. 
* 5 - Connection lot.
* 6 - Wrong password.
* 7 - Disconnected by AP.

I have a hidden WiFi SSID
-------------------------

Its not recommended to use hidden SSIDs with the ESP boards, but it will work. However the WiFi scanning will not be able to 
detect the hidden SSID so you have the following options:

* Temporarily make the SSID visible and connect the device to it.
* Use the wifi configuration tool in the web flasher which allows you to enter any SSID/password combination.
