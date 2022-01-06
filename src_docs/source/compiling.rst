Compiling the software
-----------------------

Tools
=====
I use the following tools in order to build and manage the software:

* Visual Studio Code
* PlatformIO
* Git for Windows
* VSCode plugin: Minify (used to minimise the html files)

Targets 
=======
In the repository there are 3 targets defined

* gravity-debug; Maximum logging for trouble shooting, deep sleep is disabled.
* gravity-release; Standard release
* gravity-perf; Standard release but contains code for measuring performance 

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
   * - /docs
     - Folder published to github pages
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
   * - USE_GYRO_TEMP
     - Uses temperature from gyro instead of DS18B20 (experimental)
   * - SKIP_SLEEPMODE
     - THe device never goes into sleep mode, useful when developing.
   * - CFG_DISABLE_LOGGING
     - Done include verbose logging in Config class. Excessive logging may crash device.
   * - GYRO_DISABLE_LOGGING
     - Done include verbose logging in Gyro class. Excessive logging may crash device.
   * - PUSH_DISABLE_LOGGING
     - Done include verbose logging in PushTarget class. Excessive logging may crash device.
   * - TSEN_DISABLE_LOGGING
     - Done include verbose logging in TempSensor class. Excessive logging may crash device.
   * - WEB_DISABLE_LOGGING
     - Done include verbose logging in WebServer class. Excessive logging may crash device.
   * - MAIN_DISABLE_LOGGING
     - Done include verbose logging in Main class. Excessive logging may crash device.
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

