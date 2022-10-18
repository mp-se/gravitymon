
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/total) 
![release](https://img.shields.io/github/v/release/mp-se/gravitymon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/gravitymon)
![pr](https://img.shields.io/github/issues-pr/mp-se/gravitymon)
![dev_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/PlatformIO%20CI/dev?label=dev%20build)
![doc_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/Sphinx%20Build/dev?label=doc%20build)

# Gravity Monitor for Beer Brewing

GravityMon is a replacement firmware for the iSpindle firmware. It's 100% compatible with the iSpindle hardware design so it does not require any hardware changes. 

Now also works with ESP32, either an ESP32 d1 mini or ESP32 c3 mini which both are compatible with ESP8266.

Installation can be made using https://www.brewflasher.com or https://web.brewflasher.com

The documenation can be found here: https://mp-se.github.io/gravitymon/index.html

Note! If Brewflasher being flagged as malware by your antivirus software, try the web version.

The main differences:
---------------------

* Operates in two modes gravity monitoring and configuration mode
* Gravity mode is comparable to how the iSpindle works when collectintg data
* Configuration mode has a modern HTML5 based web UI. No need to start the access point to change settings
* Offloading some of the functionallity to run in the web browser, this allows for more advanced features.
* REST API to enable scripted configuration
* Send data to multiple endpoints and services at once (2xHTTP POST, HTTP GET, MQTT, INFLUXDB2)
* Directly test all endpoints from user interface with feedback to simplify troubleshooting
* Complete format customization for all endpoints using templates (dont really need to change the software to support new services)
* Setup guides for how to send data to many popular services. Currently 10+ are documented
* Automatic temperature adjustment of gravity (just tick a checkbox)
* OTA support from webserver
* Firmware update via web interface
* Built in function to create gravity formulas, no need for additional software, just enter tilt/gravity and let GravityMon creates a formula
* Visual graph showing how formula will be interpreted based on entered values
* Using the temperature sensor in gyro instead of DS18B20 (faster)
* SSL support in all endpoints (no certificate validation due to limitations on esp8266).
* Built in performance measurements (used to optimise code)
* Storage mode when placed on cap (indefinite sleep)
* Customize various hardware parameters to opimize device functionallity.
* 
No code has been reused from the iSpindle project. 
