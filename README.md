# Gravity Monitor for Beer Brewing

I started this project out of curiosity for how a motion sensor is working and since I like to brew beer this was the result. This software can be used with iSpindle hardware and utilizes the same hardware configuration. No part of this software has been copied from that project. 

This is still in early testing and has not the same range of external integrations as the standard software, so far only standard HTTP and Brewfather endpoints have been implemented. 

Gravity formulas are compatible with the standard software so switching is easy.

Main differences towards in the design is:

* Web GUI for configuration that can be accessed when connected to network and the device is connected to power.
* Sleep mode can be disabled so angles/tilt can be easily viewed. 
* Temperature sensor adjustment values.
* Temperature correcton of calculated gravity value.
* OTA support from local webserver when connected to power.

More documentation will come when I have more time... 