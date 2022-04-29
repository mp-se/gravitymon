.. GravityMon documentation master file, created by
   sphinx-quickstart on Wed Jan  5 22:46:42 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to GravityMon's documentation!
######################################

.. note::
  This documentation reflects **v1.0**. Last updated 2022-04-29

.. image:: images/gravitymon.gif
  :width: 800
  :alt: User Inteface Walkthrough

GravityMon is used to measure gravity and temperature during fermentation of beer and report the progress. The graph below is
an example on how the fermentation process can be tracked. This is from my last brew that was over on a few days. The graph is rendered using
Fermentrack. 

.. image:: images/fermentation.png
  :width: 500
  :alt: Example fermentation

GravityMon is a replacement firmware for the iSpindle and uses the same hardware configuration and is 100% compatible. It
implements a lot of the features that has been requested in the orginal iSpindle project but has been rejected for 
various reasons. Here is a list of :ref:`main_features`.

From v0.9 the firmware now supports a iSpindle build based on an ESP32 d1 mini (pin compatible with esp8266). See :ref:`hardware`.

I started GravityMon because I like to create software and wanted to do some low level programming. I had done a few
projects based on esp8266 and also started to brew beer so this combination was quite natural. 

The hardware design comes from the fantastic iSpindle project so that is not covered in this documentation. For more 
information on this topic and function please visit `iSpindel Homepage <https://www.ispindel.de>`_ .

My approach to this software is a little different from that the original iSpindle firmware. The github repository 
can be found here; `GravityMon on Github <https://github.com/mp-se/gravitymon>`_

.. note::  
  I dont take responsibility for any errors or issues caused by the software. The software is provided as-is. I will however
  try my best to fix issues that might occur. 

  I have tested this software over the last year on 20+ brews with good results.

.. _main_features:

Main features:
--------------

* Operates in two modes gravity monitoring and configuration mode (simplify calibration). Gravity mode 
  is comparable to how the iSpindle works.
* Modern web based UI when in configuration mode. No need to start the access point changing settings.
* REST API to enable scripted configuration
* Send data to multiple endpoints and services at once
* Setup guides for how to send data to many popular services. Currently 8+ are documented.
* Automatic temperature adjustment of gravity reading
* OTA support from webserver
* Built in function to create gravity formulas, no need for additional software, just enter tilt/gravity and 
  let GravityMon create the formula. 
* Visual graph showing how formula will be interpreted based on entered values
* Using the temperature sensor in gyro instead of DS18B20 (faster)
* SSL support in standard HTTP and MQTT connections.
* Option to customize data posted to endpoints using template from the UI.
* Built in performance measurements (used to optimise code)

For a complete breakdown see the :ref:`functionallity`

This is a simple overview of the different components that the software contains. The green ones are only active during `configuration mode` in 
order to save battery.

.. image:: images/software_design.png
  :width: 600
  :alt: Software design


Credits to
----------
Ideas to some of these functions have been picked up from disucssions in the iSpindle forums. This software uses 
the following libraries and without these this would have been much more difficult to acheive:

* https://github.com/jrowberg/i2cdevlib 

  This library contains the basic code to interact with the gyro + many more chips.

* https://github.com/codeplea/tinyexpr

  Proccess the gravity formula and calculate the gravity and various corrections.

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

  Library for sending data to mqtt based on lightweight mqtt implemenentation. 

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
   functionallity
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
