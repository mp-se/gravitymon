Installation 
------------

Official esptool
================

The prefered option for flashing esp8266 device is via the official esptool. Documentation can be found 
here; https://docs.espressif.com/projects/esptool/en/latest/esp32/

The basic command for flashing on Windows is;

``esptool.py --port COM4 write_flash 0x0 firmware.bin``

In the /bin directory you will find 3 different firmware builds;

* **firmware.bin**

  This is the standard release build (prefered version) 

* **firmware-perf.bin**

  This version also submits performance data to an influx database with detailed execution times.

* **firmware-debug.bin** 
  
  Development build. Not recommended for normal use.

In these versions all the html files are embedded in the binaries. The file system is currently only used for storing 
the configuration file. 

If the software becomes so large the html files can be moved to the file system, but this is not enabled by 
default (see compiling for details). This approach makes installation much easier and ensure that html files 
and code is in sync. 


Configuring WIFI
================

When the device is flashed it will need to have WIFI configuration in order to work. If you have used other software on 
the device its possible that wifi settings exist.

If this is not configured in the device it will create an wirless access point called `GravMon`. Connect to this AP and 
enter the SSID and password you want to use. If the web page dont open automatically you can enter the following adress 
in the browser: **http://192.168.4.1**
