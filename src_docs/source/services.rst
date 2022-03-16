.. _services:

Service Integration
###################

This chapter contains a list of targets and what configuration is needed to interact with them.

Brewfather
++++++++++

Brewfather is an all in one service that allows you to manage you recepies and brews. 

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


Brewer's Friend
+++++++++++++++

Brewer's friend is an all in one service that allows you to manage you recepies and brews. 

.. warning::
  I dont have an account for brewers friend so I have not been able to verfy this completely. Its based on
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
   "temp-units": "${temp-unit}",
   "gravity": ${gravity},
   "angle": ${angle},
   "battery": ${battery},
   "rssi": ${rssi}
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

