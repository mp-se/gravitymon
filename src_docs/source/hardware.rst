.. _hardware:

Hardware
########

iSpindle based on esp8266
=========================

There are lots of resouces out there on how to build the hardware for an iSpindle so I will not go into details on that part. I typically use one of the 
excellent pcb boards that, for example the iSpindel PCB v4.0 from Cherry Philip. Here is one of my standard builds using an esp8266.

.. image:: images/ispindel_esp8266.jpg
  :width: 500
  :alt: iSpindle esp8266

Schema for esp8266 build
++++++++++++++++++++++++

.. image:: images/schema_esp8266.png
  :width: 700
  :alt: Schema esp8266

iSpindle based on esp32
=======================

I've experimented with porting my software version to esp32 and this is a selection of options i have been testing. 

.. warning::
  Work to support esp32s2 is still ongoing, ESP32-mini and ESP32c3-mini is confirmed to be working. However I would recommend the C3 variant. 

.. image:: images/esp32_hardware.jpg
  :width: 500
  :alt: iSpindle esp32 hardware options

* esp32 mini, this was the first board i tried which is a smaller form factor of the first generetion esp32 with 2 cores. Slow on connecting to wifi is the main downside. 
* esp32c3 mini, a newer version based on the latest risc v7 architecture, is seen as the replacement for the esp8266 with bluetooth support. 
* esp32s2 mini, similar to the c3 board but without bluetooth support. 

It's possible to use this PCB and mount an ESP32 mini on top of that (c3 and s2 are prefered). The esp32 mini is a larger formfactor and can be hard to fit into the tube.

.. note::
  You need to add a resistor between A0 (Analog PIN) and ground of 470k. The reason is that the esp8266 has a build in resistor for a voltage divider 
  which the esp32 does not have. So in order to get a valid voltage (less than 3.2V) on the A0 pin this is needed. Once the modification is done you might
  need to adjust the voltage factor so the battery reading is correct. 

ESP32c3 mini
++++++++++++

This is model is now fully supported by gravitymon. 

.. image:: images/ispindel_esp32c3.jpg
  :width: 500
  :alt: Esp32c3 mini build

Here is an image of where I added the resistor for the voltage divider. 

.. image:: images/esp32_res.jpg
  :width: 500
  :alt: Esp32c3 adding resistor as voltage dividier.

ESP32s2 mini
++++++++++++

This is model is now fully supported by gravitymon. Same setup as for ESP32C3 mini.

ESP32 d1 mini
+++++++++++++

I would suggest that you try how it fits into the PET tube before soldering it to the PCB. Make sure that the battery is attached since this will be a really tight fit.

.. image:: images/ispindel_esp32.jpg
  :width: 500
  :alt: Esp32 mini build

Schema for esp32 build
++++++++++++++++++++++

.. note::  
  This schema assumes that an ESP32 D1 Mini (pin compatible with ESP8266 D1 Mini is used). The ESP32 has two rows of pins but 
  only the inner row is used. The main difference is the added resistor R3 so we get a voltage divider for measuring battery. 
  The ESP8266 has a built in resistor thats not visible on the schema and this acts as a voltage divider. 

.. image:: images/schema_esp32.png
  :width: 700
  :alt: Schema esp32

Adding a reed (magnetic) reset switch
+++++++++++++++++++++++++++++++++++++

A reed switch is a switch that reacts to magnetic fields. The ones I have tested are normally open and close in proximity to 
a magnet. 

.. image:: images/reed.jpg
  :width: 400
  :alt: Reed switch

If this is connected to the reset button a magnet can be used to trigger a reset of the device. The image below shows how 
I mounted the iSPINDLE PCB v4.0 just under the cap. The lower red circle shows the reset connection point for the reed switch.

The reed switch is the glass tube visible under the esp8266.

.. image:: images/reed_build.jpg
  :width: 400
  :alt: Reed build


Floaty Hydrometer DIY based on esp32 lite
=========================================

Here we have another projects that has build a device similar to the iSpindel but based on an ESP32 instead.

The setup is much simpler and attaches the GYRO to an ESP32 with a build in charger chip. GravityMon works on 
this hardware platform as well but there are a few limitations:

* Temperature is read from the GYRO and cannot be changed. This works fine when measuring gravity but when in configuration mode the temperature will increase since it shows the chip temperature.
* No possibility to measure battery level (can be added with additional hardware). 
