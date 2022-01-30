.. _getting_started:

Getting started
===============

First you need a completed iSpindle hardware, there are several resouces around that topic so it 
will not be covered in this documentation. Please visit `iSpindel Homepage <https://www.ispindel.de>`_  for 
more information.

Step 1 - Flash the device
-------------------------

The first step is to flash the firmware, I recommend using Brewflasher as the easy option. Detailed 
instructions can be found here :ref:`installation`

Step 2 - Setup WIFI
-------------------

When the device starts up the first time it will first start an WIFI access point so that the WIFI Settings 
can be configured. The instructions for that can be found here :ref:`_setup_wifi`

Step 3 - Configuration
----------------------

Once the device can connect to WIFI it will go into `configuration mode` and start a web server for 
doing the initial configuration. In order to access the device you will need to find its name or ip adress.

It will broadcast a name like gravitymonXXXXXX.local over mDNS. Where the XXXXXX is the unique device id. You can 
find the name via an mDNS browser, check your router or connect the device to a serial monitor. On windows mDNS 
might not work so then use the IP address instead. Once connected you will meet a web interface that looks like this.

.. image:: images/index.png
  :width: 800
  :alt: Index page

The next step is then to configure the device, most settings should work but there are a few that should be changed.

Configuration - Device Settings - Device Name
+++++++++++++++++++++++++++++++++++++++++++++

Give your device a good name.

Configuration - Device Settings - Calibration
+++++++++++++++++++++++++++++++++++++++++++++

You need to place the device on a flat surface and then press the 
calibrate button. It will take a few seconds for this to complete and the angle should be close to 90 degress. Without 
calibration the device will not go into gravity mode.

Configuration - Push Settings
+++++++++++++++++++++++++++++
Add the endpoints where you want data to be transmitted. All URLs that contain a 
valid endpoint will receive the data.

Calibration 
+++++++++++

I recommend to use the calibration feature to create a gravity formula. If you have values from a 
previous calibration then you can add them here, if not follow the calibration guidelines on the iSpindle site.

Step 4 - Completed
------------------

You are now done and can enjoy the GravityMon software. Check out the :ref:`setting-up-device` section for other configuration options.

If you want to enter the configuration mode place the device flat on a surface and do a reset (or wait until it wakes up). 
Its recommended to attach the device to power when you have it in `configuration mode` so the battery is not drained. 

**If you have suggestions for more awesome features, head over to the github repository and make a request.**
