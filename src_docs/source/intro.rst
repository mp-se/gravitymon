.. _getting_started:

Getting started
===============

First you need a completed iSpindle hardware, there are several resources around that topic so it 
will not be covered in this documentation. Please visit `iSpindel Homepage <https://www.ispindel.de>`_  for 
more information.

Step 1 - Flash the device
-------------------------

The first step is to flash the firmware, I recommend using Brewflasher as the easy option. Detailed 
instructions can be found here :ref:`installation`

Step 2 - Setup WIFI
-------------------

You can set the wifi settings using the flasher on gravitymon.com which is the easy way.

When the device starts up the first time and it does not have any wifi settings, it first start an WIFI access point
so that the WIFI Settings can be configured. The wifi settings can be found under the Device - WIFI menu.

When in wifi setup mode neither the gyro or temperature sensor will be initialized. This is by design since if these would 
cause a startup error there would be no way to get into the device. 

Once the device is restarted it will connect to the wifi and be accessible as any device on your network using the 
mdns name or ip address. 

You can do this step before soldering the esp to ensure it works as expected.

Step 3 - Device Configuration
-----------------------------

The user interface does no longer require internet access for most functions, the exception is the graphs which 
require access to chartjs in order to render properly. This is due to lack of memory on the device.

The device will go into `configuration mode` and start a web server for doing the initial configuration. 
In order to access the device you will need to find its name or ip address.

It will broadcast a name like gravitymonXXXXXX.local over mDNS. Where the XXXXXX is the unique device id. You can 
find the name via an mDNS browser, check your router or connect the device to a serial monitor. On windows mDNS 
might not work so then use the IP address instead. Once connected you will meet a web interface that looks like this.

.. image:: images/ui-home.png
  :width: 800
  :alt: Index page

In order for the software to work properly there are a few steps that needs to be configured. When you open up the
index page a selftest will be run and common issues will be shown for your action.

Configuration - Device Settings - Device Name
+++++++++++++++++++++++++++++++++++++++++++++

Give your device a good name.

Configuration - Device Settings - Gyro Calibration
++++++++++++++++++++++++++++++++++++++++++++++++++

You need to place the device on a flat surface and then press the calibrate button. It will take a few seconds 
for this to complete and the angle should be close to 90 degrees. Without calibration the device will not go into gravity mode.

Configuration - Push Settings
+++++++++++++++++++++++++++++
Add the endpoints where you want data to be transmitted. All URLs that contain a valid endpoint will receive the data.

Gyro Calibration 
++++++++++++++++

I recommend to use the calibration feature to create a gravity formula. If you have values from a 
previous calibration then you can add them here, if not follow the calibration guidelines on the iSpindle site.

There are several guides for how to calibrate the device (`iSpindle Calibration <https://www.ispindel.de/docs/Calibration_en.html>`_) 

This will get the data points needed to create the formula, and the data points will be stored on the device so you can 
adjust them when needed.

.. warning::
  
  If this step is skipped the device will not enter sleep mode and the battery will be drained.

Calibrate device / Create gravity formula 
+++++++++++++++++++++++++++++++++++++++++

In order to calculate the gravity a formula needs to be added. You can either enter the formula directly or add
the measurements and let the device derive the formula for you.

Check battery voltage
+++++++++++++++++++++

Check the battery voltage so the value is correctly measured. If the wrong resistor values in the
voltage are then the calculation is off. This could block the device from going to sleep properly.   

Step 5 - Completed
------------------

You are now done and can enjoy the GravityMon software. Check out the :ref:`setting-up-device` section for other configuration options.

If you want to enter the configuration mode place the device flat on a surface and do a reset (or wait until it wakes up). 
Its recommended to attach the device to power when you have it in `configuration mode` so the battery is not drained. 

**If you have suggestions for more awesome features, head over to the github repository and make a request.**
