# Gravity Monitor for Beer Brewing

I started this project out of curiosity for how a motion sensor is working and since I like to brew beer this was the result. This software can be used with iSpindle hardware and utilizes the same hardware configuration. No code has been reused from the iSpindle project. 

### TODO

* Add support for Plato in device (today it assumes that formula is in SG). 
* Add support for converting between SG/Plato in device.
* Add support for Blynk as endpoint
* Add support for https connections (push)
* Add support for https web server (will require certificates to be created as part of build process)
* Add iSpindle 3D print cradle + small PCB (what I use for my builds)

# Functionallity

I have made a few differnt design decision compared to the standard iSpindle software. 

* All configuration options have been moved to a web gui and is accesible without using the double reset function. You only need to use this for changing the WIFI configuration. The web gui can be accessed when either the device is laying flat on a surface (90 degress) or it's beeing charged. The benefits is that it's easier to change settings and also read the angle/tilt when doing calibration. You can also change the forumla and directly see the new calculated result. 

* The software also has built in OTA support so new versions can be downloaded from a local webserver (when connected to power)

* Temperature calibration of DS18B20 sensor so you can adjust the temperature sensor readings.

* Automatic temperature gravity calculation to adjust for a lower temperature in the fermentation vessels. Useful if you are fermenting at lower temperatures. It's possible to build this into the normal gravity formula but this is an easier option. Just make sure that the calibration is done at 20°C.

* The software will read the motion sensor 50 times and use the average to calculate the angle. If the readings show that the device is moving it will wait a few seconds and retry the operation again to make sure that no invalid angles should be reported. The downside is that the device will remain running if it's moving and draw more power. For normal fermentation this should not be an issue.

## Integrations / Sending data 

In this version the software supports sending data to standad HTTP endpoint and Brewfather. These are the ones I use so more can be added on request. 

## Configuration 

Configuration is accessed by entering the URL for the device, this will be the mDNS name __device.local__ or the IP adress. The following chapter assumes the device name is __gravmon__.

### Index page

http://gravmon.local/

The main page shows the device readings; gravity, angle, temperature and battery charge. If the checkbox is active then the device will never go into sleep mode. This is useful if you are collecting angle/tilt for calibration. If this is unchecked the device will go into sleep mode and the web UI is no longer accesible. 

![Status](img/index.png)

### Device page

http://gravmon.local/device.htm

The device page shows the device settings and software versions. 

![Device](img/device.png)

### Config page

http://gravmon.local/config.htm

This page is divided into several categories of settings. The first one contains device settings, mDNS name, temperature format, and gyro calibration data. The interval setting is the amount of time the device will be in sleep mode between readings (interval is in seconds).

Calibration needs to be done or the device will not work correctly. Place the device flat on a surface with gyro up and press the calibrate button when it's stable. 

__TODO: Update image for config settings.__

![Config - Device](img/config1.png)

The second section contains the push settings, two URL's for http posts, Brewfather and settings for InfluxDB v2. 

### This is the format used for standard http posts. 
```
{ 
   "name" : "gravmon",      // mDNS name
   "ID": "2E6753",          // esp device id
   "token" : "gravmon",
   "interval": 900,     
   "temperature": 20.5,     // C or F based on setting, adjusted value.
   "temp-units": "C",       // C or F based on setting
   "gravity": 1.0050,       // 
   "angle": 45.34,
   "battery": 3.67,
   "rssi": -12,
   "run-time": 230,         // ms, Runtime for this reading, this is an additional field not part of the standard format
}
```

### This is the format for Brewfather

```
{ 
   "name" : "gravmon",      // mDNS name
   "temp": 20.5,
   "temp-unit": "C",
   "battery": 3.67,
   "gravity": 1.0050,
   "gravity_unit": "G",     // G = SG, Plato is not yet supported
}
```

### This is the format for InfluxDB v2

```
gravity,host=<mdns>,device=<id>,format=SG value=1.0004
angle,host=<mdns>,device=<id> value=45.45
temp,host=<mdns>,device=<id>,format=<C|F> value=20.1
battery,host=<mdns>,device=<id> value=3.96
rssi,host=<mdns>,device=<id> value=-18
```

__TODO: Update image for push settings.__

![Config - Push](img/config2.png)

The third section contains the gravity options, formlua and option for temperature correcting gravity.

* Gravity formula is compatible with standard iSpindle formulas so any existing calculation option can be used. I use the tool fermentrack for controlling my fermentation and I use this tool for calculating gravity. The formula can handle two keywords, __tilt__ and __temp__. This is an example of a formula; __0.00145*tilt^3+0.1445*tilt^2+0.00179*tilt+0.9436__ 

![Config - Gravity](img/config3.png)

The fouth section contains the hardware options, voltage factor for calculating the battery charge, temperature sensor adjustment and OTA base URL.  

* For the OTA to work, place the following files (version.json + firmware.bin) at the location that you pointed out in OTA URL. If the version number in the json file is newer than in the code the update will be done during startup.

Example; OTA URL (don't forget trailing dash), the name of the file should be firmware.bin
```
http://192.168.1.1/firmware/gravmon/
```

Contents version.json
```
{ "project":"gravmon", "version":"0.3.0" }
```

![Config - Hardware](img/config4.png)

# Building a device

Not yet complete.

# Compiling the software

I recommend that VSCODE with PlatformIO and Minfy extensions are used. Minify is used to reduce the size of the HTML files which are embedded into the firmware or uploaded to the file system. When using minify on a file, for example index.htm the output will be called index.min.htm. This is the file that will be used when buildning the image. 

By default the html files are embedded but there are options to upload them to the file system to reduce the size of the image if the size becomes to large for OTA. 

You can set the SSID and PWD as presets through platformio.ini by adding the settings to the following definitions:
```
-D USER_SSID=\""\""           // =\""myssid\""
-D USER_SSID_PWD=\""\""       // =\""mypwd\""
```
