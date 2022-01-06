.. GravityMon documentation master file, created by
   sphinx-quickstart on Wed Jan  5 22:46:42 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to GravityMon's documentation!
######################################

GravityMon is a replacement firmare for the iSpindle hardware. It's used to measure gravity in beer and show the progress
of fermentation. 

For more information on this topic and function please visit https://www.ispindel.de.

I started GravityMon because i like to create software and wanted to do some low level programming. I had done a few
projects based on esp8266 and also started to brew beer so this combination was quite natural. 

The hardware design comes from the fantastic iSpindle project so that is not covered in this documentation. 

My approach to this software is a little different from the approach that the orignial ispindle firmware has. 

.. note::

  This software is in the early stages even though its more than one year old so if you find issues, please 
  open a ticket on github.
  
  I dont take responsibility for any errors that can cause problems with the use. I have tested v0.4 on 5+ brews 
  over the last 6 months without any issues.

The main differences:
---------------------

* Operates in two modes `gravity monitoring` and `configuration mode`
* Send data to multiple endpoints when pushing data.
* Automatic temperature adjustment of gravity reading 
* OTA support from local webserver 
* Build in function to create gravity formulas (*New in v0.5*)

There are also a experimental features such as:

* Using the temperature sensor in gyro instead of DS18B20 (faster)
* Performance measurements (used to optimise code)

**For a complete breakdown see the functionallity section.**

Credits to
----------
This software uses the following libraries and without these this would have been much more difficult to acheive:

* https://github.com/jrowberg/i2cdevlib.git 
* https://github.com/codeplea/tinyexpr
* https://github.com/graphitemaster/incbin 		
* https://github.com/khoih-prog/ESP_DoubleResetDetector
* https://github.com/tzapu/WiFiManager
* https://github.com/thijse/Arduino-Log
* https://github.com/bblanchon/ArduinoJson
* https://github.com/PaulStoffregen/OneWire
* https://github.com/milesburton/Arduino-Temperature-Control-Library


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   functionallity
   installation
   configuration
   compiling
   contributing
   backlog

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
