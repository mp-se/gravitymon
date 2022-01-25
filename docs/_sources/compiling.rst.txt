.. _compiling-the-software:

Compiling the software
######################

Tools
=====
I use the following tools in order to build and manage the software:

* Visual Studio Code
* PlatformIO
* Git for Windows
* VSCode plugin: Minify (used to minimise the html files)

Code Formatting
===============
I use pre-commit and their cpp style checks to validate the code. Plugin defintions are found in **.pre-commit-config.yaml**

`Pre-Commit <https://www.pre-commit.com>`_

.. note::

  There is not yet any automatic checks since this does not work on Windows. It works if running under WSL2 with Ubuntu.


Targets 
=======
In the platformio config there are 3 targets defined

* gravity-debug; Maximum logging for trouble shooting, deep sleep is disabled.
* gravity-release; Standard release
* gravity-perf; Standard release but contains code for measuring performance 
* gravity32-perf: Experimental version for ESP32.

.. note::
  There is an experimental ESP32 target but since platformio only supports SDK 1.0.6 and the WIFI connection is really slow compared to ESP8266, 
  so the recommendation is to wait for support on 2.0.x branch. With the tested version an wifi connection takes 3-8s on a ESP32 compared 
  to 0.5s on an ESP8266. There is also a bug in OneWire connected to ESP32 that has not been fixed in the main repository yet.

.. warning::
  The debug target can be unstable and crash the device under certain circumstanses. Excessive logging to the serial port can cause corruption and crashes. 
  So only enable enough debugging to troubleshoot your changes.


Source structure 
================
.. list-table:: Directory structure
   :widths: 40 60
   :header-rows: 1

   * - path
     - content
   * - /bin
     - Contains compiled binaries
   * - /data
     - Directory for flashing device filesystem
   * - /doc
     - Various external documents used as input
   * - /html
     - Source for html files
   * - /img
     - Images uses in README.md
   * - /lib
     - External libraries used when compiling
   * - /script
     - Scripts used in build process
   * - /src
     - Source code for software
   * - /src_docs
     - Source code for documentation
   * - /stl
     - 3d models 
   * - /test
     - Test data for developing html files


Options 
=======
This is a list of C++ defines that is used to enable/disable functions in the code.

.. list-table:: Defines
   :widths: 40 60
   :header-rows: 1

   * - define
     - description
   * - ACTIVATE_OTA
     - Enables the OTA functionallity in the code
   * - SKIP_SLEEPMODE
     - The device never goes into sleep mode, useful when developing.
   * - xxx_DISABLE_LOGGING
     - Done include verbose logging in the corresponding class. Excessive logging may crash device.
   * - USE_LITTLEFS
     - Use the new filesystem in Ardurino
   * - EMBED_HTML
     - Html files are included in code, if not defined they are served from the file system.
   * - USER_SSID
     - If defined the device will always use this SSID
   * - USER_SSID_PWD
     - Password to the SSID
   * - CFG_APPVER
     - Defines the version of the compiled software

