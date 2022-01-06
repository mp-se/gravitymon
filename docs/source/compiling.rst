Compiling the software
-----------------------

VSCODE + PlatformIO
=============================
TODO

Targets 
=============================
TODO

Source structure 
=============================
TODO

Options 
=============================
TODO

# Compiling the software

I recommend that VSCODE with PlatformIO and Minfy extensions are used. Minify is used to reduce the size of the HTML files which are embedded into the firmware or uploaded to the file system. When using minify on a file, for example index.htm the output will be called index.min.htm. This is the file that will be used when buildning the image. 

By default the html files are embedded but there are options to upload them to the file system to reduce the size of the image if the size becomes to large for OTA. 

You can set the SSID and PWD as presets through platformio.ini by adding the settings to the following definitions:
```
-D USER_SSID=\""\""           // =\""myssid\""
-D USER_SSID_PWD=\""\""       // =\""mypwd\""
```

There are more options in teh platform.ini file that enable/disable additional functions for logging level, pushing performance data to InfluxDB and more. If i get the time I will add some documentation around these. 