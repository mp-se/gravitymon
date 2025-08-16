.. _releases:

Releases 
########


v2.3.0
======

In this version I have dropped some fetures to make room for new options and 
features due to limitations on the devices supported, mainly the 4mb flash limit. 

New features
++++++++++++
* Refactored code for gravity velocity and filter to better follow guidelines for RTCMEM on ESP32.
* Added feature flags to build so builds can be customized and UI will better adapt.
* Added RAPT bluetooth transmission covering both v1 and v2 formats.
* Added instructions for Grainfather integration.

Dropped features
++++++++++++++++
* Support for ESP32lite/Floaty and ESP32 d1 mini has been dropped. Latest version is v2.2. This to remove these boards that are quite old and make room for new options.
* Remove the gyro disabled flag which was used to run the device as a temperature probe only. Latest version is v2.2

Bugs fixed
++++++++++
* Fixed startup issue after deep sleep on some boards when RTCMEM was enabled.

v2.2.1
======

Bugs fixed
++++++++++
* Wifi mode could be triggered by sleep mode was not correcly fixed in v2.2.0.
* Ignore low angles was not working correcly in gravity mode.
* FileSystem was not closed properly when going into storage mode, could potentially cause file system corruption.

v2.2.0
======

.. note::
  Only ESP32! The voltage divider for the battery voltage has been changed since there where issues with some ESP32 boards. 220k + 220k is now the recommended values.
  When updating to this version you will need to recalculate the voltage factor if you are using an ESP32 board.

New features
++++++++++++
* THe formula page now shows the current angle with a 2s update refresh. This to make it easier to create the formula.
* Adding tcp socket transmission for http port 1 target (secure connection is not supported), some remote services dont support http.
* Added onewire scanning of devices to hardware test on esp8266
* Rewritten battery percentage to support more battery types and battery saving mode now starts when 30% capacity is left and calculation now support multiple battery types.
* Removed flash logging of runtime data since this is no longer needed.
* New gyro support ICM-42670-P
* Adds support for ImprovWiFi which allows for configuraition of wifi settings when flashed with the web flasher.
* Refactored code base so its now shared with Gravitymon Gateway and Pressuremon.
* Format editor will now validate json payloads for http post and mqtt when selecting preview. 
* Optimized reset counter code to avoid writes to flash.

New features (ESP32 only)
+++++++++++++++++++++++++
* Voltage reading on ESP32 boards have been changed to improve accuracy
* (experimental) Added gravity velocity (points per 24h) as new datapoints
* (experimental) Added filter to gyro reading in order to smooth out readings (5 point moving avegage with min/max removed)

Bugs fixed
++++++++++
* Deep sleep mode is fixed for ESP32 boards
* Wifi mode could be triggered by sleep mode.
* Fixed bug that device goes into wifi setup mode after deep sleep.
* Fixed UI sensor error messages during wifi setup mode when sensors are not initialized. Wifi setup mode is also used for fallback if there are issues with sensor intitialization
* Fixed floaty build so it will start up correcly after upgrading to OneWireNG

.. note::
  This will be the latest version were I support the Floaty hardware and the ESP32 d1 mini boards. I plan to add support for 
  newer ESP32 based boards instead. 


v2.1.0
======

New features
++++++++++++
* Updated formula creator.

  * Allow user more choice when it comes to selecting the order and varifying result.
  * Will now correctly handle Plato data as input
  * Formula will still be created for SG since this is the internal format used by Gravitymon
  * Moved formula creation from device code to UI code to allow for more flexible formula creation. 
  * Removed dependant library and API.

* Extended formula points from 10 to 20

.. note::
  Update the formula deviation (default 3) to a value according to your selected gravity format eg. SG = 0.003

* Added warning when mqtt payload exceeds 700 bytes to avoid crashes
* Optimized code to reduce memory usage to allow for larger payloads when pushing data
* Update dependant libraries to latest version (Json 7, AsyncWebServer, EspFwk)

Bugs fixed
++++++++++
* UI no shows clearly when gyro is moving and average is not calculated
* Fixed broken link to removing old iSpindle configuration.
* Fixed limitation that lowest plato was 1 (now 0)
* Corrected docs on home assistant auto registration option.
* Skip SSL when in config mode flag now works correctly

v2.0.1
======

* Disable LED on esp8266 since it might collide with gyro communication.

v2.0.0
======

New features
++++++++++++
* Refactor some of the UI to make it easier to maintain.
* Added feature to force device to search for the stronget AP.
* Fixed the issue with gyro not working on c3 board (had to disable LED since it collides with gyro).
* Removed BLE GravityMon Service option since this is not really performing well.
* Added WIFI Direct Connect option with GravityMon Gateway, I hope this will help with wifi connection issues in my steel fermenter and force a connect to the designated AP. Wifi Direct will send the data via http 
  to the Gateway and then forward this to the correct endpoint. 
* Added line breaks to format editor for easier editing
* Added check if device is responding and notify user when there is an issue
* Redesign of user interface using VueJS as the base (UI moved to separate github project mp-se/gravitymon-ui) 
* Added authorization for API access (currently basic and not really secure)
* Redesigned REST-API to function better with VueJS
* Updated documentation to match new version (Still some more work needed)
* Updated API tests to match new API (Still some more work needed)
* Moved WIFI configuration to the main application (Removed WiFi Manager)
* Added additional BLE options for ESP32 builds

Issues adressed
+++++++++++++++
* Temp sensor not always beeing detected, Switch to OnewireNG which seams to fix the problem
* Fixed led indicator when going into wifi mode
* Turning off led when going to sleep mode or doing reset

