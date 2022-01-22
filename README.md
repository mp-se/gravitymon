
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/total) 
![release](https://img.shields.io/github/v/release/mp-se/gravitymon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/gravitymon)
![pr](https://img.shields.io/github/issues-pr/mp-se/gravitymon)
![dev_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/PlatformIO%20CI/dev?label=dev%20build)
![doc_build](https://img.shields.io/github/workflow/status/mp-se/gravitymon/Sphinx%20Build/dev?label=doc%20build)

# Gravity Monitor for Beer Brewing

GravityMon is a replacement firmware for the iSpindle firmware. It's 100% compatible with the iSpindle hardware design so it does not require any hardware changes.

Installation can be made using https://www.brewflasher.com

The main differences:
---------------------

* Operates in two modes gravity monitoring and configuration mode (simplify calibration)
* Modern web based UI for configuration (in config mode)
* REST API
* Send data to multiple endpoints when pushing data (2xhttp, brewfather, influxdb v2, mqtt supported)
* Automatic temperature adjustment of gravity reading
* OTA support from local webserver
* Built in function to create gravity formulas, no need for additional software, just enter tilt/gravity. 
* Visual graph showing how formula will be interpreted
* Using the temperature sensor in gyro instead of DS18B20 (faster)
* Built in performance measurements (used to optimise code)
* SSL support in standard HTTP and MQTT connections.

No code has been reused from the iSpindle project. 

The documenation can be found here: https://mp-se.github.io/gravitymon/index.html
