.. _releases:

Releases 
########

v0.7.0
------

Development version (dev branch)

* SSL support for HTTP targets
* SSL support for MQTT targets
* SSL support for OTA

* Added error handling for calibration page.

* Added experimental target ESP32 (using an ESP32 D1 Mini which is pin compatible with ESP8266). Not 
  really usable since wifi connection is extreamly slow with current Arduino releases (3-8 seconds).

* Added experimental format editor so users can customize their data format used for pushing data. 
  This will reduce the need for custom push targets. As long as the service is supporting http 
  or https then the data format can be customized.

* Added check so that pushing data is not done if memory is low (this will make it crash)

* MQTT topic has been removed from config (handled via format templates)

* MQTT port port number added. Port over 8000 will activate SSL.

* **Breaking change**: To simplify the internal structure the 
  temp sensor adjustment is now stored in C. So if you have 
  enabled this function using F you will need to go into 
  the configuration and update the adjustment factor again (hardware config).

* **Breaking change**: The MQTT push option has been changed to match the iSpindle behaviour. If 
  the behaviour in v0.6 is wanted this can be done via the format editor. 

v0.6.0
------

Latest stable version.

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

`Release v0.6 on Github <https://github.com/mp-se/gravitymon/releases/tag/v0.6.0>`_

v0.5.0
------

* Added feature to calcuate formula on device
* Total rewrite of documentation
* WIFI settings are now stored in config file
* Defined version numbers for all dependant libraries to avoid updates breaking build.
* Cleanup of code
* Refactor code from C to C++

`Release v0.5 on Github <https://github.com/mp-se/gravitymon/releases/tag/v0.5.0>`_

v0.4.0
------

`Release v0.4 on Github <https://github.com/mp-se/gravitymon/releases/tag/v0.4.0>`_