v1.4.1
======

Issues adressed
++++++++++++++++
* Fixed broken iSpindel configuration migration for the esp8266. 
* Fixed broken gather support button. 

v1.4.0
======

Features
++++++++
* *NEW* Added led indicator: Blue / Slow Blinking = Config Mode, White/Solid = WiFi setup  
* *BUG* Reduced WIFI TX Power for C3 chip to improve WIFI connnection stability
* *NEW* Added 4 options to ble data transmission (TILT, TILT PRO, GRAVITYMON SERVICE, GRAVITYMON EDDYSTONE)
* *NEW* Added support for ESP32S3 mini board
* *NEW* Moved to async webserver to speed up performance in configuration mode
* Added feature to migrate formula and gyro calibration data from an existing ispindel configuration (don't do a full erase before flashing)
* Added possibility to set wifi settings in config (need manual reboot to apply)
* Added option that shows the average value from the gyro reads + a button to reset that on the web ui. Might make it easier to do calibration of the value jumps around.

Known issues
++++++++++++
* On ESP32c3 mini the rgb led does not turn blue when in config mode
* On ESP32s3 mini the rgb led does not work due to issue in arduino/esp32 unless manual patching of the arduino framework `#define RGB_BUILTIN LED_BUILTIN+SOC_GPIO_PIN_COUNT`

Maintenance
+++++++++++
* Changed wifi manager to modified version since the source project is no longer supported by author
* Updated dependencies to latest versions as of Aug -23
* Added UI tests to improve test coverage
* Added unit tests to improve test coverage
* Added option to run unit test code coverage with WOKWI
* Added scripts to check for code coverage on the esp32 (work in progess, not yet covering all parts of the code)
* Created script that checks for updates to dependant projects (test/converage.py)

Documentation
+++++++++++++
* Published code coverage for unit tests and apitest.py (gcov/README.md)
* Added BierBot Bricks configuration as external service target. 

Issues adressed
++++++++++++++++
* Updated code so it now works with newer versions of ArduniJSON
* Fixed issue with ESP32 were reboot didn't work sometimes

v1.3.1
======

* Added option to restart device from the configuration menu (user request).
* Updated to latest arduino builds for both esp8266 and esp32.

v1.3.0
======

Features
++++++++
* Refactored logoutput from WifiManager to be consistent with other logs.
* Added feature to do backup and restore of device settings. Data will be exported as a text (json) file. 
* Added validation feature that checks for configuration errors or missing steps in setup. Warnings will be displayed on index page.
* Added restore to factory default button under configuration
* Upgraded to bootstrap v5.2.3, jquery v3.6.3 and jchart v4.2.1
* Added feature to check for newer GravityMon releases from gravitymon.com (Will only check against promoted / stable version).
* Added chip information to startup for easier support
* Added option to select a default gravity formula under settings. This is based on measurements from several different devices.
* Increased the number of decimals for tilt/angle (3), temperature (2), plato (2), runtime (2).
* Added ${battery-percent} as new variable for use in format templates.
* Added battery saving mode. When battery drops below 20% the sleep interval will be changed to 1 hour. Can be disabled in advanced configuration.

Documentation
+++++++++++++
* Updated getting started section to clarify what needs to be done during setup.
* Updated documentation with new images and text
* Added new option for home assistant sensor registration through automation script.
* Added description for how to get voltage measurement on a floaty, see Hardware section.

Issues adressed
++++++++++++++++
* BUG: Double reset was detected incorrectly on ESP32C3, created own code and dropped douoble reset detection library for better stability. Will now require 3 taps within 3 seconds each to go into wifi configuration.
* BUG: Gyro and Tempsensor was not detected after a reset on ESP32 (needed power cycle), reverted to older Arduino version to solve the problem. 
* BUG: Pin numbers on the newer ESP32C3 is different from v1
* BUG: Wrong OTA filename for ESP32C3v1
* BUG: Fixed long statup delay when creating access point on ESP32C3
* BUG: Sometimes it could be hard to get into config mode and ended up in wifi portal instead due to double restet detect. 
* BUG: If the gyro value was invalid (moving) during startup that would cause the device to go into gravitymode instead of config mode.
* BUG: When trying to do calibration without a connected gyro the device would enter endless loop.
* BUG: Clarified that its the bucket ID and auth TOKEN for influxdb (config page) 
* BUG: Fixed wrong device classes in home assistant template.

Other
+++++
* Removed khoih-prog/ESP_DoubleResetDetector since this didnt work properly on the ESP32c3, this library has no support from author anymore.
* Updated platformio esp8266 to v4.1.0
* Updated platformio esp32 to v6.0.0
* Updated h2zero/NimBLE-Arduino to b 1.4.1
* Updated 256dpi/arduino-mqtt to v2.5.1 
* Created tag v1.0.0 for codeplea/tinyexpr to get control over releases
* Created tag v1.0.0 for graphitemaster/incbin to get control over releases
* Updated khoih-prog/ESP_WiFiManager to v1.12.1
* Updated khoih-prog/ESP_DoubleResetDetector to v1.3.2

v1.2.1
======

Issues adressed
++++++++++++++++
* BUG: Under some circumstances the last part of the format template was omitted. 

Other
+++++
* Update tinyexpr library to latest baseline (Used for forumula evaluation).

Documentation
+++++++++++++
* Minor updates and corrections to the documetation.
* Updated Q&A section

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
* Added option to download firmware updates from https://www.gravitymon.com

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
Latest stable version.

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