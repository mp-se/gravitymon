.. _releases:

Releases 
########

v1.2.0
======

Features
++++++++
* Added function to calcualate voltage factor based on measured value.
* Updated battery estimation for the various esp32 boards.
* Added support for the ESP32 lite board which is used in the Floaty Hydrometer variant.
* Added support for the ESP32 C3 mini board
* Added support for the ESP32 S2 mini board
* Serial output is written to TX/RX pins instead of the USB connection for the ESP32c3. This way the serial console can be viewed when running on battery power.
* Merged in unit tests and api tests into this project

Documentation
+++++++++++++
* Added section about the Floaty hardware
* Fixed schema errors in hardware section and linked PCB options
* Updated hardware section with options for ESP32 boards
* Updated installation instructions.

Issues adressed
++++++++++++++++
* BUG: The first portion of a format template was lost when doing conversion. 

v1.1.1
======
* BUG: The text before the first variable was missed in the conversion of a format template.

v1.1.0
======

Features
++++++++
* Added information to error log about abnormal resets (for instance crashes) to detect and fix those
* Changed storage mode so that the device will go into deep sleep until reset (sleep forever)
* Updated sensor types in home assistant for auto registration of device
* Added ${app-ver} and ${app-build} to format template as new variables
* Improved error messages when creating formula so the troublesome measurement points can be identified
* Changed default validation threshold from 1.6 SG to 3.0 SG, this should allow for some more variance when creating formula
* Updated format template for Home Assistant, aligned with new mqtt configuration format
* Added format template for Home Assistant with automatic device registration
* Added storage mode which is activated under hardware setting. When place on the cap (<5 degree tilt) the device will go into storage mode and deep sleep. 

Known issues, not yet fixed
+++++++++++++++++++++++++++
* When updating firmware and the feature `deep sleep` is active the device will activate deep sleep if the gyro is not responding. FIX: Reboot device

Issues addressed
++++++++++++++++
* Refactored error logging function to reduce memory usage and crashes. Max size of error log is 2 x 4 kb
* Refactored format template engine to reduce memory usage and crashes, can how handle slightly larger payloads than before. Increase from around 1100 chars to 1600 chars
* BUG: Refactored format api to handle larger payloads
* BUG: After manual firmware upload the device would crash and go into wifi setup mode.
* BUG: After manual firmware upload the device will in some cases not be able to connect with the gyro, the symptom is that it will say, "Gyro moving" in the web UI. In this case the device needs to be reset (or powered on/off). I havent found a way to fix this from the code. The message after firmware update has been updated with this information
* BUG: Temp corrected gravity was not used when pushing data to removed
* BUG: Low memory in format api which resulted in mqtt template to be set to null
* BUG: Large format templates could be saved but when loading it's only blank
* BUG: Copy format templates used an old format for iSpindle and Gravmon where the token was not used
* BUG: Gravity correction formula not calculating correctly

User interface
++++++++++++++
* Updated format template with information on size and warning message if the template is too large
* Added error message if gyro connection/initialization fails (before the message was Gyro Moving only)
* Added error message if no temp sensor can be found
* Added drop down menus in user interface to simplify navigation to sub pages (format, test and upload)
* Added Assistant Device registration, this is only done when format template is saved, during normal operation only data values are posted on MQTT. If HA is restarted then the device will disappear
* Calibration temperature (for temp adjustment) can now be set under advanced settings, default is 20C
* Changed length of device name from 12 to 63 chars. 63 is the max limit according to mdns.
* Under format options its now possible to select brewfather iSpindle format to avoid errors connected to using the wrong format template with the various brewfather endpoints
* Added brewblox as format under format options
* Added home assistant (with device registration) as format under format options
* User can now edit the voltage level that forces the device into config mode (device detects charging)

