.. _services:

Service Integration
###################

This chapter contains a list of targets and what configuration is needed to interact with them.

Brewfather
++++++++++

Just enter the http adress found on brewfather, not other settings are needed. The endpoint has the following format:

.. code-block::

   http://log.brewfather.net/http://log.brewfather.net/stream?id=<yourid>


The URL is found under settings.

Fermentrack
+++++++++++

`Fermentrack <https://www.fermentrack.com>`_ is a open source brewing software to monitor and control fermentation.

GravityMon can be installed and used as an iSpindle. Just register the device as an iSpindle and use the defined endpoint which normally is:

.. code-block::

   http://myservername/ispindel


UBIdots
+++++++

`UBIdots <https://www.ubidots.com>`_ is a IoT service that display data collected various sources. 

For this service there are two options to configure the integration. First you will need your default token which is found under `API Credentials` (<api-tokem> in the example below).
Swap the text <devicename> with the name you want to show in ubidots. 

**Option 1** - token as an URL parameter

Enter the following as URL:

.. code-block::

   http://industrial.api.ubidots.com/api/v1.6/devices/<devicename>/?token=<api-token>


This is the less secure option. 

**Option 2** - token as the http header

Enter the following as URL, use either standard or ssl.
   
.. code-block::

   http://industrial.api.ubidots.com/api/v1.6/devices/<devicename>
   https://industrial.api.ubidots.com/api/v1.6/devices/<devicename>


Under `Headers` (button after the http url in the UI) enter the following string: 
   
.. code-block::

   X-Auth-Token: <api-token>


This is the more secure option.

Even though ubidots can handle the default ispindle format it probably better to just post the data you want. This is an example of a 
format template that can be used. For information on customizing the format see :ref:`format-editor`.

.. code-block:: 

   { 
      "temperature": ${temp}, 
      "gravity": ${gravity}, 
      "angle": ${angle}, 
      "battery": ${battery}, 
      "rssi": ${rssi} 
   }


Home Assistant
+++++++++++++++

`HomeAssistant <https://www.homeassistant.com>`_ is a platform for home automation and can collect sensor data
from multiple devices. 

This setup uses the MQTT integration with home assistant to collect values from the device.

This part of the configuration goes into the home assistant configuration.yaml file. The example assumes that the
device is named `gravmon2`

::

   sensor:
   - platform: mqtt
      name: "gravmon2_gravity"
      state_topic: "gravmon/gravmon2/gravity"
   - platform: mqtt
      name: "gravmon2_battery"
      state_topic: "gravmon/gravmon2/battery"
   - platform: mqtt
      name: "gravmon2_rssi"
      state_topic: "gravmon/gravmon2/rssi"


Enter the name of the MQTT server in Home Assistant in the URL. You might need to install that option 
first. This is the format needed to submit the data to the correct topics as needed above. You can add as 
many sensors / topics as you want.

::

   gravmon/${mdns}/tilt:${angle}|
   gravmon/${mdns}/temperature:${temp}|
   gravmon/${mdns}/temp_units:${temp-unit}|


