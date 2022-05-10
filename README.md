
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/total) 
![release](https://img.shields.io/github/v/release/mp-se/gravitymon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/gravitymon)
![pr](https://img.shields.io/github/issues-pr/mp-se/gravitymon)
![dev_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/PlatformIO%20CI/dev?label=dev%20build)
![doc_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/Sphinx%20Build/dev?label=doc%20build)

# Gravity Monitor for Beer Brewing

GravityMon is a replacement firmware for the iSpindle firmware. It's 100% compatible with the iSpindle hardware design so it does not require any hardware changes. 

Now also works with ESP32 (use ESP32 d1 mini which is compatible with ESP8266)

Installation can be made using https://www.brewflasher.com or https://web.brewflasher.com

The documenation can be found here: https://mp-se.github.io/gravitymon/index.html

Note! If Brewflasher being flagged as malware by your antivirus software, try the web version.

The main differences:
---------------------

* Modern web based user interface for configuration when connected to WIFI
* Efficient software, long lifespan (+45 days with 5min update frequencey)
* Send data to multiple endpoints (http-post, http-get, influxdb v2, mqtt)
* Instructions for service such as; Brewfather, Fermentrack, Ubidots, Home Assistant, Brewers Friend, Brewspy, Thingspeak, Blynk. 
* SSL support for all remote endpoints
* ESP32 support with Bluetooth push
* Customize data format to be pushed
* Automatic temperature adjustment of gravity when enabled
* Use the temperature sensor in gyro instead of DS18B20
* Built in function to create gravity formulas, no need for additional software, just enter tilt/gravity. 
* Visual graph showing how gravity formula will be interpreted
* OTA support or firmware upload via web interface
* REST API for scripting

No code has been reused from the iSpindle project. 
