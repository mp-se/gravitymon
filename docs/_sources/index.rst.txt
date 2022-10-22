.. GravityMon documentation master file, created by
   sphinx-quickstart on Wed Jan  5 22:46:42 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to GravityMon
#####################

.. note::
  This documentation reflects **v1.2 - beta 1**. Last updated 2022-10-22

User interface overview
-----------------------

This animation shows how the user interface is structured, it reflects an older version but the structure is the same.

.. image:: images/gravitymon.gif
  :width: 800
  :alt: User Inteface Walkthrough

.. _main_features:

Main features
-------------

* Support either an ESP8266-d1-mini, ESP32-mini, ESP32S2-mini or ESP32C3-mini board (see :ref:`hardware`)
* Operates in two modes gravity monitoring and configuration mode
* Gravity mode is comparable to how the iSpindle works when collecting data
* Configuration mode has a modern HTML5 based web UI. No need to start the access point to change settings
* Offloading some of the functionality to run in the web browser, this allows for more advanced features.
* REST API to enable scripted configuration
* Send data to multiple endpoints and services at once (2xHTTP POST, HTTP GET, MQTT, INFLUXDB2)
* Directly test all endpoints from user interface with feedback to simplify troubleshooting
* Complete format customization for all endpoints using templates (don't really need to change the software to support new services)
* Setup guides for how to send data to many popular services. Currently 10+ are documented
* Automatic temperature adjustment of gravity (just tick a checkbox)
* OTA support from webserver
* Firmware update via web interface
* Built in function to create gravity formulas, no need for additional software, just enter tilt/gravity and let GravityMon creates a formula
* Visual graph showing how formula will be interpreted based on entered values
* Using the temperature sensor in gyro instead of DS18B20 (faster)
* SSL support in all endpoints (no certificate validation due to limitations on esp8266).
* Built in performance measurements (used to optimize code)
* Storage mode when placed on cap (indefinite sleep)
* Customize various hardware parameters to optimize device functionality.

For a complete breakdown see the :ref:`functionality`

.. note::
  If you are missing some feature, please reach out on `Github <https://github.com/mp-se/gravitymon/discussions>`_ or `homebrewtalk.com <https://www.homebrewtalk.com/threads/replacement-firmware-for-ispindel-gravitymon.698058/>`_

What is GravityMon?
--------------------

GravityMon is used to measure gravity and temperature during fermentation of beer and report the progress. The graph below is
an example on how the fermentation process can be tracked. The graph has been rendered using Fermentrack. 

.. image:: images/fermentation.png
  :width: 500
  :alt: Example fermentation

GravityMon is a replacement firmware for the iSpindle and uses the same hardware configuration and is 100% compatible. It
implements a lot of the features that has been requested in the original iSpindle project but never implemented for 
various reasons. Here is a list of :ref:`main_features`.

From v0.9 the firmware also supports a iSpindle built using an ESP32 d1 mini (pin compatible with esp8266). Currently this is an experimental 
version but since it has more memory and processing capacity it could support more functions. See :ref:`hardware`.

I started GravityMon because I like to create software and wanted to do some microcontroller programming. I had done a few
projects based on esp8266 and also started to brew beer so this combination was quite natural. 

The hardware design comes from the fantastic iSpindle project so that is not covered in this documentation. For more 
information on this topic and function please visit `iSpindel Homepage <https://www.ispindel.de>`_ .

My approach to this software is a little different from that the original iSpindle firmware as can be seen in the list of features. 

The github repository can be found here; `GravityMon on Github <https://github.com/mp-se/gravitymon>`_

.. note::  
  I don't take responsibility for any errors or issues caused by the software. The software is provided as-is. I will however
  try my best to fix issues that might occur. 

  I have tested this software on 40+ brews with good results.


Documentation for older versions
--------------------------------

* Docs for: `v1.0 <https://mp-se.github.io/gravitymon/v1.0/index.html>`_
* Docs for: `v0.9 <https://mp-se.github.io/gravitymon/v0.9/index.html>`_


Software architecture
---------------------

This is a simple overview of the different components that the software contains. The green ones are only active during `configuration mode` in 
order to save battery.

.. image:: images/software_design.png
  :width: 600
  :alt: Software design


Credits to
----------
Ideas to some of these functions have been picked up from discussions in the iSpindle forums. This software uses 
the following libraries and without these this would have been much more difficult to achieve:

* https://github.com/jrowberg/i2cdevlib 

  This library contains the basic code to interact with the gyro + many more chips.

* https://github.com/codeplea/tinyexpr

  Process the gravity formula and calculate the gravity and various corrections.

* https://github.com/graphitemaster/incbin 		

  Include binary files into the code, used to service html files.

* https://github.com/khoih-prog/ESP_DoubleResetDetector

  Can detect if the reset button is pressed twice, is used to enter WIFI config mode.

* https://github.com/khoih-prog/ESP_WiFiManager

  Configure wifi settings.

* https://github.com/thijse/Arduino-Log

  Logging library for handling different loglevels and configure what sent over the serial.

* https://github.com/bblanchon/ArduinoJson

  Json parser/creator used in configuration files and API's

* https://github.com/PaulStoffregen/OneWire

  Communication library used for interacting with temperature sensor.

* https://github.com/milesburton/Arduino-Temperature-Control-Library

  Interaction with the DS18B20 sensor

* https://github.com/Rotario/arduinoCurveFitting

  Create the gravity formula.

* https://github.com/256dpi/arduino-mqtt

  Library for sending data to mqtt based on lightweight mqtt implementation. 

* https://graphjs.com/

  Render the graphs in the UI.

* https://getbootstrap.com/

  CSS templates for the web page.

* https://github.com/lorol/LITTLEFS

  LittleFS library for ESP32 framework 1.x

* https://github.com/h2zero/NimBLE-Arduino

  Bluetooth library for ESP32 framework 1.x

* https://github.com/spouliot/tilt-sim

  Excellent project for simulating a tilt device. 

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   intro
   releases
   functionality
   installation
   configuration
   troubleshooting
   q_and_a
   formula
   services
   advanced
   api
   data
   compiling
   contributing
   hardware
   license

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
