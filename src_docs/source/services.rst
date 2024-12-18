.. _services:

Service Integration
###################

This chapter contains a list of targets and what configuration is needed to interact with them.

Brewfather
++++++++++

Brewfather is an all in one service that allows you to manage you recipes and brews. 

**Option 1** - iSpindle Endpoint

This option makes use of the standard http (1 or 2) endpoints in the push section. If you are using SG then the device name needs to end with [SG] or brewfather will assume 
that the data is in plato. You can also modify the format template using the following options:

Update the following part `"gravity": ${gravity-plato},` or `"name" : "${mdns}[SG]",``

This makes use of the standard format template, no changes needed.

.. code-block::

   http://log.brewfather.net/ispindel?id=<yourid>


Documentation on this can be found under `Brewfather iSpindle Endpoint <https://docs.brewfather.app/integrations/ispindel>`_


**Option 2** - Custom Stream

This option makes use of the http push endpoint with a custom format template. Just enter the http stream address found 
on brewfather, not other settings are needed. The stream endpoint URL has the following format:

.. code-block::

   http://log.brewfather.net/stream?id=<yourid>

Documentation on this can be found under `Brewfather Custom Endpoint <https://docs.brewfather.app/integrations/custom-stream>`_

The implementation is basically a http request with the following format template:

.. code-block::

   {
      "name": "${mdns}",
      "temp": ${temp},
      "aux_temp": 0,
      "ext_temp": 0,
      "temp_unit": "${temp-unit}",
      "gravity": ${gravity},
      "gravity_unit": "${gravity-unit}",
      "pressure": 0,
      "pressure_unit": "PSI",
      "ph": 0,
      "bpm": 0,
      "comment": "",
      "beer": "",
      "battery": ${battery}
   }


Fermentrack
+++++++++++

`Fermentrack <https://www.fermentrack.com>`_ is a open source brewing software to monitor and control fermentation.

GravityMon can be installed and used as an iSpindle. Just register the device as an iSpindle and use the defined endpoint which normally is:

.. code-block::

   http://myservername/ispindel


UBIdots
+++++++

`UBIdots <https://www.ubidots.com>`_ is a IoT service that display data collected various sources. There is a limitation on this service where it can only handle 10 variables per device so 
you might need to reduce the number of values sent to the service. It will also treat every parameter as a number unless you create a custom device type and explicit define the string values 
as text. This will require a paid subscription (as I interpret the documentation). The example format below will only send numbers so that should work fine with the paid subscription. 

For this service there are two options to configure the integration. First you will need your default token which is found under `API Credentials` (<api-token> in the example below).
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

Even though ubidots can handle the default iSpindle format it probably better to just post the data you want. This is an example of a 
format template that can be used. For information on customizing the format see :ref:`format-template`.

.. code-block:: 

   { 
      "temperature": ${temp}, 
      "gravity": ${gravity}, 
      "angle": ${angle}, 
      "battery": ${battery}, 
      "RSSI": ${rssi} 
   }


Home Assistant
+++++++++++++++

`HomeAssistant <https://www.homeassistant.io>`_ is a platform for home automation and can collect sensor data
from multiple devices. 

This setup uses the MQTT integration with home assistant to collect values from the device.

**option 1** - Static MQTT sensor setup

This part of the configuration goes into the home assistant configuration.yaml file. The example assumes that the
device is named `gravmon2`

::

   mqtt:
      sensor:
         - name: "Gravmon2-Gravity"
            state_topic: "gravmon/gravmon2/gravity"
            unique_id: gravmon2_grav
            unit_of_measurement: "SG"
         - name: "Gravmon2-RSSI"
            state_topic: "gravmon/gravmon2/rssi"
            unique_id: gravmon2_rssi
            unit_of_measurement: "dBm"


Enter the name of the MQTT server in Home Assistant in the URL. You might need to install that option 
first. This is the format needed to submit the data to the correct topics as needed above. You can add as 
many sensors / topics as you want. It's also possible that you will need to create a user and supply the 
username / password to be able to publish messages on a topic.

::

   gravmon/${mdns}/gravity:${gravity}|
   gravmon/${mdns}/rssi:${rssi}|
   gravmon/${mdns}/battery:${battery}|
   gravmon/${mdns}/temperature:${temp}|
   gravmon/${mdns}/tilt:${tilt}|
   
**option 2** - Autodiscover using HA automation

Replace the ${key} with data applicable to your device. Add this as an automation script (edit in yaml). 
It should show up as an entity under the MQTT integration. If not, check the logfiles for any parsing errors. 

