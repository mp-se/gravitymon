.. GravityMon documentation master file, created by
   sphinx-quickstart on Wed Jan  5 22:46:42 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to GravityMon
#####################

.. note::
  This documentation reflects **v2.2.0**. Last updated 2025-06-25

  Link to `previous documentation here <https://mp-se.github.io/gravitymon/v1.4/>`_ 

  
What is GravityMon?
--------------------

GravityMon is a electronic hydrometer software that can be used to measure gravity and temperature during 
fermentation of beer and report the progress to a number of external services. Is uses a gyro to measure the 
gravity since there is a correlation between gravity an the density of the fluid. Based on the density the 
hydrometer will float at a different angle. The graph below is an example on how the fermentation process 
can be tracked. The graph has been rendered using Fermentrack. 

.. image:: images/fermentation.png
  :width: 500
  :alt: Example fermentation

GravityMon is a replacement firmware for the iSpindle and uses the same hardware configuration and is 100% compatible. It
implements a lot of the features that has been requested in the original iSpindle project but never implemented for 
various reasons. 

The firmware also supports a iSpindle built using an ESP32 which is pin compatible with esp8266, currently there are 3 options that are 
supported ESP32-D1 mini, ESP32-C3 mini, ESP32-S2 mini, ESP32-S3 mini, ESP32 lite (floaty). See :ref:`hardware`. The more modern hardware 
will allow better support for SSL and other memory intensive functions.

I started GravityMon because I like to create software and wanted to do some microcontroller programming. I had done a few
projects based on esp8266 and also started to brew beer so this combination was quite natural. 

The hardware design comes from the fantastic iSpindle project so that is not covered in this documentation. For more 
information on this topic and function please visit `iSpindel Homepage <https://www.ispindel.de>`_ .

My approach to this software is a little different from that the original iSpindle firmware as can be seen in the list of features. 

The github repository can be found here; `GravityMon on Github <https://github.com/mp-se/gravitymon>`_

.. note::  
  I don't take responsibility for any errors or issues caused by the software or hardware. 
  The software is provided as-is. I will however try my best to fix issues that might occur. 

  I have tested this software on 40+ brews with good results.


Other resouces
--------------

* `HomeBrewTalk Forum, discussion forum for GravityMon <https://www.homebrewtalk.com/threads/replacement-firmware-for-ispindel-gravitymon.698058/>`_
* `Article on diybrewers (Spanish) <https://www.diyhomebrewers.com/2023/02/gravity-mon-software-alternativo-para.html>`_
* `Podcast S2E4 with Peter Symons <https://homebrewingdiy.beer/index.php/2022/04/28/s2-e4-beer-history-and-open-source-projects-with-peter-symons/>`_


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

* https://github.com/thijse/Arduino-Log

  Logging library for handling different loglevels and configure what sent over the serial.

* https://github.com/bblanchon/ArduinoJson

  Json parser/creator used in configuration files and API's

* https://github.com/pstolarz/OneWireNg

  Communication library used for interacting with temperature sensor.

* https://github.com/milesburton/Arduino-Temperature-Control-Library

  Interaction with the DS18B20 sensor

* https://github.com/256dpi/arduino-mqtt

  Library for sending data to mqtt based on lightweight mqtt implementation. 

* https://graphjs.com/

  Render the graphs in the UI.

* https://getbootstrap.com/

  CSS templates for the web page.

* https://github.com/h2zero/NimBLE-Arduino

  Bluetooth library for ESP32 framework 1.x

* https://github.com/spouliot/tilt-sim

  Excellent project for simulating a tilt device. 

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   functionality
   releases
   intro
   installation
   configuration
   format-template
   hardware
   services
   data
   gateway
   custom
   compiling
   contributing
   license
   q_and_a

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
