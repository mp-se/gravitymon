.. _releases:

Releases 
########

v1.0.1
======

Documentation
+++++++++++++
* Fixed errors in data format section
* Added q&a on formula creation and value deviation
* Added documentation for Brewpiless as target
* Updated docs for ubidots service integration.

User interface
++++++++++++++
* Under format options its now possible to select brewfather ispindle format to avoid mixing endpoints. 

Issues adressed
++++++++++++++++
* BUG: Copy format templates used an old format for iSpindle and Gravmon where the token was not used.

v1.0.0
======

Documentation
+++++++++++++
* Update documentation to match v1.0
* Installation instructions updated on how to find the device after wifi has been configured. 
* Documentation on brewfather has been updated to adress SG/Plato conversion
* Added circuit diagram for esp8266 and esp32
* Added additional http error codes to troubleshooting documentation

User interface
++++++++++++++
* Upgraded to bootstrap v5.1 for web pages.
* Added button on indexpage to direct to github issues.
* Added button to extract important information for support requests. 
* First point in gravity formula is now reserved for water gravity, this to allow detection of angles below water that can be filtered out.
* Changed layout on index page with measured data on top.
* Added tooltips to all fields in user interface
* Added function on format page so that it's easy to copy a format template from the docs (simplify service integration).

Features
++++++++
* Added advanced setting to ignore angles that are lower than water. This is disabled by default.
* Added support for MPU6500 (standard is MPU6050).
* Removed brewfather option (can use standard HTTP options), the old apporach can still be used via changing format template.
* Added 5 more points for formula creation, so a total of 10 angles/gravity values can be stored.
* Added https support for Influxdb v2
* Added possibility to set 2 wifi ssid where the second acts as a fallback in case it fails to connect. If succesful the seconday becomes the new primary.
* SSL connections are skipped on ESP8266 when in config mode since there is a high probability it will crash due to low memory. 
* Advanced settings: Added possibility to have variable push intervals for different endpoints so that different frequency can be used, for example; 5min mqtt, 15min brewfather. 
* Advanced settings: Changes how many times the gyro is read (less reads, quicker but less accurate)
* Advanced settings: Set amount of gyro movement is allowed for a accurate read.
* Advanced settings: What deviation is acceptable for creating formula deviation
* Advanced settings: Various timeouts, wifi connect, wifi portal, http connects.
* Advanced settings: Adjust resolution of temp sensor (9 bits to 12 bits), higher resolution takes longer thus reducing batterylife

Issues adressed
++++++++++++++++
* BUG: Fixed issue in formula calculation in case there were a gap in the data series
* BUG: Field name for wifi strength changed from "rssi" to "RSSI"
* BUG: Fixed issue with probing MFLN on non standard ports
* BUG: Changed http connection to keep-alive so that server does not close port before the client has had a chance to read the response.

v0.9.0
======
* Added one http push target that uses HTTP GET. This can be used with ubidots or blynk api's. 
* Added function to test push targets from configuration page. It will send data and show the return code as a first step. 
* Added documetation on how to integrate with Blynk.io using http get.
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
* BUG: Temperature sensor adjusmemnt value was not handled properly when using Farenheight.
* BUG: If the ID was to low the device id could end up with a leading space causing errors in data post. Added leading zero to ID.
* BUG: Entering wifi setup and a timeout occured the wifi settings could be deleted.

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
* Tested batterylife, 47 days using an update frequency of 5 min

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
  really usable since wifi connection is extreamly slow with current Arduino releases (3-8 seconds).
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
* Added feature to calcuate formula on device
* Total rewrite of documentation
* WIFI settings are now stored in config file
* Defined version numbers for all dependant libraries to avoid updates breaking build.
* Cleanup of code
* Refactor code from C to C++

v0.4.0
======
* First public release