::

   alias: gravmon_${id}
   sequence:
     - service: mqtt.publish
       data:
         qos: 0
         retain: true
         topic: homeassistant/sensor/gravmon_${id}/gravity/config
         payload: >-
           {"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},
           "uniq_id":"${id}_grav", "name":"gravity",
           "unit_of_meas":"${gravity-unit}","stat_t":"gravmon/${mdns}/gravity"
           }   
     - service: mqtt.publish
       data:
         qos: 0
         retain: true
         topic: homeassistant/sensor/gravmon_${id}/temperature/config
         payload: >-
           {"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},
           "uniq_id":"${id}_temp","name":"temperature",
           "dev_cla":"temperature","unit_of_meas":"°${temp-unit}","stat_t":"gravmon/${mdns}/temperature"
           }   
     - service: mqtt.publish
       data:
         retain: true
         topic: homeassistant/sensor/gravmon_${id}/rssi/config
         payload: >-
           {"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},
           "uniq_id":"${id}_rssi","name":"rssi",
           "dev_cla":"signal_strength","unit_of_meas":"dBm","stat_t":"gravmon/${mdns}/rssi"
           }
   - service: mqtt.publish
      data:
         retain: true
         topic: homeassistant/sensor/gravmon_${id}/tilt/config
         payload: >-
           {"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},
           "uniq_id":"${id}_tilt","name":"tilt","stat_t":"gravmon/${mdns}/tilt"
           }
   - service: mqtt.publish
      data:
         retain: true
         topic: homeassistant/sensor/gravmon_${id}/battery/config
         payload: >-
           {"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},
           "uniq_id":"${id}_batt","name":"battery",
           "dev_cla":"voltage","unit_of_meas":"V","stat_t":"gravmon/${mdns}/battery"
           }
   mode: restart


**option 3** - Autodiscover using gravitymon

It's also possible to allow home assistant to do auto discovery and automatically create the sensor. This format 
template will create two sensors and update the values for them. 

.. warning::

   The registration will be sent on every push so the downside is that this will consume more time thus reducing battery life.   

.. warning::

  This method is not recommended for an ESP8266 since the low RAM memory will not be enough and there is a high probability that the device
  will crash insted. Testing shows that more than 1000 characters of data will likley crash the device. 

::

   gravmon/${mdns}/temperature:${temp}|
   gravmon/${mdns}/gravity:${gravity}|
   gravmon/${mdns}/rssi:${rssi}|
   gravmon/${mdns}/tilt:${tilt}|
   gravmon/${mdns}/battery:${battery}|
   homeassistant/sensor/gravmon_${id}/temperature/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_temp","name":"temperature","dev_cla":"temperature","unit_of_meas":"°${temp-unit}","stat_t":"gravmon/${mdns}/temperature"}|
   homeassistant/sensor/gravmon_${id}/gravity/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_grav","name":"gravity","unit_of_meas":" ${gravity-unit}","stat_t":"gravmon/${mdns}/gravity"}|
   homeassistant/sensor/gravmon_${id}/rssi/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_rssi","name":"rssi","dev_cla":"signal_strength","unit_of_meas":"dBm","stat_t":"gravmon/${mdns}/rssi"}|
   homeassistant/sensor/gravmon_${id}/tilt/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_tilt","name":"tilt","stat_t":"gravmon/${mdns}/tilt"}|
   homeassistant/sensor/gravmon_${id}/battery/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_batt","name":"battery","dev_cla":"voltage","unit_of_meas":"V","stat_t":"gravmon/${mdns}/battery"}|


Brewer's Friend
+++++++++++++++

Brewer's friend is an all in one service that allows you to manage you recipes and brews. 

.. warning::
  I don't have an account for brewers friend so I have not been able to verify this completely. Its based on
  the available documentation. If this works please let 

You can find you API key when logged in to the service. Follow these `instructions <https://docs.brewersfriend.com/devices/ispindel>`_

**Note there are different URLs if you are using plato or specific gravity!**

.. code-block::

   http://log.brewersfriend.com/ispindel/[API KEY]
   http://log.brewersfriend.com/ispindel_sg/[API KEY]


From what I can read in the documentation you need to add the API key as a token as well. This can be done using a custom 
format for the endpoint. Just add you API key after token.

.. code-block:: 

  {
   "name" : "${mdns}",
   "ID": "${id}",
   "token" : "[API KEY]",
   "interval": ${sleep-interval}, 
   "temperature": ${temp},
   "temp_units": "${temp-unit}",
   "gravity": ${gravity},
   "angle": ${angle},
   "battery": ${battery},
   "RSSI": ${rssi}
  }


Brewspy
+++++++

BrewSpy is a service that can show the history and manage the brew.

You need to enter the Token found in brewspy. The field is found under the field for http configuration.

.. code-block::

   http://brew-spy.com/api/ispindel


Thingspeak
++++++++++

Thingspeak is an IoT platform for receiving data which can be visualized.

In order to use this platform you need to create a channel (channel = device) and get the APIKEY for 
writing to the channel. Each channel can handle up to 8 measurements. In the http field enter the following URL.

.. code-block::

   http://api.thingspeak.com/update.json


You also need to create a custom format for the selected endpoint where the field1-field8 contains the data 
you want to include. The example below sends 5 different values to the channel identified by the API key.