Documentation
+++++++++++++
* Added documentation for Brewpiless as target
* Added documentation for BVrewblox as target
* Updated documentation for HA integration since described method was deprecated
* Updated documentation for ubidots service integration
* Updated documentation in data format section
* Updated hardware section with documentation on installing reed switch
* Updated configuration section with documentation on new settings
* Added q&a on formula creation and value deviation
 
Other
+++++
* Upgraded framework for ESP8266 to v5.0.0
* Upgraded framework for ESP32 to v2.0.2
* Updated OneWire library to be complaint with new ESP32 SDK
* Fixed issue in i2cdev connected to wrong usage of TwoWire on ESP32 (Gyro initialization hang). 


v1.0.0
======

Documentation
+++++++++++++
* Update documentation to match v1.0
* Installation instructions updated on how to find the device after wifi has been configured. 
* Documentation on brewfather has been updated to address SG/Plato conversion
* Added circuit diagram for esp8266 and esp32
* Added additional http error codes to troubleshooting documentation

User interface
++++++++++++++
* Upgraded to bootstrap v5.1 for web pages.
* Added button on index page to direct to github issues.
* Added button to extract important information for support requests. 
* First point in gravity formula is now reserved for water gravity, this to allow detection of angles below water that can be filtered out.
* Changed layout on index page with measured data on top.
* Added tooltips to all fields in user interface
* Added function on format page so that it's easy to copy a format template from the docs (simplify service integration).

Features
++++++++
* Added advanced setting to ignore angles that are lower than water. This is disabled by default.
* Added support for MPU6500 (standard is MPU6050).
* Removed brewfather option (can use standard HTTP options), the old approach can still be used via changing format template.
* Added 5 more points for formula creation, so a total of 10 angles/gravity values can be stored.
* Added https support for Influxdb v2
* Added possibility to set 2 wifi ssid where the second acts as a fallback in case it fails to connect. If successful the secondary becomes the new primary.
* SSL connections are skipped on ESP8266 when in config mode since there is a high probability it will crash due to low memory. 
* Advanced settings: Added possibility to have variable push intervals for different endpoints so that different frequency can be used, for example; 5min mqtt, 15min brewfather. 
* Advanced settings: Changes how many times the gyro is read (less reads, quicker but less accurate)
* Advanced settings: Set amount of gyro movement is allowed for a accurate read.
* Advanced settings: What deviation is acceptable for creating formula deviation
* Advanced settings: Various timeouts, wifi connect, wifi portal, http connects.
* Advanced settings: Adjust resolution of temp sensor (9 bits to 12 bits), higher resolution takes longer thus reducing battery life

Issues addressed
++++++++++++++++
* BUG: Fixed issue in formula calculation in case there were a gap in the data series
* BUG: Field name for wifi strength changed from "rssi" to "RSSI"
* BUG: Fixed issue with probing MFLN on non standard ports
* BUG: Changed http connection to keep-alive so that server does not close port before the client has had a chance to read the response.

v0.9.0
======
* Added one http push target that uses HTTP GET. This can be used with ubidots or blynk api's. 
* Added function to test push targets from configuration page. It will send data and show the return code as a first step. 
* Added documentation on how to integrate with Blynk.io using http get.
* Config page now shows the estimated runtime for the device (based on a full battery and previous average runtime)
* Experimental release of firmware using an esp32 instead of esp8266
* Merged index and device pages into one so that all the needed information is available on the index page.
* Removed api for device (/api/device), it's now merged into the /api/status api.
* Test function in format editor now uses real data and not fake. 
* Split push configuration into two sections to make it fit better on smaller devices
* Updated WifiManager and DoubleReset libraries
* Updated esp32 target with littlefs support
* Updated esp32 target with BLE send support (it will simulate a tilt)
* Mounted esp32 d1 mini mounted to a iSpindle PCB 4.0 (CherryPhilip) which worked fine.
* Documented hardware changes on esp32
* Default mDNS name is now shown on WIFI setup page.
* Added option to manually update/downgrade firmware under hardware settings. 
* BUG: Corrected PIN for voltage read on ESP32
* BUG: If using plato and not gravity formula was defined the value was set to null.
* BUG: Temp format name was incorrect in iSpindle format causing receiver to incorrectly read temperature.
* BUG: Temperature sensor adjustment value was not handled properly when using Fahrenheit.
* BUG: If the ID was to low the device id could end up with a leading space causing errors in data post. Added leading zero to ID.
* BUG: Entering wifi setup and a timeout occurred the wifi settings could be deleted.

