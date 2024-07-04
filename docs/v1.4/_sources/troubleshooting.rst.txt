.. _troubleshooting:

Troubleshooting
###############

Errors in the logfile
+++++++++++++++++++++

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

* Influxdb push failed response
* HTTP push failed response

  All these errors are standard http error codes. This are the common ones;

  *  400 - Bad request. Probably an issue with the post format. Check format in the format editor.
  *  401 - Unauthorized. The service needs an token or other means to authenticate the device. 
  *  403 - Forbidden. Could be an issue with token or URL. 
  *  404 - Not found. Probably a wrong URL.
  
  In some cases there can be negative error codes which have the following meaning:

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

* MQTT push on <topic> failed error

  * -1 - Buffer to short
  * -2 - Overflow
  * -3 - Network failed connected
  * -4 - Network timeout
  * -5 - Network read failed
  * -6 - Network write failed
  * -10 - Connection denied
  * -11 - Failed subscription

* WIFI Connection errors
  
  * 1 - No SSID is available / found
  * 4 - Connection failed
  * 5 - Connection lost
  * 6 - Wrong password
  * 7 - Disconnected, failed to establish a connections

ESP8266 wifi compatibility
++++++++++++++++++++++++++

The ESP has limited support for modern wifi standards, if you have problems to connect then consider the following:

* Require 2.4Ghz frequency (802.11 b/g/n modes)
* No space in SSID or password
* SSID must be visible (not hidden)
* Channels supported must be between 1-13
* Channels between 20-25 Mhz wide bands