.. code-block::

   { 
      "api_key": "<your write api key for channel>",
      "field1": ${gravity}, 
      "field2": ${temp},
      "field3": ${angle},
      "field4": ${battery},
      "field5": ${rssi}
   }


Blynk.io
++++++++

Blynk is an IoT service which can be updated via http get. It also has an mobile device that can be used to show the data.

In order to use this platform you need to create a device which can be used to receive the data. Each device will have a 
unique token that is used to identify it. You need to use the HTTP GET option (http-3) on the device for this to work. Enter the 
following URL in the UI. This will allow us to update several data points at once. I usually enter the token in the ``token2`` field 
so the format template does not contain any sensitive data and it's easier to switch to another device. 

.. code-block::

   http://blynk.cloud/external/api/batch/update


In the format editor you can enter this template which will send 3 values to blynk. You can add as many as you want but make sure 
these are configured on the device with the correct validation option such as data type and range. The value should be on one line 
starting with a ``?``. This string will be added to the URL above when doing the post. You can add more values if you want. 

.. code-block::

   ?token=${token2}&v1=${temp}&v2=${gravity}&v3=${angle}


BrewPi Remix
++++++++++++

`BrewPi Remix <https://brewpiremix.com>`_ is a temperature fermentation controller that runs on a Pi and an Arduino or ESP8266 microcontroller.
BrewPi Remix can recieve SG, Voltage and Temperature from Gravitymon.

- In Gravitymon, take note of your Host name (Home --> Device --> Host name:)

- On your Pi running BrewPi Remix, edit this file: `/home/brewpi/settings/config.php` and add this line:

.. code-block::

   iSpindel = <your-Gravitymon-host-name>
 
- If not already set, add these lines (Clamps the readings to prevent wide swings):

.. code-block::

   clampSGUpper = 1.175
   clampSGLower = 0.970
 
* After saving the changes, restart the script:

.. code-block::

   sudo systemctl restart brewpi.service

- In Gravitymon, add push settings in: Configuration --> Push settings --> HTTP 1 (POST)

.. code-block::

   http://<ip-address-of-your-BrewPi>/brewpi-api.php

- Save settings and run a test push: (Configuration --> Test push --> Test)


Brewpiless
++++++++++

If you connect the device to the brewpiless access point there is not way to access the user interface for configuration so it's recommended to connect the device to your normal network. 

The device need to have a name starting with iSpindle, for example `iSpindel000`. Set the URL for one of the http POST targets to `http://ip/gravity` where ip is the ip address of Brewpiless. 


BrewBlox
++++++++++

To send iSpindel data to brewblox over mqtt you need to modify the format template to match the expected format. Once you have configured the mqtt information you also need to update the format template 
for this target. 

This format template will post the expected json document on the topic, don't forget the `|` character at the end of the line which is needed to parse the payload. The first to words are the topic 
name and after the first `:` this is the json payload. Text within the brackets will be used as the unit for the value and degC is displayed as °C. You can add other parameters under the data section 
in the json document if you need other values as well.

.. code-block::

   brewcast/history:{"key":"${mdns}","data":{"Temperature[degC]": ${temp-c},"Temperature[degF]": ${temp-f},"Battery[V]":${battery},"Tilt[deg]":${angle},"Rssi[dBm]":${rssi},"SG":${gravity-sg},"Plato":${gravity-plato}}}|


The json message on the mqtt topic  would look like this:

.. code-block:: json

   {
      "key": "gravitymon",
      "data": {
         "Temperature[degC]": 27,
         "Temperature[degF]": 80,
         "Battery[V]": 4.1,
         "Tilt[deg]": 25,
         "Rssi[dBm]": -78,
         "SG": 1,
         "Plato": 0
      }
   }

BierBot Bricks
++++++++++++++

For this service you need to change the following for it to work.

- Set Interval to ``910`` seconds (This service will no accept data more often than 15 min)
- Set token to your ``API KEY``

Enter the following URL for HTTP POST 1 target

.. code-block::

   https://brewbricks.com/api/iot/v1

You also need to add the following HTTP Header to POST 1 under the empty header (keep the existing one).

.. code-block::

   `User-Agent: iSpindel`

Finally add the following format template to HTTP POST 1

.. code-block::

   {
      "apikey": "${token}",
      "type":"gravitymon",
      "brand":"wemos_d1_mini",
      "version":"${app-ver}",
      "chipid":"${id}",
      "s_number_wort_0":${gravity-plato},
      "s_number_temp_0":${temp-c},
      "s_number_voltage_0":${battery},
      "s_number_wifi_0":${rssi},
      "s_number_tilt_0":${tilt}
   }

.. note::
  If you are using the ESP8266 you can't use the test feature from the web UI since we are using SSL. Due to the 
  small RAM size on that board Its likley to crash when using SSL and the web server is running.

  To test this set a short sleep interval and tilt the device so it goes into gravity mode. If you have issues you can 
  check the error log for any HTTP errors.
