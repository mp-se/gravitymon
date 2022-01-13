.. GravityMon documentation master file, created by
   sphinx-quickstart on Wed Jan  5 22:46:42 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to GravityMon's documentation!
######################################

.. note::
  This documentation reflects **v0.6**. Last updated 2022-01-11
  

GravityMon is a replacement firmare for the iSpindle firmware, it uses the same hardware configuration so 
you can easily switch between them. It's used to measure gravity in beer and show the progress of fermentation. 

For more information on this topic and function please visit `iSpindel Homepage <https://www.ispindel.de>`_ .

I started GravityMon because i like to create software and wanted to do some low level programming. I had done a few
projects based on esp8266 and also started to brew beer so this combination was quite natural. 

The hardware design comes from the fantastic iSpindle project so that is not covered in this documentation. 

My approach to this software is a little different from that the original ispindle firmware. The github repository can 
be found here; `GravityMon on Github <https://github.com/mp-se/gravitymon>`_

.. note::
  This software is in the early stages even though its more than one year old so if you find issues, please 
  open a ticket on github.
  
  I dont take responsibility for any errors that can cause problems with the use. I have tested v0.4 on 5+ brews 
  over the last 6 months without any issues.


The main differences:
---------------------

* Operates in two modes ``gravity monitoring`` and ``configuration mode``
* Send data to multiple endpoints when pushing data.
* Automatic temperature adjustment of gravity reading 
* OTA support from local webserver 
* Build in function to create gravity formulas

There are also a experimental features such as:

* Using the temperature sensor in gyro instead of DS18B20 (faster)
* Performance measurements (used to optimise code)

For a complete breakdown see the :ref:`functionallity`

Credits to
----------
Ideas to some of these functions have been picked up from disucssions in the iSpindle forums. This software uses 
the following libraries and without these this would have been much more difficult to acheive:

* https://github.com/jrowberg/i2cdevlib.git 

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


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   license
   releases
   functionallity
   installation
   configuration
   compiling
   contributing
   q_and_a
   backlog

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