v0.8.0
======
* Added option to set http headers (2 per http endpoint), these can be used for 
  other http formats than json (default) and for adding authentication headers.
* Added possibility to view last 10 errors on device page. 
* Added possibility to define token parameter used in iSpindle format.
* Added instructions for how to configure integration with Brewspy
* Added instructions for how to configure integration with Thingspeak
* Added option to do a factory reset via API. 
* Added logging of the runtime, how long a measurement take (last 10 are stored). This can be 
  used to check how good the wifi connection is and estimate the lifetime when on battery. 
  Check the device page in the UI for this information.
* Refactored code to free up more RAM to make SSL more stable. 
* Before connecting to an SSL endpoint the device will try to use a new SSL feature 
  called MFLN (Maximum Fragment Length Negotiation) that allow us to reduce the buffers 
  from 16k to 2k. This can make a huge difference on a device with only 40k RAM. Not all 
  servers might support this feature.
* Updated documentation pages.
* Tested battery life, 47 days using an update frequency of 5 min

v0.7.1
======
* Added instructions for how to configure integration with Fermentrack
* Added instructions for how to configure integration with Ubidots
* Added instructions for how to configure integration with HomeAssistant
* Added instructions for how to configure integration with Brewers Friend (not verified)
* BUG: Defined mqtt port was ignored, used default values.
* BUG: Extended length of HTTP url fields from 100 to 120 chars.
* BUG: Fixed issue with default template so it now includes the device name correctly. 

v0.7.0
======
Latest stable version. `Release v0.7 on Github <https://github.com/mp-se/gravitymon/releases/tag/v0.7.0>`_

* SSL support for HTTP targets
* SSL support for MQTT targets
* SSL support for OTA
* Added support for Plato
* Added error handling for calibration page.
* Added experimental target ESP32 (using an ESP32 D1 Mini which is pin compatible with ESP8266). Not 
  really usable since wifi connection is extremely slow with current Arduino releases (3-8 seconds).
* Added experimental format editor so users can customize their data format used for pushing data. 
  This will reduce the need for custom push targets. As long as the service is supporting http 
  or https then the data format can be customized.
* Added check so that pushing data is not done if memory is low (this will avoid crashes)
* MQTT topic has been removed from config (handled via format templates)
* MQTT port port number added. Port over 8000 will activate SSL.

* **Breaking change**: To simplify the internal structure the 
  temp sensor adjustment is now stored in C. So if you have 
  enabled this function using F you will need to go into 
  the configuration and update the adjustment factor again (hardware config).

* **Breaking change**: The MQTT push option has been changed to match the iSpindle behaviour. If 
  the behaviour in v0.6 is wanted this can be done via the format editor. 

v0.6.0
======
* Changed the wifi manager and refactored wifi.cpp
* LED is now turned on when Wifi Portal is open
* Refactored main.cpp to make it easier to read
* Tested runtime performance
* Improved documentation
* Added warning on config page when sleep is <300
* Enabled selection of gyro temperature sensor under Hardware settings
* Added warning when short sleep interval and gyro temp is enabled
* Added support for MQTT
* Bug: MPU init sometimes caused crash during startup.

v0.5.0
======
* Added feature to calculate formula on device
* Total rewrite of documentation
* WIFI settings are now stored in config file
* Defined version numbers for all dependant libraries to avoid updates breaking build.
* Cleanup of code
* Refactor code from C to C++

v0.4.0
======
* First public release