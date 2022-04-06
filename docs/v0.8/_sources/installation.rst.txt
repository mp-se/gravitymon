.. _installation:

Installation 
------------

Brewflasher
===========

The prefered option for flashing GravityMon is using BrewFlasher, its a tools that support many brewing related firmwares for ESP8266 and ESP32. This works 
on both Windows and Mac. You can download the latest version from here: `Brewflasher <https://www.brewflasher.com/>`_

.. image:: images/brewflasher.png
  :width: 600
  :alt: Serial output

Binaries
********

In the /bin directory you will find 2 different firmware builds;

* **firmware.bin**

  This is the standard release build (prefered version) 

* **firmware-perf.bin**

  This version also submits performance data to an influx database with detailed execution times.

In these versions all the html files are embedded in the binaries. The file system is currently only used for storing 
the configuration file. 

If the software becomes so large the html files can be moved to the file system, but this is not enabled by 
default (see compiling for details). This approach makes installation much easier and ensure that html files 
and code is in sync. 

Esptool
=======

The other option for flashing esp8266 device is via the official esptool. Documentation can be found 
here; `esptool home page <https://docs.espressif.com/projects/esptool/en/latest/esp32/>`_

Windows 10 should install a driver for the USB -> Serial automatically when you connect a esp8266.

The basic command for flashing on Windows is;

``esptool.py --port COM4 write_flash 0x0 firmware.bin``

If there are issues you can try do erase the flash first using this command;

``esptool.py --port COM4 erase_flash``

Serial Monitoring
=================

To check output from the device (logs) there are several tools out there. I found this simple tool in the Windows Store called ``Serial Port Monitoring``. 
Just select a baud rate of 115200, 8N1.

.. image:: images/serial.png
  :width: 800
  :alt: Serial output


.. _setup_wifi:

Configuring WIFI
================

When the device is flashed it will need to have WIFI configuration in order to work. If you have used other software on 
the device its possible that wifi settings exist.

If this is not configured in the device it will create an wirless access point called `GravMon`. The default password is `password`.

Connect to this AP and enter the SSID and password you want to use. If the web page dont open automatically you can enter the following adress 
in the browser: **http://192.168.4.1**

.. image:: images/wifi.png
  :width: 200
  :alt: Wifi page

